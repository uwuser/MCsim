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




//MemorySystem.cpp
//
//Class file for JEDEC memory system wrapper
//

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
	else if (deviceGene == "DDR4") {
		// DDR4* ddr4 = new DDR4(configs["org"], configs["speed"]);
		// memSys = new Ramulator<DDR4>(&configs, ddr4, dataBusSize);  	 			
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
	else if (deviceGene == "DDR4") {
		// DDR4* ddr4 = new DDR4(configs["org"], configs["speed"]);
		// memSys = new Ramulator<DDR4>(&configs, ddr4, dataBusSize);  	 			
	} 
	else {
		std::cout<<"Wrong DRAM standard"<<std::endl;
	}

	memDev->connectMemoryController(memoryController); 
	memoryController->connectMemoryDevice(memDev);

}


MemorySystem::~MemorySystem()
{
	/* the MemorySystem should exist for all time, nothing should be destroying it */  
//	ERROR("MEMORY SYSTEM DESTRUCTOR with ID "<<systemID);
//	abort();

	delete(memoryController);
	delete(memDev);

/***************************************************************/
/* This shoud be only for trace based simulation  
	for(int i=0; i<numberRequestors; i++) {
		requestorsMap[i]->printResult();
		delete requestorsMap[i];
		requestorsMap.erase(i);
	}
*/
/******************************************************************/

	
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

//update the memory systems state
void MemorySystem::update()
{

	//PRINT(" ----------------- Memory System Update ------------------");

	//updates the state of each of the objects
	// NOTE - do not change order
	//pendingTransactions will only have stuff in it if MARSS is adding stuff
	/*if (pendingTransactions.size() > 0 && memoryController->WillAcceptTransaction())
	{
		memoryController->addTransaction(pendingTransactions.front());
		pendingTransactions.pop_front();
	}*/
	memoryController->update();
	memDev->update();

	
	//memoryController->step();
	//memDev->step();
	this->step();

	//PRINT("\n"); // two new lines
}

void MemorySystem::RegisterCallbacks( Callback_t* readCB, Callback_t* writeCB)
{

	
	ReturnReadData = readCB;
	WriteDataDone = writeCB;
	
}

} /*namespace DRAMSim */



// This function can be used by autoconf AC_CHECK_LIB since
// apparently it can't detect C++ functions.
// Basically just an entry in the symbol table
extern "C"
{
	void libdramsim_is_present(void)
	{
		;
	}
}

