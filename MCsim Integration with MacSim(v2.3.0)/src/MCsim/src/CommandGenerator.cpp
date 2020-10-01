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

#include "global.h"
#include "CommandGenerator.h"

#include <algorithm>  

using namespace MCsim;
using namespace std;

CommandGenerator::CommandGenerator(unsigned int dataBus,std::vector<CommandQueue*>& commandQueues):
	dataBusSize(dataBus),
	commandQueue(commandQueues)
{}

CommandGenerator::~CommandGenerator()
{
	lookupTable.clear();
	std::queue<BusPacket*> empty;
	std::swap(commandBuffer, empty);
}
// Return the head cmd from the general cmd buffer
BusPacket* CommandGenerator::getCommand() 
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
