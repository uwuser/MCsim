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
	refreshMachine = new RefreshMachine(commandQueue, ranks, banks, getTiming("tREFI"), getTiming("tRFC"));
}

unsigned int CommandScheduler::getTiming(const string& param) // Accsess the Ramulator backend to determine the timing specifications
{
	return memoryDevice->get_constraints(param);
}

bool CommandScheduler::refreshing() // Perform refresh
{
	return refreshMachine->refreshing();
}

void CommandScheduler::refresh() // Indicator of reaching refresh interval
{
	BusPacket* tempCmd = NULL;
	refreshMachine->refresh(tempCmd);
	if(tempCmd != NULL) {
		if(isIssuable(tempCmd)) {
			memoryDevice->receiveFromBus(tempCmd);
			refreshMachine->popCommand();
			
		}	
	}
	tempCmd = NULL;
	delete tempCmd;
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

bool CommandScheduler::isIssuable(BusPacket* cmd) // Check if the command is issueable on the channel
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
		// For the command trace option
		if(cmd->busPacketType == PRE)						
		{
			TRACE_CMD("TRACE-COMMAND:PRE"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == RD){
			TRACE_CMD("TRACE-COMMAND:RD"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);									
		}
		else if(cmd->busPacketType == WR){
			TRACE_CMD("TRACE-COMMAND:WR"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == ACT || cmd->busPacketType == ACT_R || cmd->busPacketType == ACT_W){
			TRACE_CMD("TRACE-COMMAND:ACT"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);							
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
		// For the command trace option
		if(cmd->busPacketType == PRE)						
		{
			TRACE_CMD("TRACE-COMMAND:PRE"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == RD){
			TRACE_CMD("TRACE-COMMAND:RD"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);											
		}
		else if(cmd->busPacketType == WR){
			TRACE_CMD("TRACE-COMMAND:WR"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);											
		}
		else if(cmd->busPacketType == ACT || cmd->busPacketType == ACT_R || cmd->busPacketType == ACT_W){
			TRACE_CMD("TRACE-COMMAND:ACT"<<"\t"<<clock<<":"<<"\t\tAddress: "<<cmd->address<<"\t\tBank: "<<cmd->bank<<"\t\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);								
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
