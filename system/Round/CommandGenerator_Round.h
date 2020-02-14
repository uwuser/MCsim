#ifndef COMMANDGENERATOR_Round_H
#define COMMANDGENERATOR_Round_H

#include "../../src/CommandGenerator.h"

namespace MCsim
{
	class CommandGenerator_Round: public CommandGenerator
	{
	public:
		CommandGenerator_Round(unsigned int dataBus):CommandGenerator(dataBus)
		{}
		// The command generator should know which command queue to check for schedulability
		bool commandGenerate(Request* request, bool open)
		{
			int actdefine = 0;
			BusPacketType CAS = RD;
			if(request->requestType == DATA_WRITE) {
				CAS = WR;
				actdefine = 1;
			}
			unsigned size = request->requestSize/dataBusSize; 
			unsigned id = request->requestorID;
			unsigned long long address = request->address;
			unsigned rank = 0;//request->rank;
			unsigned bank = request->bank;
			unsigned row = request->row;
			unsigned col = request->col;
			if(!open && !first[request->bank]) 
			{
				if (actdefine == 0){
					commandBuffer.push(new BusPacket(PRE, id, address, 0, row, bank, rank, NULL, 0));
					commandBuffer.push(new BusPacket(ACT_R, id, address, 0, row, bank, rank, NULL, 0));	
				}
				else{
					commandBuffer.push(new BusPacket(PRE, id, address, 0, row, bank, rank, NULL, 0));
					commandBuffer.push(new BusPacket(ACT_W, id, address, 0, row, bank, rank, NULL, 0));		
				}
			}
			else if(!open && first[request->bank]) 
			{		
				if (actdefine == 0){
					
					commandBuffer.push(new BusPacket(ACT_R, id, address, 0, row, bank, rank, NULL, 0));	
					first[request->bank] = false;
				}
				else{
					commandBuffer.push(new BusPacket(ACT_W, id, address, 0, row, bank, rank, NULL, 0));		
					first[request->bank] = false;
				}
			}
			for(unsigned int x = 0; x < size; x++) {
				commandBuffer.push(new BusPacket(CAS, id, address, col+size, row, bank, rank, request->data, 0));
			}
			return true;					
		}
	};
}
#endif /* COMMANDGENERATOR_Round_H */
