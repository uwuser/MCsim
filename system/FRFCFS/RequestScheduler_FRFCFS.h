
#ifndef REQUESTSCHEDULER_FRFCFS_H
#define REQUESTSCHEDULER_FRFCFS_H

#include "../../src/RequestScheduler.h"


namespace MCsim{
	class RequestScheduler_FRFCFS: public RequestScheduler{
	private:
	public:
		RequestScheduler_FRFCFS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){}
		// Simple FR FCFS scheduler in the Request Queueu structure
		void requestSchedule()
		{			
			for(size_t index = 0; index < requestQueue.size(); index++){ // Loop over the queueing structure
				if(requestQueue[index]->getSize(false,0) > 0)
				{					
					scheduledRequest = scheduleFR(index);  // Take the candidate request from the correspoding queue	
					if(scheduledRequest != NULL)
					{					
						if(isSchedulable(scheduledRequest,isRowHit(scheduledRequest))) // Determine if the request target is an open row or not
						{ 							
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device																				
							requestQueue[index]->removeRequest(); // Remove the request that has been choosed							
						}
					}	
				}
				scheduledRequest = NULL;
			}
		}
	};
}

#endif /* REQUESTSCHEDULER_FRFCFS_H */
