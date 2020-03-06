#include "RequestScheduler.h"
#include "CommandGenerator.h"
#include "CommandScheduler.h"
#include "BusPacket.h"

#include "../generalScheduler/RequestScheduler_DIRECT.h"
#include "../system/FRFCFS/RequestScheduler_FRFCFS.h"
#include "../system/FRFCFS_Batch/RequestScheduler_FRFCFS_Batching.h"
#include "../system/FCFS/RequestScheduler_FCFS.h"
#include "../generalScheduler/CommandScheduler_FCFS.h"
#include "../generalScheduler/CommandScheduler_PUSH.h"
#include "../generalScheduler/RequestScheduler_RR.h"
#include "../generalScheduler/CommandGenerator_OPEN.h"
#include "../system/Round/CommandGenerator_Round.h"
#include "../generalScheduler/CommandGenerator_CLOSE.h"
#include "../system/FRFCFS/CommandScheduler_FRFCFS.h"
#include "../system/BLISS/RequestScheduler_BLISS.h" 
#include "../system/PAR-BS/RequestScheduler_PAR-BS.h" 
#include "../system/Round/CommandScheduler_Round.h"
#include "../system/RTMem/RequestScheduler_RTMem.h"
#include "../system/RTMem/CommandScheduler_RTMem.h"
#include "../system/PMC/RequestScheduler_PMC.h"
#include "../system/ORP/CommandScheduler_ORP.h"
#include "../system/MEDUSA/RequestScheduler_MEDUSA.h"
#include "../system/DCmc/CommandScheduler_DCmc.h"
#include "../system/MAG/CommandScheduler_MAG.h"
#include "../system/ReOrder/CommandScheduler_ReOrder.h"
#include "../system/ROC/CommandScheduler_ROC.h"
#include "../system/MCMC/RequestScheduler_MCMC.h"
#include "../system/MCMC/CommandGenerator_MCMC.h"
#include "../system/RankReOrder/CommandScheduler_RankReOrder.h"
#include "../system/PipeCAS/CommandGenerator_PipeCAS.h"
#include "../system/PipeCAS/CommandScheduler_PIPECAS.h"

namespace MCsim
{
	class SchedulerRegister
	{
	public:
		SchedulerRegister(unsigned int dataBus, const map<unsigned int, bool>& requestorTable, 
			vector<RequestQueue*>& requestQueues, vector<CommandQueue*>& commandQueues)
		{
			// User define the initialized scheduler and components for easy access
			requestSchedulerTable["DIRECT"] = new RequestScheduler_Direct(requestQueues, commandQueues, requestorTable);
			requestSchedulerTable["RR"] = new RequestScheduler_RR(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["FCFS"] = new RequestScheduler_FCFS(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["FRFCFS"] = new RequestScheduler_FRFCFS(requestQueues, commandQueues, requestorTable);
			requestSchedulerTable["FRFCFS_Batching"] = new RequestScheduler_FRFCFS_BATCHING(requestQueues, commandQueues, requestorTable);
			requestSchedulerTable["BLISS"] = new RequestScheduler_BLISS(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["PARBS"] = new RequestScheduler_PARBS(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["RTMem"] = new RequestScheduler_RTMem(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["PMC"] = new RequestScheduler_PMC(requestQueues, commandQueues, requestorTable);
			requestSchedulerTable["MCMC"] = new RequestScheduler_MCMC(requestQueues, commandQueues, requestorTable, dataBus);
			requestSchedulerTable["MEDUSA"] = new RequestScheduler_MEDUSA(requestQueues, commandQueues, requestorTable);
			commandGeneratorTable["OPEN"] = new CommandGenerator_Open(dataBus);
			commandGeneratorTable["ROUND"] = new CommandGenerator_Round(dataBus);
			commandGeneratorTable["CLOSE"] = new CommandGenerator_Close(dataBus);
			commandGeneratorTable["MCMC"] = new CommandGenerator_MCMC(dataBus);
			commandGeneratorTable["PipeCAS"] = new CommandGenerator_PipeCAS(dataBus, requestorTable);
			commandSchedulerTable["RTMem"] = new CommandScheduler_RTMem(commandQueues, requestorTable);
			commandSchedulerTable["Round"] = new CommandScheduler_Round(commandQueues, requestorTable);
			commandSchedulerTable["FRFCFS"] = new CommandScheduler_FRFCFS(commandQueues, requestorTable);
			commandSchedulerTable["ORP"] = new CommandScheduler_ORP(commandQueues, requestorTable);
			commandSchedulerTable["DCmc"] = new CommandScheduler_DCmc(commandQueues, requestorTable);
			commandSchedulerTable["MAG"] = new CommandScheduler_MAG(commandQueues, requestorTable);
			commandSchedulerTable["ReOrder"] = new CommandScheduler_ReOrder(commandQueues, requestorTable);
			commandSchedulerTable["ROC"] = new CommandScheduler_ROC(commandQueues, requestorTable);
			commandSchedulerTable["FCFS"] = new CommandScheduler_FCFS(commandQueues, requestorTable);
			commandSchedulerTable["PUSH"] = new CommandScheduler_PUSH(commandQueues, requestorTable);
			commandSchedulerTable["PipeCAS"] = new CommandScheduler_PIPECAS(commandQueues, requestorTable, 0);
			commandSchedulerTable["rankReOrder"] = new CommandScheduler_RankReOrder(commandQueues, requestorTable);
		}

		~SchedulerRegister()
		{
			for(auto it=requestSchedulerTable.begin(); it!=requestSchedulerTable.end(); it++) {
				delete it->second;
			}
			requestSchedulerTable.clear();
			for(auto it=commandSchedulerTable.begin(); it!=commandSchedulerTable.end(); it++) {
				delete it->second;
			}
			commandSchedulerTable.clear();
			for(auto it=commandGeneratorTable.begin(); it!=commandGeneratorTable.end(); it++) {
				delete it->second;
			}
			commandGeneratorTable.clear();
		}

		RequestScheduler* getRequestScheduler(string schedulerName) {
			if(requestSchedulerTable.find(schedulerName) == requestSchedulerTable.end()) {
				DEBUG("NO SUCH REQUEST SCHEDULER "<<schedulerName);
				abort();
			}
			return requestSchedulerTable[schedulerName];
		}
		CommandGenerator* getCommandGenerator(string schedulerName) {
			if(commandGeneratorTable.find(schedulerName) == commandGeneratorTable.end()) {
				DEBUG("NO SUCH COMMAND GENERATOR");
				abort();
			}			
			return commandGeneratorTable[schedulerName];
		}	
		CommandScheduler* getCommandScheduler(string schedulerName) {
			if(commandSchedulerTable.find(schedulerName) == commandSchedulerTable.end()) {
				DEBUG("NO SUCH COMMAND SCHEDULER");
				abort();
			}			
			return commandSchedulerTable[schedulerName];
		}
		
	private:
		std::map<std::string, RequestScheduler*> requestSchedulerTable;
		std::map<std::string, CommandGenerator*> commandGeneratorTable;
		std::map<std::string, CommandScheduler*> commandSchedulerTable;
	};
}


