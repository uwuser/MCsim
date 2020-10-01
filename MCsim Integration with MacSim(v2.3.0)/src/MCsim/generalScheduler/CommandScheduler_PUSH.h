
#ifndef COMMANDSCHEDULER_PUSH_H
#define COMMANDSCHEDULER_PUSH_H

#include "../src/CommandScheduler.h"


namespace MCsim
{
	class CommandScheduler_PUSH: public CommandScheduler
	{
	private:
	public:
		CommandScheduler_PUSH(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable){}		
		BusPacket* commandSchedule()
		{			
			checkCommand = NULL;
			checkCommand_temp_1 = NULL;		
			
			unsigned int index_temp = 0;
			for(unsigned int index = 0; index < commandQueue.size(); index++) // Take the first command that is ready and issueable 
			{					
				scheduledCommand = NULL;				
				if(commandQueue[index]->getSize(true) > 0)
				{						
					checkCommand = commandQueue[index]->getCommand(true);
					if(checkCommand != NULL)
					{																
						if(isReady(checkCommand,index))   // If the command is intra-ready
						{							
							if(isIssuable(checkCommand))   // If the command is inter-ready
							{																	
								checkCommand_temp_1 = checkCommand;
								index_temp = index;
							}									
						}
					}					
					checkCommand = NULL;							
				}
			}
			for(unsigned int index = 0; index < commandQueue.size(); index++) // Take the first command that is ready and issueable 
			{					
				scheduledCommand = NULL;				
				if(commandQueue[index]->getSize(true) > 0)
				{						
					checkCommand = commandQueue[index]->getCommand(true);
					if(checkCommand != NULL)
					{																
						if(isReady(checkCommand,index))   // If the command is intra-ready
						{							
							if(isIssuable(checkCommand))   // If the command is inter-ready
							{
								if(checkCommand_temp_1->arriveTime >= checkCommand->arriveTime){
									checkCommand_temp_1 = checkCommand;
									index_temp = index;
								}																								
							}									
						}
					}					
					checkCommand = NULL;							
				}
			}		
			if(checkCommand_temp_1 != NULL){
				scheduledCommand = checkCommand_temp_1;
				sendCommand(scheduledCommand,index_temp, false);
				return scheduledCommand;
			}	
			return scheduledCommand;
		}						
	};
}

#endif /* COMMANDSCHEDULER_FCFS_H */
