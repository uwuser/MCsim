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

#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H

#include <queue>
#include <map>
#include "BusPacket.h"

namespace MCsim
{
	class CommandQueue
	{
	public:
		CommandQueue(bool perRequestor);
		virtual ~CommandQueue();
		// Insert cmd based on either general criticality or requestorID
		bool insertCommand(BusPacket* command, bool critical);
		bool ACTdiff;
		bool isPerRequestor();
		// Check size of buffer
		unsigned int getRequestorIndex();
		unsigned int getRequestorSize(unsigned int index);
		unsigned int getSize(bool critical);

		// Access the cmd on top of each cmd queue
		BusPacket* getRequestorCommand(unsigned int index);
		BusPacket* getCommand(bool critical);
		BusPacket* checkCommand(bool critical, unsigned int index);

		// Remove most recently accessed cmd (from requestor queue or general fifo)
		void removeCommand(unsigned int requestorID);
		void removeCommand();
		void setACT(unsigned int x);
		
	private:
		// Indicate if perRequestor buffer is enabled
		bool perRequestorEnable;
		// Indicate the last accessed queue : critical or nonCritical
		bool scheduledQueue;
		bool requestorQueue;
		unsigned int requestorIndex;
		// Order of requestorID in the buffer map; used to determine the requestor scheduling order and find in the bufferMap
		std::vector<unsigned int> requestorMap;
		// Dynamic allocate buffer to individual requestors that share the same resource level
		std::map< unsigned int, std::vector<BusPacket*> > requestorBuffer;
		std::map< unsigned int, std::vector<BusPacket*> > requestorBuffer_RT;
		// General buffer for critical cmds
		std::vector<BusPacket*> hrtBuffer;
		// General buffer for nonCritical cmds
		std::vector<BusPacket*> srtBuffer;
	};
}

#endif /* COMMANDQUEUE_H */
