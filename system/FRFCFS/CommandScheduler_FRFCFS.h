/* This function implement a FRFCFS command scheduing - Meanwhile, it priotaize the CAS 
   Commands over ACT and PRE
*/ 
#ifndef COMMANDSCHEDULER_FRFCFS_H
#define COMMANDSCHEDULER_FRFCFS_H

#include "../../src/CommandScheduler.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

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
			/*
			int tRCD = getTiming("tRCD");
			cout<<"tRCD is : "<<tRCD<<endl;
			int tCCDS = getTiming("tCCDS");
			cout<<"tCCDS is : "<<tCCDS<<endl;
			int tCCDL = getTiming("tCCDL");
			cout<<"tCCDL is : "<<tCCDL<<endl;
			int tRRDL = getTiming("tRRDL");
			cout<<"tRRDL is : "<<tRRDL<<endl;
			int tRRDS = getTiming("tRRDS");
			cout<<"tRRDS is : "<<tRRDS<<endl;
			int tWL = getTiming("tWL");
			cout<<"tWL is : "<<tWL<<endl;
			*/
			scheduledCommand = NULL;
			checkCommand = NULL;
			checkCommand_1 = NULL;
			checkCommand_2 = NULL;
			index_temp = 0;
			for(unsigned int index = 0; index < commandQueue.size(); index++)
			{
				//cout<<"1"<<endl;
				if(commandQueue[index]->isPerRequestor()) 
				{	
					//cout<<"2"<<endl;
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
												checkCommand_1 = checkCommand;
												break;
											}
										}	
									}	
								}
								checkCommand = NULL;
							}
						}
						if(checkCommand_1 != NULL){
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
													if(checkCommand->arriveTime < checkCommand_1->arriveTime)
													{
														checkCommand_1 = checkCommand;
													}
												}
											}	
										}	
									}
									checkCommand = NULL;
								}
							}
						}						
						if(checkCommand_1 != NULL)
						{		
							scheduledCommand = checkCommand_1;
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
													checkCommand_1 = checkCommand;													
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
													if(checkCommand->arriveTime < checkCommand_1->arriveTime)
													{
														checkCommand_1 = checkCommand;
													}													
												}	
											}	
										}	
									}
									checkCommand = NULL;
								}
							}
							if(checkCommand_1 != NULL)
							{		
								scheduledCommand = checkCommand_1;
								sendCommand(scheduledCommand, index, false);					
								return scheduledCommand;
							}	
						}		
					}						
				}
				else
				{
					//cout<<"2"<<endl;
					if(commandQueue[index]->getSize(true) > 0)
					{
						//cout<<"3"<<endl;
						checkCommand = commandQueue[index]->getCommand(true);
						if(checkCommand != NULL)
						{		
							//cout<<"4"<<endl;						
							if(isReady(checkCommand,index))
							{
								//cout<<"5"<<endl;
								if(isIssuable(checkCommand))
								{
									//cout<<"6"<<endl;
									if(checkCommand_2 != NULL)
									{
										////cout<<"7"<<endl;
										if((checkCommand->busPacketType == RD || checkCommand->busPacketType == WR)){
											//cout<<"8"<<endl;
											if(checkCommand->arriveTime < checkCommand_2->arriveTime){
												//cout<<"9"<<endl;
											checkCommand_2 = checkCommand;
											index_temp = index;
											}
										}
										else if(checkCommand->busPacketType == ACT){
											//cout<<"10"<<endl;
											if(checkCommand_2->busPacketType == ACT){
												//cout<<"11"<<endl;
												if(checkCommand->arriveTime < checkCommand_2->arriveTime){
													//cout<<"12"<<endl;
													checkCommand_2 = checkCommand;
													index_temp = index;
												}	
											}
											else if(checkCommand_2->busPacketType == PRE){
												//cout<<"13"<<endl;
												checkCommand_2 = checkCommand;
												index_temp = index;
											}
										}	
										else if(checkCommand->busPacketType == PRE){
											//cout<<"14"<<endl;
											if(checkCommand_2->busPacketType == PRE){
												//cout<<"15"<<endl;
												if(checkCommand->arriveTime < checkCommand_2->arriveTime){
													//cout<<"16"<<endl;
													checkCommand_2 = checkCommand;
													index_temp = index;
												}
											}		
										}
										
									}
									else
									{
										//cout<<"17"<<endl;
										checkCommand_2 = checkCommand;
										index_temp = index;
									}																													
								}									
							}
						}	
						checkCommand = NULL;	
					}
					if(index == commandQueue.size()-1){
						//cout<<"18"<<endl;
						if(checkCommand_2 != NULL){
							//cout<<"19"<<endl;
							scheduledCommand = checkCommand_2;
							checkCommand_2 = NULL;
							/*
							if(scheduledCommand->busPacketType == PRE)
							{
								cout<<"PRE"<<"\t\t"<<clock<<":"<<"\tBank: "<<scheduledCommand->bank<<"\tAddress: "<<scheduledCommand->address<<"\tColumn: "<<scheduledCommand->column<<"\tRow: "<<scheduledCommand->row<<endl;										
							}
							else if(scheduledCommand->busPacketType == RD){
								cout<<"RD"<<"\t\t"<<clock<<":"<<"\tBank: "<<scheduledCommand->bank<<"\tAddress: "<<scheduledCommand->address<<"\tColumn: "<<scheduledCommand->column<<"\tRow: "<<scheduledCommand->row<<endl;								
							}
							else if(scheduledCommand->busPacketType == WR){
								cout<<"WR"<<"\t\t"<<clock<<":"<<"\tBank: "<<scheduledCommand->bank<<"\tAddress: "<<scheduledCommand->address<<"\tColumn: "<<scheduledCommand->column<<"\tRow: "<<scheduledCommand->row<<endl;									
							}
							else if(scheduledCommand->busPacketType == ACT){
								cout<<"ACT"<<"\t\t"<<clock<<":"<<"\tBank: "<<scheduledCommand->bank<<"\tAddress: "<<scheduledCommand->address<<"\tColumn: "<<scheduledCommand->column<<"\tRow: "<<scheduledCommand->row<<endl;								
							}
							*/
							/*
							if(scheduledCommand->busPacketType == PRE){
								//fout<<"   PRE "<<"\t\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\t"<<scheduledCommand->row<<"\t"<<scheduledCommand->column<<endl;		
								//cout<<"PRE"<<"\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\tFrom requestor:  "<<scheduledCommand->requestorID<<"\tRow:\t"<<scheduledCommand->row<<"\tCol:\t"<<scheduledCommand->column<<endl;								
							}
							else if(scheduledCommand->busPacketType == RD){
								//fout<<"   RD "<<"\t\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\t"<<scheduledCommand->row<<"\t"<<scheduledCommand->column<<endl;
								//cout<<"RD"<<"\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\tFrom requestor:  "<<scheduledCommand->requestorID<<"\tRow:\t"<<scheduledCommand->row<<"\tCol:\t"<<scheduledCommand->column<<endl;									
							}
							else if(scheduledCommand->busPacketType == WR){
								//fout<<"   WR "<<"\t\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\t"<<scheduledCommand->row<<"\t"<<scheduledCommand->column<<endl;									
								//cout<<"WR"<<"\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\tFrom requestor:  "<<scheduledCommand->requestorID<<"\tRow:\t"<<scheduledCommand->row<<"\tCol:\t"<<scheduledCommand->column<<endl;
							}
							else if(scheduledCommand->busPacketType == ACT){
								//fout<<"   ACT "<<"\t\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\t"<<scheduledCommand->row<<"\t"<<scheduledCommand->column<<endl;							
								//cout<<"ACT"<<"\t"<<clock<<":"<<"\t"<<scheduledCommand->bank<<"\tFrom requestor:  "<<scheduledCommand->requestorID<<"\tRow:\t"<<scheduledCommand->row<<"\tCol:\t"<<scheduledCommand->column<<endl;	
							}
							*/
							sendCommand(scheduledCommand, index_temp,false);
							//cout<<"20"<<endl;
							return scheduledCommand;
						}						
					}
				}				
			}
			return scheduledCommand;
		}
	};
}
#endif
