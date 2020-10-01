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
		std::map< unsigned int, std::map< unsigned int, unsigned int long > > bankTable_cmd;
		const std::map<unsigned int, bool>& requestorCriticalTable;	
		void updateRowTable_cmd(unsigned rank, unsigned bank, unsigned row);
		bool isRowHit_cmd(BusPacket* cmd);
		// Get timing constraint from memory system interface
		unsigned int getTiming(const string& param);
		// Check if a cmd satisfy the timing constraint on its own queue
		bool isReady(BusPacket* cmd, unsigned int index);
		// Check if a cmd can be issued
		bool isIssuable(BusPacket* cmd);
		// Issue the cmd
		bool isIssuableRefresh(BusPacket* cmd); 
		unsigned int isReadyTimer(BusPacket* cmd, unsigned int index);
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
