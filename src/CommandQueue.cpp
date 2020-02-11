#include <iostream>
#include "CommandQueue.h"
#include "global.h"
#include <algorithm>   
#include "CommandScheduler.h"

using namespace MCsim;
using namespace std;

CommandQueue::CommandQueue(bool perRequestor)
{
	scheduledQueue = true;
	requestorQueue = false;
	requestorIndex = 0;
	ACTdiff = false;
	perRequestorEnable = perRequestor;
}

CommandQueue::~CommandQueue()
{
	std::vector<BusPacket*> empty;
	std::swap(hrtBuffer, empty);
	std::swap(srtBuffer, empty);
}

bool CommandQueue::isPerRequestor()
{
	return perRequestorEnable;
}

unsigned int CommandQueue::getRequestorIndex()
{
	return requestorMap.size();
}
// Get the size of individual buffer cmd for each requestor
unsigned int CommandQueue::getRequestorSize(unsigned int index) 
{
	if(index > requestorMap.size()) {
		DEBUG("COMMAND QUEUE: ACCESSED QUEUE OS OUT OF RANGE");
		abort();
	}
	else {
		return requestorBuffer[requestorMap[index]].size(); 
	}
}
// Get the size of the general cmd buffers based on the criticality
unsigned int CommandQueue::getSize(bool critical)
{
	if(critical) { 
		return hrtBuffer.size(); 
	}
	else { 
		return srtBuffer.size(); 
	}	
}
// Inserting the cmd to the corresponding queues according to the queuing structure
bool CommandQueue::insertCommand(BusPacket* command, bool critical)
{
	if(perRequestorEnable){
		if(requestorBuffer.find(command->requestorID) == requestorBuffer.end()) {
			requestorBuffer[command->requestorID] = std::vector<BusPacket*>();
			requestorMap.push_back(command->requestorID);
		}
		requestorBuffer[command->requestorID].push_back(command);
	}
	else {
		if(critical) { 
			hrtBuffer.push_back(command); 
		}
		else {
			srtBuffer.push_back(command); 
		}		
	}
	return true;
}
// Get a cmd from either general cmd buffers
BusPacket* CommandQueue::getCommand(bool critical)
{
	scheduledQueue = critical;
	if(critical && !hrtBuffer.empty()) { 
		return hrtBuffer.front(); 
	}
	else if(!critical && !srtBuffer.empty()) { 
		return srtBuffer.front(); 
	}
	else {
		DEBUG("COMMAND QUEUE: CHECK THE SIZE OF getCommand");
		abort();
		return NULL; 
	}
}
// Check a cmd from general buffers while for the purpose of checking
BusPacket* CommandQueue::checkCommand(bool critical, unsigned int index)
{
	if(critical && !hrtBuffer.empty()) { 
		return hrtBuffer[index]; 
	}
	else if(!critical && !srtBuffer.empty()) { 
		return srtBuffer[index]; 
	}
	else {
		DEBUG("COMMAND QUEUE: CHECK THE SIZE OF checkCommand");
		abort();
		return NULL; 
	}
}
// Get a cmd from a particular requestor cmd buffer
BusPacket* CommandQueue::getRequestorCommand(unsigned int index)
{
	return requestorBuffer[requestorMap[index]].front();
}
// Remove a cmd from the beginning of the specific requestor queue
void CommandQueue::removeCommand(unsigned int requestorID)
{
	requestorBuffer[requestorID].erase(requestorBuffer[requestorID].begin());
}
// Remove a cmd from the general buffers (hrt/srt) 
void CommandQueue::removeCommand()
{
	if(scheduledQueue == true) {
		hrtBuffer.erase(hrtBuffer.begin());
	}
	else {
		srtBuffer.erase(srtBuffer.begin());
	}
}
// Determine if the scheduler require different ACT for read/write
void CommandQueue::setACT(unsigned int x)
{
	if(x == 1){ACTdiff = true;}
	else
	{
		ACTdiff = false;
	}
}

