#ifndef REQUESTSCHEDULER_RTMEM_H
#define REQUESTSCHEDULER_RTMEM_H

#include <queue>
#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_RTMem: public RequestScheduler_RR
	{
	public:
		RequestScheduler_RTMem(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler_RR(requestQueues, commandQueues, requestorTable, dataBus) 
		{}
	private:
		bool isSchedulable(Request* request, bool open) 
		{
			for(unsigned int index=0; index < commandQueue.size(); index++) {
				if(commandQueue[index]->getSize(true) >= 2 || commandQueue[index]->getSize(false)>=2) {
					return false;
				}
			}
			commandGenerator->commandGenerate(request, open);
			return true;
		}
	};
}

#endif /* REQUESTSCHEDULER_RTMEM_H */