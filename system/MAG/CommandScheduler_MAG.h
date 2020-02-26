#ifndef COMMANDSCHEDULER_MAG_H
#define COMMANDSCHEDULER_MAG_H

#include "../../src/CommandScheduler.h"

namespace MCsim
{
	class CommandScheduler_MAG: public CommandScheduler
	{
	private:
		unsigned int indexRT;
		unsigned int indexHP;
		bool bypass;
		bool casBlocking;

		vector< vector<BusPacket*> > compensateCommands; 

	public:
		CommandScheduler_MAG(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable)
		{
			casBlocking = false;
			indexRT = 0;
			indexHP = 0;
			bypass = false;
			commandQueue[0]->setACT(0);

			for(unsigned int index=0; index<commandQueue.size(); index++) {
				compensateCommands.push_back(vector<BusPacket*>());
			}
		}
		// Generate Preemptive Commands
		void compensateGenerator(unsigned queueIndex)
		{
			if(commandQueue[queueIndex]->getSize(false) > 0)
			{
				BusPacket* tempCmd = commandQueue[queueIndex]->getCommand(false);
				if(tempCmd->busPacketType < ACT) {
					compensateCommands[queueIndex].push_back(new BusPacket(PRE, 0, 0, 0, 0, tempCmd->bank, tempCmd->rank, NULL, 0));
					compensateCommands[queueIndex].push_back(new BusPacket(ACT, 0, 0, 0, 0, tempCmd->bank, tempCmd->rank, NULL, 0));
				}
				else if(tempCmd->busPacketType == ACT) {
					compensateCommands[queueIndex].push_back(new BusPacket(PRE, 0, 0, 0, 0, tempCmd->bank, tempCmd->rank, NULL, 0));
				}
				else {
					return;
				}
			}
			else
			{
				return;
			}
		}
		BusPacket* commandSchedule()
		{
			scheduledCommand = NULL;
			BusPacket* tempCmd = NULL;
			for(unsigned int index = 0; index < commandQueue.size(); index++)	// Schedule Real-Time Commands
			{
				if(!casBlocking) {
					if(commandQueue[indexRT]->getSize(true) > 0) 
					{
						tempCmd = commandQueue[indexRT]->getCommand(true);
						
						if(tempCmd->busPacketType < ACT) {
							if(isReady(tempCmd, indexRT)) {
								if(isIssuable(tempCmd)) {scheduledCommand = tempCmd;}
								else {casBlocking = true;}
							}
						}
						else {
							if(isReady(tempCmd, indexRT))
							{
								if(isIssuable(tempCmd)) 
								{
									scheduledCommand = tempCmd;
								}
							}	
						}
					}
				}
				else {
					tempCmd = commandQueue[indexRT]->getCommand(true);
					if(isIssuable(tempCmd)){
						scheduledCommand = tempCmd;
						casBlocking = false;
					}
				}
				// Preemption Generator
				if(tempCmd != NULL && commandQueue[indexRT]->getSize(false)> 0 && compensateCommands[index].empty()) 
				{	
					compensateGenerator(index);
				}
				// If index to next queue or blocked by CAS or skip for scheduled command
				if(casBlocking) {break;}
				else {
					indexRT++;
					if(indexRT == commandQueue.size()) {indexRT = 0;}
					if(scheduledCommand != NULL) {break;}
				}
			}
			// Schedule HP commands if no RT is schedulable
			bool compCommand = false;
			if(scheduledCommand == NULL && !casBlocking) {
				for(unsigned int index = 0; index < commandQueue.size(); index++)
				{
					if(!compensateCommands[indexHP].empty()) {
						tempCmd = compensateCommands[indexHP].front();
						compCommand = true;
					}
					else if(commandQueue[indexHP]->getSize(false) > 0) {
						tempCmd = commandQueue[indexHP]->getCommand(false);
						compCommand = false;
					}
					if(tempCmd == NULL){
					}
					if(tempCmd != NULL)
					{
						if(isIssuable(tempCmd))
						{
							scheduledCommand = tempCmd;
							if(compCommand) 
								{
									compensateCommands[indexHP].erase(compensateCommands[indexHP].begin());
									bypass = true;
								}
						}					
					indexHP++; // Round Robin Indexer
					if(indexHP == commandQueue.size()) {indexHP = 0;}
					if(scheduledCommand != NULL) {break;}
					}
				}					
			}
			if(scheduledCommand != NULL) 
			{			
				sendCommand(scheduledCommand, scheduledCommand->bank, bypass);
			}
			tempCmd = NULL;
			bypass = false;
			delete(tempCmd);
			return scheduledCommand;
		}
	};
}
#endif /* COMMANDSCHEDULER_MAG_H */