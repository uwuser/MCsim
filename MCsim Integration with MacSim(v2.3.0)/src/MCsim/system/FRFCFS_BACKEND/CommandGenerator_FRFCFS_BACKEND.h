#ifndef COMMANDGENERATOR_FRFCFS_BACKEND_H
#define COMMANDGENERATOR_FRFCFS_BACKEND_H

#include "../../src/CommandGenerator.h"
namespace MCsim
{
	class CommandGenerator_FRFCFS_BACKEND: public CommandGenerator
	{
	public:
		CommandGenerator_FRFCFS_BACKEND(unsigned int dataBus,std::vector<CommandQueue*>& commandQueues):
			CommandGenerator(dataBus,commandQueues) {}			

		bool commandGenerate(Request* request, bool open)
		{			
			BusPacketType CAS = RD;
			if(request->requestType == DATA_WRITE) {
				CAS = WR;
			}
			//cout<<"---------------------------------Inside Command Generator---------------------------------"<<endl;
			unsigned size = request->requestSize/dataBusSize; 
			unsigned id = request->requestorID;
			unsigned long long address = request->address;
			unsigned rank = request->rank; // 0 *****;
			unsigned bank = request->bank;
			unsigned row = request->row;
			unsigned col = request->col;	
			unsigned sa = request->subArray;
			//cout<<"11 the id is  "<<id<<"    "<<commandQueue[id]->getSize(true)<<endl;	
			if(commandQueue[id]->getSize(true) == 0)
			{
				//cout<<"generating the command"<<endl;
				// Crack the request to the DRAM command depending on being open or close 			
				if(!open) { // Assuming the initial state of the banks in device is idle
					commandBuffer.push(new BusPacket(PRE, id, address, col, row, bank, rank, sa, NULL, 0));
					commandBuffer.push(new BusPacket(ACT, id, address, col, row, bank, rank, sa, NULL, 0));
				}
				for(unsigned int x = 0; x < size; x++) {
					commandBuffer.push(new BusPacket(CAS, id, address, col+x, row, bank, rank, sa, request->data, 0));
				}
			}
			else if	(commandQueue[id]->getSize(true) > 0)
			{
				//cout<<"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT    for   "<<id<<endl;				
				return false;
			}
			//cout<<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"<<endl;
			return true;
		}
	};
}

#endif /* COMMANDGENERATOR_OPEN_H*/