#ifndef REQUESTSCHEDULER_DIRECT_H
#define REQUESTSCHEDULER_DIRECT_H

#include "../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_Direct: public RequestScheduler
	{
	private:
		bool roundRobin_Level = false;  // Determine the RR if true
		vector<unsigned int> requestorIndex;

	public:
		RequestScheduler_Direct(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{
			for(unsigned int index = 0; index < requestQueue.size(); index++) {
				requestorIndex.push_back(0);
			}
		}

		void requestSchedule()
		{
			for(size_t index =0; index < requestQueue.size(); index++) {
				// Requestor Direct Connection per Memory Level - If using per requestor queue, it implements a RR
				if(requestQueue[index]->isPerRequestor())
				{
					if(requestQueue[index]->getQueueSize() > 0)
					{
						for(unsigned int num=0; num<requestQueue[index]->getQueueSize(); num++)
						{
							scheduledRequest = NULL;
							// requestorIndex[index]=num;   ENABLE if interested in having RR arbitration
							if(requestQueue[index]->getSize(true, requestorIndex[index]) > 0) 
							{
								// Take the first request of the corresponding request queue
								scheduledRequest = requestQueue[index]->getRequest(requestorIndex[index], 0);
								// Determine if the request target an open row or not
								if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest)))
								{
									// Update the open row table for the device												
									updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);
									// Remove the request that has been choosed
									requestQueue[index]->removeRequest();
								}
							}
							// Useful when using RR arbitration
							requestorIndex[index]++;
							if(requestorIndex[index] == requestQueue[index]->getQueueSize()) {
								requestorIndex[index]=0;
							}
							if(roundRobin_Level == true && scheduledRequest!=NULL) // since it is true here, it take one instance from each requestor
							{
								break;
							}
						}
					}
					scheduledRequest = NULL;
				}				
				else
				{
					// Using a global queue -> It will implement a FRFCFS scheduling
					if(requestQueue[index]->getSize(false,0) > 0)
					{
						// Take the candidate request from the correspoding queue
						scheduledRequest = scheduleFR(index);
						// Determine if the request target an open row or not
						if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest)))
						{
							// Update the open row table for the device
							updateRowTable(scheduledRequest->rank, scheduledRequest->bank, scheduledRequest->row);
							// Remove the request that has been choosed
							requestQueue[index]->removeRequest();
						}
						scheduledRequest = NULL;
					}
				}					
			}
		}
	};
}

#endif

