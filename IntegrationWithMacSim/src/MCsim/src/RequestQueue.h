

#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include <vector>
#include <map>
#include "Request.h"

using namespace std;

namespace MCsim
{
	class RequestQueue
	{
	public:
		RequestQueue(bool perRequestor, bool writeQueueEable);
		virtual ~RequestQueue();
		bool isWriteEnable();
		bool isPerRequestor();
		bool flag;
		bool insertRequest(Request* request);
		// Get the number of requestor buffer 
		unsigned int getQueueSize();	
		unsigned int writeSize();	
		// Get the number of requests in either the individual buffer, or general buffer
		// The index for requestor is the index in the requestorOrder vector, not ID
		unsigned int getSize(bool requestor, unsigned int index);		
		// Get request from individual buffer
		// ReqInde also indicates the index in requestorOrder vector
		// Index then indicates the index in the corresponding requestorBuffer	
		Request* getRequest(unsigned int reqIndex, unsigned int index);	
		Request* checkRequestIndex(unsigned int reqIndex, unsigned int index);
		Request* scheduleWritecheck();
		void removeWriteRequest();
		bool switchMode();
		
		// Check request from general buffer	
		Request* getRequestCheck(unsigned int index);

		// Get request from general buffer																
		Request* getRequest(unsigned int index);
		// Remove the most recently accessed request
		void removeRequest();
		Request* earliestperBankperReq(unsigned int p, unsigned int b);
		bool isEmpty();

		class WriteQueue
		{
		public:
			WriteQueue(unsigned int low, unsigned int high){
				lowwatermark = low;
				highwatermark = high;
			}
			virtual ~WriteQueue() {
				for(auto it=writeQueue.begin(); it!=writeQueue.end(); it++) { delete (*it); }
				writeQueue.clear();
			}
			void insertWrite(Request* request){ writeQueue.push_back(request); }
			unsigned int bufferSize() {return writeQueue.size(); }
			bool highWatermark() {
				if(writeQueue.size() >= highwatermark) {return true;}
				else{ return false; }
			}
			bool lowWatermark() {
				if(writeQueue.size() <= lowwatermark) { return true; }
				else {return false; }
			}
			Request* getWrite(unsigned int index){ 
				return writeQueue[index]; }
			Request* popWrite(unsigned int index)
			{ 
				Request* temp = NULL;
				temp = writeQueue[index]; 
				writeQueue.erase(writeQueue.begin()+index);
				return temp; 
			}
			void removeWrite(unsigned int index){ 
				writeQueue.erase(writeQueue.begin()+index);
				}


		private:
			unsigned int lowwatermark;
			unsigned int highwatermark;
			std::vector<Request*> writeQueue;
		};
		WriteQueue* writeQueue;

	private:
		bool writeQueueEnable;
		bool perRequestorEnable;

		std::vector<Request*> generalBuffer;
		std::map<unsigned int, std::vector<Request*> > requestorBuffer;
		// Record the requestor target to the request queue
		// Order matters for intra-bank scheduling among all requestors
		std::vector<unsigned int> requestorOrder;

		// Bool indicate as requestor or general buffer
		std::pair<bool, std::pair<unsigned int, unsigned int>> scheduledRequest;		
	};
}
#endif




