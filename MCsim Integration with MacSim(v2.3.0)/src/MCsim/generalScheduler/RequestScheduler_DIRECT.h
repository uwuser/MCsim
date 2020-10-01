#ifndef REQUESTSCHEDULER_DIRECT_H
#define REQUESTSCHEDULER_DIRECT_H

#include "../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_Direct: public RequestScheduler
	{
	private:
		bool roundRobin_Level = false;  // Switch to RR arbitration
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
								scheduledRequest = requestQueue[index]->getRequest(requestorIndex[index], 0); // Take the first request of the corresponding request queue
								// Determine if the request target an open row or not
								if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest)))
								{																				
									updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device										
									requestQueue[index]->removeRequest(); // Remove the request that has been choosed
								}
							}
							// When using RR arbitration
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
						//scheduledRequest = scheduleFR(index); // Take the candidate request from the correspoding queue						
						if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))) // Determine if the request target an open row or not
						{							
							updateRowTable(scheduledRequest->rank, scheduledRequest->bank, scheduledRequest->row); // Update the open row table for the device							
							requestQueue[index]->removeRequest(); // Remove the request that has been choosed
						}
						scheduledRequest = NULL;
					}
				}					
			}
		}
	};
}

#endif  /* REQUESTSCHEDULER_DIRECT_H */

