#ifndef COMMANDSCHEDULER_DCMC_H
#define COMMANDSCHEDULER_DCMC_H

#include "../../src/CommandScheduler.h"

namespace MCsim
{
	class CommandScheduler_DCmc: public CommandScheduler
	{
	private:
		vector<pair<BusPacket*, unsigned int>> cmdFIFO;		// FIFO contains ready commands and queue index
		vector<pair<BusPacket*, unsigned int>> srtFIFO;
		vector<unsigned int> queuePending;					// If a command already in FIFO

	public:
		CommandScheduler_DCmc(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable)
		{
			commandQueue[0]->setACT(0);
			for(unsigned int index = 0; index < commandQueue.size(); index++) {
				queuePending.push_back(false);
			}
		}
		~CommandScheduler_DCmc()
		{
			for(auto it=cmdFIFO.begin(); it!=cmdFIFO.end(); it++) {
				delete it->first;
			}
			cmdQueueTimer.clear();
			cmdFIFO.clear();
			queuePending.clear();
		}
		BusPacket* commandSchedule()
		{
			checkCommand = NULL;
			for(unsigned int index = 0; index < commandQueue.size(); index++)
			{
				if(!queuePending[index]) {
					if(commandQueue[index]->getSize(true) > 0) {
						checkCommand = commandQueue[index]->getCommand(true);
					}
					if(checkCommand == NULL && commandQueue[index]->getSize(false) > 0) {
						checkCommand = commandQueue[index]->getCommand(false);
					}
					if(checkCommand != NULL) {
						if(isReady(checkCommand, index)) {
							if(requestorCriticalTable.at(checkCommand->requestorID) == true) {
								cmdFIFO.push_back(std::make_pair(checkCommand,index));
							}
							else {
								srtFIFO.push_back(std::make_pair(checkCommand,index));
							}
							queuePending[index] = true;
						}
						checkCommand = NULL;
					}
				}
			}
			scheduledCommand = NULL;
			// Schedule the FIFO with CAS blocking
			bool casBlocking = false;
			if(cmdFIFO.size() > 0) {
				for(unsigned int index = 0; index < cmdFIFO.size(); index++) {
					checkCommand = cmdFIFO[index].first;
					if(checkCommand->busPacketType < ACT && casBlocking) {
						continue;
					}
					if(isIssuable(checkCommand)) {
						scheduledCommand = checkCommand;
						sendCommand(scheduledCommand, cmdFIFO[index].second, false);
						queuePending[cmdFIFO[index].second] = false;
						cmdFIFO.erase(cmdFIFO.begin() + index);
						return scheduledCommand;
					}
					else{
						if(checkCommand->busPacketType < ACT) {
							casBlocking = true;
						}
					}
				}
			}
			else {
				if(srtFIFO.size() > 0) {
					for(unsigned int index = 0; index < srtFIFO.size(); index++) {
						checkCommand = srtFIFO[index].first;
						if(checkCommand->busPacketType < ACT && casBlocking) {
							continue;
						}
						if(isIssuable(checkCommand)) {
							scheduledCommand = checkCommand;
							sendCommand(scheduledCommand, srtFIFO[index].second, false);
							queuePending[srtFIFO[index].second] = false;
							srtFIFO.erase(srtFIFO.begin() + index);
							return scheduledCommand;
						}
					}
				}
			}
			return NULL;
		}
	};
}
#endif /* COMMANDSCHEDULER_DCMC_H */

