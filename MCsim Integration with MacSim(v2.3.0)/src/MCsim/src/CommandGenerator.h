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

#ifndef COMMANDGENERATOR_H
#define COMMANDGENERATOR_H

#include <queue>
#include "Request.h"
#include "BusPacket.h"
#include "CommandQueue.h"

namespace MCsim
{
	class CommandGenerator
	{
	public:
		CommandGenerator(unsigned int dataBus,std::vector<CommandQueue*>& commandQueues);
		virtual ~CommandGenerator();
		virtual bool commandGenerate(Request* request, bool open) = 0;

		BusPacket* getCommand();
		void removeCommand();
		unsigned int bufferSize();

	protected:
		unsigned int dataBusSize;
		std::vector<CommandQueue*>& commandQueue;
		bool first[8] = {true,true,true,true,true,true,true,true};
		std::map<unsigned int, std::pair<unsigned int, unsigned int>>lookupTable;
		std::queue< BusPacket* > commandBuffer;
	};
}

#endif /* COMMANDGENERATOR_H */