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


#include "SimulatorObject.h"
#include "Request.h"

#include "MemorySystem.h"
#include "Callback.h"
#include "ClockDomain.h"
#include <string>


using namespace std;

namespace MCsim {


class MultiChannelMemorySystem : public SimulatorObject 
{
	public: 

	MultiChannelMemorySystem(unsigned int numberRequestors, const string &systemIniFilename_, const string &deviceGene,  const string &deviceSpeed, const string &deviceSize, unsigned int channels, unsigned int ranks);

	MultiChannelMemorySystem(unsigned int numberRequestors, const string &systemIniFilename_, const string &deviceGene,  const string &deviceSpeed, const string &deviceSize, unsigned int channels, unsigned int ranks, function<void(Request&)> callback);

	
	virtual ~MultiChannelMemorySystem();
	bool addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size);

			
	void update();
	void printStats(bool finalStats=false);
	ostream &getLogFile();
	void RegisterCallbacks( 
			TransactionCompleteCB *readDone,
			TransactionCompleteCB *writeDone);


	void InitOutputFiles(string tracefilename);
	void setCPUClockSpeed(uint64_t cpuClkFreqHz);

	
	float getClk();
	void displayConfiguration();
	void flushWrite(bool sw);
	unsigned int generalBufferSize();

	//output file
	std::ofstream visDataOut;
	ofstream mcsim_log; 


	

	private:
		unsigned int numberRequestors;
		string systemIniFilename;
		string deviceGene;
		float tCK;
		string deviceSpeed;
		string deviceSize;
		unsigned int numberChannels;
		unsigned int numberRanks;

		unsigned findChannelNumber(unsigned int requestorID, uint64_t addr);
		void actual_update(); 
		vector<MemorySystem*> channels; 
		unsigned megsOfMemory; 
		string deviceIniFilename;
		string traceFilename;
		string pwd;
		string *visFilename;
		ClockDomain::ClockDomainCrosser clockDomainCrosser; 
		static void mkdirIfNotExist(string path);
		static bool fileExists(string path); 
		


	};
}
