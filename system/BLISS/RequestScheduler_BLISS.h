
#ifndef REQUESTSCHEDULER_BLISS_H
#define REQUESTSCHEDULER_BLISS_H

#include "../../src/RequestScheduler.h"
#include <queue>


namespace MCsim
{
	class RequestScheduler_BLISS: public RequestScheduler
	{
	private:
		unsigned int last_pid;
		unsigned int counterstreak;
		unsigned int threshold;		
		unsigned long int temporal_clock;

	public:

		RequestScheduler_BLISS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler(requestQueues, commandQueues, requestorTable)	{    // Ctor
			counterstreak = 0;
			last_pid = 4;
			threshold = 4;
			temporal_clock = 0;
			for (unsigned int p = 0; p < requestorCriticalTable.size(); p++){				
                blacklist[p] = 0;
			}
		}
	    void clear_marking() {
            for (unsigned int p = 0; p < requestorCriticalTable.size(); p++)
                blacklist[p] = 0;
        }
		void requestSchedule(){
			temporal_clock++;
			scheduledRequest = NULL;

            if (temporal_clock%10000 == 0 && temporal_clock != 1) {             
                clear_marking();
            }
			for(size_t index = 0; index < requestQueue.size(); index++) 
			{						
				if(requestQueue[index]->getSize(false,0) > 0)
				{						
					scheduledRequest = scheduleBLISS(index);
					if(scheduledRequest != NULL){
						if(scheduledRequest->requestorID == last_pid && counterstreak < threshold){
							counterstreak++;
						}
						else if (scheduledRequest->requestorID == last_pid && counterstreak == threshold){
							blacklist[scheduledRequest->requestorID] = 1;
							counterstreak = 1;
						}
						else {
							counterstreak = 1;
						}
						for(unsigned int k=0; k<requestorCriticalTable.size();k++){
							if(blacklist[k] == 1){
							}
						}
						last_pid = scheduledRequest->requestorID;
						if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest)))	{
							updateRowTable(scheduledRequest->rank, scheduledRequest->bank, scheduledRequest->row);
							requestQueue[index]->removeRequest();
						}
						scheduledRequest = NULL;			
					}	
				}						
			}
		}
	};
}

#endif /* REQUESTSCHEDULER_BLISS_H */
