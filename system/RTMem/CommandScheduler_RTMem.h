#ifndef COMMANDSCHEDULER_RTMEM_H
#define COMMANDSCHEDULER_RTMEM_H

#include "../../src/CommandScheduler.h"

namespace MCsim
{
	class CommandScheduler_RTMem: public CommandScheduler
	{
	private:

	public:
		CommandScheduler_RTMem(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable)
		{}
		BusPacket* commandSchedule()
		{
			scheduledCommand = NULL;
			checkCommand = NULL;
			BusPacket* tempCAS = NULL;
			BusPacket* tempACT = NULL;
			commandQueue[0]->setACT(0);
			for(unsigned int index = 0; index < commandQueue.size(); index++) {
				if(commandQueue[index]->getSize(true) > 0) {
					checkCommand = commandQueue[index]->getCommand(true);
				}
				else if(commandQueue[index]->getSize(false) > 0){
					checkCommand = commandQueue[index]->getCommand(false);
				}
				if(checkCommand!=NULL) {
					if(isReady(checkCommand, checkCommand->bank)) {
						if(checkCommand->busPacketType < ACT) {
							if(tempCAS != NULL) {
								if(checkCommand->arriveTime < tempCAS->arriveTime) {
									tempCAS = checkCommand;
								}
							}
							else {tempCAS = checkCommand;}
						}
						else {
							if(tempACT != NULL) {
								if(checkCommand->arriveTime < tempACT->arriveTime) {
									tempACT = checkCommand;}
							}
							else {tempACT = checkCommand;}
						}
					}
					checkCommand = NULL;
				}
			}
			if(tempCAS != NULL) {
				if(isIssuable(tempCAS)) {
					scheduledCommand = tempCAS;
				}	
			}
			if(scheduledCommand == NULL && tempACT != NULL) {
				if(isIssuable(tempACT)) {
					scheduledCommand = tempACT;
				}					
			}
			if(scheduledCommand != NULL) {
				sendCommand(scheduledCommand, scheduledCommand->bank, false);
			}
			tempCAS = NULL;
			tempACT = NULL;
			delete(tempCAS);
			delete(tempACT);
			checkCommand = NULL;
			return scheduledCommand;
		}
	};
}
#endif /* COMMANDSCHEDULER_RTMEM_H */
