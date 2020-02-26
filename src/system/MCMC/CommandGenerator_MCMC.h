#ifndef COMMANDGENERATOR_MCMC_H
#define COMMANDGENERATOR_MCMC_H

#include "../../src/CommandGenerator.h"

namespace MCsim
{
	class CommandGenerator_MCMC: public CommandGenerator
	{
	public:
		CommandGenerator_MCMC(unsigned int dataBus):
			CommandGenerator(dataBus)
		{
			lookupTable[dataBus] = make_pair(1,1);
			lookupTable[dataBus*2] = make_pair(2,1);
			lookupTable[dataBus*4] = make_pair(4,1);	
		}

		// The command generator should know which command queue to check for schedulability
		bool commandGenerate(Request* request, bool open)
		{
			BusPacketType CAS = RDA;
			if(request->requestType == DATA_WRITE) {
				CAS = WRA;
			}
			unsigned id = request->requestorID;
			unsigned address = request->address;
			unsigned rank = request->rank;
			unsigned bank = request->bank;
			unsigned row = request->row;
			unsigned col = request->col;

			unsigned size = request->requestSize/dataBusSize; 
			unsigned interleave = 1;
			unsigned time = 1;
			if(size <= 4) {
				interleave = size;
			}
			else {
				DEBUG("MAXIMUM INTERLEAVING THROUGH 4 BANKS");
				abort();
			}
			for(unsigned int i=0; i < time; i++) 
			{
				switch(interleave) {
					case 1:
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, 0, 0));
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank, rank, request->data, 0));
						commandBuffer.back()->postCommand = true;
						break;
					case 2:
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, 0, 0));
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank+1, rank, 0, 0));	
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank, rank, request->data, 0));
						commandBuffer.back()->postCommand = true;
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank+1, rank, request->data, 0));	
						commandBuffer.back()->postCommand = true;
						break;					
					case 4:
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, 0, 0));
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank+1, rank, 0, 0));	
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank, rank, request->data, 0));	
						commandBuffer.back()->postCommand = true;
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank+2, rank, 0, 0));
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank+1, rank, request->data, 0));
						commandBuffer.back()->postCommand = true;
						commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank+3, rank, 0, 0));	
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank+2, rank, request->data, 0));	
						commandBuffer.back()->postCommand = true;
						commandBuffer.push(new BusPacket(CAS, id, address, col, row, bank+3, rank, request->data, 0));	
						commandBuffer.back()->postCommand = true;
						break;	
					default:
						DEBUG("MCMC CONTROLLER: NOT SUPPORTED SIZE");
						abort();
				}
			}
			return true;
		}
	};
}

#endif /* COMMANDGENERATOR_MCMC_H */