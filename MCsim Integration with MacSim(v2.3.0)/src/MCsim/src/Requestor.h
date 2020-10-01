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

#ifndef REQUESTOR_H
#define REQUESTOR_H

#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include "Requestor.h"
#include "MemoryController.h"
#include "MultiChannelMemorySystem.h"
#include "RequestQueue.h"
#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include "Request.h"

using namespace std;

namespace MCsim
{
	class MemoryController;
	class CommandGenerator;
	class MultiChannelMemorySystem;
	class CommandGenerator;
	class RequestQueue;

	class Cache
	{
	public:
		void add(unsigned long address, unsigned int size, void* data)
		{
			cache = std::make_pair(address, data);
			lowAddr = address;
			highAddr = address + size;
		}
		bool find(unsigned long address)
		{
			// if(address > lowAddr && address < highAddr) {return true;}
			// else {return false;}
			return false;
		}
	private:
		unsigned long lowAddr;
		unsigned long highAddr;
		std::pair<unsigned long, void*> cache;
	};

	class Requestor
	{
	public:
		Requestor(int id, bool inOrder, const string& traceFile);
		virtual ~Requestor();

		void connectMemorySystem(MultiChannelMemorySystem* memSys);
		void setMemoryClock(float clk);
		void sendRequest(Request* request);
		void returnData(Request* returnData);
		bool sim_end();
		bool bypass_read;
		void update();

		void printResult();
		long int long_l;
		float memoryClock;
		unsigned RequestSize;

	private:
		unsigned requestorID;
		unsigned prevArrive;
		unsigned prevComplete;
		unsigned currentClockCycle;
		unsigned int RequestBufferSize;
		unsigned int hitRatioCtr;
		unsigned long requestRequest;
		unsigned long completeRequest;
		unsigned long latency;
		unsigned long wcLatency;
		unsigned long compTime;
		unsigned long rowHitAddr;
		unsigned long rowMissAddr;
		unsigned long currAddr;
		bool inOrder;
		MemoryController* memoryController;
		RequestScheduler* requestSCHEDULER;
		MultiChannelMemorySystem* memorySystem;
		Cache* localCache;

		ifstream transFile;

		bool waitingRequest;
		bool readDone;
		ofstream outTrace;

		Request* pendingRequest;
		std::vector< Request* > corePendingData;
			
		bool sim_done;
		bool readingTraceFile();
		void parseTraceFileLine(string &line, uint64_t &addr, enum RequestType &requestType, 
			uint64_t &compDelay, uint64_t &clockCycle);
	};
}

#endif




