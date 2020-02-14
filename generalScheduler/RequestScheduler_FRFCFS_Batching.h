#ifndef REQUESTSCHEDULER_FRFCFS_Batching_H
#define REQUESTSCHEDULER_FRFCFS_Batching_H

#include "../src/RequestScheduler.h"

using namespace std;

namespace MCsim{
	class RequestScheduler_FRFCFS_Batching: public RequestScheduler{
	private:
	public:
		RequestScheduler_FRFCFS_Batching(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){}
		// Simple FR FCFS scheduler in the Request Queueu structure while incorporating write batching weith high and low watermark in write queue
		void requestSchedule(){
			
			for(size_t index = 0; index < requestQueue.size(); index++){ // Loop over the queueing structure
				if(requestQueue[index]->getSize(false,0) > 0){									
					scheduledRequest = scheduleFR(index); // Take the candidate request from the correspoding queue						
					if(scheduledRequest != NULL){						
						if(isSchedulable(scheduledRequest,FR_open)){ // Determine if the request target is an open row or not							
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device							
							bool WriteEnabled = requestQueue[0]->isWriteEnable(); // is write batching enabled?
							if(!WriteEnabled)
								requestQueue[index]->removeRequest(); // Remove the request that has been choosed							
							else
							{
								if(scheduledRequest->requestType == DATA_READ)													
									requestQueue[index]->removeRequest(); // Remove the request that has been choosed								
								else														
									requestQueue[index]->removeWriteRequest();	// Remove the request from write queue that has been choosed								
							}	
						}
					}
				}
				if(switch_enable && requestQueue[index]->writeSize() > 0 && requestQueue[0]->isWriteEnable()){ // do we need to flush the write requests remained in the write queue
					scheduledRequest = requestQueue[index]->scheduleWritecheck();
					if(scheduledRequest != NULL){
						if(isSchedulable(scheduledRequest,FR_open)){
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device	
							requestQueue[index]->removeWriteRequest(); // Remove the request from write queue that has been choosed
						}
					}		
				}
				scheduledRequest = NULL;
			}
		}
	};
}

#endif  /* REQUESTSCHEDULER_FRFCFS_Batching_H */
