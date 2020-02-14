#ifndef REQUESTSCHEDULER_MCMC_H
#define REQUESTSCHEDULER_MCMC_H

#include "../../src/RequestScheduler.h"

#include <queue>

namespace MCsim
{
	class RequestScheduler_MCMC: public RequestScheduler
	{
	private:
		unsigned int requestorIndex;
		unsigned int slotCounter;
		int dataBusSize;

		vector<int> scheduleSlot;
		vector<int> timeSlot;
		vector<vector<int>> srtSlot;
		vector<unsigned int> srtIndex;
		vector<queue<Request*>> reqSubBuffer;

	public:
		RequestScheduler_MCMC(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{			
			slotCounter = 0;
			requestorIndex = 0;
			dataBusSize = dataBus;
		}

		void requestSchedule()
		{	
			if(clockCycle == 1) {
				for(auto req = requestorCriticalTable.begin(); req != requestorCriticalTable.end(); req++) {
					if(req->second) {
						scheduleSlot.push_back(req->first);
						timeSlot.push_back(10);
					}
					reqSubBuffer.push_back(queue<Request*>());
				}
				for(unsigned int index=0; index<scheduleSlot.size(); index++) {
					srtSlot.push_back(vector<int>());
					srtIndex.push_back(0);
				}
				for(auto req = requestorCriticalTable.begin(); req != requestorCriticalTable.end(); req++) {
					if(!req->second) {
						srtSlot[req->first%scheduleSlot.size()].push_back(req->first);
					}
				}	
			}
			step();	
			if(slotCounter == 0) {
				scheduledRequest = NULL;
				if(reqSubBuffer[scheduleSlot[requestorIndex]].empty()) {
					if(requestQueue[0]->getSize(true, scheduleSlot[requestorIndex]) > 0) {
						scheduledRequest = requestQueue[0]->getRequest(scheduleSlot[requestorIndex], 0);

						for(unsigned int index=0; index < scheduledRequest->requestSize/dataBusSize; index++) {
							reqSubBuffer[scheduleSlot[requestorIndex]].push(new Request(scheduledRequest->requestorID, scheduledRequest->requestType, 
								dataBusSize, scheduledRequest->address, NULL));
							reqSubBuffer[scheduleSlot[requestorIndex]].back()->rank = scheduledRequest->rank;
							reqSubBuffer[scheduleSlot[requestorIndex]].back()->bank = scheduledRequest->bank;
							reqSubBuffer[scheduleSlot[requestorIndex]].back()->row = scheduledRequest->row;
							reqSubBuffer[scheduleSlot[requestorIndex]].back()->col = scheduledRequest->col;
						}
						requestQueue[0]->removeRequest();
					}
				}
				if(!reqSubBuffer[scheduleSlot[requestorIndex]].empty()) {
					scheduledRequest = reqSubBuffer[scheduleSlot[requestorIndex]].front();
					if(isSchedulable(scheduledRequest, false)) {
						delete scheduledRequest;
						scheduledRequest = NULL;
						reqSubBuffer[scheduleSlot[requestorIndex]].pop();
					}
					scheduledRequest = NULL;
				}
				else {
					unsigned srtQueue = 0;
					for(unsigned int index=0; index<srtSlot[requestorIndex].size(); index++) {
						srtQueue = srtSlot[requestorIndex][srtIndex[requestorIndex]];
						if(reqSubBuffer[srtQueue].empty() &&
							requestQueue[0]->getSize(true, srtQueue) > 0) 
						{
							scheduledRequest = requestQueue[0]->getRequest(srtQueue,0);
							for(unsigned int index=0; index < scheduledRequest->requestSize/dataBusSize; index++) {
								reqSubBuffer[srtQueue].push(new Request(scheduledRequest->requestorID, scheduledRequest->requestType, 
									dataBusSize, scheduledRequest->address, NULL));
								reqSubBuffer[srtQueue].back()->rank = scheduledRequest->rank;
								reqSubBuffer[srtQueue].back()->bank = scheduledRequest->bank;
								reqSubBuffer[srtQueue].back()->row = scheduledRequest->row;
								reqSubBuffer[srtQueue].back()->col = scheduledRequest->col;
							}								
							requestQueue[0]->removeRequest();
						}
						if(!reqSubBuffer[srtQueue].empty())
						{
							scheduledRequest = reqSubBuffer[srtQueue].front();
							if(isSchedulable(scheduledRequest,false)) {
								reqSubBuffer[srtQueue].pop();
							}
						}
						srtIndex[requestorIndex]++;
						if(srtIndex[requestorIndex] == srtSlot[requestorIndex].size()) {
							srtIndex[requestorIndex] = 0;
						}
						if(scheduledRequest != NULL) {
							delete scheduledRequest;
							scheduledRequest = NULL;
							break;
						}
					}						
				}
				slotCounter = timeSlot[scheduleSlot[requestorIndex]]-1;
				requestorIndex++;
				if(requestorIndex == scheduleSlot.size()) {
					requestorIndex = 0;
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

#endif /* REQUESTSCHEDULER_MCMC_H */
