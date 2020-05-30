#include "RequestQueue.h"
#include "CommandGenerator.h"

using namespace MCsim;

#define DEBUG(str) std::cerr<< str <<std::endl;

RequestQueue::RequestQueue(bool perRequestor, bool writeQueueEnable):
	writeQueueEnable(writeQueueEnable),
	perRequestorEnable(perRequestor)
{
	flag = false;
	if(writeQueueEnable) { // Low and high watermark must be chosen according to the design specifications 		
		writeQueue = new WriteQueue(5,20);
	} 
	else {
		writeQueue = NULL;
	}
	generalBuffer = std::vector< Request* >();
	requestorBuffer = std::map<unsigned int, std::vector<Request*>>();
}

RequestQueue::~RequestQueue()
{
	delete writeQueue;
	for(auto it=generalBuffer.begin(); it!=generalBuffer.end(); it++) {
		delete (*it);
	}
	generalBuffer.clear();
	for(auto it=requestorBuffer.begin(); it!=requestorBuffer.end(); it++) {
		for(auto req=it->second.begin(); req!=it->second.end(); req++) {
			delete (*req);
		}
		it->second.clear();
	}
	requestorBuffer.clear();
}

bool RequestQueue::isWriteEnable()
{
	return writeQueueEnable;
}

bool RequestQueue::isPerRequestor()
{
	return perRequestorEnable;
}

// Add request based on criticality
bool RequestQueue::insertRequest(Request* request)
{	
	if(writeQueueEnable && request->requestType == DATA_WRITE) {
		writeQueue->insertWrite(request);
		return true;
	}

	if(perRequestorEnable) {
		if(requestorBuffer.find(request->requestorID) == requestorBuffer.end()) {
			requestorBuffer[request->requestorID] = std::vector<Request*>();
			requestorOrder.push_back(request->requestorID);
		}
		requestorBuffer[request->requestorID].push_back(request);
	}
	else {		
		generalBuffer.push_back(request);
	}
	return true;
}
// How many requestor share the same queue
unsigned int RequestQueue::getQueueSize()
{
	return requestorOrder.size();
}

unsigned int RequestQueue::getSize(bool requestor, unsigned int index)
{
	if(requestor) {
		if(requestorOrder.size() == 0) {
			return 0; 
		}
		else {
			// No such requestor
			if(requestorBuffer.find(requestorOrder[index]) == requestorBuffer.end()){ 
				return 0; 
			}	
			else { 
				return requestorBuffer[requestorOrder[index]].size();
			}
		}
	}
	else 
	{ 
		return generalBuffer.size();
	}
}
// If care about the fairness
Request* RequestQueue::getRequest(unsigned int reqIndex, unsigned int index)
{
	// Scan the requestorqueue by index, instead of requestorID value 
	if(requestorBuffer[requestorOrder[reqIndex]].empty()) {
		return NULL;
	}
	else {
		scheduledRequest.first = true;
		scheduledRequest.second = std::make_pair(requestorOrder[reqIndex],index);
		return requestorBuffer[requestorOrder[reqIndex]][index];
	}
}
// Scan the requestorqueue by index, instead of requestorID value 
Request* RequestQueue::checkRequestIndex(unsigned int reqIndex, unsigned int index)
{
	if(requestorBuffer[requestorOrder[reqIndex]].empty()) {
		return NULL;
	}
	else {
		return requestorBuffer[requestorOrder[reqIndex]][index];
	}
}

// Check wether the general buffer is empty 
bool RequestQueue::isEmpty()
{
	if(generalBuffer.empty()) {
		return true;
	}
	return false;
}

// If does not care about the fairness
Request* RequestQueue::getRequest(unsigned int index)
{
	if(generalBuffer.empty()) {
		return NULL;
	}
	else {
		scheduledRequest.first = false;
		scheduledRequest.second = std::make_pair(0,index);
		return generalBuffer[index];		
	}
}
// Take the request without removing from the buffer - per requestor
Request* RequestQueue::getRequestCheck(unsigned int index)
{
	if(generalBuffer.empty()) {
		return NULL;
	}
	else {
		return generalBuffer[index];		
	}
}
bool RequestQueue::switchMode()
{
	if(flag)
	{			
		if(writeQueue->bufferSize() == 0){
			flag = false;
			return true;
		}
		return false;
	}
	else
	{
		if(writeQueue->highWatermark())
		{
			flag = true;
			return false;
		}	
		return true;
	}	
	return true;
}

unsigned int RequestQueue::writeSize()
{
	return writeQueue->bufferSize();
}

Request* RequestQueue::scheduleWritecheck()
{
	return writeQueue->getWrite(0);
}

void RequestQueue::removeWriteRequest()
{
	writeQueue->removeWrite(0);
}
// Find the earliest request per requestor per bank
Request* RequestQueue::earliestperBankperReq(unsigned int p, unsigned int b)
{
	Request* temp_init = NULL;
	Request* temp_sec = NULL;
	Request* temp = NULL;
	// search in the buffer to find latest req from p and b
	if(generalBuffer.empty()) {
		return NULL;
	}
	else 
	{
		for(unsigned int k = 0; k < generalBuffer.size() ; k++)
		{
			temp_init = generalBuffer[k];
			if(temp_init->bank == b && temp_init->requestorID == p)
			{
				temp = temp_init;
				break;	
			}
		}
		for(unsigned int l = 0; l < generalBuffer.size() ; l++)
		{			
			temp_sec = generalBuffer[l];
			if(temp_sec->bank == b && temp_sec->requestorID == p)
			{
				if(temp_sec->arriveTime < temp->arriveTime)
				{
					temp = temp_sec;
				}				
			}
		}
		return temp;
	}
}
// Remove the previously access request, Once a request is buffered to another location, remove from the request queue
void RequestQueue::removeRequest()
{
	unsigned id = scheduledRequest.second.first;
	unsigned index = scheduledRequest.second.second;
	if(scheduledRequest.first) {
		requestorBuffer[id].erase(requestorBuffer[id].begin() + index);
	}
	else {
		generalBuffer.erase(generalBuffer.begin() + index);
	}	
}








