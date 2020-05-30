
#ifndef COMMANDSCHEDULER_FCFS_H
#define COMMANDSCHEDULER_FCFS_H

#include "../src/CommandScheduler.h"


namespace MCsim
{
	class CommandScheduler_FCFS: public CommandScheduler
	{
	private:
	public:
		CommandScheduler_FCFS(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable){}		
		BusPacket* commandSchedule()
		{			
			checkCommand = NULL;
			checkCommand_temp_1 = NULL;			
			for(unsigned int index = 0; index < commandQueue.size(); index++) // Take the first command that is ready and issueable 
			{				
				if(commandQueue[index]->isPerRequestor())  // If using per requestor queue structure for the commandQueues
				{
					if(commandQueue[index]->getRequestorIndex() > 0) // There is at least one requestor in the system
					{
						for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
						{
							if(commandQueue[index]->getRequestorSize(num) > 0 ) // Return the buffer size of the requestor
							{
								checkCommand = commandQueue[index]->getRequestorCommand(num);
								if(isReady(checkCommand, index)) // If the command is intra-ready
								{
									if(isIssuable(checkCommand))  // If the command is issue able (inter-ready)
									{
										checkCommand_temp_1 = checkCommand;
										checkCommand = NULL;
										break;
									}												
								}
								checkCommand = NULL;								
							}
						}
						// Check if there exist any other command that is ready and issueable but is arrived earlier
						for(unsigned int num_1 = 0; num_1 < commandQueue[index]->getRequestorIndex(); num_1++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
						{
							if(commandQueue[index]->getRequestorSize(num_1) > 0 ) // Return the buffer size of the requestor
							{
								checkCommand = commandQueue[index]->getRequestorCommand(num_1);			
								if(isReady(checkCommand, index)) // If the command is intra-ready
								{
									if(isIssuable(checkCommand)) // If the command is inter-ready
									{
										if(checkCommand_temp_1->arriveTime >= checkCommand->arriveTime)
										{
											checkCommand_temp_1 = checkCommand;
											checkCommand = NULL;
										}
									}
								}
								checkCommand = NULL;	
							}
						}
						if(checkCommand_temp_1 != NULL){
							scheduledCommand = checkCommand_temp_1;
							sendCommand(scheduledCommand,index, false);
							return scheduledCommand;
						}					
					}
				}
				else
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
									scheduledCommand = checkCommand;									
									sendCommand(scheduledCommand, index,false);
									return scheduledCommand;																					
								}									
							}
						}					
						checkCommand = NULL;							
					}	
				}			
			}
			return scheduledCommand;
		}
	};
}

#endif /* COMMANDSCHEDULER_FCFS_H */
