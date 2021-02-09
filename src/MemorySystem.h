

#ifndef MEMORYSYSTEM_H
#define MEMORYSYSTEM_H

#include "SimulatorObject.h"
#include "MemoryController.h"
#include "Callback.h"
#include <deque>
#include "Request.h"
#include "Ramulator_DDR3.h"
#include "Ramulator_DDR4.h"
#include "Ramulator_DSARP.h"

namespace MCsim
{
	typedef CallbackBase<void, unsigned, uint64_t, uint64_t> Callback_t;
	class MemorySystem : public SimulatorObject
	{

	public:
		//functions
		MemorySystem(unsigned int numRequestors, unsigned id, const string &systemIniFilename, const string &deviceGene, const string &deviceSpeed, const string &deviceSize, unsigned int ranks);
		MemorySystem(unsigned int numRequestors, unsigned id, const string &systemIniFilename, const string &deviceGene, const string &deviceSpeed, const string &deviceSize, unsigned int ranks, function<void(Request &)> callback);
		virtual ~MemorySystem();
		void update();
		bool addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size);
		void flushWrite(bool sw);
		bool isWriteModeFromController();
		unsigned int generalBufferSize();
		void displayConfiguration();
		void printStats(bool finalStats);

		void RegisterCallbacks(
			Callback_t *readDone,
			Callback_t *writeDone);

		//fields
		MemoryController *memoryController;
		MemoryDevice *memDev;
		//function pointers
		Callback_t *ReturnReadData;
		Callback_t *WriteDataDone;
		//TODO: make this a functor as well?

		unsigned systemID;
		unsigned int numberRequestors;

	private:
		string systemIniFilename;
		string deviceGene;
		string deviceSpeed;
		string deviceSize;

		unsigned int numberRanks;
	};
} // namespace MCsim

#endif
