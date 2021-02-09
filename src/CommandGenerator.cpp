#include "global.h"
#include "CommandGenerator.h"

#include <algorithm>

using namespace MCsim;
using namespace std;

CommandGenerator::CommandGenerator(unsigned int dataBus) : dataBusSize(dataBus)
{
}

CommandGenerator::~CommandGenerator()
{
	lookupTable.clear();
	std::queue<BusPacket *> empty;
	std::swap(commandBuffer, empty);
}
// Return the head cmd from the general cmd buffer
BusPacket *CommandGenerator::getCommand()
{
	return commandBuffer.front();
}
// Pop the cmd from the head of the general cmd buffer
void CommandGenerator::removeCommand()
{
	commandBuffer.pop();
}
// Return the general cmd buffer size
unsigned CommandGenerator::bufferSize()
{
	return commandBuffer.size();
}
