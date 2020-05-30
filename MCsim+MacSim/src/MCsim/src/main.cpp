#include <iostream>
#include <fstream>
#include <sstream>
#include <getopt.h>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <cstring>
#include <functional>

#include "Requestor.h"
#include "Request.h"
//#include "MemoryController.h"
//#include "MultiChannelMemorySystem.h"
#include "Ramulator.h"
#include "Ramulator_DDR4.h"

using namespace std;
using namespace MCsim;

void usage() {
	cout << "MCsim Usage: " << endl;
	cout << "MCsim -n 8 -s system.ini -d device.ini -t memoryTraces -c cycles" <<endl;
	cout << "\t-n, --Requestor=# \t\tspecify number of requestors to run the simulation for [default=8] "<<endl;
	cout << "\t-t, --tracefile=FILENAME \tspecify multiple tracefile to run  "<<endl;
	cout << "\t-s, --systemini=FILENAME \tspecify multiple ini file that describes the memory system parameters  "<<endl;
	cout << "\t-c, --cycles=# \t\t\tspecify number of cycles to run the simulation for [default=30] "<<endl;
	cout << "\t-C, --Channel=# \t\tspecify number of channels "<<endl;
	cout << "\t-R, --Ranks=# \t\t\tspecify number of ranks per Channel "<<endl;
	cout << "\t-G, --DevGene=string \t\tspecify DRAM device generation "<<endl;
	cout << "\t-D, --DevVersion=string \tspecify DRAM device "<<endl;
	cout << "\t-S, --DevSize=string \t\tspecify DRAM device organization "<<endl;			
}

int main(int argc, char **argv)
{	
	int argument = 0;
	string systemIniFilename = "system.ini";
	unsigned requestors = 8;
	string traceFileName = "mem.trc";
	unsigned cycles = 0;
	unsigned int channels = 1;
	unsigned int ranks = 1;
	string deviceGene = "DDR3";
	string deviceSpeed = "1600H";
	string deviceSize = "2Gb_x8";

	while(1) {
		static struct option options[] = {
			{"numREQs", required_argument, 0, 'n'},
			{"sysFile", required_argument, 0, 's'},
			{"devFile", required_argument, 0, 'd'},
			{"trcFile", required_argument, 0, 't'},
			{"numCycl", required_argument, 0, 'c'},
			{"numChan", required_argument, 0, 'C'},
			{"numRank", required_argument, 0, 'R'},
			{"DeviceGeen", required_argument, 0, 'G'},
			{"DeviceSpeed", required_argument, 0, 'D'},
			{"DeviceSize", required_argument, 0, 'S'},
			{"help", no_argument, 0, 'h'}
		};

		int option_index = 0;
		argument = getopt_long(argc, argv, "n:s:d:t:c:C:R:G:D:S:h", options, &option_index);
		if (argument == -1) {
			break;
		}
		switch (argument) {
			case 'h':
			case '?':
				usage();
				exit(0);
			case 'n':
				requestors = atoi(optarg);
				break;
			case 's':
				systemIniFilename = string(optarg);
				break;
			case 't':
				traceFileName = string(optarg);
				break;
			case 'c':
				cycles = atoi(optarg);
				break;
			case 'C':
				channels = atoi(optarg);
				break;
			case 'R':
				ranks = atoi(optarg);
				break;
			case 'G':
				deviceGene = string(optarg);
				break;
			case 'D':
				deviceSpeed = string(optarg);
				break;
			case 'O':
				deviceSize = string(optarg);
				break;
		}
	}
	
	// Channel, MemoryController, Requestor



	




	map<int, Requestor*> requestorsMap;
	// Callback function pass complete request to requestor
	auto callBack = [&requestorsMap](MCsim::Request& r) {
		requestorsMap[r.requestorID]->returnData(&r);
	};


	MultiChannelMemorySystem *memorySystem = new MultiChannelMemorySystem(requestors, systemIniFilename, deviceGene,  deviceSpeed, deviceSize, channels, ranks,callBack);
	DEBUG("CHANNELS IN THE MEMORY SYSTEM:  "<<channels);


	for(unsigned int c = 0; c < channels; c++) {
		DEBUG("MEMORY CONTROLLER IS:   "<<systemIniFilename);
		

	}
	ifstream memTrace;
	memTrace.open(traceFileName.c_str());
	if(!memTrace.is_open()) {
		DEBUG("ERROR:  COULD NOT OPEN TRACE FILE");
		exit(0);
	}
	bool inOrder = true;
	int requestSize = 64;
	string line;
	for(unsigned int id = 0; id < requestors; id++) {
		getline(memTrace, line);		
		switch(id)
		{
			case 0:
				inOrder = false;
				requestSize = 64;
				break;
			case 1:
				inOrder = true;
				requestSize = 64;
				break;
			case 2:
				inOrder = true;
				requestSize = 64;
				break;
			case 3:		
				inOrder = true;
				requestSize = 64;
				break;
			case 4:
				inOrder = true;
				requestSize = 64;
				break;
			case 5:
				inOrder = true;
				requestSize = 64;
				break;
			case 6:
				inOrder = true;
				requestSize = 64;
				break;
			case 7:
				inOrder = true;
				requestSize = 64;
				break;
			default:
				requestSize = 64;
				break;
		}
		requestorsMap[id] = new Requestor(id, inOrder, line);
		requestorsMap[id]->RequestSize = requestSize;
		// Channel Assignment
		//int ch = (int)(id%channels);
		requestorsMap[id]->connectMemorySystem(memorySystem);
		
		requestorsMap[id]->memoryClock = memorySystem->getClk();
	}
	memTrace.close();
	
	PRINT("\nStart Simulation");
	memorySystem->displayConfiguration();
	
	/* Simple Requestor Simulation Engine	*/
	uint64_t currentClockCyle = 1;
	clock_t begin = clock();
	bool simDone = false;
	while(!simDone){
		//if(cycles != 1 && currentClockCyle == cycles) {
		if(cycles != 0 && currentClockCyle == cycles) {
			simDone = true;}
		 
		// Step Requestor
		for(unsigned int id = 0; id < requestors; id++) {
			requestorsMap[id]->update();
			// Determine if simulation is complete
			if(requestorsMap[id]->sim_end()) {
				simDone = true;
			}
		}
		// Step Memory System
		memorySystem->update();			
		currentClockCyle++;
	}
	clock_t end = clock();

	// print stats
	memorySystem->printStats();
	// --- Memory Deallocation
	

	printf("-----------------------------------------Summary Per Requestor-------------------------------------------");
	printf(" \n");
	for(unsigned int i=0; i<requestors; i++) {
		requestorsMap[i]->printResult();
		delete requestorsMap[i];
		requestorsMap.erase(i);
	}
	requestorsMap.clear();
	std::cout<<"Simulation End @ "<<currentClockCyle<<" time = "<<((end-begin))<<std::endl;
	exit(0);
	return 0;
}
