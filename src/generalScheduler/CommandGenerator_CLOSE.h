#ifndef COMMANDGENERATOR_CLOSE_H
#define COMMANDGENERATOR_CLOSE_H

#include "../src/CommandGenerator.h"

namespace MCsim
{
	class CommandGenerator_Close: public CommandGenerator
	{
	public:
		CommandGenerator_Close(unsigned int dataBus):
			CommandGenerator(dataBus)
		{
			// Crack the requests size if needed more particles - For instace AMC utilizes this feature
			lookupTable[dataBus] = make_pair(1,1);
			lookupTable[dataBus*2] = make_pair(2,1);
			lookupTable[dataBus*4] = make_pair(4,1);
			lookupTable[dataBus*8] = make_pair(8,1);
			lookupTable[dataBus*16] = make_pair(8,2);	
		}

		// The command generator generate the request type
		bool commandGenerate(Request* request, bool open)
		{
			// Since it is close page, RD and WR will be inserted attached to a auto precharge (RDA or WRA)
			BusPacketType CAS = RDA;
			if(request->requestType == DATA_WRITE) {
				CAS = WRA;
			}
			unsigned id = request->requestorID;
			unsigned address = request->address;
			unsigned rank = request->rank; //  Can be disabled (0) if there is one rank			
			unsigned row = request->row;
			unsigned col = request->col;

			for(unsigned int bi = 0; bi < lookupTable[request->requestSize].first; bi++)
			{
				unsigned bank = (request->bank + bi)%16;
				for(unsigned int bc = 0; bc < lookupTable[request->requestSize].second; bc++)
				{
					// Crack the request to the DRAM commands. It's close page -> ACT + RDA || ACT + WRA
					commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, 0, 0));
					commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank, rank, request->data, 0));
				}
			}
			return true;
		}
	};
}

#endif /* COMMANDGENERATOR_CLOSE_H */