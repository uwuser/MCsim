// The code below is not the optimized one in terms of the LOC neither efficiency
#include "../../src/CommandScheduler.h"
#include <cstdlib>

namespace MCsim
{
	class CommandScheduler_Round: public CommandScheduler
	{
	private:
		vector<unsigned int> Order;
		vector<unsigned int> OrderPRE;
		std::map<unsigned long long, bool> closestatemap; // false means close 
		std::map<unsigned int, bool> queuePending; 	// Pending Command indicator based on requestorID
		std::map<unsigned int, BusPacket*> tempqueue;
		int  servicebuffer[16];
		int  consideredScheduled[16];	
		int checkslot,lastACT,signCheck,checkB;
		unsigned long int lastCAScounter;
		unsigned int countACT, countFAW, time_issued_first_ACT, ACTtimer, CAStimer,countCAS,counter;
		unsigned int tCCD,tWtoR,compensation;
		bool blockACT, blockCAS,first,jump_1, jump_2, jump_3,roundType;
	public:
		CommandScheduler_Round(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable) {
			counter = 0;
			roundType = true; // true = Read and false = write
			checkslot = 0;
			lastACT = 0;
			countFAW = 0;
			CAStimer =0;
			lastCAScounter = 0;
			ACTtimer =0;
			time_issued_first_ACT =0;
			blockACT = false;
			compensation =  1;
			blockCAS = false ;
			first = true;
			for (unsigned int i = 0 ; i < 8 ; i++)
				servicebuffer[i] = 0;
			for (unsigned int i = 0; i < 8; i++)
				consideredScheduled[i] = 0;
			for (unsigned int i = 0; i < 8; i++)
				OrderPRE.push_back(i);
		}
		~CommandScheduler_Round() {
			tempqueue.clear();
			Order.clear();
			OrderPRE.clear();
			queuePending.clear();
		}
		void init_config(){
			blockACT = false;
			blockCAS = false;
			countACT = 0;
			countCAS = 0;	
			countFAW = 0;
			counter = 0;	
			ACTtimer = 0;
			first = true;
		}
		void clean_buffers(){
			for(uint64_t i = 0 ; i < 16; i++)				
				servicebuffer[i] = 0;				
			for (unsigned int i = 0; i < 16; i++)				
				consideredScheduled[i] = 0;				
		}
		void send_precedure(BusPacket* checkcommand, bool PRE, int RR, int i){
			if(PRE == true){
				OrderPRE.erase(OrderPRE.begin() + i);
				OrderPRE.push_back(RR);
				scheduledCommand = checkCommand;
				sendCommand(scheduledCommand,0,false);
				tempqueue.erase(RR);
				queuePending[scheduledCommand->requestorID] = false;
				counter++;														
			}
			else
			{
				if(checkcommand->busPacketType == ACT_W || checkcommand->busPacketType == ACT_R)
				{
					scheduledCommand = checkCommand;
					sendCommand(scheduledCommand,0,false);
					tempqueue.erase(RR);
					if(first){
						first =false;
					}
					queuePending[scheduledCommand->requestorID] = false;
					if(countACT == 0){
						time_issued_first_ACT = 0;
						ACTtimer = getTiming("tRRD");
						countACT++;
					}
					else if (countACT == 3){
						if(time_issued_first_ACT < getTiming("tFAW")){
							ACTtimer = getTiming("tFAW") - time_issued_first_ACT;
							countACT = 0;
							time_issued_first_ACT = 0;
						}
					}
					else
					{
						ACTtimer = getTiming("tRRD");
						countACT++;
					}	
					counter++;				
				}
				else if(checkcommand->busPacketType == WR || checkcommand->busPacketType == RD)
				{
					Order.erase(Order.begin() + i);
					scheduledCommand = checkCommand;
					sendCommand(scheduledCommand,0,false);
					tempqueue.erase(RR);
					queuePending[scheduledCommand->requestorID] = false;
					lastCAScounter = clock;				
					CAStimer = tCCD;
					if(first){
						first =false;
					}
					closestatemap[checkCommand->requestorID] = false;
					servicebuffer[RR] = 1;
					consideredScheduled[RR] = 0;
					counter++;
						
				}
			}
		}
		BusPacket* commandSchedule()
		{	
			scheduledCommand = NULL;
			tCCD = 4;
			tWtoR = getTiming("tWL") + getTiming("tBUS") + getTiming("tWTR");
			checkCommand = NULL;
			checkCommand_1 = NULL;
			blockACT = false;
			blockCAS = false;
			if(CAStimer > 0)
				CAStimer--;
			else
				CAStimer = 0;
			if(ACTtimer > 0)
				ACTtimer--;	
			time_issued_first_ACT++;
			for(unsigned int index = 0; index < commandQueue.size(); index++)
			{				
				if(commandQueue[index]->isPerRequestor()) {
					if(commandQueue[index]->getRequestorIndex() > 0) {  
						for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) {
							if(commandQueue[index]->getRequestorSize(num) > 0 ) {
								checkCommand = commandQueue[index]->getRequestorCommand(num);						
								if(queuePending.find(checkCommand->requestorID) == queuePending.end()) 																
									queuePending[checkCommand->requestorID] = false;																		
								if(queuePending[checkCommand->requestorID] == false && isReady(checkCommand, index)) {											
									if(checkCommand->busPacketType == ACT_R || checkCommand->busPacketType == ACT_W) {
										closestatemap[checkCommand->requestorID] = true;
										tempqueue[checkCommand->requestorID]= checkCommand;														
										queuePending[checkCommand->requestorID] = true;
										Order.push_back(checkCommand->requestorID);
									}	
									else if(checkCommand->busPacketType == RD || checkCommand->busPacketType == WR) {
										if(closestatemap[checkCommand->requestorID] == false) {
											tempqueue[checkCommand->requestorID] = checkCommand;														
											queuePending[checkCommand->requestorID] = true;
											Order.push_back(checkCommand->requestorID);
										}																																
										else if (closestatemap[checkCommand->requestorID] == true) {											
											tempqueue[checkCommand->requestorID] = checkCommand;														
											queuePending[checkCommand->requestorID] = true;
										}											
									}
									else {
										tempqueue[checkCommand->requestorID] = checkCommand;														
										queuePending[checkCommand->requestorID] = true;
									}
								}
								checkCommand = NULL;
							}
						}
					}	
				}		
			}
			int checkC = 0;
			for (int i = 0 ; i < 16 ; i++)
			{
				if(consideredScheduled[i] == 1)	
					checkC = checkC + consideredScheduled[i];				
			}			
			if(checkC == 0)
			{	
				if (roundType == true)
				{
					for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
					{			
						checkCommand = it->second;
						if (checkCommand->busPacketType == ACT_W) 
						{	
							clean_buffers();
							roundType = false; // WR
							if(clock - lastCAScounter < getTiming("tRTW"))								
								CAStimer = getTiming("tRTW") - (clock - lastCAScounter);							
							else
								CAStimer = 0;										
							init_config();								
							ACTtimer = getTiming("tRRD");
							jump_1 = true;				
						}
					}	
					if(!jump_1)
					{							
						for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
						{			
							checkCommand = it->second;
							if (checkCommand->busPacketType == WR)
							{										
								clean_buffers();
								roundType = false;										
								if(clock - lastCAScounter < getTiming("tRTW"))										
									CAStimer = getTiming("tRTW") - (clock - lastCAScounter);
								else
									CAStimer = tCCD;	
								init_config();								
								jump_2 = true;
							}
						}
					}
					if (!jump_1)
					{
						if(!jump_2)
						{
							for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
							{			
								checkCommand = it->second;
								if (checkCommand->busPacketType == RD) 																															
									jump_3 = true;															
							}	
							for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
							{			
								checkCommand = it->second;
								if (checkCommand->busPacketType == ACT_R)													
									jump_3 = true;								
							}								
							if (jump_3)
							{								
								clean_buffers();
								init_config();
								jump_3 = false;																				
							}					
						}
					}																													
				}
				else if (roundType == false )
				{
					for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
					{		
						checkCommand = it->second;
						if (checkCommand->busPacketType == ACT_R) 
						{
							clean_buffers();
							roundType = true;	
							if(clock - lastCAScounter < tWtoR)									
								CAStimer = tWtoR - (clock - lastCAScounter);
							else
								CAStimer = 0;
							init_config();
							jump_1 = true;			
						}
					}	
					if(!jump_1)
					{
						for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
						{			
							checkCommand = it->second;
							if (checkCommand->busPacketType == RD)
							{
								clean_buffers();
								roundType = true;
								if(clock - lastCAScounter < tWtoR)										
									CAStimer = tWtoR - (clock - lastCAScounter);								
								else
									CAStimer = tCCD;
									
								jump_2 = true;
								init_config();
							}
						}
					}
					if (!jump_1)
					{
						if(!jump_2)
						{
							for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
							{			
								checkCommand = it->second;
								if (checkCommand->busPacketType == WR) 																																
									jump_3 = true;										
							}	
							for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
							{			
								checkCommand = it->second;
								if (checkCommand->busPacketType == ACT_W)														
									jump_3 = true;									
							}								
							if (jump_3)
							{									
								clean_buffers();
								init_config();
								jump_3 = false;									
							}
						}
					}							
				}
				jump_1 =false;
				jump_2 =false;
				jump_3 = false;
			}
			if (ACTtimer == 0)
			{
				if(roundType == true)
				{
					if(first == false)
					{
						int checkA = 0;
						int checkD = 0;
						for(unsigned int i = 0; i < Order.size(); i++)
						{									
							int k = Order.at(i);
							if(tempqueue.find(k) == tempqueue.end())
							{							
								continue;
							}
							else
							{							
								if (consideredScheduled[k] == 1 && servicebuffer[k] == 0)
								{
									checkCommand = tempqueue[k];
									if(checkCommand->busPacketType == ACT_R)
										checkA = 1;
								}
							}
						}
						for(unsigned int i = 0; i < Order.size(); i++)
						{									
							int k = Order.at(i);
							if(tempqueue.find(k) == tempqueue.end())
							{							
								continue;
							}
							else
							{	
								if(consideredScheduled[k] == 0 && servicebuffer[k] == 0){
										checkCommand = tempqueue[k];
										if(checkCommand->busPacketType == RD)
											checkD++;									
								}
							}	
						}	
						if (checkA == 0)
						{
							checkB = 0;
							for(unsigned int i = 0; i < Order.size(); i++)
							{
								int k = Order.at(i);													
								if (consideredScheduled[k] == 1 && servicebuffer[k] == 0)									
									checkB++;											
							}
							signCheck =(((checkB+checkD)*tCCD) + CAStimer) - (getTiming("tRCD")) - compensation;		
							if(signCheck< 0)							
								blockACT = true;																	
							else							
								blockACT = false;																					
						}
					}
				}					
				else if(roundType ==false)
				{
					if(first == false)
					{
						int checkA = 0;
						int checkD = 0;
						for(unsigned int i = 0; i < Order.size(); i++)
						{									
							int k = Order.at(i);
							if(tempqueue.find(k) == tempqueue.end())
							{							
								continue;
							}
							else
							{
								if (consideredScheduled[k] == 1 && servicebuffer[k] == 0)
								{
									checkCommand = tempqueue[k];
									if(checkCommand->busPacketType == ACT_W)										
										checkA = 1;
									
								}
							}	
						}
						for(unsigned int i = 0; i < Order.size(); i++)
						{									
							int k = Order.at(i);
							if(tempqueue.find(k) == tempqueue.end())
							{							
								continue;
							}
							else
							{	
								if(consideredScheduled[k] == 0 && servicebuffer[k] == 0){
									checkCommand = tempqueue[k];
									if(checkCommand->busPacketType == RD)										
										checkD++;							
								}
							}	
						}	
						if (checkA == 0)
						{
							checkB = 0;
							for(unsigned int i = 0; i < Order.size(); i++)
							{
								int k = Order.at(i);
								if (consideredScheduled[k] == 1 && servicebuffer[k] == 0)
									checkB++;																
														
							}
							signCheck =(((checkB+checkD)*tCCD) + CAStimer) - (getTiming("tRCD")) - compensation;				
							if(signCheck< 0)							
								blockACT = true;																		
							else					
								blockACT = false;											
						}
					}
				}						
			}
			signCheck=0;
			for(unsigned int i = 0; i < Order.size(); i++)
			{	
				int RR = Order.at(i);
				if(tempqueue.find(RR) == tempqueue.end())
				{
					continue;
				}
				else
				{
					checkCommand = tempqueue[RR];
					if (checkCommand->busPacketType == ACT_R)
					{
						if(roundType == true)
						{
							
							if(servicebuffer[RR] == 0 && consideredScheduled[RR] == 0)
							{			
								if(!blockACT)																		
									consideredScheduled[RR] = 1;									
								else
									blockCAS = true;							
							}
						}
					}
					else if (checkCommand->busPacketType == ACT_W)
					{
						if(roundType == false)
						{							
							if(servicebuffer[RR] == 0 && consideredScheduled[RR] == 0)
							{		
								if(!blockACT)																		
									consideredScheduled[RR] = 1;									
								else									
									blockCAS = true;	
							}															
						}
					}
					else if (checkCommand->busPacketType == RD)
					{
						if(roundType == true)
						{
							if(servicebuffer[RR] == 0)
							{													
								if (consideredScheduled[RR] == 0)
								{
									for(unsigned int i = 0; i < Order.size(); i++)
									{											
										int RR1 = Order.at(i);
										if(tempqueue.find(RR1) == tempqueue.end())
										{
											continue;
										}
										else
										{
											checkCommand_1 = tempqueue[RR1];	
											if (checkCommand_1->busPacketType == ACT_R)
											{
												if(!blockACT)
												{
													if(servicebuffer[RR1] == 0 && consideredScheduled[RR1] == 0)
													{	
														if(!blockACT)																								
															consideredScheduled[RR1] = 1;															
														else															
															blockCAS = true;	
													}
												}
											}	
										}																																					
									}
									if(!blockCAS)
										consideredScheduled[RR] = 1;																																							
								}	
							}
						}
					}		
					else if (checkCommand->busPacketType == WR)
					{
						if(roundType == false)
						{
							if(servicebuffer[RR] == 0)
							{					
								if (consideredScheduled[RR] == 0)
								{										
									for(unsigned int i = 0; i < Order.size(); i++)
									{											
										int RR1 = Order.at(i);
										if(tempqueue.find(RR1) == tempqueue.end())
										{
											continue;
										}
										else
										{
											checkCommand_1 = tempqueue[RR1];	
											if (checkCommand_1->busPacketType == ACT_W)
											{
												if(!blockACT)
												{
													if(servicebuffer[RR1] == 0 && consideredScheduled[RR1] == 0)
													{		
														if(!blockACT)																							
															consideredScheduled[RR1] = 1;															
														else															
															blockCAS = true;
													}
												}
											}	
										}																																					
									}
									if(!blockCAS)
										consideredScheduled[RR] = 1;										
								}	
							}
						}
					}
				}
			}	
			if(!tempqueue.empty())
			{
				for(unsigned int i = 0; i < Order.size(); i++)
				{
					int RR = Order.at(i);
					if(tempqueue.find(RR) == tempqueue.end())
					{
						continue;
					}
					else
					{			
						checkCommand = tempqueue[RR];
						if (checkCommand->busPacketType == ACT_R)
						{
							if(roundType == true)
							{
								if(servicebuffer[RR] == 0)
								{
									if(!blockACT)
									{
										if(isIssuable(checkCommand))
										{																				
											send_precedure(checkCommand,false,RR,i);
											return checkCommand;
										}	
									}	
								}	
							}												
						checkCommand = NULL;
						}
						else if (checkCommand->busPacketType == ACT_W)
						{
							if (roundType == false)
							{
								if(servicebuffer[RR] == 0)
								{	
									if(!blockACT)
									{
										if(isIssuable(checkCommand))
										{																						
											send_precedure(checkCommand,false,RR,i);
											return checkCommand;
										}
									}	
								}	
							}
						checkCommand = NULL;
						}
						else if (checkCommand->busPacketType == RD)
						{
							if (roundType == true)
							{
								if(servicebuffer[RR] == 0)
								{
									for(unsigned int i = 0; i < Order.size(); i++)
									{											
										int RR1 = Order.at(i);
										if(tempqueue.find(RR1) == tempqueue.end())
										{
											continue;
										}
										else
										{
											checkCommand_1 = tempqueue[RR1];	
											if (checkCommand_1->busPacketType == ACT_R)
											{										
												if(servicebuffer[RR1] == 0)
												{
													if(!blockACT)
													{
														if(isIssuable(checkCommand_1))
														{															
															send_precedure(checkCommand_1,false,RR1,i);
															return checkCommand_1;
														}
													}																									
												}
											}	
										}																																			
									}
									if(consideredScheduled[RR] == 1)
									{
										if(isIssuable(checkCommand))
										{												
											send_precedure(checkCommand,false,RR,i);
											return checkCommand;
										}
									}																																																																	
								}								
							}
						checkCommand = NULL;	
						}	
						else if (checkCommand->busPacketType == WR)
						{
							if (roundType == false)
							{
								if(servicebuffer[RR] == 0)
								{
									for(unsigned int i = 0; i < Order.size(); i++)
									{
										int RR1 = Order.at(i);
										if(tempqueue.find(RR1) == tempqueue.end())
										{
											continue;
										}	
										else
										{
											checkCommand_1 = tempqueue[RR1];	
											if (checkCommand_1->busPacketType == ACT_W)
											{
												if(servicebuffer[RR1] == 0)
												{
													if(!blockACT)
													{
														if(isIssuable(checkCommand_1))
														{																									
															send_precedure(checkCommand_1,false,RR1,i);
															return checkCommand_1;
														}	
													}																							
												}
											}
										}																																			
									}
									if(consideredScheduled[RR] == 1)
									{
										if (isIssuable(checkCommand))
										{												
											send_precedure(checkCommand,false,RR,i);
											return checkCommand;
										}											
									}
								}
							}
							checkCommand = NULL;
						}
						checkCommand = NULL;
					}
				}						
				for(unsigned int i = 0; i < OrderPRE.size(); i++)
				{
					int RR = OrderPRE.at(i);
					if(tempqueue.find(RR) == tempqueue.end())
					{
						continue;
					}
					else
					{			
						checkCommand = tempqueue[RR];
						if (checkCommand->busPacketType == PRE)
						{
							if(isIssuable(checkCommand)){
								send_precedure(checkCommand, true,RR,i);
								return checkCommand;
							}	
						checkCommand = NULL;																						
						}
					}
				}			
			counter++;
			}				
		return NULL;		
		}		
	};	
}