#include "MemorySystem.h"
#include <unistd.h>

using namespace std;

unsigned NUM_DEVICES;
unsigned NUM_RANKS;
unsigned NUM_RANKS_LOG;

namespace MCsim
{

	MemorySystem::MemorySystem(unsigned int numRequestors_, unsigned id, const string &systemIniFilename_, const string &deviceGene_, const string &deviceSpeed_, const string &deviceSize_, unsigned int ranks_) : ReturnReadData(NULL),
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

		DEBUG("===== MemorySystem " << systemID << " =====");

		memoryController = new MemoryController(this, systemIniFilename);

		const string GeneSpeed = deviceGene + '_' + deviceSpeed;
		const string GeneSize = deviceGene + '_' + deviceSize;
		if (deviceGene == "DDR3")
		{
			DDR3 *ddr3 = new DDR3(GeneSize, GeneSpeed);
			memDev = new Ramulator_DDR3<DDR3>(ddr3, numberRanks);
		}
		else if (deviceGene == "DDR4")
		{
			DDR4 *ddr4 = new DDR4(GeneSize, GeneSpeed);
			memDev = new Ramulator_DDR4<DDR4>(ddr4, numberRanks);
		}
		else if (deviceGene == "DSARP")
		{
			DSARP::Org test_org = DSARP::Org::DSARP_8Gb_x8;
			DSARP *dsddr3_dsarp = new DSARP(test_org, DSARP::Speed::DSARP_1333, DSARP::Type::DSARP, 64);
			memDev = new Ramulator_DSARP<DSARP>(dsddr3_dsarp, numberRanks);
		}
		else
		{
			std::cout << "Wrong DRAM standard" << std::endl;
		}
		memDev->connectMemoryController(memoryController);
		memoryController->connectMemoryDevice(memDev);
	}

	MemorySystem::MemorySystem(unsigned int numRequestors_, unsigned id, const string &systemIniFilename_, const string &deviceGene_, const string &deviceSpeed_, const string &deviceSize_, unsigned int ranks_, function<void(Request &)> callback) : ReturnReadData(NULL),
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

		DEBUG("===== MemorySystem " << systemID << " =====");

		memoryController = new MemoryController(this, systemIniFilename, callback);

		const string GeneSpeed = deviceGene + '_' + deviceSpeed;
		const string GeneSize = deviceGene + '_' + deviceSize;
		if (deviceGene == "DDR3")
		{
			DDR3 *ddr3 = new DDR3(GeneSize, GeneSpeed);
			memDev = new Ramulator_DDR3<DDR3>(ddr3, numberRanks);
		}
		else if (deviceGene == "DDR4")
		{
			DDR4 *ddr4 = new DDR4(GeneSize, GeneSpeed);
			memDev = new Ramulator_DDR4<DDR4>(ddr4, numberRanks);
		}
		else if (deviceGene == "DSARP")
		{
			DSARP::Org test_org = DSARP::Org::DSARP_8Gb_x8;
			DSARP *dsddr3_dsarp = new DSARP(test_org, DSARP::Speed::DSARP_1333, DSARP::Type::DSARP, 64);
			memDev = new Ramulator_DSARP<DSARP>(dsddr3_dsarp, numberRanks);
		}
		else
		{
			std::cout << "Wrong DRAM standard" << std::endl;
		}

		memDev->connectMemoryController(memoryController);
		memoryController->connectMemoryDevice(memDev);
	}

	MemorySystem::~MemorySystem()
	{
		delete (memoryController);
		delete (memDev);

		/****************************MH***********************************/
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
	bool MemorySystem::isWriteModeFromController()
	{
		return memoryController->isWriteModeFromController();
	}
	unsigned int MemorySystem::generalBufferSize()
	{
		return memoryController->generalBufferSize();
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
		memoryController->update();
		memDev->update();
		this->step();
	}

	void MemorySystem::RegisterCallbacks(Callback_t *readCB, Callback_t *writeCB)
	{
		ReturnReadData = readCB;
		WriteDataDone = writeCB;
	}

} // namespace MCsim
extern "C"
{
	void libdramsim_is_present(void)
	{
		;
	}
}
