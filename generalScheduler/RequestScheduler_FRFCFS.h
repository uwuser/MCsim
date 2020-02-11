#ifndef REQUESTSCHEDULER_FRFCFS_H
#define REQUESTSCHEDULER_FRFCFS_H

#include "../src/RequestScheduler.h"

using namespace std;

namespace MCsim{
	class RequestScheduler_FRFCFS: public RequestScheduler{
	private:
	public:
		RequestScheduler_FRFCFS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){
		}
		// Simple FR FCFS scheduler in the Request Queueu structure
		void requestSchedule(){
			// Loop over the queueing structure
			for(size_t index = 0; index < requestQueue.size(); index++){
				if(requestQueue[index]->getSize(false,0) > 0){
					//cout<<"request queue is greater than zero"<<endl;
					// Take the candidate request from the correspoding queue
					scheduledRequest = scheduleFR(index); 	
					//cout<<"done with schedulerFR "<<endl;
					if(scheduledRequest != NULL){
						// Determine if the request target is an open row or not
						if(isSchedulable(scheduledRequest,FR_open)){
							// Update the open row table for the device
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);
							// Remove the request that has been choosed
							bool WriteEnabled = requestQueue[0]->isWriteEnable();
							if(!WriteEnabled){
								requestQueue[index]->removeRequest();
							}
							else
							{
								if(scheduledRequest->requestType == DATA_READ){													
								requestQueue[index]->removeRequest();
								}
								else
								{						
									requestQueue[index]->removeWriteRequest();	
								}
							}	
						}
					}
					//cout<<"request is picked in FRFCFS"<<endl;	
				}
				if(switch_enable && requestQueue[index]->writeSize() > 0 && requestQueue[0]->isWriteEnable()){
					scheduledRequest = requestQueue[index]->scheduleWritecheck();
					if(scheduledRequest != NULL){
						if(isSchedulable(scheduledRequest,FR_open)){
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);
							requestQueue[index]->removeWriteRequest();
						}
					}		
				}
				scheduledRequest = NULL;
			}
		}
	};
}

#endif