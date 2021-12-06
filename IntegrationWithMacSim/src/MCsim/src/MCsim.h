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


#ifndef MCSIM_H
#define MCSIM_H
/*
 * This is a public header for MCsim including this along with libmcsim.so should
 * provide all necessary functionality to talk to an external simulator
 */
#include "Callback.h"
#include <string>
using std::string;

namespace MCsim 
{

	class MultiChannelMemorySystem {
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

}

#endif
