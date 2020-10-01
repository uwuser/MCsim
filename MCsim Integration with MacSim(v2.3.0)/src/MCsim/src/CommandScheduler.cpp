/*
MIT License

Copyright (c) 2020 University of Waterloo, Reza Mirosanlou @rmirosan@uwaterloo.ca

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <algorithm>
#include "CommandScheduler.h"
#include "MemoryDevice.h"
#include "global.h"
using namespace std;

using namespace MCsim;

CommandScheduler::CommandScheduler(
	vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
commandQueue(commandQueues),
requestorCriticalTable(requestorTable)
{
	// If the command queue is used as per memory level
	cmdQueueTimer = std::map<unsigned int, std::map<unsigned int, unsigned int> >();
	// If the command queue is used as per requestor queue
	for(unsigned int index=0; index<commandQueue.size(); index++) {
		reqCmdQueueTimer.push_back(std::map<unsigned int, std::map<unsigned int, unsigned int> >());
	}

	memoryDevice = NULL;
	refreshMachine = NULL;
	scheduledCommand = NULL;
	checkCommand = NULL;
	clock = 1;
	ranks = 0;
	banks = 0;
}
CommandScheduler::~CommandScheduler()
{
	cmdQueueTimer.clear();	
}
void CommandScheduler::connectMemoryDevice(MemoryDevice* memDev)
{
	memoryDevice = memDev;
	ranks = memoryDevice->get_Rank();
	banks = memoryDevice->get_Bank();
	refreshMachine = new RefreshMachine(commandQueue, ranks, banks, getTiming("tREFI"), getTiming("tRFCpb"), getTiming("tRFCab"), getTiming("tRFC"),getTiming("tRCD"));
}

unsigned int CommandScheduler::getTiming(const string& param) // Accsess the Ramulator backend to determine the timing specifications
{
	return memoryDevice->get_constraints(param);
}

bool CommandScheduler::refreshing() // Perform refresh
{
	return refreshMachine->refreshing();
}
void CommandScheduler::updateRowTable_cmd(unsigned rank, unsigned bank, unsigned row)
{
	bankTable_cmd[rank][bank] = row;
}
bool CommandScheduler::isRowHit_cmd(BusPacket* cmd)
{
	
	bool isHit = false;
	if(bankTable_cmd.find(cmd->rank) != bankTable_cmd.end()) {
		if(bankTable_cmd[cmd->rank].find(cmd->bank) != bankTable_cmd[cmd->rank].end()) {
			if(bankTable_cmd[cmd->rank][cmd->bank] == cmd->row){ 
				isHit = true; 
			}
		}	
	}	
	return isHit;
}
void CommandScheduler::refresh() // Indicator of reaching refresh interval
{
	BusPacket* tempCmd = NULL;
	if(refreshMachine->reachInterval())
	{
		refreshMachine->refresh(tempCmd);
		if(tempCmd != NULL) {
			if(isIssuableRefresh(tempCmd)) {
				if(tempCmd->busPacketType == PRE)						
				{
					TRACE_CMD("TRACE-COMMAND:PRE"<<"\t"<<clock<<":"<<"\t\tAddress: "<<tempCmd->address<<"\t\tBank: "<<tempCmd->bank<<"\t\tColumn: "<<tempCmd->column<<"\tRow: "<<tempCmd->row);										
				}
				else if(tempCmd->busPacketType == REF)
				{
					TRACE_CMD("TRACE-COMMAND:REF"<<"\t"<<clock<<":"<<"\t\tAddress: "<<tempCmd->address<<"\t\tBank: "<<tempCmd->bank<<"\t\tColumn: "<<tempCmd->column<<"\tRow: "<<tempCmd->row);	
				}
				else if(tempCmd->busPacketType == REFPB)
				{				
					TRACE_CMD("TRACE-COMMAND:REFPB"<<"\t"<<clock<<":"<<"\t\tAddress: "<<tempCmd->address<<"\t\tBank: "<<tempCmd->bank<<"\t\tColumn: "<<tempCmd->column<<"\tRow: "<<tempCmd->row);	
				}
				else if(tempCmd->busPacketType == PREA)
				{
				 	TRACE_CMD("TRACE-COMMAND:PREA"<<"\t"<<clock<<":"<<"\t\tAddress: "<<tempCmd->address<<"\t\tBank: "<<tempCmd->bank<<"\t\tColumn: "<<tempCmd->column<<"\tRow: "<<tempCmd->row);	
				}	
				else if(tempCmd->busPacketType == ACT)
				{
					if(refreshMachine->busyBank(tempCmd->bank) == false)
				 		TRACE_CMD("TRACE-COMMAND:ACT"<<"\t"<<clock<<":"<<"\t\tAddress: "<<tempCmd->address<<"\t\tBank: "<<tempCmd->bank<<"\t\tColumn: "<<tempCmd->column<<"\tRow: "<<tempCmd->row);	
				}					
				memoryDevice->receiveFromBus(tempCmd);
				refreshMachine->popCommand();				
			}	
		}
	tempCmd = NULL;
	delete tempCmd;
	}	
}
bool CommandScheduler::isReady(BusPacket* cmd, unsigned int index) // Each command queue share the same command ready time
{
	if(commandQueue[index]->isPerRequestor()) {
		if(reqCmdQueueTimer[index].find(cmd->requestorID) == reqCmdQueueTimer[index].end()) {
			reqCmdQueueTimer[index][cmd->requestorID] = map<unsigned int, unsigned int>();
		}
		if(reqCmdQueueTimer[index][cmd->requestorID].find(cmd->busPacketType) != reqCmdQueueTimer[index][cmd->requestorID].end())
		{
			if(reqCmdQueueTimer[index][cmd->requestorID][cmd->busPacketType] != 0)
			{
				return false;
			}
		}
	}
	else 
	{
		if(cmdQueueTimer.find(index) == cmdQueueTimer.end()) {
			cmdQueueTimer[index] = map<unsigned int, unsigned int>();
		}
		if(cmdQueueTimer[index].find(cmd->busPacketType) != cmdQueueTimer[index].end()) {
			if(cmdQueueTimer[index][cmd->busPacketType] != 0){	
				return false;
			}
		}		
	}
	return true;
}
unsigned int CommandScheduler::isReadyTimer(BusPacket* cmd, unsigned int index) 
{
	if(cmdQueueTimer.find(index) == cmdQueueTimer.end()) {
		cmdQueueTimer[index] = map<unsigned int, unsigned int>();
	}
	if(cmdQueueTimer[index].find(cmd->busPacketType) != cmdQueueTimer[index].end()) {
			return cmdQueueTimer[index][cmd->busPacketType];		
	}
	return 0;			
}
bool CommandScheduler::isIssuable(BusPacket* cmd) // Check if the command is issueable on the channel
{
	return memoryDevice->command_check(cmd);
}
bool CommandScheduler::isIssuableRefresh(BusPacket* cmd) 
{		
	return memoryDevice->command_check(cmd);
}
void CommandScheduler::sendCommand(BusPacket* cmd, unsigned int index, bool bypass) // Send the actual command to the device
{
	// Update command counter
	if(commandQueue[index]->isPerRequestor()) {
		for(unsigned int type = RD; type != PDE; type++) {
			reqCmdQueueTimer[index][cmd->requestorID][type] = std::max(reqCmdQueueTimer[index][cmd->requestorID][type], 
			memoryDevice->command_timing(cmd, static_cast<BusPacketType>(type)));
		}
		commandQueue[index]->removeCommand(cmd->requestorID);
	}
	else {
		if(!bypass)
		{
			for(unsigned int type = RD; type != PDE; type++) {
				cmdQueueTimer[index][type] = std::max(cmdQueueTimer[index][type], 
					memoryDevice->command_timing(cmd, static_cast<BusPacketType>(type)));
			}		
			commandQueue[index]->removeCommand();	
		}
		else
		{
			for(unsigned int type = RD; type != PDE; type++) {
				cmdQueueTimer[index][type] = std::max(cmdQueueTimer[index][type], 
					memoryDevice->command_timing(cmd, static_cast<BusPacketType>(type)));
			}
		}	
	}
	memoryDevice->receiveFromBus(scheduledCommand);
}
void CommandScheduler::commandClear()
{
	delete scheduledCommand;
	scheduledCommand = NULL;
}

void CommandScheduler::tick()
{
	// Countdown command ready timer
	for(unsigned int index = 0; index < commandQueue.size(); index++) {
		if(commandQueue[index]->isPerRequestor()) {
			for(auto &requestor : reqCmdQueueTimer[index]) {
				for(auto &counter : reqCmdQueueTimer[index][requestor.first]) {
					if(counter.second > 0) {
						counter.second--;
					}
				}
			}
		}
		else
		{
			if(!cmdQueueTimer[index].empty()) {
				for(auto &counter : cmdQueueTimer[index]) {
					if(counter.second > 0) {
						counter.second--;
					}
				}
			}			
		}
	}
	refreshMachine->step();
	clock++;
}
