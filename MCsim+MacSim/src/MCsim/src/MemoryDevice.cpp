/*********************************************************************************
*  Copyright (c) 2015-2016, Danlu Guo
*                             University of Waterloo
*                
*  All rights reserved.
*	This is the memory system and we have add Ramulator into this platform
*********************************************************************************/

#include "MemoryDevice.h"
#include "MemoryController.h"

using namespace MCsim;

MemoryDevice::MemoryDevice(unsigned int ranks): 
	
	ranks(ranks)
{
	clockCycle = 0;
	bankGroups = 1;
	banks = 1;
	subArrays = 1;
	rows = 0;
	columns = 0;
	dataBusWidth = 0;
	memoryController = NULL;
	commandTrace.open("cmdTrace.txt");
}

MemoryDevice::~MemoryDevice()
{
	commandTrace.close();
	dataCycles.clear();
	memoryArray.clear();
	// for(auto it=pendingReadData.begin(); it!=pendingReadData.end(); it++) {
	// 	delete (*it);
	// }
	pendingReadData.clear();
}

void MemoryDevice::connectMemoryController(MemoryController* memCtlr) {
	memoryController = memCtlr;
}

unsigned int MemoryDevice::get_DataBus()
{
	return dataBusWidth*8;
}
unsigned int MemoryDevice::get_Rank()
{
	return ranks;
}
unsigned int MemoryDevice::get_BankGroup()
{
	return bankGroups;
}
unsigned int MemoryDevice::get_Bank()
{
	return banks;
}
unsigned long MemoryDevice::get_Row()
{
	return rows;
}
unsigned long MemoryDevice::get_Column()
{
	return columns;
}

void MemoryDevice::update() {
	// cout<<"MH: Memory Device update and pendingReadData.empty()="<<pendingReadData.empty()<<endl;
	if(!pendingReadData.empty()) {
		for(unsigned int index=0; index<dataCycles.size(); index++) {
			if(dataCycles[index]>0) {
				dataCycles[index]--;
			}
		}
		// cout<<"dataCycles.front()= "<<dataCycles.front()<<endl;
		if(dataCycles.front() == 0) {
			dataCycles.erase(dataCycles.begin());
			memoryController->receiveData(pendingReadData.front());
			// ********************* --- ************
			// delete pendingReadData.front();
			pendingReadData.erase(pendingReadData.begin());
		}
	}
	if(!postBuffer.empty()) {
		for(unsigned int index=0; index<postBuffer.size(); index++) {
			if(postCounter[index] > 0) {
				postCounter[index]--;
			}
		}
		if(postCounter.front() == 0) {
			receiveFromBus(postBuffer.front());
			delete postBuffer.front();
			postCounter.erase(postCounter.begin());
			postBuffer.erase(postBuffer.begin());
		}
	}
	// MH: now this is done by the step function of all simulator objs
	clockCycle++;
}

void MemoryDevice::generateData(BusPacket* cmd) {
	// cout<<"MH: inside genreate Data"<<endl;
	if(cmd->busPacketType == DATA) {
		// cout<<"MH: command bus type = DATA"<<endl;
		updateDataArray(cmd);
	}
	else {
		// cout<<"MH: command type not Data"<<endl;
		if(cmd->busPacketType == RD || cmd->busPacketType == RDA) {
			// cout<<"MH: read request push to pending readdata"<<endl;
			pendingReadData.push_back(new BusPacket(DATA, cmd->requestorID, cmd->address, 0, 0, 0, 0, cmd->data, cmd->arriveTime));
			dataCycles.push_back(get_constraints("tRL") + get_constraints("tBus"));
		}
	}
}

void* MemoryDevice::readDataArray(BusPacket* load) {
	return 0;//&memoryArray[load->rank][load->bank][load->row][load->column];
}

void MemoryDevice::updateDataArray(BusPacket* store) {
	//memoryArray[store->rank][store->bank][store->row][store->column] = &store->data;
}
