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

#ifndef REQUEST_H
#define REQUEST_H

#include <stdint.h>
#include <iostream>
#include <map>

namespace MCsim
{
	enum RequestType
	{
		DATA_READ,
		DATA_WRITE,
	};

	class Request
	{
	public:
		Request(unsigned int id, RequestType requestType, unsigned int size, unsigned long long addr, void *data):
			requestType(requestType),
			requestorID(id),
			requestSize(size),
			address(addr),
			data(data),
			returnTime(0)
		{}
		// Fields
		RequestType requestType;
		unsigned int requestorID;
		unsigned int requestSize;
		unsigned long long address;
		void *data;
		unsigned int arriveTime;
		unsigned int returnTime;
		unsigned int rank;
		unsigned int bankGroup;
		unsigned int bank;
		unsigned int subArray;
		unsigned int row;
		unsigned int col;
		unsigned int addressMap[4];
		// Rank, BankGroup, Bank, SubArray
	};
}

#endif /* REQUEST_H */
