
#ifndef REQUESTSCHEDULER_MEDUSA_H
#define REQUESTSCHEDULER_MEDUSA_H

#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_MEDUSA: public RequestScheduler
	{
	private:
		bool writeEnable;
		unsigned int queueIndex;
		//bool roundRobin_Level = false;
		bool scheduled = false;
		vector<unsigned int> requestorIndex;

	public:
		RequestScheduler_MEDUSA(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable):  
			RequestScheduler(requestQueues, commandQueues, requestorTable)
		{
			for(unsigned int index = 0; index < requestQueue.size(); index++) {
				requestorIndex.push_back(0);
			}
			queueIndex = 0;
		}

		void requestSchedule()
		{
			Request* tempRequest = NULL;
			RequestQueue* tempQueue = NULL;
			// Read Request Checking - Batching
			// If there exist a read request to the reserved banks, it must be issued no matther what happen to the high/low watermark
			writeEnable = true;
			for(size_t index = 0; index < requestQueue.size(); index++)
			{
				if(requestQueue[index]->isPerRequestor())
				{
					if(requestQueue[index]->getQueueSize() > 0)
					{
						for(unsigned int num=0; num < requestQueue[index]->getQueueSize(); num++)
						{					
							if(requestorCriticalTable.at(requestorIndex[index]) == true)
							{	
								scheduledRequest = NULL;
								if(requestQueue[index]->getSize(true, requestorIndex[index]) > 0) 
								{
									scheduledRequest = requestQueue[index]->getRequest(requestorIndex[index], 0);
									if(scheduledRequest != NULL)
									{
										writeEnable = false;
										if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))) 
										{
											updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);										
											scheduled = true;
											requestQueue[index]->removeRequest();
										}
									}
								}	
								requestorIndex[index]++;
								if(requestorIndex[index] == requestQueue[index]->getQueueSize()) 
								{
									requestorIndex[index]=0;
								}
								if(scheduled == true)
								{
									scheduled = false;
									return;
								}
							}
							scheduledRequest = NULL;
						}	
					}
					// No read request to the reserved banks found
					if(requestQueue[index]->getQueueSize() > 0)
					{	
						for(unsigned int num=0; num<requestQueue[index]->getQueueSize(); num++)
						{	
							scheduledRequest = NULL;
							if(requestQueue[index]->getSize(true, requestorIndex[index]) > 0) 
							{
								scheduledRequest = requestQueue[index]->getRequest(num, 0);	
								if(scheduledRequest != NULL) {
									writeEnable = false;
									if(isRowHit(scheduledRequest)) {
										if(isSchedulable(scheduledRequest,true)){
											updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);\
											scheduled = true;
											requestQueue[index]->removeRequest();
										}							
									}
									if(scheduled == true)
									{
										scheduled = false;
										return;
									}
								}
								scheduledRequest = NULL;
							}	
						}
					}	
				}				
			}	
			if(writeEnable) {
				RequestQueue::WriteQueue* writeQ = NULL;
				for(unsigned int index=0; index < requestQueue.size(); index++) {
					writeQ = requestQueue[index]->writeQueue;
					if( writeQ != NULL) {
						if(writeQ->highWatermark()) {
							for(unsigned int qIndex=0; qIndex < writeQ->bufferSize(); qIndex++) {
								tempRequest = writeQ->getWrite(qIndex);
								if(isRowHit(tempRequest)) {
									if(isSchedulable(tempRequest, true)) {
										writeQ->popWrite(qIndex);
									}
								}
							}
						}
						int qIndex = 0;
						while(!writeQ->lowWatermark()) {
							tempRequest = writeQ->getWrite(qIndex);
							if(isSchedulable(tempRequest, isRowHit(tempRequest))) {
								writeQ->popWrite(qIndex);
							}
							qIndex++;
						}
					}
				}
			}
			
			tempQueue = NULL;
			tempRequest = NULL;
			delete(tempQueue);
			delete(tempRequest);
		}
	};
}
#endif /* REQUESTSCHEDULER_MEDUSA_H */

