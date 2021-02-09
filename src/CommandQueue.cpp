#include <iostream>
#include "CommandQueue.h"
#include "global.h"
#include <algorithm>
#include "CommandScheduler.h"
using namespace std;

using namespace MCsim;

CommandQueue::CommandQueue(bool perRequestor) // ctor
{
	scheduledQueue = true;
	requestorQueue = false;
	requestorIndex = 0;
	perRequestorEnable = perRequestor;
}

CommandQueue::~CommandQueue() //dtor
{
	std::vector<BusPacket *> empty;
	std::swap(hrtBuffer, empty);
	std::swap(srtBuffer, empty);
}

bool CommandQueue::isPerRequestor() // determine if the command queue structure is per requestor not
{
	return perRequestorEnable;
}

unsigned int CommandQueue::getRequestorIndex() // return the size command queue
{
	return requestorMap.size();
}

unsigned int CommandQueue::getRequestorSize(unsigned int index) // Get the size of individual buffer cmd for each requestor
{
	if (index > requestorMap.size())
	{
		DEBUG("COMMAND QUEUE: ACCESSED QUEUE OS OUT OF RANGE");
		abort();
	}
	else
	{
		return requestorBuffer[requestorMap[index]].size();
	}
}

unsigned int CommandQueue::getSize(bool critical) // Get the size of the general cmd buffers based on the criticality
{
	if (critical)
		return hrtBuffer.size();
	else
		return srtBuffer.size();
}

bool CommandQueue::insertCommand(BusPacket *command, bool critical) // Inserting the cmd to the corresponding queues according to the queuing structure
{
	if (perRequestorEnable)
	{
		if (requestorBuffer.find(command->requestorID) == requestorBuffer.end())
		{
			requestorBuffer[command->requestorID] = std::vector<BusPacket *>();
			requestorMap.push_back(command->requestorID);
		}
		requestorBuffer[command->requestorID].push_back(command);
	}
	else
	{
		if (critical)
			hrtBuffer.push_back(command);
		else
			srtBuffer.push_back(command);
	}
	return true;
}

BusPacket *CommandQueue::getCommand(bool critical) // Get a cmd from either general cmd buffers
{
	scheduledQueue = critical;
	if (critical && !hrtBuffer.empty())
	{
		return hrtBuffer.front();
	}
	else if (!critical && !srtBuffer.empty())
	{
		return srtBuffer.front();
	}
	else
	{
		DEBUG("COMMAND QUEUE: CHECK THE SIZE OF getCommand");
		abort();
		return NULL;
	}
}

BusPacket *CommandQueue::checkCommand(bool critical, unsigned int index) // Check a cmd from general buffers while for the purpose of checking
{
	if (critical && !hrtBuffer.empty())
	{
		return hrtBuffer[index];
	}
	else if (!critical && !srtBuffer.empty())
	{
		return srtBuffer[index];
	}
	else
	{
		DEBUG("COMMAND QUEUE: CHECK THE SIZE OF checkCommand");
		abort();
		return NULL;
	}
}

BusPacket *CommandQueue::getRequestorCommand(unsigned int index) // Get a cmd from a particular requestor cmd buffer
{
	return requestorBuffer[requestorMap[index]].front();
}

void CommandQueue::removeCommand(unsigned int requestorID) // Remove a cmd from the beginning of the specific requestor queue
{
	requestorBuffer[requestorID].erase(requestorBuffer[requestorID].begin());
}

void CommandQueue::removeCommand() // Remove a cmd from the general buffers (hrt/srt)
{
	if (scheduledQueue == true)
	{
		hrtBuffer.erase(hrtBuffer.begin());
	}
	else
	{
		srtBuffer.erase(srtBuffer.begin());
	}
}