#ifndef COMMANDSCHEDULER_PIPECAS_H
#define COMMANDSCHEDULER_PIPECAS_H

#include "../../src/CommandScheduler.h"
#include <algorithm>    // std::max
#include <math.h>       /* ceil */
#include <iostream>

namespace MCsim
{
	class CommandScheduler_PIPECAS: public CommandScheduler
	{
	private:
		BusPacketType roundType;
		BusPacketType roundSRT;
		unsigned int srtN_ACT;
		unsigned int srtNum_ACT;
/********* HRT Buffer System **********/
		bool roundStart;
		BusPacketType prevCAS;
		// SRT CAS stop point
		unsigned int casCounter;
		// Snapshot cycle
		unsigned int actCounter;
		vector<unsigned int> fawCounter;

		// Buffer queue index that have commands before round starts
		vector<unsigned int> queueScheduled;
		vector<unsigned int> queueServed;
		vector<std::pair<BusPacket*, int>> hrtCAS;
		vector<std::pair<BusPacket*, int>> hrtACT;
		// if a command already in FIFO
		vector<bool> queuePending;		

/********* SRT Buffer System **********/
		vector<std::pair<BusPacket*, int>> srtCAS;
		vector<std::pair<BusPacket*, int>> srtACT;
		vector<std::pair<BusPacket*, int>> srtPRE;
		unsigned int srtRD;
		unsigned int srtWR;
		unsigned int scheduledIndex;
		void schedule_HRT_ACT() {
			if(!hrtACT.empty()) {
				if(isIssuable(hrtACT.front().first)) {
					scheduledCommand = hrtACT.front().first;
					scheduledIndex = hrtACT.front().second;
					hrtACT.erase(hrtACT.begin());
				}
			}	
		}
		void schedule_HRT_CAS() {
			if(!hrtCAS.empty()) {
				if(isIssuable(hrtCAS.front().first)) {
					scheduledCommand = hrtCAS.front().first;
					scheduledIndex = hrtCAS.front().second;
					hrtCAS.erase(hrtCAS.begin());
					// Update the queue is served when the CAS is issued
					queueServed[scheduledIndex] = true;
				}
			}
		}
		void schedule_SRT_ACT() {
			if(!srtACT.empty()) {
				if(isIssuable(srtACT.front().first)) {
					scheduledCommand = srtACT.front().first;
					scheduledIndex = srtACT.front().second;
					srtACT.erase(srtACT.begin());
				}
			}			
		}
		void schedule_SRT_CAS() {
			if(!srtCAS.empty()) {
				for(unsigned int index = 0; index < srtCAS.size(); index++) {
					if(isIssuable(srtCAS[index].first) && srtCAS[index].first->busPacketType == roundSRT) {
						scheduledCommand = srtCAS[index].first;
						scheduledIndex = srtCAS[index].second;
						srtCAS.erase(srtCAS.begin()+index);	
						break;
					}							
				}
			}			
		}
		void schedule_SRT_PRE() {
			if(!srtPRE.empty()) {
				if(isIssuable(srtPRE.front().first)) {
					scheduledCommand = srtPRE.front().first;
					scheduledIndex = srtPRE.front().second;
					srtPRE.erase(srtPRE.begin());
				}
			}			
		}
		unsigned int exeACT(unsigned int num) {
			return 5*(num-1) + ceil((num-1)/4)*(4);
		}
		unsigned int tCAS_SRT(bool read, unsigned int num, unsigned int srtNum) {
			if(read) 
				return max(max(0, int(15-4*(num-1))) + exeACT(num), exeACT(num+srtNum)) + 9 - ((4+1)*(num-1)+4);
			else 
				return max(max(0, int(21-4*(num-1))) + exeACT(num), exeACT(num+srtNum)) + 9 - ((4+1)*(num-1)+4);
		}
	public:
		CommandScheduler_PIPECAS(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable, unsigned int srtSlot):
			CommandScheduler(commandQueues, requestorTable)
		{
			srtN_ACT = 0;
			srtNum_ACT = 0;
			actCounter = 0;
			casCounter = 0;
			commandQueue[0]->setACT(0);
			roundType = BusPacketType::RDA;
			roundSRT = BusPacketType::RD;
			prevCAS = BusPacketType::WRA;
			for(unsigned int index = 0; index<commandQueue.size(); index++) {
				queueServed.push_back(false);
				queuePending.push_back(false);
			}
			roundStart = false;
		}
		~CommandScheduler_PIPECAS() // dtor
		{
			queueScheduled.clear();
			hrtCAS.clear();
			hrtACT.clear();
			srtCAS.clear();
			srtACT.clear();
			srtPRE.clear();
			queuePending.clear();
		}	
		BusPacket* commandSchedule()
		{
			scheduledCommand = NULL;
			checkCommand = NULL;
			scheduledIndex = 0;
			for(unsigned int index=0; index<commandQueue.size(); index++) { 			// Buffer SRT commands
				if(!queuePending[index] && commandQueue[index]->getSize(false) > 0) {
					checkCommand = commandQueue[index]->getCommand(false);
					if(isReady(checkCommand, index)){
						queuePending[index] = true;
						if(checkCommand->busPacketType == ACT) {
							srtACT.push_back(std::make_pair(checkCommand, index));
						}
						else if (checkCommand->busPacketType < ACT) {
							srtCAS.push_back(std::make_pair(checkCommand, index));
						}
						else {
							srtPRE.push_back(std::make_pair(checkCommand, index));
						}
					}
				}
			}
			roundType = prevCAS; // previous round is done, or executing outRound scheduler
			if(queueScheduled.empty()) {
				for(unsigned int index=0; index < commandQueue.size(); index++) {
					if(!queuePending[index] && commandQueue[index]->getSize(true) > 0) {
						roundStart = true;
						for(unsigned cmdIndex = 0; cmdIndex < commandQueue[index]->getSize(true); cmdIndex++) {
							checkCommand = commandQueue[index]->checkCommand(true, cmdIndex);
							if(checkCommand->busPacketType < ACT && checkCommand->busPacketType != prevCAS) {
								// Determine the Type
								roundType = checkCommand->busPacketType;
								break;
							}
						}
					}
				}
			}
			if(roundStart && queueScheduled.empty()) {
				srtRD = 0;
				srtWR = 0;
				if(roundType == RDA) {
					roundSRT = RD;
					srtNum_ACT = 0;
				}
				else {
					roundSRT = WR;
					srtNum_ACT = 0;
				}				
				if(actCounter == 0) {	// Snapshot
					for(unsigned int index=0; index < commandQueue.size(); index++) {
						if(!queuePending[index] && commandQueue[index]->getSize(true) > 0) {
							for(unsigned cmdIndex = 0; cmdIndex < commandQueue[index]->getSize(true); cmdIndex++) {
								checkCommand = commandQueue[index]->checkCommand(true, cmdIndex);
								if(checkCommand->busPacketType == roundType) {
									queueScheduled.push_back(index);
								}
							}
						}
					}
					if(roundType == RDA) {
						casCounter = tCAS_SRT(1, queueScheduled.size(), srtNum_ACT);
					}
					else {
						casCounter = tCAS_SRT(0, queueScheduled.size(), srtNum_ACT);
					}			
				}
				else {
					schedule_SRT_CAS();
					if(scheduledCommand == NULL) {
						schedule_SRT_PRE();
					}
				}
			}
			// There are hrt commands, scan through the scheduled queues
			if(!queueScheduled.empty()) {
				for(unsigned int index=0; index < queueScheduled.size(); index++){
					if(!queuePending[queueScheduled[index]] && !queueServed[queueScheduled[index]]) {
						checkCommand = commandQueue[queueScheduled[index]]->getCommand(true);
						if(isReady(checkCommand, queueScheduled[index])) {
							queuePending[queueScheduled[index]] = true;
							if(checkCommand->busPacketType == ACT) {
								hrtACT.push_back(make_pair(checkCommand, queueScheduled[index]));
							}
							else if(checkCommand->busPacketType < ACT) {
								hrtCAS.push_back(make_pair(checkCommand, queueScheduled[index]));
							}
						}												
					}
				}
				if(srtNum_ACT > 0) {// During SRT ACT slot
					if(actCounter == 0) {
						srtNum_ACT--;
						schedule_SRT_ACT();	
						if(scheduledCommand == NULL) { schedule_HRT_ACT();}
					}
				}
				else { schedule_HRT_ACT(); }	
				if(scheduledCommand == NULL) {	// Before reaching the SRT CAS Stop Point
					if(casCounter > 0) {
						schedule_SRT_CAS(); 
						if(scheduledCommand == NULL) { schedule_HRT_CAS(); }
					}
					else {
						schedule_HRT_CAS();
					}
				}				
				if(scheduledCommand == NULL) { // Executing PRE during HRT Round
					schedule_SRT_PRE();
				}				
				bool roundDone = true; // Check if the round is complete, is all the CAS commands are issued
				for(unsigned int index = 0; index < queueScheduled.size(); index++)	{
					if(queueServed[queueScheduled[index]] == false) {
						roundDone = false;
						break;
					}
				}
				if(roundDone) {
					roundStart = false;
					queueScheduled.clear();
					for(unsigned int index=0; index<queueServed.size(); index++) {
						queueServed[index]=false;
					}
				}				
			}		
			if(queueScheduled.empty() && roundStart == false && scheduledCommand == NULL) { 	// Not In a HRT Round
				if(srtRD == 10) {
					roundSRT = WR;
					srtRD = 0;
				}
				if(srtWR == 10) {
					roundSRT = RD;
					srtWR = 0;
				}
				schedule_SRT_CAS();
				if(scheduledCommand == NULL) 
					schedule_SRT_ACT();
				if(scheduledCommand == NULL) 
					schedule_SRT_PRE();
			}
			if(scheduledCommand != NULL) {
				sendCommand(scheduledCommand, scheduledIndex, false);
				queuePending[scheduledIndex] = false;
				if(scheduledCommand->busPacketType == ACT){
					actCounter = getTiming("tRRD");
					fawCounter.push_back(getTiming("tFAW"));
					if(fawCounter.size() == 4) {
						actCounter = max(actCounter, fawCounter.front());
					}
				}
				else if (scheduledCommand->busPacketType < ACT){
					if(scheduledCommand->busPacketType == RDA || scheduledCommand->busPacketType == RD) {
						prevCAS = RDA;
						if(scheduledCommand->busPacketType == RD) {
							if(roundStart == false) {
								srtRD++;
							}
						}
					}
					else {
						prevCAS = WRA;
						if(scheduledCommand->busPacketType == WR) {
							if(roundStart == false) {
								srtWR++;
							}
						}
					}
				}				
			}
			if(actCounter > 0) { actCounter--; }
			if(casCounter > 0) { casCounter--; }
			if(!fawCounter.empty()) {
				for(unsigned int index=0; index<fawCounter.size(); index++) { fawCounter[index]--;}
				if(fawCounter.front() == 0) { fawCounter.erase(fawCounter.begin()); }
			}
			return scheduledCommand;
		}
	};
}
#endif /* COMMANDSCHEDULER_PIPECAS_H */