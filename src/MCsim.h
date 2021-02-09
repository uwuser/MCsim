
#ifndef MCSIM_H
#define MCSIM_H

#include "Callback.h"
#include <string>
using std::string;

namespace MCsim
{

	class MultiChannelMemorySystem
	{
	public:
		bool addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size);
		void setCPUClockSpeed(uint64_t cpuClkFreqHz);
		void update();
		void printStats(bool finalStats);
		bool willAcceptTransaction();
		bool willAcceptTransaction(uint64_t addr);
		std::ostream &getLogFile();

		void RegisterCallbacks(
			TransactionCompleteCB *readDone,
			TransactionCompleteCB *writeDone);
		int getIniBool(const std::string &field, bool *val);
		int getIniUint(const std::string &field, unsigned int *val);
		int getIniUint64(const std::string &field, uint64_t *val);
		int getIniFloat(const std::string &field, float *val);
	};

	MultiChannelMemorySystem *getMemorySystemInstance(unsigned int numberRequestors, const string &systemIniFilename_, const string &deviceGene, const string &deviceSpeed, const string &deviceSize, unsigned int channels, unsigned int ranks);

} // namespace MCsim

#endif
