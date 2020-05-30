#ifndef _COMMANDSCHEDULER_H
#define _COMMANDSCHEDULER_H

#include "BusPacket.h"
#include "CommandQueue.h"
#include "RefreshMachine.h"
#include <iostream>
#include <fstream>
#include <map>
#include <utility>
#include <vector>

namespace MCsim
{
	class MemoryDevice;
	class CommandScheduler
	{
	public:
		CommandScheduler(vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable);
		virtual ~CommandScheduler();
		void connectMemoryDevice(MemoryDevice* memDev);
		// Scheduling Algoirthm
		virtual BusPacket* commandSchedule() = 0;
		void commandClear();
		// Indicator of reaching refresh interval
		void refresh();
		// Perform refresh
		bool refreshing();
		// next clk cycle
		void tick();
		
	protected:
		
		MemoryDevice* memoryDevice;
		RefreshMachine* refreshMachine;
		
		std::vector<CommandQueue*>& commandQueue;
		std::map<unsigned int, std::map<unsigned int, unsigned int> > cmdQueueTimer;
		std::vector<std::map<unsigned int, std::map<unsigned int, unsigned int>> > reqCmdQueueTimer;

		const std::map<unsigned int, bool>& requestorCriticalTable;	

		// Get timing constraint from memory system interface
		unsigned int getTiming(const string& param);
		// Check if a cmd satisfy the timing constraint on its own queue
		bool isReady(BusPacket* cmd, unsigned int index);
		// Check if a cmd can be issued
		bool isIssuable(BusPacket* cmd);
		// Issue the cmd
		void sendCommand(BusPacket* cmd, unsigned int index, bool bypass);
		void init_config();
		void clean_buffers();

		BusPacket* scheduledCommand;
		BusPacket* checkCommand;
		BusPacket* checkCommand_temp_1;
		BusPacket* checkCommand_temp_2;
		void send_precedure(MCsim::BusPacket *checkcommand, bool PRE, int RR );

		ofstream fout;

		bool skipCAS;
		unsigned int ranks, banks;
		unsigned index_temp;
		unsigned long clock;

	};
}

#endif /* _COMMANDSCHEDULER_H */
