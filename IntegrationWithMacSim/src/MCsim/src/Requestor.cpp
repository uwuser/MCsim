#include "Requestor.h"
//#include "MemoryController.h"
//#include "MultiChannelMemorySystem.h"
#include "RequestQueue.h"
#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include <sstream>

using namespace MCsim;

Requestor::Requestor(int id, bool inOrder, const string& traceFile):
	requestorID(id),
	inOrder(inOrder) 
{
	transFile.open(traceFile.c_str());
	localCache = new Cache();
	sim_done = false;
	bypass_read= false;
	currentClockCycle = 1; // Initial clock cycle
	//currentClockCycle = 0; 
	prevArrive = 1; // just for verification
	//prevArrive = 0;
	prevComplete = 0; // Finish time of the prev request from individual requestor
	wcLatency = 0;
	compTime = 0;
	long_l = 0;
	RequestBufferSize = 32; 
	completeRequest = 0;
	requestRequest = 0;
	latency = 0;
	waitingRequest = false;

	memorySystem = NULL;
	pendingRequest = NULL;

	hitRatioCtr = 10;
	string addressStr = "0x2a8b6ca0";
	istringstream iniHit(addressStr.substr(2)); //Gets rid of 0x
	iniHit>>hex>>rowHitAddr;
	addressStr = "0x23588c0";
	istringstream iniMiss(addressStr.substr(2)); //Gets rid of 0x
	iniMiss>>hex>>rowMissAddr;
	currAddr = rowHitAddr;
}

Requestor::~Requestor()
{
	corePendingData.clear();
	// delete localCache;
	transFile.close();
}

void Requestor::connectMemorySystem(MultiChannelMemorySystem* memSys) {
	memorySystem = memSys;
}

void Requestor::setMemoryClock(float clk) {
	memoryClock = clk;
}

void Requestor::sendRequest(Request* request) {
	bool R_W = true;
	if(request->requestType == DATA_WRITE) {
		R_W = false;
	}
	if(memorySystem->addRequest(request->requestorID, request->address, R_W, request->requestSize))
	{		
		// Adding the request to the memory controller
		requestRequest++;
		latency = 0;
		waitingRequest = true;
		// Register the arrival time for request
		request->arriveTime = currentClockCycle;
		// Pushed to the corePending queue for the future use upon returning
		corePendingData.push_back(request);	
	}
}

void Requestor::returnData(Request* returnTrans)
{
	if(!corePendingData.empty()) {
		for(unsigned index = 0; index < corePendingData.size(); index++) {
			if(returnTrans->address == corePendingData[index]->address) {
				if(currentClockCycle - corePendingData[index]->arriveTime > wcLatency) {
					wcLatency = currentClockCycle - corePendingData[index]->arriveTime;
				}	
				prevComplete = currentClockCycle;
				completeRequest++;
				latency = 0;
				waitingRequest = false;
				// *** Deallocation
				delete corePendingData[index];
				corePendingData.erase(corePendingData.begin() + index);
				break;
			}
		}
		if(latency != 0) {
			DEBUG("REQUEST NOT FOUND");
			abort();
		}
	}	
}


void Requestor::update()
{
	latency++;		
	// Control the suspecious WC latency for RT controllers - Disable for High Performance Controllers
	//if(wcLatency >= 1000 && requestorID == 0) {
	//DEBUG("worst case @ "<<currentClockCycle<<" REQ"<<requestorID<<" == "<<wcLatency);
	//DEBUG("A Worst-case is exceed 1000 cycles and the simulation aborted. If running OoO, please deactivate this control");
	//abort();
	//}	 
	sim_done = readingTraceFile();
	if(pendingRequest != NULL) {
		// Send the request if the arrival time is reached
		if(pendingRequest->arriveTime <= currentClockCycle && corePendingData.size() <= RequestBufferSize) {
		//if(pendingRequest->arriveTime <= currentClockCycle) {
			sendRequest(pendingRequest);
			pendingRequest = NULL;
		}	
	}
	else
	{
		if(bypass_read){
			memorySystem->flushWrite(true);			
		}
	}
	if(!corePendingData.empty()){
		sim_done = false;
	}
	currentClockCycle++;
}

bool Requestor::sim_end(){
	if(sim_done) {
		DEBUG("REQ"<<requestorID<<" COMPLETED");
		transFile.close();
	}
	return sim_done;
}

void Requestor::printResult() 
{
	if(completeRequest != 0){
		PRINT("Requestor "<<requestorID<<": Worst-Case "<<wcLatency-1<<" Done "<<completeRequest<<" Request From "<<requestRequest
			<<" Computation Time = "<<compTime<<" @ "<<memoryClock);
	}
	else
	{
		PRINT("Requestor "<<requestorID<<": Not Finished a Request Yet, Done "<<completeRequest<<" Request From "<<requestRequest
			<<" Computation Time = "<<compTime<<" @ "<<memoryClock);
	}		
}

bool Requestor::readingTraceFile()
{
	string transLine;
	uint64_t addr;
	uint64_t clockCycle;
	uint64_t compDelay;
	
	enum RequestType requestType;

	if(inOrder && !corePendingData.empty()) {
		return false;
	}

	if(pendingRequest == NULL) {
		if(!transFile.eof()) {		
			getline(transFile, transLine);			
			if (transLine.size() > 0) {
				long_l++;
				parseTraceFileLine(transLine, addr, requestType, compDelay, clockCycle);				
				pendingRequest = new Request(requestorID, requestType, RequestSize, addr, NULL);
				pendingRequest->arriveTime = clockCycle;
			}
			else			
				bypass_read = true;			
							
		}
		else {
			DEBUG("Tracefile Finished");			
			return true;
		}
	}
	return false;
}

void Requestor::parseTraceFileLine(string &line, uint64_t &addr, enum RequestType &requestType, uint64_t &compDelay, uint64_t &clockCycle)
{
	size_t previousIndex=0;
	size_t spaceIndex=0;
	string addressStr="", cmdStr="", dataStr="", ccStr="", idStr="", delayStr="";
	spaceIndex = line.find_first_of(" ", 0);

	// Address Decoding
	spaceIndex = line.find_first_not_of(" ", previousIndex);
	addressStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);
	istringstream b(addressStr.substr(2)); // Gets rid of 0x
	b>>hex>>addr;
	addr = addr >> 6; // For verification of Ramulator
	previousIndex = line.find_first_of(" ", spaceIndex);

	//Command Decoding
	spaceIndex = line.find_first_not_of(" ", previousIndex);
	cmdStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);

	if (cmdStr.compare("WRITE")==0) {requestType = DATA_WRITE;}
	else if (cmdStr.compare("READ")==0) {requestType = DATA_READ;}
	else {exit(0); }
	previousIndex = line.find_first_of(" ", spaceIndex);

	//Arrival Time Decoding
	spaceIndex = line.find_first_not_of(" ", previousIndex);
	ccStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);
	istringstream c(ccStr);
	c>>compDelay;

	// Important for the traces
	compDelay = (long)(compDelay/memoryClock);
	//compDelay = ((long)(compDelay)); // for verification of Ramulator

	/*
	if(requestorID > 0) {
	 	compDelay = 0;
	}
	*/	
	if(inOrder) {
		// If config is inOrder for a requestor, then the next request must wait until the previous request return
		compTime = compTime + compDelay;
		clockCycle = compDelay + prevComplete;	
	}
	else {
		if(compDelay == 0 && currentClockCycle != 1){   // just for verification
			prevArrive++;    		 				    // Just for verification
			clockCycle = prevArrive; 					// Just for verification
		}					 		 				
		else
		{
			clockCycle = compDelay + prevArrive;
			prevArrive = clockCycle;
		}	
	/*
		clockCycle = compDelay + prevArrive;
		prevArrive = clockCycle;
	*/	
		
	}
}


