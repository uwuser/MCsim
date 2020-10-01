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
	if(!pendingReadData.empty()) {
		for(unsigned int index=0; index<dataCycles.size(); index++) {
			if(dataCycles[index]>0) {
				dataCycles[index]--;
			}
		}
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
	clockCycle++;
}

void MemoryDevice::generateData(BusPacket* cmd) {
	if(cmd->busPacketType == DATA) {
		updateDataArray(cmd);
	}
	else {
		if(cmd->busPacketType == RD || cmd->busPacketType == RDA) {
			pendingReadData.push_back(new BusPacket(DATA, cmd->requestorID, cmd->address, 0, 0, 0, 0, 0, cmd->data, cmd->arriveTime));
			dataCycles.push_back(get_constraints("tRL") + get_constraints("tBus"));
		}
	}
}

void* MemoryDevice::readDataArray(BusPacket* load) {
	return 0;
}

void MemoryDevice::updateDataArray(BusPacket* store) {
}
