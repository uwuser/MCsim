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
		void step();
		
	protected:
		unsigned long clockCycle;
		// Table to track for criticality for scheduling (Copy of global tables)
		const std::map<unsigned int, bool>& requestorCriticalTable;
		// Connect to Request Queue and request buffer
		const std::vector<RequestQueue*>& requestQueue;
		const std::vector<CommandQueue*>& commandQueue;
		// Connect to Command Generator
		CommandGenerator *commandGenerator;
		// Checking for commandQueue availability
		virtual bool isSchedulable(Request* request, bool open);
		// Find the First-Ready command [FCFS]
		Request* scheduleFR(unsigned int qIndex);
		Request* scheduleF(unsigned int qIndex);
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
		Request* scheduledRequest_temp;
		uint index_temp;
		Request* checkRequest;
		Request* req1;
		Request* req2;
		bool FR_open;
		unsigned int id;
		int blacklist [36];
		bool flag;
	};
}

#endif /* _REQUESTSCHEDULER_H */
