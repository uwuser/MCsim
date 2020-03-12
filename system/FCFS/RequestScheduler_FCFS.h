#ifndef REQUESTSCHEDULER_FCFS_H
#define REQUESTSCHEDULER_FCFS_H

#include <queue>
#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_FCFS: public RequestScheduler{
	private:
		unsigned int numIndex;

	public:
		RequestScheduler_FCFS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){}
		void requestSchedule()
		{
			scheduledRequest = NULL;
			checkRequest = NULL;

			for(unsigned int index=0; index<requestQueue.size(); index++) {				
				if(requestQueue[index]->isPerRequestor()){		// Requestor queue per Memory Level
					for(unsigned int num=0; num < requestQueue[index]->getQueueSize(); num++){
						if(requestQueue[index]->getSize(true,num) > 0){							
							checkRequest = requestQueue[index]->checkRequestIndex(num, 0); // Take the first request from this requestor
							numIndex = num;
							break;
						}
					}
					// Check if there is any other request from other requestors that arrived earlier
					for(unsigned int num_1=0; num_1 < requestQueue[index]->getQueueSize(); num_1++){
						if(requestQueue[index]->getSize(true,num_1) > 0){								
							if(checkRequest->arriveTime >= requestQueue[index]->checkRequestIndex(num_1,0)->arriveTime){
								checkRequest = requestQueue[index]->checkRequestIndex(num_1, 0);
								numIndex = num_1;
							}
						}
					}	
					// Take the target request from numIndex
					if(checkRequest != NULL) {
						scheduledRequest = requestQueue[index]->getRequest(numIndex,0);						
						if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))){ // Determine if the request target an open row or not								
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);	// Update the open row table for the device							
							requestQueue[index]->removeRequest();	// Remove the request that has been choosed						
						}
						else{							
							scheduledRequest = NULL;
						}
					}
					scheduledRequest = NULL;
				}
				else
				{
					// If request queue is general not per requestor
					scheduledRequest = NULL;
					if(requestQueue[index]->getSize(false,0) > 0) {						
						scheduledRequest = requestQueue[index]->getRequest(0);	// Take the request from the head of the queue
						if(scheduledRequest != NULL){							
							if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))){ // Determine if the request target an open row or not								
								updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device								
								requestQueue[index]->removeRequest(); // Remove the request that has been choosed
							}
						}	
					}
					scheduledRequest = NULL;
				}	
			}
		}
	};
}
#endif /* REQUESTSCHEDULER_FCFS_H */