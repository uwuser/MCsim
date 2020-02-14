#ifndef COMMANDSCHEDULER_ROC_H
#define COMMANDSCHEDULER_ROC_H

#include <algorithm>
#include "../../src/CommandScheduler.h"

namespace MCsim
{
	class CommandScheduler_ROC: public CommandScheduler
	{
	private:
		vector< vector<BusPacket*> > rankFIFO_L3PA;
		vector< vector<BusPacket*> > rankFIFO_L3CAS;
		vector<BusPacket*> l2_PA;
		unsigned int l2_PAindex;
		vector<BusPacket*> l2_CAS;
		vector<unsigned long long> l2_CAStimer;
		unsigned int l2_CASindex;
		vector<bool> queuePending;		// indicate if already in FIFO
		unsigned long long prevEnd;
		unsigned int prevCmd;
		unsigned int prevRank;
		unsigned int tCCD;
		unsigned int tRTR;
		unsigned int readLatency;
		unsigned int writeLatency;
		unsigned int readToWrite;
		unsigned int writeToRead;
		unsigned int ranks;

	public:
		CommandScheduler_ROC(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable)
		{
			prevEnd = 0;
			prevCmd = RD;
			prevRank = 0;
			ranks = 4;
			commandQueue[0]->setACT(0);
		}
		BusPacket* commandSchedule()
		{
			// Initialize the FIFO system
			if(clock == 1) {
				tCCD = getTiming("tCCD");
				tRTR = getTiming("tRTR");
				readLatency = getTiming("tRL");// + getTiming("tBus");
				writeLatency = getTiming("tWL");// + getTiming("tBus");
				readToWrite = getTiming("tRTW") + writeLatency;
				writeToRead = writeLatency + getTiming("tBus") + getTiming("tWTR") + readLatency;

				for(unsigned rank=0; rank < ranks; rank++) {
					rankFIFO_L3PA.push_back(vector<BusPacket*>());
					rankFIFO_L3CAS.push_back(vector<BusPacket*>());

					l2_PA.push_back(NULL);
					l2_PAindex = 0;
					l2_CAS.push_back(NULL);
					l2_CAStimer.push_back(0);
					l2_CASindex = 0;
				}
				for(unsigned req = 0; req < requestorCriticalTable.size(); req++) {				
					queuePending.push_back(false);
				}
			}
			// Scheduling Alogrithm
			scheduledCommand = NULL;
			// Level 3 Arbitration
			checkCommand = NULL;
			for(unsigned int index = 0; index < commandQueue.size(); index++) {
				if(!queuePending[index]){
					checkCommand = NULL;

					if(commandQueue[index]->getRequestorIndex() > 0) {
						for(unsigned int req=0; req < commandQueue[index]->getRequestorIndex(); req++) {
							if(commandQueue[index]->getRequestorSize(req) > 0) {
								checkCommand = commandQueue[index]->getRequestorCommand(req);

								if(!queuePending[checkCommand->requestorID]) {
									if(isReady(checkCommand, index)) {
										if(checkCommand->busPacketType < ACT) {
											rankFIFO_L3CAS[checkCommand->rank].push_back(checkCommand);
										}
										else {
											rankFIFO_L3PA[checkCommand->rank].push_back(checkCommand);
										}
										queuePending[checkCommand->requestorID] = true;								
									}
								}
							}
						}
					}
				}
			}
			// Level 2 Arbitration
			checkCommand = NULL;
			for(unsigned int rank=0; rank < ranks; rank++) {
				if(!rankFIFO_L3PA[rank].empty() && l2_PA[rank] == NULL) {
					for(unsigned int index = 0; index < rankFIFO_L3PA[rank].size(); index++) {
						if(isIssuable(rankFIFO_L3PA[rank][index])) {
							l2_PA[rank] = rankFIFO_L3PA[rank][index];
							rankFIFO_L3PA[rank].erase(rankFIFO_L3PA[rank].begin()+index);
							break;
						}							
					}
				}
				if(!rankFIFO_L3CAS[rank].empty() && l2_CAS[rank] == NULL) {
					l2_CAS[rank] = rankFIFO_L3CAS[rank].front();
					rankFIFO_L3CAS[rank].erase(rankFIFO_L3CAS[rank].begin());
				}
			}
			// Level 1 Arbitration
			checkCommand = NULL;
			unsigned scheduledIndex = 0;
			for(unsigned int index=0; index < l2_CAStimer.size(); index++) {
				if(l2_CAS[l2_CASindex] != NULL && l2_CAStimer[l2_CASindex] <= prevEnd + tRTR) {
					scheduledIndex = l2_CASindex;
					checkCommand = l2_CAS[l2_CASindex];	
					if(isIssuable(checkCommand)) {
						scheduledCommand = checkCommand;
					}
					else {
						break;
					}
					// break;
				}
				l2_CASindex++;
				if(l2_CASindex == l2_CAS.size()) {
					l2_CASindex = 0;
				}
				if(scheduledCommand != NULL) {
					break;
				}
			}
			if(checkCommand == NULL) {
				unsigned long long smallSD = l2_CAStimer.front();
				for(unsigned int index = 0; index < l2_CAStimer.size(); index++) {
					if(l2_CAS[index] != NULL && l2_CAStimer[index] < smallSD) {
						smallSD = l2_CAStimer[index];
						scheduledIndex = index;
					}
				}
				if(l2_CAS[scheduledIndex] != NULL) {
					checkCommand = l2_CAS[scheduledIndex];	
					if(isIssuable(checkCommand)) {
						scheduledCommand = checkCommand;
					}
				}				
			}		

			if(scheduledCommand != NULL) {
				// scheduledCommand = checkCommand;
				prevCmd = scheduledCommand->busPacketType;
				prevRank = 0 ;//scheduledCommand->rank;
				if(prevCmd == RD) { 
					prevEnd = clock + readLatency + tCCD; 
				}
				else { 
					prevEnd = clock + writeLatency + tCCD; 
				}
				for(unsigned int index=0; index < l2_CAS.size(); index++) {
					// check other l2CAS
					if(index != scheduledIndex) {
						l2_CAStimer[index] = prevEnd + tRTR;
					}
				}
				l2_CAS[scheduledIndex] = NULL;	
			}
			if(scheduledCommand == NULL) {
				for(unsigned int index = 0; index < l2_PA.size(); index++) {
					if(l2_PA[l2_PAindex]!= NULL) {
						if(isIssuable(l2_PA[l2_PAindex])) {
							scheduledCommand = l2_PA[l2_PAindex];
							l2_PA[l2_PAindex] = NULL;
						}						
					}
					l2_PAindex++;
					if(l2_PAindex == l2_PA.size()) {
						l2_PAindex = 0;
					}
					if(scheduledCommand != NULL) {
						break;
					}
				}
			}
			if(scheduledCommand != NULL) {
				sendCommand(scheduledCommand, 0, false);
				queuePending[scheduledCommand->requestorID] = false;
			}
			return scheduledCommand;
		}
	};
}
#endif /* COMMANDSCHEDULER_ROC_H */
