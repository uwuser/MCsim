#ifndef REQUESTSCHEDULER_FRFCFS_BACKEND_H
#define REQUESTSCHEDULER_FRFCFS_BACKEND_H

#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_FRFCFS_BACKEND: public RequestScheduler
	{
	private:
		bool roundRobin_Level = false;  // Switch to RR arbitration
		map<unsigned int, bool> req_pending;
		vector<unsigned int> requestorIndex;
		std::map<unsigned long int, unsigned long int> deadline_track;

	public:
		RequestScheduler_FRFCFS_BACKEND(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{
			for(unsigned int index = 0; index < requestQueue.size(); index++) {
				requestorIndex.push_back(0);
			}
			for(unsigned int i =0; i<8;i++)
				req_pending[i] = false;
		}

		void requestSchedule()
		{
			//cout<<"---------------------------------Inside Command Generator-----------------------------------------"<<endl;
			for(size_t index =0; index < requestQueue.size(); index++) 
			{
				if(requestQueue[index]->isPerRequestor())
				{
					if(requestQueue[index]->getQueueSize() > 0)
					{
						//cout<<"1"<<endl;
						for(unsigned int num = 0; num < requestQueue[index]->getQueueSize(); num++)
						{
							//cout<<"2"<<endl;
							scheduledRequest = NULL;
							if(requestQueue[index]->getSize(true,num) > 0) 
							{	
								//cout<<"3"<<endl;
								scheduledRequest = scheduleFR_BACKEND(index,num);
								//cout<<"9"<<endl;
								if(scheduledRequest != NULL){
									// Determine if the request target an open row or not																											
									updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row); // Update the open row table for the device	
									//cout<<"10"<<endl;
									requestQueue[index]->removeRequest(); // Remove the request that has been choosed
								}																	
							
								
							}
						}
					}
					scheduledRequest = NULL;
				}
			}			
		}
	};
}

#endif  /* REQUESTSCHEDULER_DIRECT_H */


