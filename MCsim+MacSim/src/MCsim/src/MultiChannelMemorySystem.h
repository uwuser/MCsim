/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/
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
