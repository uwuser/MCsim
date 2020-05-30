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



#ifndef MEMORYSYSTEM_H
#define MEMORYSYSTEM_H

//MemorySystem.h
//
//Header file for JEDEC memory system wrapper
//

#include "SimulatorObject.h"

#include "MemoryController.h"


#include "Callback.h"

#include <deque>


//#include "Requestor.h"
#include "Request.h"
#include "Ramulator.h"



namespace MCsim
{
typedef CallbackBase<void,unsigned,uint64_t,uint64_t> Callback_t;
class MemorySystem : public SimulatorObject
{
	
public:
	//functions
	MemorySystem(unsigned int numRequestors, unsigned id, const string &systemIniFilename, const string &deviceGene,  const string &deviceSpeed, const string &deviceSize, unsigned int ranks);
	MemorySystem(unsigned int numRequestors, unsigned id, const string &systemIniFilename, const string &deviceGene,  const string &deviceSpeed, const string &deviceSize, unsigned int ranks, function<void(Request&)> callback);
	virtual ~MemorySystem();
	void update();
	bool addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size);
	void flushWrite(bool sw);
	void displayConfiguration();
	void printStats(bool finalStats);
	
	void RegisterCallbacks(
	    Callback_t *readDone,
	    Callback_t *writeDone);

	//fields
	MemoryController *memoryController;
	MemoryDevice* memDev;

	// deque<Transaction *> pendingTransactions; 


	//function pointers
	Callback_t* ReturnReadData;
	Callback_t* WriteDataDone;
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
}

#endif

