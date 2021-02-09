
#ifndef REQUESTSCHEDULER_FRFCFS_BATCHING_H
#define REQUESTSCHEDULER_FRFCFS_BATCHING_H

#include "../../src/RequestScheduler.h"

namespace MCsim
{
	class RequestScheduler_FRFCFS_BATCHING : public RequestScheduler
	{
	private:
	public:
		RequestScheduler_FRFCFS_BATCHING(std::vector<RequestQueue *> &requestQueues, std::vector<CommandQueue *> &commandQueues, const std::map<unsigned int, bool> &requestorTable) : RequestScheduler(requestQueues, commandQueues, requestorTable)
		{
		}
		void requestSchedule()
		{
			Request *tempRequest = NULL;

			bool writeQueue = writeEnable(0);
			bool cont_1 = true;
			bool cont_3 = true;
			bool cont_2 = false;

			if (writeQueue)
			{
				RequestQueue::WriteQueue *writeQ = NULL;
				for (unsigned int index = 0; index < requestQueue.size(); index++)
				{
					writeQ = requestQueue[index]->writeQueue;
					if (writeQ != NULL)
					{
						if (writeQ->highWatermark())
						{
							sw = true;
							requestQueue[0]->setWriteMode(true);
						}
						if (sw)
						{
							int qIndex = 0;
							if (!writeQ->lowWatermark())
							{
								tempRequest = writeQ->getWrite(qIndex);
								if (isSchedulable(tempRequest, isRowHit(tempRequest)))
								{
									updateRowTable(tempRequest->addressMap[Rank], tempRequest->addressMap[Bank], tempRequest->row);
									writeQ->popWrite(qIndex);
									cont_1 = false;
								}
								qIndex++;
							}
							else
							{
								sw = false;
								requestQueue[0]->setWriteMode(false);
								cont_1 = true;
							}
						}
					}
				}
			}
			if (cont_1)
			{
				for (size_t index = 0; index < requestQueue.size(); index++)
				{
					for (size_t iindex = 0; iindex < commandQueue.size(); iindex++)
					{
						if (commandQueue[iindex]->getSize(true) == 0)
						{
							scheduledRequest = scheduleFR(index);
							if (scheduledRequest != NULL)
							{
								if (isSchedulable(scheduledRequest, isRowHit(scheduledRequest)))
								{
									updateRowTable(scheduledRequest->addressMap[Rank], scheduledRequest->addressMap[Bank], scheduledRequest->row);
									requestQueue[index]->removeRequest();
									cont_3 = false;
								}
							}
						}
					}
				}
			}
			for (unsigned int index = 0; index < commandQueue.size(); index++)
			{
				if (commandQueue[index]->getSize(true) > 0)
				{
					cont_2 = true;
				}
			}
			if (!cont_2 && cont_1 && cont_3)
			{
				RequestQueue::WriteQueue *writeQ = NULL;
				writeQ = requestQueue[0]->writeQueue;
				if (writeQ->bufferSize() != 0)
				{
					tempRequest = writeQ->getWrite(0);
					if (isSchedulable(tempRequest, isRowHit(tempRequest)))
					{
						updateRowTable(tempRequest->addressMap[Rank], tempRequest->addressMap[Bank], tempRequest->row);
						writeQ->popWrite(0);
						cont_2 = false;
					}
				}
			}
			scheduledRequest = NULL;
		}
	};
} // namespace MCsim
#endif
