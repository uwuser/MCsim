
#ifndef REQUESTSCHEDULER_FRFCFS_BATCHING_H
#define REQUESTSCHEDULER_FRFCFS_BATCHING_H

#include "../../src/RequestScheduler.h"


namespace MCsim
{
	class RequestScheduler_FRFCFS_BATCHING: public RequestScheduler{
	private:

	public:

		RequestScheduler_FRFCFS_BATCHING(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){
		}
		void requestSchedule(){
			Request* tempRequest = NULL;

			bool writeQueue = writeEnable(0);
			bool cont_1 = true;
			bool cont_3 = true;
			bool cont_2 = false;
			bool sw = false;
			if(writeQueue)
			{
				RequestQueue::WriteQueue* writeQ = NULL;
				for(unsigned int index=0; index < requestQueue.size(); index++) {
					writeQ = requestQueue[index]->writeQueue;
					if( writeQ != NULL) {
						if(writeQ->highWatermark()) {
							sw = true;
							for(unsigned int qIndex=0; qIndex < writeQ->bufferSize(); qIndex++) {
								tempRequest = writeQ->getWrite(qIndex);
								if(isRowHit(tempRequest)) {
									if(isSchedulable(tempRequest, true)) {
										writeQ->popWrite(qIndex);
										cont_1 = false;
									}
								}
							}
						}
						if(sw)
						{
							int qIndex = 0;
							while(!writeQ->lowWatermark()) {
								tempRequest = writeQ->getWrite(qIndex);
								if(isSchedulable(tempRequest, isRowHit(tempRequest))) {
									updateRowTable(tempRequest->addressMap[Rank], tempRequest->addressMap[Bank], tempRequest->row);
									writeQ->popWrite(qIndex);
									cont_1 = false;
								}
								qIndex++;
							}
						}	
					}
				}
			}
			if(cont_1)
			{
				for(size_t index = 0; index < requestQueue.size(); index++)
				{
					scheduledRequest = scheduleFR(index); 
					if(scheduledRequest != NULL){
						if(isSchedulable(scheduledRequest,isRowHit(scheduledRequest))){
							updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);
							requestQueue[index]->removeRequest();
							cont_3 = false;
						}
					}
				}
			}
			for(unsigned int index=0; index < commandQueue.size(); index++){
				if (commandQueue[index]->getSize(true) > 0){
					cont_2 = true;
				}				
			}
			if(!cont_2 && cont_1 && cont_3){				
				RequestQueue::WriteQueue* writeQ = NULL;
				writeQ = requestQueue[0]->writeQueue;
				if(writeQ->bufferSize() != 0)
				{
					tempRequest = writeQ->getWrite(0);
					if(isSchedulable(tempRequest, isRowHit(tempRequest))) {
						updateRowTable(tempRequest->addressMap[Rank], tempRequest->addressMap[Bank], tempRequest->row);
						writeQ->popWrite(0);
						cont_2 = false;
					}
				}
			}	
			scheduledRequest = NULL;
		}
	};
}
#endif

