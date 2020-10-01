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

#ifndef _REQUESTSCHEDULER_H
#define _REQUESTSCHEDULER_H

#include "Request.h"
#include "BusPacket.h"
#include "RequestQueue.h"
#include "CommandQueue.h"
#include <map>
#include <utility>
#include <queue>

using namespace MCsim;

namespace MCsim
{
	class CommandGenerator;
	class RequestScheduler
	{
	public:
		RequestScheduler(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable);
		virtual ~RequestScheduler();
		void connectCommandGenerator(CommandGenerator *generator);
		// Used to implement the request scheduling algorithm
		virtual void requestSchedule() = 0;
		void flushWriteReq(bool sw);
		bool writeMode();
		void step();
		
	protected:
		unsigned long clockCycle;
		// Table to track for criticality for scheduling (Copy of global tables)
		const std::map<unsigned int, bool>& requestorCriticalTable;
		void set_deadline();
		// Connect to Request Queue and request buffer
		const std::vector<RequestQueue*>& requestQueue;
		const std::vector<CommandQueue*>& commandQueue;
		// Connect to Command Generator
		CommandGenerator *commandGenerator;
		// Checking for commandQueue availability
		virtual bool isSchedulable(Request* request, bool open);
		// Find the First-Ready command [FCFS]
		Request* scheduleF(unsigned int qIndex);
		Request* scheduleFR_BACKEND(unsigned int qIndex, unsigned int reqIndex);
		Request* scheduleFR_Next(unsigned int qIndex);
		Request* scheduleBLISS(unsigned int qIndex);
		// Open Row Checker
		std::map< unsigned int, std::map< unsigned int, unsigned int long > > bankTable;
		// Update open row
		void updateRowTable(unsigned rank, unsigned bank, unsigned row);
		bool isRowHit(Request* request);
		bool writeEnable(int qIndex);
		bool serviceWrite(int qIndex);
		
		unsigned int bufferSize(unsigned int qIndex);
		bool switch_enable;
		Request* scheduledRequest;
		Request* scheduledRequest_HP;
		Request* scheduledRequest_RT;
		Request* scheduledRequest_temp;
		uint index_temp;
		Request* checkRequest;
		Request* req1;
		Request* req2;
		bool sw;
		bool FR_open;
		unsigned int id;
		int blacklist [36];
		bool flag;
	};
}

#endif /* _REQUESTSCHEDULER_H */
