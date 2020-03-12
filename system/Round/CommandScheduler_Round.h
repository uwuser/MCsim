// The code below is not the optimized one in terms of the LOC neither efficiency
#include "../../src/CommandScheduler.h"
#include <cstdlib>

namespace MCsim
{
	class CommandScheduler_Round: public CommandScheduler
	{
	private:
		vector<unsigned int> Order, OrderPRE;
		std::map<unsigned long long, bool> closestatemap;	 // false means close 
		std::map<unsigned int, bool> queuePending; 			 // Pending Command indicator based on requestorID
		std::map<unsigned int, BusPacket*> tempqueue;
		int servicebuffer[16], consideredScheduled[16],signCheck,checkB; 							 // 16 is the maximum REQ number
		unsigned long int lastCAScounter;
		unsigned int countACT,time_issued_first_ACT,ACTtimer,CAStimer,counter, tCCD,tWtoR,compensation;
		bool blockACT,first,jump,roundType;
	public:
		CommandScheduler_Round(vector<CommandQueue*>& commandQueues, const map<unsigned int, bool>& requestorTable):
			CommandScheduler(commandQueues, requestorTable) {
			counter = 0;
			roundType = true; // true = Read and false = write
			CAStimer = 0;
			lastCAScounter = 0;
			ACTtimer = 0;
			time_issued_first_ACT = 0;
			compensation =  1;
			first = true;
			for (unsigned int i = 0; i < 8; i++)
				OrderPRE.push_back(i);
		}
		~CommandScheduler_Round() {
			tempqueue.clear();
			Order.clear();
			OrderPRE.clear();
			queuePending.clear();
		}
		void clean_buffers(){
			for(uint64_t i = 0 ; i < 16; i++)				
				servicebuffer[i] = 0;				
			for (unsigned int i = 0; i < 16; i++)				
				consideredScheduled[i] = 0;	
			blockACT = false;
			countACT = 0;
			counter = 0;	
			ACTtimer = 0;
			first = true;				
		}
		void send_precedure(BusPacket* checkcommand, bool PRE, int RR, int i){
			scheduledCommand = checkcommand;
			if(PRE == true){
				OrderPRE.erase(OrderPRE.begin() + i);
				OrderPRE.push_back(RR);
				PRE = false;
				sendCommand(scheduledCommand,0,false);
				counter++;														
			}
			else
			{
				if((scheduledCommand->busPacketType == ACT_W) || (scheduledCommand->busPacketType == ACT_R))
				{					
					sendCommand(scheduledCommand,0,false);					
					if(first)
						first = false;		
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
				else if((scheduledCommand->busPacketType == WR) || (scheduledCommand->busPacketType == RD))
				{
					Order.erase(Order.begin() + i);
					sendCommand(scheduledCommand,0,false);
					lastCAScounter = clock;				
					CAStimer = tCCD;
					if(first)
						first = false;					
					closestatemap[scheduledCommand->requestorID] = false;
					servicebuffer[RR] = 1;
					consideredScheduled[RR] = 0;
					counter++;
				}
			}
			tempqueue.erase(RR);
			queuePending[scheduledCommand->requestorID] = false;
		}
		BusPacket* commandSchedule()
		{	
			scheduledCommand = NULL;
			tCCD = 4;
			tWtoR = getTiming("tWL") + getTiming("tBUS") + getTiming("tWTR");
			checkCommand = NULL;
			checkCommand_temp_1 = NULL;
			blockACT = false;
			if(CAStimer > 0)
				CAStimer--;
			else
				CAStimer = 0;
			if(ACTtimer > 0)
				ACTtimer--;	
			time_issued_first_ACT++;
			for(unsigned int index = 0; index < commandQueue.size(); index++)
			{
				if(commandQueue[index]->getRequestorIndex() > 0) {  
					for(unsigned int num = 0; num < commandQueue[index]->getRequestorIndex(); num++) {
						if(commandQueue[index]->getRequestorSize(num) > 0 ) {
							checkCommand = commandQueue[index]->getRequestorCommand(num);						
							if(queuePending.find(checkCommand->requestorID) == queuePending.end()) 																
								queuePending[checkCommand->requestorID] = false;																		
							if(queuePending[checkCommand->requestorID] == false && isReady(checkCommand, index)) {											
								if(checkCommand->busPacketType == ACT_R || checkCommand->busPacketType == ACT_W) {
									closestatemap[checkCommand->requestorID] = true;													
									queuePending[checkCommand->requestorID] = true;
									Order.push_back(checkCommand->requestorID);
								}	
								else if(checkCommand->busPacketType == RD || checkCommand->busPacketType == WR) {
									if(closestatemap[checkCommand->requestorID] == false) {														
										queuePending[checkCommand->requestorID] = true;
										Order.push_back(checkCommand->requestorID);
									}																																
									else if (closestatemap[checkCommand->requestorID] == true) {														
										queuePending[checkCommand->requestorID] = true;
									}											
								}
								else 												
									queuePending[checkCommand->requestorID] = true;									
								tempqueue[checkCommand->requestorID] = checkCommand;
							}
							checkCommand = NULL;
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
			if(checkC == 0)   // Should we switch or reset
			{	
				if (roundType == true)
				{
					for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
					{			
						checkCommand = it->second;
						if (checkCommand->busPacketType == ACT_W || checkCommand->busPacketType == WR){
							clean_buffers();
							roundType = false;
							if(clock - lastCAScounter < getTiming("tRTW"))								
								CAStimer = getTiming("tRTW") - (clock - lastCAScounter);
							else if (checkCommand->busPacketType == ACT_W){
								CAStimer = 0;
								ACTtimer = getTiming("tRRD");
							}
							else if (checkCommand->busPacketType == WR){
								CAStimer = tCCD;
							}							
							jump = true;	
						} 
					}
					if (!jump)
					{
						for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
						{			
							checkCommand = it->second;
							if (checkCommand->busPacketType == RD || checkCommand->busPacketType == ACT_R) {
								clean_buffers();														
							}	
						}	
					}																													
				}
				else if (roundType == false )
				{
					for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
					{		
						checkCommand = it->second;
						if (checkCommand->busPacketType == ACT_R || checkCommand->busPacketType == RD) 
						{
							clean_buffers();
							roundType = true;	
							if(clock - lastCAScounter < tWtoR)									
								CAStimer = tWtoR - (clock - lastCAScounter);
							else if(checkCommand->busPacketType == ACT_R ){
								CAStimer = 0;
								ACTtimer = getTiming("tRRD");
							}
							else if( checkCommand->busPacketType == RD){
								CAStimer = tCCD;
							}	
								CAStimer = 0;
							jump = true;			
						}
					}	
					if (!jump)
					{
						for(std::map<unsigned int, BusPacket*>::iterator it=tempqueue.begin(); it!=tempqueue.end(); it++) // for all requestors from "num". getRequestorIndex() gives the number of requestors	
						{			
							checkCommand = it->second;
							if (checkCommand->busPacketType == WR || checkCommand->busPacketType == ACT_W){ 		
								clean_buffers();
							}								
						}						
					}							
				}
				jump = false;
			}
			if (ACTtimer == 0) // should we block?
			{
				if(roundType == true && first == false)
				{
					int checkA = 0;
					int checkD = 0;
					for(unsigned int i = 0; i < Order.size(); i++)
					{									
						int k = Order.at(i);
						if(tempqueue.find(k) == tempqueue.end())													
							continue;							
						else if(consideredScheduled[k] == 1 && servicebuffer[k] == 0)
						{	
							checkCommand = tempqueue[k];
							if(checkCommand->busPacketType == ACT_R)
								checkA = 1;
						}
					}
					for(unsigned int i = 0; i < Order.size(); i++)
					{									
						int k = Order.at(i);
						if(tempqueue.find(k) == tempqueue.end())													
							continue;							
						else if(consideredScheduled[k] == 0 && servicebuffer[k] == 0)
						{	
							checkCommand = tempqueue[k];
							if(checkCommand->busPacketType == RD)
								checkD++;	
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
						signCheck = (((checkB+checkD)*tCCD) + CAStimer) - (getTiming("tRCD")) - compensation;		
						if(signCheck < 0)							
							blockACT = true;																	
						else							
							blockACT = false;																					
					}
				}					
				else if(roundType == false && first == false)
				{
					int checkA = 0;
					int checkD = 0;
					for(unsigned int i = 0; i < Order.size(); i++)
					{									
						int k = Order.at(i);
						if(tempqueue.find(k) == tempqueue.end())					
							continue;
						else if(consideredScheduled[k] == 1 && servicebuffer[k] == 0)
						{
							checkCommand = tempqueue[k];
							if(checkCommand->busPacketType == ACT_W)										
								checkA = 1;	
						}	
					}
					for(unsigned int i = 0; i < Order.size(); i++)
					{									
						int k = Order.at(i);
						if(tempqueue.find(k) == tempqueue.end())												
							continue;						
						else if(consideredScheduled[k] == 0 && servicebuffer[k] == 0)
						{	
							checkCommand = tempqueue[k];
							if(checkCommand->busPacketType == RD)										
								checkD++;	
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
						signCheck = (((checkB+checkD)*tCCD) + CAStimer) - (getTiming("tRCD")) - compensation;				
						if(signCheck< 0)							
							blockACT = true;																		
						else					
							blockACT = false;											
					}					
				}						
			}
			signCheck = 0;
			for(unsigned int i = 0; i < Order.size(); i++)  // Accepting the command procedure
			{	
				int RR = Order.at(i);
				if(tempqueue.find(RR) == tempqueue.end())
					continue;				
				else
				{
					checkCommand = tempqueue[RR];
					if ((checkCommand->busPacketType == ACT_R) && (roundType == true) && (servicebuffer[RR] == 0) && (consideredScheduled[RR] == 0))
					{			
						if(!blockACT)																		
							consideredScheduled[RR] = 1;	
					}
					else if ((checkCommand->busPacketType == ACT_W) && (roundType == false) && (servicebuffer[RR] == 0) && (consideredScheduled[RR] == 0))
					{
						if(!blockACT)																		
							consideredScheduled[RR] = 1;
					}
					else if (checkCommand->busPacketType == RD)
					{
						if((roundType == true) && (servicebuffer[RR] == 0) && (consideredScheduled[RR] == 0))
						{
							for(unsigned int i = 0; i < Order.size(); i++)
							{											
								int RR1 = Order.at(i);
								if(tempqueue.find(RR1) == tempqueue.end())
									continue;										
								else
								{
									checkCommand_temp_1 = tempqueue[RR1];	
									if ((checkCommand_temp_1->busPacketType == ACT_R) && (!blockACT) && (servicebuffer[RR1] == 0) && (consideredScheduled[RR1] == 0))
									{
										if(!blockACT)																								
											consideredScheduled[RR1] = 1;	
									}	
								}																																					
							}
							if(!blockACT)
								consideredScheduled[RR] = 1;
						}
					}		
					else if ((checkCommand->busPacketType == WR) && (roundType == false) && (servicebuffer[RR] == 0) && (consideredScheduled[RR] == 0))
					{										
						for(unsigned int i = 0; i < Order.size(); i++)
						{											
							int RR1 = Order.at(i);
							if(tempqueue.find(RR1) == tempqueue.end())										
								continue;										
							else
							{
								checkCommand_temp_1 = tempqueue[RR1];	
								if ((checkCommand_temp_1->busPacketType == ACT_W) && (!blockACT) && (servicebuffer[RR1] == 0) && (consideredScheduled[RR1] == 0))
								{
									if(!blockACT)																							
										consideredScheduled[RR1] = 1;
								}	
							}																																					
						}
						if(!blockACT)
							consideredScheduled[RR] = 1;	
					}
				}
			}	
			if(!tempqueue.empty()) // Sending the command procedure 
			{
				for(unsigned int i = 0; i < Order.size(); i++)
				{
					int RR = Order.at(i);
					if(tempqueue.find(RR) == tempqueue.end())					
						continue;					
					else
					{			
						checkCommand = tempqueue[RR];
						if (((checkCommand->busPacketType == ACT_R) && (roundType == true) && (servicebuffer[RR] == 0) && (!blockACT) && (isIssuable(checkCommand))) || ((checkCommand->busPacketType == ACT_W) && (roundType == false) && (servicebuffer[RR] == 0) && (!blockACT) && (isIssuable(checkCommand))))
						{																			
							send_precedure(checkCommand,false,RR,i);
							return checkCommand;
						}
						else if ((checkCommand->busPacketType == RD) && (roundType == true) && (servicebuffer[RR] == 0))
						{
							for(unsigned int i = 0; i < Order.size(); i++)
							{											
								int RR1 = Order.at(i);
								if(tempqueue.find(RR1) == tempqueue.end())
									continue;
								else
								{
									checkCommand_temp_1 = tempqueue[RR1];	
									if ((checkCommand_temp_1->busPacketType == ACT_R) && (servicebuffer[RR1] == 0) && (!blockACT) && (isIssuable(checkCommand_temp_1)))
									{															
										send_precedure(checkCommand_temp_1,false,RR1,i);
										return checkCommand_temp_1;
									}	
								}																																			
							}
							if(consideredScheduled[RR] == 1 && isIssuable(checkCommand))
							{											
								send_precedure(checkCommand,false,RR,i);
								return checkCommand;
							}		
						checkCommand = NULL;	
						}	
						else if ((checkCommand->busPacketType == WR) && (roundType == false) && (servicebuffer[RR] == 0))
						{
							for(unsigned int i = 0; i < Order.size(); i++)
							{
								int RR1 = Order.at(i);
								if(tempqueue.find(RR1) == tempqueue.end())										
									continue;									
								else
								{
									checkCommand_temp_1 = tempqueue[RR1];	
									if ((checkCommand_temp_1->busPacketType == ACT_W) && (servicebuffer[RR1] == 0) && (!blockACT) && (isIssuable(checkCommand_temp_1)))
									{																								
										send_precedure(checkCommand_temp_1,false,RR1,i);
										return checkCommand_temp_1;
									}
								}																																			
							}
							if((consideredScheduled[RR] == 1) && (isIssuable(checkCommand)))
							{										
								send_precedure(checkCommand,false,RR,i);
								return checkCommand;																		
							}
						checkCommand = NULL;
						}
					}
				}						
				for(unsigned int i = 0; i < OrderPRE.size(); i++)
				{
					int RR = OrderPRE.at(i);
					if(tempqueue.find(RR) == tempqueue.end())
						continue;
					else
					{			
						checkCommand = tempqueue[RR];
						if (checkCommand->busPacketType == PRE && (isIssuable(checkCommand)))
						{
							send_precedure(checkCommand,true,RR,i);
							return checkCommand;																																			
						}
						checkCommand = NULL;	
					}
				}			
				counter++;
			}				
		return NULL;		
		}		
	};	
}
