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


#include <errno.h> 
#include <sstream> //stringstream
#include <stdlib.h> // getenv()
// for directory operations 
#include <sys/stat.h>
#include <sys/types.h>

#include "MultiChannelMemorySystem.h"
#include "AddressMapping.h"




using namespace MCsim; 


MultiChannelMemorySystem::MultiChannelMemorySystem(unsigned int numberRequestors_, const string &systemIniFilename_, const string &deviceGene_, const string &deviceSpeed_, const string &deviceSize_, unsigned int channels_, unsigned int ranks_)
	:numberRequestors(numberRequestors_),
	systemIniFilename(systemIniFilename_),
	deviceGene(deviceGene_),
	deviceSpeed(deviceSpeed_),
	deviceSize(deviceSize_),
	numberChannels(channels_),
	numberRanks(ranks_),
	clockDomainCrosser(new ClockDomain::Callback<MultiChannelMemorySystem, void>(this, &MultiChannelMemorySystem::actual_update))
{
	clockCycle=0; 


	if (numberChannels == 0) 
	{
		cout<<"\nERROR: Zero channels"; 
		abort(); 
	}
	for (size_t i=0; i<numberChannels; i++)
	{
		MemorySystem *channel = new MemorySystem(numberRequestors, i, systemIniFilename, deviceGene, deviceSpeed, deviceSize, numberRanks);
		channels.push_back(channel);
	}
	tCK = channels[0]->memDev->get_constraints("tCK");
}


MultiChannelMemorySystem::MultiChannelMemorySystem(unsigned int numberRequestors_, const string &systemIniFilename_, const string &deviceGene_, const string &deviceSpeed_, const string &deviceSize_, unsigned int channels_, unsigned int ranks_, function<void(Request&)> callback)
	:numberRequestors(numberRequestors_),
	systemIniFilename(systemIniFilename_),
	deviceGene(deviceGene_),
	deviceSpeed(deviceSpeed_),
	deviceSize(deviceSize_),
	numberChannels(channels_),
	numberRanks(ranks_),
	clockDomainCrosser(new ClockDomain::Callback<MultiChannelMemorySystem, void>(this, &MultiChannelMemorySystem::actual_update))
{
	clockCycle=0; 


	if (numberChannels == 0) 
	{
		cout<<"\nERROR: Zero channels"; 
		abort(); 
	}
	for (size_t i=0; i<numberChannels; i++)
	{
		MemorySystem *channel = new MemorySystem(numberRequestors, i, systemIniFilename, deviceGene, deviceSpeed, deviceSize, numberRanks, callback);
		channels.push_back(channel);
	}
	tCK = channels[0]->memDev->get_constraints("tCK");
}


float MultiChannelMemorySystem::getClk()
{

	return tCK;
}

/* Initialize the ClockDomainCrosser to use the CPU speed 
	If cpuClkFreqHz == 0, then assume a 1:1 ratio (like for TraceBasedSim)
	*/
void MultiChannelMemorySystem::setCPUClockSpeed(uint64_t cpuClkFreqHz)
{
	
	uint64_t mcsimClkFreqHz = (uint64_t)(1.0/(tCK*1e-9));
	clockDomainCrosser.clock1 = mcsimClkFreqHz; 
	clockDomainCrosser.clock2 = (cpuClkFreqHz == 0) ? mcsimClkFreqHz : cpuClkFreqHz; 
}

bool fileExists(string &path)
{
	struct stat stat_buf;
	if (stat(path.c_str(), &stat_buf) != 0) 
	{
		if (errno == ENOENT)
		{
			return false; 
		}
		ERROR("Warning: some other kind of error happened with stat(), should probably check that"); 
	}
	return true;
}

string FilenameWithNumberSuffix(const string &filename, const string &extension, unsigned maxNumber=100)
{
	string currentFilename = filename+extension;
	if (!fileExists(currentFilename))
	{
		return currentFilename;
	}

	// otherwise, add the suffixes and test them out until we find one that works
	stringstream tmpNum; 
	tmpNum<<"."<<1; 
	for (unsigned i=1; i<maxNumber; i++)
	{
		currentFilename = filename+tmpNum.str()+extension;
		if (fileExists(currentFilename))
		{
			currentFilename = filename; 
			tmpNum.seekp(0);
			tmpNum << "." << i;
		}
		else 
		{
			return currentFilename;
		}
	}
	// if we can't find one, just give up and return whatever is the current filename
	ERROR("Warning: Couldn't find a suitable suffix for "<<filename); 
	return currentFilename; 
}



MultiChannelMemorySystem::~MultiChannelMemorySystem()
{
	for (size_t i=0; i<numberChannels; i++)
	{
		delete channels[i];
	}
	channels.clear(); 


}
void MultiChannelMemorySystem::update()
{
	clockDomainCrosser.update(); 
}
void MultiChannelMemorySystem::actual_update() 
{
	for (size_t i=0; i<numberChannels; i++)
	{
		channels[i]->update(); 
	}


	clockCycle++; 
}
unsigned MultiChannelMemorySystem::findChannelNumber(unsigned int requestorID, uint64_t addr)
{
	// Single channel case is a trivial shortcut case 
	if (numberChannels == 1)
	{
		return 0; 
	}
 
	unsigned channelNumber;
	channelNumber = requestorID%numberChannels;
	
	if (channelNumber >= numberChannels)
	{
		ERROR("Got channel index "<<channelNumber<<" but only "<<numberChannels<<" exist"); 
		abort();
	}
	//DEBUG("Channel idx = "<<channelNumber<<" totalbits="<<totalBits<<" channelbits="<<channelBits); 

	return channelNumber;

}
ostream &MultiChannelMemorySystem::getLogFile()
{
	return mcsim_log; 
}


bool MultiChannelMemorySystem::addRequest(unsigned int requestorID, unsigned long long address, bool R_W, unsigned int size)
{
	unsigned channelNumber = findChannelNumber(requestorID, address); 
	return channels[channelNumber]->addRequest(requestorID, address, R_W, size); 
}

void MultiChannelMemorySystem::flushWrite(bool sw)
{

	for (size_t i=0; i<numberChannels; i++)
	{
		channels[i]->flushWrite(sw);
	}

}

void MultiChannelMemorySystem::displayConfiguration()
{

	for (size_t i=0; i<numberChannels; i++)
	{
		channels[i]->displayConfiguration();
	}


}
unsigned int MultiChannelMemorySystem::generalBufferSize()
{
	for (size_t i=0; i<numberChannels; i++)
	{
		channels[i]->generalBufferSize();
	}
}


void MultiChannelMemorySystem::printStats(bool finalStats) {

	
	for (size_t i=0; i<numberChannels; i++)
	{
		PRINT("==== Channel ["<<i<<"] ====");
		channels[i]->printStats(finalStats); 
		PRINT("//// Channel ["<<i<<"] ////");
	}
	
}
void MultiChannelMemorySystem::RegisterCallbacks( 
		TransactionCompleteCB *readDone,
		TransactionCompleteCB *writeDone)
{

	for (size_t i=0; i<numberChannels; i++)
	{
		channels[i]->RegisterCallbacks(readDone, writeDone); 
	}
}



namespace MCsim {

MultiChannelMemorySystem *getMemorySystemInstance(unsigned int numberRequestors, const string &systemIniFilename, const string &deviceGene,  const string &deviceSpeed, const string &deviceSize, unsigned int channels, unsigned int ranks)
{
	return new MultiChannelMemorySystem(numberRequestors, systemIniFilename, deviceGene, deviceSpeed, deviceSize, channels, ranks);
}




}