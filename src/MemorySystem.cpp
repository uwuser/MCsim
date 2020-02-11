#include "MemorySystem.h"
#include "MemoryController.h"

using namespace MCsim;

MemorySystem::MemorySystem(unsigned int ranks): 
	clockCycle(1),
	ranks(ranks)
{
	bankGroups = 1;
	banks = 1;
	subArrays = 1;
	rows = 0;
	columns = 0;
	dataBusWidth = 0;
	memoryController = NULL;
	commandTrace.open("cmdTrace.txt");
}

MemorySystem::~MemorySystem()
{
	commandTrace.close();
	dataCycles.clear();
	memoryArray.clear();
	pendingReadData.clear();
}

void MemorySystem::connectMemoryController(MemoryController* memCtlr) {
	memoryController = memCtlr;
}

unsigned int MemorySystem::get_DataBus()
{
	return dataBusWidth*8;
}
unsigned int MemorySystem::get_Rank()
{
	return ranks;
}
unsigned int MemorySystem::get_BankGroup()
{
	return bankGroups;
}
unsigned int MemorySystem::get_Bank()
{
	return banks;
}
unsigned long MemorySystem::get_Row()
{
	return rows;
}
unsigned long MemorySystem::get_Column()
{
	return columns;
}

void MemorySystem::update() {
	if(!pendingReadData.empty()) {
		for(unsigned int index=0; index<dataCycles.size(); index++) {
			if(dataCycles[index]>0) {
				dataCycles[index]--;
			}
		}
		if(dataCycles.front() == 0) {
			dataCycles.erase(dataCycles.begin());
			memoryController->receiveData(pendingReadData.front());			
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

void MemorySystem::generateData(BusPacket* cmd) {
	if(cmd->busPacketType == DATA) {
		updateDataArray(cmd);
	}
	else {
		if(cmd->busPacketType == RD || cmd->busPacketType == RDA) {
			pendingReadData.push_back(new BusPacket(DATA, cmd->requestorID, cmd->address, 0, 0, 0, 0, cmd->data, cmd->arriveTime));
			dataCycles.push_back(get_constraints("tRL") + get_constraints("tBus"));
		}
	}
}

void* MemorySystem::readDataArray(BusPacket* load) {
	return 0;
}

void MemorySystem::updateDataArray(BusPacket* store) {
}