#include "BusPacket.h"
#include "CommandQueue.h"

#include <vector>
using namespace std;

namespace MCsim
{
	class RefreshMachine
	{
	public:
		RefreshMachine(const vector<CommandQueue*>& commandQueues, size_t ranks, size_t banks, size_t tREFI, size_t tRFCpb, size_t tRFCab, size_t tRFC, size_t tRCD):
			commandQueue(commandQueues),
			ranks(ranks),
			banks(banks),
			tREFI(tREFI),
			tRFCpb(tRFCpb),
			tRFCab(tRFCab),
			tRFC(tRFC),
			tRCD(tRCD)

		{			
			interval_pb = tREFI/8;
			Round_robin = 0;
			globeRefreshCountdown = interval_pb;
			isRefreshing = false;
			isInterval = false;
			flag = false;
			refresh_mechanism = "none";  // none; distributed;  per-bank (use DSARP with SA)
			
		}
		virtual ~RefreshMachine()
		{
			commandBuffer.clear();
		}

		bool refreshing()
		{
			return isRefreshing;
		}
		bool reachInterval()
		{
			return isInterval;
		}
		bool busyBank(unsigned int r_bank)
		{
			return isRefreshingBank[r_bank];
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
				isInterval = false;
				isRefreshing = false;
				if(commandBuffer.size() != 0)
					refCmd = commandBuffer.front();
				if(refresh_mechanism == "per-bank")
					isRefreshingBank[bank_release] = false;
				if(refresh_mechanism == "distributed"){
					for(size_t bank_i =0 ; bank_i<banks; bank_i++)
						isRefreshingBank[bank_i] = false;
				}
			}
		}
		
		void popCommand()
		{
			if(commandBuffer.front()->busPacketType == REFPB) {
				bank_release = commandBuffer.front()->bank;
				refreshCounting = tRFCpb;
				flag = true;
			}
			else if(commandBuffer.front()->busPacketType == REF) {
				bank_release = commandBuffer.front()->bank;
				refreshCounting = tRFC;
				flag = true;
			}
			if(commandBuffer.size() != 0){
				BusPacket* tempCmd = commandBuffer.front();
				commandBuffer.erase(commandBuffer.begin());
				if(commandBuffer.front()->busPacketType == ACT)
				{	
					refreshCounting = refreshCounting + tRCD;
				}
				delete tempCmd;
			}	
		}
		void commandGenerate()
		{
			targetbank = false;			
			
			if(refresh_mechanism == "distributed"){
				for(size_t rank=0; rank<ranks; rank++) {
					commandBuffer.push_back(new BusPacket(PREA, 0, 0, 0, 0, 0, rank, 0, NULL, 0));
					commandBuffer.push_back(new BusPacket(REF, 0, 0, 0, 0, 0, rank, 0, NULL, 0));
					for(size_t bank_i =0 ; bank_i<banks; bank_i++)
						isRefreshingBank[bank_i] = true;
				}
			}
			for(unsigned index=0; index < commandQueue.size(); index++)
			{		
				if(commandQueue[index]->getSize(true) > 0 ) 
				{
					BusPacket* tempCmd = NULL;
					tempCmd = commandQueue[index]->getCommand(true);		
					if(tempCmd!=NULL)
					{
						if(tempCmd->busPacketType < ACT) 
						{	
							if(refresh_mechanism == "distributed")
							{
								commandBuffer.push_back(new BusPacket(ACT, 0, 0, 0, tempCmd->row, tempCmd->bank, tempCmd->rank, 0, NULL, 0));
							}
							if(refresh_mechanism == "per-bank")
							{
								if(tempCmd->bank == Round_robin)		
								{	
									commandBuffer.push_back(new BusPacket(PRE, 0, 0, 0, 0, Round_robin, 0, 0, NULL, 0));
									commandBuffer.push_back(new BusPacket(REFPB, 0, 0, 0, 0, Round_robin, 0, 0, NULL, 0));	
									commandBuffer.push_back(new BusPacket(ACT, 0, 0, 0, tempCmd->row, tempCmd->bank, tempCmd->rank,0, NULL, 0));
									targetbank = true;
									isRefreshingBank[Round_robin] = true;
								}	
							}
						}			
					}	
				}	
			}
			if(refresh_mechanism == "per-bank")
			{
				if(!targetbank){
					commandBuffer.push_back(new BusPacket(REFPB, 0, 0, 0, 0, Round_robin, 0, 0, NULL, 0));	
					isRefreshingBank[Round_robin] = true;
				}	
				Round_robin++;	
				if(Round_robin == 8)
					Round_robin = 0;		
			}		
		}
				
		void step()
		{
			if(refresh_mechanism != "none")
				globeRefreshCountdown--; // To disable the refresh machine comment this line	
			if(globeRefreshCountdown == 0) {
				isRefreshing = true;
				isInterval = true;
				if(refresh_mechanism == "per-bank")
					globeRefreshCountdown = interval_pb;				
				if(refresh_mechanism == "distributed")
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
		std::map< unsigned int, bool > isRefreshingBank;

		size_t ranks;
		size_t banks;
		size_t tREFI;
		size_t tRFCpb;
		size_t tRFCab;
		size_t tRFC;
		size_t tRCD;

		string refresh_mechanism;

		bool isRefreshing;
		bool isInterval;
		unsigned int Round_robin;
		bool targetbank;
		bool flag;
		bool smartRef;
		unsigned int bank_release;
		unsigned int interval_pb;
		unsigned globeRefreshCountdown;
		unsigned refreshCounting;
	};
}
