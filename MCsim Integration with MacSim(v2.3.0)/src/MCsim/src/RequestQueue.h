/*
MIT License

Copyright (c) 2020 University of Waterloo, Reza Mirosanlou @rmirosan@uwaterloo.ca

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
		bool flag, writeMode;
		bool insertRequest(Request* request);
		// Get the number of requestor buffer 
		unsigned int getQueueSize();	
		unsigned int writeSize();	
		void setWriteMode(bool set);
		bool isWriteMode();
		// Get the number of requests in either the individual buffer, or general buffer
		// The index for requestor is the index in the requestorOrder vector, not ID
		unsigned int getSize(bool requestor, unsigned int index);		
		// Get request from individual buffer
		// ReqInde also indicates the index in requestorOrder vector
		// Index then indicates the index in the corresponding requestorBuffer	
		Request* getRequest(unsigned int reqIndex, unsigned int index);	
		void syncRequest(unsigned int reqIndex, unsigned int index);
		Request* checkRequestIndex(unsigned int reqIndex, unsigned int index);
		// Update open row
	
		void updateRowTable(unsigned rank, unsigned bank, unsigned row);
		unsigned int returnRowTable(unsigned rank, unsigned bank);

		Request* scheduleWritecheck();
		void removeWriteRequest();
		bool switchMode();
		unsigned int generalReadBufferSize(bool requestor);
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
				if(writeQueue.size() > highwatermark) {
					
					return true;
					}
				else{ return false; }
			}
			bool lowWatermark() {
				if(writeQueue.size() < lowwatermark) { 
					return true; }
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
		std::map< unsigned int, std::map< unsigned int, unsigned int long > > bankTable;
		// Bool indicate as requestor or general buffer
		std::pair<bool, std::pair<unsigned int, unsigned int>> scheduledRequest;	
		std::pair<bool, std::pair<unsigned int, unsigned int>> scheduledRequest_RT[8];	
		std::pair<bool, std::pair<unsigned int, unsigned int>> scheduledRequest_HP[8];		
	};
}
#endif




