#ifndef REQUESTSCHEDULER_PMC_H
#define REQUESTSCHEDULER_PMC_H

#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_PMC: public RequestScheduler
	{
	private:
		unsigned int requestorIndex;
		unsigned int slotCounter;

		vector<int> scheduleSlot;
		vector<int> timeSlot;

	public:
		RequestScheduler_PMC(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{
			// The time slot must be calculated based on the device and pattern
			// ex.DDR3-1600H: RCD + WL + Bus + WR + RP = 42
			slotCounter = 0;
			requestorIndex = 0;
		}

		void requestSchedule()
		{
		
			if(clockCycle == 1) {
				scheduleSlot = {0, 1, 2, 3, 4, 5, 6, 7};
				// for(int index=0; index<requestQueue.size(); index++) {
				// 	scheduleSlot.push_back(index);
				// 	timeSlot.push_back(42);  // 8 57 146
				// }
				timeSlot = {47, 47, 47, 47, 47, 47, 47, 47};
			}

			if(slotCounter == 0) {
				scheduledRequest = NULL;
				for(unsigned int index=0; index < scheduleSlot.size(); index++) 
				{
				
					if(requestQueue[0]->getSize(true, scheduleSlot[requestorIndex]) > 0) {
					
						scheduledRequest = requestQueue[0]->getRequest(scheduleSlot[requestorIndex],0);
					}

					if(scheduledRequest != NULL) {
						
						if(isSchedulable(scheduledRequest, false)) {
							requestQueue[0]->removeRequest();
							slotCounter = timeSlot[scheduleSlot[requestorIndex]]-1;
						}
						else{
							DEBUG("ISSUE "<<requestorIndex);
							abort();
						}
					}
					requestorIndex++;
					if(requestorIndex == scheduleSlot.size()) {
						requestorIndex = 0;
					}
					if(scheduledRequest != NULL) {
						scheduledRequest = NULL;
						break;
					}
				}
			}
			else {
				if(slotCounter > 0) {
					slotCounter--;
				}
			}
		}
	};
}

#endif /* REQUESTSCHEDULER_PMC_H */