
#ifndef REQUESTSCHEDULER_RR_H
#define REQUESTSCHEDULER_RR_H

#include <queue>

#include "../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_RR: public RequestScheduler
	{
	private:
		unsigned int slotCounter, timeSlot, dataBusSize;		
		unsigned int queueIndex; // Check order of queue index		
		std::vector<unsigned> queueRequestorIndex; // Requestor queue index in individual memory level 
		std::vector<Request*> srtRequestBuffer;
		bool fixedPrioirty;

	public:
		RequestScheduler_RR(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{
			// The time slot must be calculated based on the device and pattern
			// ex.DDR3-1600H: RCD + WL + Bus + WR + RP = 42
			// request scheduler picks a request at each timeslot and then move to the next requestor based on the RR arbitration 
			fixedPrioirty = false;
			slotCounter = 0;
			dataBusSize = dataBus;
			timeSlot = 42;
			queueIndex = 0;

			for(unsigned int index=0; index<requestQueue.size(); index++){
				queueRequestorIndex.push_back(0);
			}
		}

		void requestSchedule()
		{
			
			if(slotCounter == 0)
			{
				scheduledRequest = NULL;
				for(unsigned int index=0; index<requestQueue.size(); index++) 
				{					
					if(requestQueue[queueIndex]->isPerRequestor()) 
					{
						if(requestQueue[queueIndex]->getQueueSize() > 0) 
						{
							for(unsigned int subIndex=0; subIndex < requestQueue[queueIndex]->getQueueSize(); subIndex++)
							{
								if(requestQueue[queueIndex]->getSize(true, queueRequestorIndex[queueIndex]) > 0)
								{
									scheduledRequest = requestQueue[queueIndex]->getRequest(queueRequestorIndex[queueIndex],0);
								}
								if(scheduledRequest != NULL)
								{
									if(fixedPrioirty == true) {
										if(requestorCriticalTable.at(scheduledRequest->requestorID) == false) {
											srtRequestBuffer.push_back(scheduledRequest);
											requestQueue[queueIndex]->removeRequest();
											scheduledRequest = NULL;
										}
									}
								}
								if(scheduledRequest != NULL) {
									if(isSchedulable(scheduledRequest, false)) {
										requestQueue[index]->removeRequest();
									}
									else {
										break;
									}
								}													
								queueRequestorIndex[queueIndex]++; // Update to next requestor
								if(queueRequestorIndex[queueIndex] == requestQueue[queueIndex]->getQueueSize()) {
									queueRequestorIndex[queueIndex] = 0;
								}
								if(scheduledRequest != NULL) {
									break;
								}
							}
							if(scheduledRequest == NULL) {
								if(fixedPrioirty == true && !srtRequestBuffer.empty()) {
									if(isSchedulable(srtRequestBuffer.front(), false)) {
										srtRequestBuffer.erase(srtRequestBuffer.begin());
									}
								}
							}
						}
					}
					else
					{
						if(requestQueue[queueIndex]->getSize(false, 0) > 0)
						{
							scheduledRequest = requestQueue[queueIndex]->getRequest(0);
							if(isSchedulable(scheduledRequest, false)) {
								requestQueue[index]->removeRequest();
							}
						}
					}
					queueIndex++;
					if(queueIndex == requestQueue.size()) {
						queueIndex = 0;
					}					
					if(scheduledRequest != NULL) 
					{
						slotCounter = timeSlot - 1;
						break;
					}
				}
			}
			else
			{
				if(slotCounter > 0) {
					slotCounter--;
				}
			}
		}
	};
}

#endif /* REQUESTSCHEDULER_RR_H  */
