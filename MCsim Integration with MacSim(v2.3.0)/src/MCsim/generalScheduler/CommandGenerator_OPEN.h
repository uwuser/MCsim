#ifndef COMMANDGENERATOR_OPEN_H
#define COMMANDGENERATOR_OPEN_H

#include "../src/CommandGenerator.h"

namespace MCsim
{
	class CommandGenerator_Open: public CommandGenerator
	{
	public:
		CommandGenerator_Open(unsigned int dataBus,std::vector<CommandQueue*>& commandQueues):
			CommandGenerator(dataBus,commandQueues) {}			

		bool commandGenerate(Request* request, bool open)
		{			
			BusPacketType CAS = RD;
			if(request->requestType == DATA_WRITE) {
				CAS = WR;
			}
			unsigned size = request->requestSize/dataBusSize; 
			unsigned id = request->requestorID;
			unsigned address = request->address;
			unsigned rank = request->rank; // 0 *****;
			unsigned bank = request->bank;
			unsigned row = request->row;
			unsigned col = request->col;	
			unsigned sa = request->subArray;	

			// Crack the request to the DRAM command depending on being open or close 			
			if(!open && !first[request->bank]) { // Assuming the initial state of the banks in device is idle
				commandBuffer.push(new BusPacket(PRE, id, address, col, row, bank, rank, sa, NULL, 0));
				commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, sa, NULL, 0));
			}
			else if(!open && first[request->bank]) {
				commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, sa, NULL, 0));
				first[request->bank] = false;
			}
			for(unsigned int x = 0; x < size; x++) {
				commandBuffer.push(new BusPacket(CAS, id, address, col+x, row, bank, rank, sa, request->data, 0));
			}
			return true;
		}
	};
}

#endif /* COMMANDGENERATOR_OPEN_H*/