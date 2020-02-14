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
		CommandGenerator(unsigned int dataBus);
		virtual ~CommandGenerator();
		virtual bool commandGenerate(Request* request, bool open) = 0;

		BusPacket* getCommand();
		void removeCommand();
		unsigned int bufferSize();

	protected:
		unsigned int dataBusSize;
		bool first[8] = {true,true,true,true,true,true,true,true};
		std::map<unsigned int, std::pair<unsigned int, unsigned int>>lookupTable;
		std::queue< BusPacket* > commandBuffer;
	};
}

#endif /* COMMANDGENERATOR_H */
