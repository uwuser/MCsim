/*********************************************************************************
*  Copyright (c) 2019 Reza Mirosanlou
*                             University of Waterloo
            
*  All rights reserved.
*********************************************************************************/
#ifndef REQUESTSCHEDULER_PARBS_H
#define REQUESTSCHEDULER_PARBS_H

#include "../../src/RequestScheduler.h"

#include <queue>
#include <bits/stdc++.h> 
using namespace std;

namespace Memulator
{
	class RequestScheduler_PARBS: public RequestScheduler
	{
	private:
		unsigned int MarkingCap;
		unsigned int bmax;
		unsigned int rmax;
		unsigned int markedMaxLoadPerProc[10]; 
		unsigned int markedTotalLoadPerProc[10];
		unsigned int localBcount;
		unsigned int markedCnt;
		Request* temp;
		Request* temp_1;
		std::map<Request*, bool> marked;
		std::map<Request*, unsigned int> rank;
		std::map<std::pair<unsigned int, unsigned int>,std::vector<Request*>> MarkableQ;
		
	public:

		RequestScheduler_PARBS(std::vector<RequestQueue*>&requestQueues, std::vector<CommandQueue*>& commandQueues, const std::map<unsigned int, bool>& requestorTable, int dataBus): 
			RequestScheduler(requestQueues, commandQueues, requestorTable){
			markedCnt = 0;
			temp = NULL;
			bmax = 8; //for all DDR3 devices
			rmax = 1;
			localBcount = rmax * bmax;
			MarkingCap = 5; //can be optionally modified
		}

		Request* BetterRequest(Request* req1,Request* req2){

			if(marked[req1] ^ marked[req2]){
				if(marked[req1]) return req1;
				else{
					return req2;
				}				
			}

			if(isRowHit(req1) ^ isRowHit(req2)){
				if(isRowHit(req1))
					return req1;
				else
					return req2;
			}

			// Ranking	
			//cout<<"inside ranking and markedMaxLoadPerProc[req1->requestorID] is "<<markedMaxLoadPerProc[req1->requestorID]<<endl;
			//cout<<"inside second ranking and markedMaxLoadPerProc[req1->requestorID] is "<<markedMaxLoadPerProc[req2->requestorID]<<endl;	
			if(markedMaxLoadPerProc[req1->requestorID] != markedMaxLoadPerProc[req2->requestorID] ){
				if(markedMaxLoadPerProc[req1->requestorID] > markedMaxLoadPerProc[req2->requestorID]){
					return req2;
				}
				else{
					return req1;
				}
			}
			else {
			//cout<<"inside ranking 3 and markedMaxLoadPerProc[req1->requestorID] is "<<markedTotalLoadPerProc[req1->requestorID]<<endl;
			//cout<<"inside ranking 4 and markedMaxLoadPerProc[req1->requestorID] is "<<markedTotalLoadPerProc[req2->requestorID]<<endl;	
				if(markedTotalLoadPerProc[req1->requestorID] != markedTotalLoadPerProc[req2->requestorID] ){
					if(markedTotalLoadPerProc[req1->requestorID] > markedTotalLoadPerProc[req2->requestorID]){
						return req2;
					}
					else{
						return req1;
					}
				}
			}
			return req2;
			// if(req1->arriveTime <= req2->arriveTime){
			// 	return req1;
			// }
			// else{
			// 	return req2;
			// }
		}	
		//Form the batch
		void form_batch()
		{ 	
			cout<<"Form Batch"<<endl;
			for (unsigned int b = 0; b < localBcount; b++){
                for (unsigned int p = 0; p < requestorCriticalTable.size() ; p++){
                    MarkableQ[{p,b}].clear();
					markedMaxLoadPerProc[p] = 0;
					markedTotalLoadPerProc[p] = 0;
                }
            }
			Request* nominate;
			//add requests to corresponding queues depending on the arrival times
			//localBcount = rmax * bmax;
			for (unsigned int p = 0; p < requestorCriticalTable.size() ; p++){
				for (unsigned int b = 0; b < localBcount; b++){
					if(MarkableQ[{p,b}].size() == MarkingCap){
						break;
					}
					else if(MarkableQ[{p,b}].size() < MarkingCap){
						nominate = requestQueue[0]->earliestperBankperReq(p,b); // find earliest arriving requests for each processor at each bank
						if(nominate != NULL){
							if(isSchedulable(nominate, isRowHit(nominate))){
								updateRowTable(nominate->rank, nominate->bank, nominate->row);
								markedCnt--;
								requestQueue[0]->removeRequest();
								MarkableQ[{p,b}].insert(MarkableQ[{p,b}].begin(),nominate);
								markedTotalLoadPerProc[p]++;
								markedCnt++;
								marked[nominate] = true;  // added 
								if(markedCnt > markedMaxLoadPerProc[p]){
									markedMaxLoadPerProc[p] = markedCnt;
								}
								nominate = NULL;	
								b = b - 1;	
							}
						}
						else
						{
							nominate = NULL;
						}																					
					}
				}
			}
		}

		void requestSchedule(){
			//Pick the nominate request
			if (markedCnt == 0)
				form_batch();     
				        			
			unsigned temp_index = 0;
			temp_1 = NULL;

			for (unsigned int p = 0; p < requestorCriticalTable.size() ; p++){
				for (unsigned int b = 0; b < localBcount; b++){
					for (int i = 0; i < MarkableQ[{p,b}].size(); i++ ){
						temp = MarkableQ[{p,b}].at(i);

					}



			

			if(temp != NULL){
				for(unsigned int k = 1; k<requestQueue[0]->getSize(false,0); k++){
					temp_1 = temp;
					temp = BetterRequest(requestQueue[0]->getRequestCheck(k),temp);
					if(temp_1 == temp){
						continue;
					}
					else{
						temp_index = k;
					}
				}

				scheduledRequest = requestQueue[0]->getRequest(temp_index);
				if(isSchedulable(scheduledRequest, isRowHit(scheduledRequest))){
					updateRowTable(scheduledRequest->rank, scheduledRequest->bank, scheduledRequest->row);
					markedCnt--;
					marked[scheduledRequest] = false;
					requestQueue[0]->removeRequest();
				}
				
				scheduledRequest = NULL;
				return;
				temp_1 = NULL;		
				temp_index = 0;
				temp =NULL;	
			}	
		}		
	};
}
#endif