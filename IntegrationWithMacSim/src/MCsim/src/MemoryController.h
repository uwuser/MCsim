#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H


#include "SimulatorObject.h"

#include <stdint.h>
#include <cstdlib>
#include <string>
#include <map>
#include <functional>
#include "global.h"
#include "Request.h"
#include "BusPacket.h"
#include "RequestQueue.h"
#include "CommandQueue.h"
#include "SchedulerRegister.h"
#include "AddressMapping.h"
#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include "CommandScheduler.h"
//#include "RefreshMachine.h"

namespace MCsim
{
	class MemoryDevice;
	class MemorySystem;
	class MemoryController
	{
	public:
		// systemConfigFile includes info on memory channel and memory controller architecture
		MemoryController(const string& systemConfigFile, function<void(Request&)> callback);
		MemoryController(MemorySystem *ms, const string& systemConfigFile);
		MemoryController(MemorySystem *ms, const string& systemConfigFile, function<void(Request&)> callback);
		virtual ~MemoryController();
		void displayConfiguration();
		void setRequestor(unsigned int id, bool criticality);
		void connectMemoryDevice(MemoryDevice* memDev);
		//void connectMemorySystem(MemorySystem* memSys);
		bool addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size);
		void receiveData(BusPacket *bpacket);
		void flushWrite(bool sw);
		void update();
		void returnReadData(const Request *req);
		void writeDataDone(const Request *req);
		//void step();

		void printResult();
		void trackError(int id);
		
	private:
		// General Information
		unsigned long clockCycle;
		// Information on memory structure
		unsigned int memTable[Column+1];
		// Return served requests back to requestors
		function<void(Request&)> callback; 
		// Number of pins connect to the memory channel
		unsigned int dataBusWidth;

		bool enqueueCommand(BusPacket* command);

		// Bus Traffic
		Request* incomingRequest;
		BusPacket* outgoingData;
		BusPacket* outgoingCmd;
		void sendData(BusPacket* bpacket);

		// Memory Controller Configuration (scheduler name and memory version)
		std::map<std::string, std::string> configTable; 
		std::map<unsigned int, bool> requestorCriticalTable;
		//MemorySystem* memorySystem;
		MemorySystem *parentMemorySystem;
		MemoryDevice* memoryDevice;


		// Hardware Components
		SchedulerRegister* schedulerRegister;
		AddressMapping* addressMapping;
		RequestScheduler* requestScheduler;
		CommandGenerator* commandGenerator;
		CommandScheduler* commandScheduler;

		// Request and Command Queues
		vector<RequestQueue*> requestQueue;
		vector<unsigned int> reqMap;
		bool writeQueueEnable;
		bool requestorReqQ;
		vector<CommandQueue*> commandQueue;
		vector<unsigned int> cmdMap;
		bool requestorCmdQ;
						
		// Request Buffer
		deque< Request* > pendingReadRequest;
		deque< Request* > pendingWriteRequest;

		// Data Buffer
		vector< unsigned > sendDataCounter;
		vector< BusPacket* > sendDataBuffer;

		// Read Value from Configuration File
		void readConfigFile(const string& filename);
		void SetKey(string key, string valueString);
		// Queue Structure
		bool isNumeric(const std::string& input);
		unsigned int convertNumber(const std::string& input);
		void setQueue(const std::string& queueOrg, vector<unsigned int>& queueMap);
		unsigned int queueNumber(vector<unsigned int>& queueMap);
		unsigned int decodeQueue(const unsigned int (&addressMap)[4], vector<unsigned int>& queueMap);
		
		// Statistics
		struct Statistics{
			unsigned long long totalRequest;
			unsigned long long openRead;
			unsigned long long closeRead;
			unsigned long long openWrite;
			unsigned long long closeWrite;
			unsigned long long readBytes;
			unsigned long long writeBytes;
			unsigned long long open;		
			unsigned long long close;		
			unsigned long long open_counter;
			unsigned long long close_counter;
			unsigned long long openRead_Latency;
			unsigned long long openWrite_Latency;
			unsigned long long closeRead_Latency;
			unsigned long long closeWrite_Latency;
			bool closeRequest;
		} stats;
		ofstream myTrace;
	};
}

#endif /* MEMORYCONTROLLER_H */
