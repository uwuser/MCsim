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

#include "MemorySystem.h"

#include <unistd.h>


using namespace std;


ofstream cmd_verify_out; //used in Rank.cpp and MemoryController.cpp if VERIFICATION_OUTPUT is set

unsigned NUM_DEVICES;
unsigned NUM_RANKS;
unsigned NUM_RANKS_LOG;

namespace MCsim {





MemorySystem::MemorySystem(unsigned int numRequestors_, unsigned id, const string &systemIniFilename_, const string &deviceGene_,  const string &deviceSpeed_, const string &deviceSize_, unsigned int ranks_) :
		ReturnReadData(NULL),
		WriteDataDone(NULL),
		systemID(id),
		numberRequestors(numRequestors_),
		systemIniFilename(systemIniFilename_),
		deviceGene(deviceGene_),
		deviceSpeed(deviceSpeed_),
		deviceSize(deviceSize_),
		numberRanks(ranks_)


{
	
	clockCycle = 0;

	DEBUG("===== MemorySystem "<<systemID<<" =====");


	memoryController = new MemoryController(this, systemIniFilename);

	const string GeneSpeed = deviceGene + '_' + deviceSpeed;
	const string GeneSize = deviceGene + '_' + deviceSize;
	if (deviceGene == "DDR3") {
		DDR3* ddr3 = new DDR3(GeneSize, GeneSpeed);
		memDev = new Ramulator<DDR3>(ddr3, numberRanks);
	} 
	else {
		std::cout<<"Wrong DRAM standard"<<std::endl;
	}

	memDev->connectMemoryController(memoryController); 
	memoryController->connectMemoryDevice(memDev);

}


MemorySystem::MemorySystem(unsigned int numRequestors_, unsigned id, const string &systemIniFilename_, const string &deviceGene_,  const string &deviceSpeed_, const string &deviceSize_, unsigned int ranks_, function<void(Request&)> callback) :
		ReturnReadData(NULL),
		WriteDataDone(NULL),
		systemID(id),
		numberRequestors(numRequestors_),
		systemIniFilename(systemIniFilename_),
		deviceGene(deviceGene_),
		deviceSpeed(deviceSpeed_),
		deviceSize(deviceSize_),
		numberRanks(ranks_)


{
	
	clockCycle = 0;

	DEBUG("===== MemorySystem "<<systemID<<" =====");


	memoryController = new MemoryController(this, systemIniFilename, callback);

	const string GeneSpeed = deviceGene + '_' + deviceSpeed;
	const string GeneSize = deviceGene + '_' + deviceSize;
	if (deviceGene == "DDR3") {
		DDR3* ddr3 = new DDR3(GeneSize, GeneSpeed);
		memDev = new Ramulator<DDR3>(ddr3, numberRanks);
	}
	else {
		std::cout<<"Wrong DRAM standard"<<std::endl;
	}

	memDev->connectMemoryController(memoryController); 
	memoryController->connectMemoryDevice(memDev);

}


MemorySystem::~MemorySystem()
{
	delete(memoryController);
	delete(memDev);	
}





bool MemorySystem::addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size)
{
	
	
		return memoryController->addRequest(requestorID, address, R_W, size); 
	
}

void MemorySystem::flushWrite(bool sw)
{

	memoryController->flushWrite(sw);
}



//prints statistics
void MemorySystem::printStats(bool finalStats)
{
	memoryController->printResult();
}

void MemorySystem::displayConfiguration()
{

	memoryController->displayConfiguration();

}

unsigned int MemorySystem::generalBufferSize()
{

	memoryController->generalBufferSize();

}
//update the memory systems state
void MemorySystem::update()
{
	memoryController->update();
	memDev->update();
	this->step();
}

void MemorySystem::RegisterCallbacks( Callback_t* readCB, Callback_t* writeCB)
{

	
	ReturnReadData = readCB;
	WriteDataDone = writeCB;
	
}

} /*namespace DRAMSim */
extern "C"
{
	void libmcsim_is_present(void)
	{
		;
	}
}

