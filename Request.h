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
