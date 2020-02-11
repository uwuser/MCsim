#include <iostream>
#include <algorithm>
#include "CommandScheduler.h"
#include "MemorySystem.h"
#include "global.h"

// using namespace std;
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

	memorySystem = NULL;
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
	// delete refreshMachine;
}
void CommandScheduler::connectMemorySystem(MemorySystem* memSys)
{
	memorySystem = memSys;
	ranks = memorySystem->get_Rank();
	banks = memorySystem->get_Bank();
	refreshMachine = new RefreshMachine(commandQueue, ranks, banks, getTiming("tREFI"), getTiming("tRFC"));
}
// Accsess the Ramulator backend to determine the timing specifications
unsigned int CommandScheduler::getTiming(const string& param)
{
	return memorySystem->get_constraints(param);
}
// Perform refresh
bool CommandScheduler::refreshing()
{
	return refreshMachine->refreshing();
}
// Indicator of reaching refresh interval
void CommandScheduler::refresh()
{
	BusPacket* tempCmd = NULL;
	refreshMachine->refresh(tempCmd);
	if(tempCmd != NULL) {
		if(isIssuable(tempCmd)) {
			memorySystem->receiveFromBus(tempCmd);
			refreshMachine->popCommand();
			
		}
		else
		{
		}		
	}
	tempCmd = NULL;
	delete tempCmd;
}
// Each command queue share the same command ready time
bool CommandScheduler::isReady(BusPacket* cmd, unsigned int index)
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
// Check if the command is issueable on the channel
bool CommandScheduler::isIssuable(BusPacket* cmd)
{
	return memorySystem->command_check(cmd);
}
// Send the actual command to the device
void CommandScheduler::sendCommand(BusPacket* cmd, unsigned int index, bool bypass)
{
	// Update command counter
	if(commandQueue[index]->isPerRequestor()) {
		for(unsigned int type = RD; type != PDE; type++) {
			reqCmdQueueTimer[index][cmd->requestorID][type] = std::max(reqCmdQueueTimer[index][cmd->requestorID][type], 
			memorySystem->command_timing(cmd, static_cast<BusPacketType>(type)));
		}
		// For the command trace option
		if(cmd->busPacketType == PRE)						
		{
			TRACE_CMD("TRACE-COMMAND:     CMD: PRE"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == RD){
			TRACE_CMD("TRACE-COMMAND:     CMD: RD"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);									
		}
		else if(cmd->busPacketType == WR){
			TRACE_CMD("TRACE-COMMAND:     CMD: WR"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == ACT || cmd->busPacketType == ACT_R || cmd->busPacketType == ACT_W){
			TRACE_CMD("TRACE-COMMAND:     CMD: ACT"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);							
		}	
		commandQueue[index]->removeCommand(cmd->requestorID);
	}
	else {
		if(!bypass)
		{
			for(unsigned int type = RD; type != PDE; type++) {
				cmdQueueTimer[index][type] = std::max(cmdQueueTimer[index][type], 
					memorySystem->command_timing(cmd, static_cast<BusPacketType>(type)));
			}		
			commandQueue[index]->removeCommand();	
		}
		else
		{
			for(unsigned int type = RD; type != PDE; type++) {
				cmdQueueTimer[index][type] = std::max(cmdQueueTimer[index][type], 
					memorySystem->command_timing(cmd, static_cast<BusPacketType>(type)));
			}
		}
		// For the command trace option
		if(cmd->busPacketType == PRE)						
		{
			TRACE_CMD("TRACE-COMMAND:     CMD: PRE"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);										
		}
		else if(cmd->busPacketType == RD){
			TRACE_CMD("TRACE-COMMAND:     CMD: RD"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);											
		}
		else if(cmd->busPacketType == WR){
			TRACE_CMD("TRACE-COMMAND:     CMD: WR"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);											
		}
		else if(cmd->busPacketType == ACT || cmd->busPacketType == ACT_R || cmd->busPacketType == ACT_W){
			TRACE_CMD("TRACE-COMMAND:     CMD: ACT"<<"\t\t"<<clock<<":"<<"\tAddress: "<<cmd->address<<"\tBank: "<<cmd->bank<<"\tColumn: "<<cmd->column<<"\tRow: "<<cmd->row);								
		}	
	}
	memorySystem->receiveFromBus(scheduledCommand);
}
void CommandScheduler::commandClear()
{
	delete scheduledCommand;
	scheduledCommand = NULL;
}
// Tick the scheduler
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



