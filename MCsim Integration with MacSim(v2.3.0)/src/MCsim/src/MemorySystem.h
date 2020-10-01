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

#ifndef MEMORYSYSTEM_H
#define MEMORYSYSTEM_H

#include "SimulatorObject.h"

#include "MemoryController.h"


#include "Callback.h"

#include <deque>
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

	unsigned int generalBufferSize();
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

