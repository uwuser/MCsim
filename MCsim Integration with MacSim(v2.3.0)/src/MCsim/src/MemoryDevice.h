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

#ifndef MEMORYDEVICE_H
#define MEMORYDEVICE_H

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <fstream>
#include "SimulatorObject.h"
#include "BusPacket.h"

namespace MCsim
{
	class MemoryController;
	class MemoryDevice
	{
	public:
		MemoryDevice(unsigned int ranks);
		virtual ~MemoryDevice();
		void connectMemoryController(MemoryController* memCtlr);


		void update();
		unsigned int get_DataBus();
		unsigned int get_Rank();
		unsigned int get_BankGroup();
		unsigned int get_Bank();
		unsigned long get_Row();
		unsigned long get_Column();

		virtual float get_constraints(const std::string& parameter) = 0;
		virtual long command_timing(BusPacket* command) = 0;
		virtual unsigned int command_timing(BusPacket* command, int type) = 0;
		virtual bool command_check(BusPacket* command) = 0;
		virtual void receiveFromBus(BusPacket* busPacket) = 0;

	protected:
		unsigned int clockCycle;
		unsigned int ranks;
		unsigned int bankGroups;
		unsigned int banks;
		unsigned int subArrays;
		unsigned long rows;
		unsigned long columns;

		unsigned int dataBusWidth;
		MemoryController* memoryController;

		std::vector<unsigned int> dataCycles;
		std::vector<BusPacket*> pendingReadData;
		
		typedef std::map<unsigned int, std::map<unsigned int, long> > dataArray;
		std::vector< std::vector< dataArray > > memoryArray;

		std::vector<BusPacket*> postBuffer;
		std::vector<int> postCounter;
		
		void* readDataArray(BusPacket* load);
		void updateDataArray(BusPacket* store);

		void generateData(BusPacket* cmd);

		std::ofstream commandTrace;
	};
}

#endif
