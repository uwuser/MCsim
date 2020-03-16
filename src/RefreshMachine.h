#include "BusPacket.h"
#include "CommandQueue.h"

#include <vector>
using namespace std;

namespace MCsim
{
	class RefreshMachine
	{
	public:
		RefreshMachine(const vector<CommandQueue*>& commandQueues, size_t ranks, size_t banks, size_t tREFI, size_t tRFC):
			commandQueue(commandQueues),
			ranks(ranks),
			// banks(banks),
			tREFI(tREFI),
			tRFC(tRFC)
		{
			globeRefreshCountdown = tREFI;
			isRefreshing = false;
			flag = false;
		}
		virtual ~RefreshMachine()
		{
			commandBuffer.clear();
		}

		bool refreshing()
		{
			return isRefreshing;
		}
	
		void refresh(BusPacket* &refCmd)
		{
			if(!commandBuffer.empty()) {
				if(!flag){
					refCmd = commandBuffer.front();
				}
			}
			
			if(refreshCounting == 0 && flag ==true) {
				flag = false;
				isRefreshing = false;
			}
		}
		
		void popCommand()
		{
			if(commandBuffer.front()->busPacketType == REF) {
				refreshCounting = tRFC;
				flag = true;
			}
			BusPacket* tempCmd = commandBuffer.front();
			commandBuffer.erase(commandBuffer.begin());
			delete tempCmd;
		}
		void commandGenerate()
		{
			for(size_t rank=0; rank<ranks; rank++) {
				commandBuffer.push_back(new BusPacket(PREA, 0, 0, 0, 0, rank, 0, NULL, 0));
				commandBuffer.push_back(new BusPacket(REF, 0, 0, 0, 0, rank, 0, NULL, 0));
			}
			for(unsigned index=0; index < commandQueue.size(); index++) {
				/* This part needs to be determined arbitrary depending on the controller wether using Cr/NCr or not
				BusPacket* tempCmd = commandQueue[index]->getCommand(true);
				if( tempCmd == NULL) {
					tempCmd = commandQueue[index]->getCommand(false);
				}
				*/
				for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) 
				{
					if(commandQueue[index]->getRequestorSize(num) > 0 ) 
					{
						BusPacket* tempCmd = NULL;
						tempCmd = commandQueue[index]->getRequestorCommand(num);				
						if(tempCmd!=NULL)
						{
							if(commandQueue[0]->ACTdiff == false)
							{
								if(tempCmd->busPacketType < ACT) 
								{
									commandBuffer.push_back(new BusPacket(ACT, 0, 0, 0, tempCmd->row, tempCmd->bank, tempCmd->rank, NULL, 0));
								}
							}							
							else
							{
								if(tempCmd->busPacketType == RD || tempCmd->busPacketType == RDA) 
								{
									commandBuffer.push_back(new BusPacket(ACT_R, 0, 0, 0, tempCmd->row, tempCmd->bank, tempCmd->rank, NULL, 0));
								}
								else if(tempCmd->busPacketType == WR || tempCmd->busPacketType == WRA) 
								{
									commandBuffer.push_back(new BusPacket(ACT_W, 0, 0, 0, tempCmd->row, tempCmd->bank, tempCmd->rank, NULL, 0));
								}
							}
						}	
					}		
				}
			}
		}
				
		void step()
		{
			//globeRefreshCountdown--; // To disable the refresh machine comment this line
			if(globeRefreshCountdown == 0) {
				isRefreshing = true;
				globeRefreshCountdown = tREFI;
				// Generate refresh commands 
				commandGenerate();
			}
			if(refreshCounting != 0) {
				refreshCounting--;
			}	
		}

	private:
		// connect to commandQueue so that compensate commands can be generated
		const vector<CommandQueue*>& commandQueue;
		std::vector<BusPacket*> commandBuffer;

		size_t ranks;
		// size_t banks;
		size_t tREFI;
		size_t tRFC;
		bool isRefreshing;
		bool flag;
		unsigned globeRefreshCountdown;
		unsigned refreshCounting;
	};
}
