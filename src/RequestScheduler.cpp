#include <math.h>
#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include "global.h"

using namespace MCsim;

RequestScheduler::RequestScheduler(std::vector<RequestQueue *> &requestQueues, std::vector<CommandQueue *> &commandQueues, const std::map<unsigned int, bool> &requestorTable) : requestorCriticalTable(requestorTable),
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
	bankTable.clear();
}

void RequestScheduler::connectCommandGenerator(CommandGenerator *generator)
{
	commandGenerator = generator;
}

bool RequestScheduler::isRowHit(Request *request)
{

	bool isHit = false;
	if (bankTable.find(request->addressMap[Rank]) != bankTable.end())
	{
		if (bankTable[request->rank].find(request->addressMap[Bank]) != bankTable[request->addressMap[Rank]].end())
		{
			if (bankTable[request->addressMap[Rank]][request->addressMap[Bank]] == request->row)
			{
				isHit = true;
			}
		}
	}
	return isHit;
}
void RequestScheduler::flushWriteReq(bool sw)
{
	switch_enable = sw;
}
bool RequestScheduler::serviceWrite(int qIndex)
{
	if (requestQueue[qIndex]->isWriteEnable() && requestQueue[qIndex]->getSize(false, 0) == 0)
		return true;
	else
		return false;
}
bool RequestScheduler::writeEnable(int qIndex)
{
	return requestQueue[qIndex]->isWriteEnable();
}
unsigned int RequestScheduler::bufferSize(unsigned int qIndex)
{
	return requestQueue[qIndex]->getSize(false, 0);
}

Request *RequestScheduler::scheduleFR(unsigned int qIndex)
{
	for (unsigned int index = 0; index < requestQueue[qIndex]->getSize(false, 0); index++)
	{
		if (isRowHit(requestQueue[qIndex]->getRequestCheck(index)))
		{
			return requestQueue[qIndex]->getRequest(index);
		}
	}
	return requestQueue[qIndex]->getRequest(0);
}

Request *RequestScheduler::scheduleBLISS(unsigned int qIndex)
{
	flag = true;
	bool hit_1;
	bool hit_2;
	if (requestQueue[qIndex]->getSize(false, 0) > 0)
	{
		for (unsigned int index = 0; index < requestQueue[qIndex]->getSize(false, 0); index++)
		{
			req1 = requestQueue[qIndex]->getRequestCheck(index);
			if (req1 != NULL)
			{
				id = index;
				break;
			}
		}
		for (unsigned int index = id; index < requestQueue[qIndex]->getSize(false, 0); index++)
		{
			req2 = requestQueue[qIndex]->getRequestCheck(index);
			if (req2 != NULL)
			{
				if ((blacklist[req1->requestorID] != 1) ^ (blacklist[req2->requestorID] != 1))
				{
					if (blacklist[req1->requestorID] == 1)
					{
						req2 = requestQueue[qIndex]->getRequest(index);
						return req2;
					}
					else
					{
						req1 = requestQueue[qIndex]->getRequest(index);
						return req1;
					}
				}
				if (isRowHit(requestQueue[qIndex]->getRequestCheck(id)))
					hit_1 = true;
				if (isRowHit(requestQueue[qIndex]->getRequestCheck(index)))
					hit_2 = true;
				if (hit_1 ^ hit_2)
				{
					if (hit_1)
					{
						req1 = requestQueue[qIndex]->getRequest(id);
						return req1;
					}
					else
					{
						req2 = requestQueue[qIndex]->getRequest(index);
						return req2;
					}
				}
			}
		}
		flag = true;
		hit_2 = false;
		hit_1 = false;
		req1 = requestQueue[qIndex]->getRequest(id);
		return req1;
	}
	return NULL;
}
Request *RequestScheduler::scheduleFR_Next(unsigned int qIndex)
{
	if (requestQueue[qIndex]->getSize(false, 0) > 0)
	{
		for (unsigned int index = 1; index < requestQueue[qIndex]->getSize(false, 0); index++)
		{
			if (isRowHit(requestQueue[qIndex]->getRequest(index)))
			{
				return requestQueue[qIndex]->getRequest(index);
			}
		}
		return requestQueue[qIndex]->getRequest(1);
	}
	else
	{
		return NULL;
	}
}
bool RequestScheduler::isSchedulable(Request *request, bool open)
{
	if (request->requestType == DATA_READ)
	{
		if (request->address > 999999)
		{
			TRACE_REQ("TRACE-REQUEST:READ"
					  << "\t\t" << clockCycle << ":"
					  << "\t\tAddress: " << request->address << "\tBank: " << request->bank << "\t\tColumn: " << request->col << "\t\tRow: " << request->row);
		}
		else
		{
			TRACE_REQ("TRACE-REQUEST:READ"
					  << "\t\t" << clockCycle << ":"
					  << "\t\tAddress: " << request->address << "\t\tBank: " << request->bank << "\t\tColumn: " << request->col << "\t\tRow: " << request->row);
		}
	}
	else if (request->requestType == DATA_WRITE)
	{
		if (request->address > 999999)
		{
			TRACE_REQ("TRACE-REQUEST:WRITE"
					  << "\t\t" << clockCycle << ":"
					  << "\t\tAddress: " << request->address << "\tBank: " << request->bank << "\t\tColumn: " << request->col << "\t\tRow: " << request->row);
		}
		else
		{
			TRACE_REQ("TRACE-REQUEST:WRITE"
					  << "\t\t" << clockCycle << ":"
					  << "\t\tAddress: " << request->address << "\t\tBank: " << request->bank << "\t\tColumn: " << request->col << "\t\tRow: " << request->row);
		}
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
