#ifndef REQUESTSCHEDULER_PIPECAS_H
#define REQUESTSCHEDULER_PIPECAS_H

#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_PipeCAS: public RequestScheduler
	{
	private:
		bool newRound;
		bool firstACT;
		RequestType bundlingType;
		vector<unsigned int> scheduledCmdQueue;
		bool isSchedulable(Request* request, bool open)
		{
			if(requestorCriticalTable.at(scheduledRequest->requestorID)== true) {
				for(unsigned int index=0; index<scheduledCmdQueue.size(); index++) {
					if(commandQueue[scheduledCmdQueue[index]]->getSize(true) < 2) 
						return false;					
				}
			}
			return true;
		}

	public:
		RequestScheduler_PipeCAS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable) 
		{
			newRound = true;
			bundlingType = RequestType::DATA_READ;
		}
		void requestSchedule()
		{					
			newRound = true; // Check if all the commands for scheduled requests are executed		
			firstACT = false; 	// Check if the execution already starts?
			if(scheduledCmdQueue.size() > 0) {
				for(unsigned int index=0; index<scheduledCmdQueue.size(); index++) {
					if(commandQueue[scheduledCmdQueue[index]]->getSize(true) > 0) {
						newRound = false;
						if(commandQueue[scheduledCmdQueue[index]]->getSize(true) < 2) {
							firstACT = true;
						}
					}
				}
				if(newRound) 
					scheduledCmdQueue.clear();				
			}
			if(newRound || !firstACT) {
				for(int index=0; index < requestQueue.size(); index++)
				{
					if(requestQueue[index]->getSize(false, 0) > 0) {
						scheduledRequest = scheduleFR(index);
						if(scheduledRequest->requestType == bundlingType) {
							if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))) {
								if(requestorCriticalTable.at(scheduledRequest->requestorID) == true ){
									scheduledCmdQueue.push_back(scheduledRequest->requestorID);
								}
								updateRowTable(scheduledRequest->rank, scheduledRequest->bank, scheduledRequest->row);
								requestQueue[index]->removeRequest();
							}
						}
					}
				}
			}
			else {
				if(bundlingType == RequestType::DATA_WRITE) {bundlingType = RequestType::DATA_WRITE; }
				else {bundlingType = RequestType::DATA_WRITE; }
			}
		}
	};
}
#endif /* REQUESTSCHEDULER_PIPECAS_H */