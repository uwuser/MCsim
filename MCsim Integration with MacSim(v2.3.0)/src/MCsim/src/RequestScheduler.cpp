/*
MIT License

Copyright (c) 2020 University of Waterloo, Reza Mirosanlou @rmirosan@uwaterloo.ca

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <math.h>
#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include "global.h"

using namespace MCsim;

RequestScheduler::RequestScheduler(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable):
	requestorCriticalTable(requestorTable),
	requestQueue(requestQueues),
	commandQueue(commandQueues)
{
	commandGenerator = NULL;
	scheduledRequest = NULL;
	req1 = NULL;
	req2 = NULL;
	id = 0;
	clockCycle = 1;
}

RequestScheduler::~RequestScheduler()
{
	//bankTable.clear();
}

void RequestScheduler::connectCommandGenerator(CommandGenerator *generator)
{
	commandGenerator = generator;
}

void RequestScheduler::flushWriteReq(bool sw)
{ 
	switch_enable =  sw;
}
bool RequestScheduler::serviceWrite(int qIndex)
{
	if(requestQueue[qIndex]->isWriteEnable() && requestQueue[qIndex]->getSize(false, 0) == 0)
		return true;
	else
		return false;	
}
bool RequestScheduler::writeEnable(int qIndex)
{
	return requestQueue[qIndex]->isWriteEnable();
}
unsigned int RequestScheduler::bufferSize(unsigned int qIndex){
	return requestQueue[qIndex]->getSize(false, 0);
}

Request* RequestScheduler::scheduleFR_BACKEND(unsigned int qIndex, unsigned int reqIndex)
{
	for(unsigned int index=0; index < requestQueue[qIndex]->getSize(true, reqIndex); index++) {			
		if(isRowHit(requestQueue[qIndex]->checkRequestIndex(reqIndex,index))){
			if(isSchedulable(requestQueue[qIndex]->checkRequestIndex(reqIndex,index), isRowHit(requestQueue[qIndex]->checkRequestIndex(reqIndex,index)))){							
				return requestQueue[qIndex]->getRequest(reqIndex,index);
			}			
		}	
	}
	for(unsigned int index=0; index < requestQueue[qIndex]->getSize(true, reqIndex); index++) {
		if(isSchedulable(requestQueue[qIndex]->checkRequestIndex(reqIndex,index),isRowHit(requestQueue[qIndex]->checkRequestIndex(reqIndex,index)))){
			return requestQueue[qIndex]->getRequest(reqIndex,index);
		}
	}	
	return NULL;
}
bool RequestScheduler::isRowHit(Request* request)
{
	
	bool isHit = false;
	if(bankTable.find(request->addressMap[Rank]) != bankTable.end()) {
		if(bankTable[request->rank].find(request->addressMap[Bank]) != bankTable[request->addressMap[Rank]].end()) {
			if(bankTable[request->addressMap[Rank]][request->addressMap[Bank]] == request->row){ 
				isHit = true; 
			}
		}	
	}	
	return isHit;
}
bool RequestScheduler::isSchedulable(Request* request, bool open)
{
	if(request->requestType == DATA_READ)						
	{
		if(request->address > 999999){
			TRACE_REQ("TRACE-REQUEST:READ"<<"\t\t"<<clockCycle<<":"<<"\t\tAddress: "<<request->address<<"\tBank: "<<request->bank<<"\t\tColumn: "<<request->col<<"\t\tRow: "<<request->row);										}
		else {
			TRACE_REQ("TRACE-REQUEST:READ"<<"\t\t"<<clockCycle<<":"<<"\t\tAddress: "<<request->address<<"\t\tBank: "<<request->bank<<"\t\tColumn: "<<request->col<<"\t\tRow: "<<request->row);}
	}
	else if(request->requestType == DATA_WRITE)	{
		if(request->address > 999999){
			TRACE_REQ("TRACE-REQUEST:WRITE"<<"\t\t"<<clockCycle<<":"<<"\t\tAddress: "<<request->address<<"\tBank: "<<request->bank<<"\t\tColumn: "<<request->col<<"\t\tRow: "<<request->row);}
		else {
			 TRACE_REQ("TRACE-REQUEST:WRITE"<<"\t\t"<<clockCycle<<":"<<"\t\tAddress: "<<request->address<<"\t\tBank: "<<request->bank<<"\t\tColumn: "<<request->col<<"\t\tRow: "<<request->row);}								
	}
	return commandGenerator->commandGenerate(request, open);
}

void RequestScheduler::updateRowTable(unsigned rank, unsigned bank, unsigned row)
{
	bankTable[rank][bank] = row;
}

void RequestScheduler::step()
{
	clockCycle++;
}