
#ifndef COMMANDSCHEDULER_FRFCFS_H
#define COMMANDSCHEDULER_FRFCFS_H

#include "../../src/CommandScheduler.h"
using namespace std;

namespace MCsim
{
	class CommandScheduler_FRFCFS: public CommandScheduler
	{
	private:
		map<unsigned int, bool> queuePending;
	public:
		CommandScheduler_FRFCFS(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable)
		{
			fout.open("dump_result.txt");
		}

		BusPacket* commandSchedule()
		{
			
			scheduledCommand = NULL;
			checkCommand = NULL;
			checkCommand_temp_1 = NULL;
			checkCommand_temp_2 = NULL;
			index_temp = 0;
			for(unsigned int index = 0; index < commandQueue.size(); index++)
			{
				if(commandQueue[index]->isPerRequestor()) 
				{	
					if(commandQueue[index]->getRequestorIndex() > 0) // There is more than 0 requestor in the design
					{		
						for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
						{
							if(commandQueue[index]->getRequestorSize(num) > 0 ) // Return the buffer size of the requestor
							{
								checkCommand = commandQueue[index]->getRequestorCommand(num);
								if(checkCommand != NULL) 
								{
									if(checkCommand->busPacketType == RD || checkCommand->busPacketType == WR)
									{
										if(isReady(checkCommand,index))
										{
											if(isIssuable(checkCommand))
											{
												checkCommand_temp_1 = checkCommand;
												break;
											}
										}	
									}	
								}
								checkCommand = NULL;
							}
						}
						if(checkCommand_temp_1 != NULL){
							for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
							{
								if(commandQueue[index]->getRequestorSize(num) > 0 ) // Return the buffer size of the requestor
								{
									checkCommand = commandQueue[index]->getRequestorCommand(num);
									if(checkCommand != NULL) 
									{
										if(checkCommand->busPacketType == RD || checkCommand->busPacketType == WR)
										{
											if(isReady(checkCommand,index))
											{
												if(isIssuable(checkCommand))
												{
													if(checkCommand->arriveTime < checkCommand_temp_1->arriveTime)
													{
														checkCommand_temp_1 = checkCommand;
													}
												}
											}	
										}	
									}
									checkCommand = NULL;
								}
							}
						}						
						if(checkCommand_temp_1 != NULL)
						{		
							scheduledCommand = checkCommand_temp_1;
							sendCommand(scheduledCommand, index, false);					
							return scheduledCommand;
						}
						else
						{
							for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
							{
								if(commandQueue[index]->getRequestorSize(num) > 0 ) // Return the buffer size of the requestor
								{
									checkCommand = commandQueue[index]->getRequestorCommand(num);
									if(checkCommand != NULL) 
									{
										if(checkCommand->busPacketType == ACT || checkCommand->busPacketType == PRE)
										{
											if(isReady(checkCommand,index))
											{
												if(isIssuable(checkCommand))
												{
													checkCommand_temp_1 = checkCommand;													
												}	
											}	
										}	
									}
									checkCommand = NULL;
								}
							}
							for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) // For all requestors from "num". getRequestorIndex() gives the number of requestors
							{
								if(commandQueue[index]->getRequestorSize(num) > 0 ) // Return the buffer size of the requestor
								{
									checkCommand = commandQueue[index]->getRequestorCommand(num);
									if(checkCommand != NULL) 
									{
										if(checkCommand->busPacketType == ACT || checkCommand->busPacketType == PRE)
										{
											if(isReady(checkCommand,index))
											{
												if(isIssuable(checkCommand))
												{
													if(checkCommand->arriveTime < checkCommand_temp_1->arriveTime)
													{
														checkCommand_temp_1 = checkCommand;
													}													
												}	
											}	
										}	
									}
									checkCommand = NULL;
								}
							}
							if(checkCommand_temp_1 != NULL)
							{		
								scheduledCommand = checkCommand_temp_1;
								sendCommand(scheduledCommand, index, false);					
								return scheduledCommand;
							}	
						}		
					}						
				}
				else
				{
					if(commandQueue[index]->getSize(true) > 0)
					{
						checkCommand = commandQueue[index]->getCommand(true);
						if(checkCommand != NULL)
						{								
							if(isReady(checkCommand,index))
							{
								if(isIssuable(checkCommand))
								{
									if(checkCommand_temp_2 != NULL){
										if((checkCommand->busPacketType == RD || checkCommand->busPacketType == WR)){
											if(checkCommand->arriveTime < checkCommand_temp_2->arriveTime){
											checkCommand_temp_2 = checkCommand;
											index_temp = index;
											}
										}
										else if(checkCommand->busPacketType == ACT){
											if(checkCommand_temp_2->busPacketType == ACT){
												if(checkCommand->arriveTime < checkCommand_temp_2->arriveTime){
													checkCommand_temp_2 = checkCommand;
													index_temp = index;
												}	
											}
											else if(checkCommand_temp_2->busPacketType == PRE){
												checkCommand_temp_2 = checkCommand;
												index_temp = index;
											}
										}	
										else if(checkCommand->busPacketType == PRE){
											if(checkCommand_temp_2->busPacketType == PRE){
												if(checkCommand->arriveTime < checkCommand_temp_2->arriveTime){
													checkCommand_temp_2 = checkCommand;
													index_temp = index;
												}
											}		
										}
									}
									else
									{
										checkCommand_temp_2 = checkCommand;
										index_temp = index;
									}																													
								}									
							}
						}	
						checkCommand = NULL;	
					}
					if(index == commandQueue.size()-1){
						if(checkCommand_temp_2 != NULL){
							scheduledCommand = checkCommand_temp_2;
							checkCommand_temp_2 = NULL;
							sendCommand(scheduledCommand, index_temp,false);
							return scheduledCommand;
						}						
					}
				}				
			}
			return scheduledCommand;
		}
	};
}
#endif /* COMMANDSCHEDULER_FRFCFS_H */
