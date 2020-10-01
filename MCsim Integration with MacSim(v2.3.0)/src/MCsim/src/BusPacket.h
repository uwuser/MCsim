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

#ifndef BUSPACKET_H
#define BUSPACKET_H

#include <cstdint>

namespace MCsim
{
	enum BusPacketType
	{
		RD, RDA, WR, WRA,						// CAS access
		ACT_R, ACT_W, ACT, PRE, PREA,			// Open/Close operation - ACT_R and ACT_W are basically same but it was differentiated since it could be useful in some controllers.
		REF, REFPB,									// Refresh
		PDE, PDX, SRE, SRX,						// Power Related Command
		DATA
	};

	class BusPacket
	{
	public:
		BusPacket(BusPacketType packetType, uint64_t id, uint64_t addr, 
			 unsigned col, unsigned rw, unsigned bank, unsigned rank, unsigned sa, void *data, unsigned time):
		busPacketType(packetType),
		requestorID(id),
		address(addr),
		column(col),
		row(rw),
		subArray(sa),
		bank(bank),
		rank(rank),
		data(data),
		arriveTime(time)
		{
			postCommand = false;
			postCounter = 0;
		}
		BusPacketType busPacketType;
		uint64_t requestorID;
		uint64_t address;
		unsigned column;
		unsigned row;
		unsigned subArray;
		unsigned bank;
		unsigned bankGroup;
		unsigned rank;
		void *data;
		unsigned arriveTime;
		bool postCommand;
		unsigned postCounter;
		unsigned int addressMap[4];
	};
}

#endif /* BUSPACKET_H */
