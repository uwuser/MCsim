#ifndef RAMULATOR_DDR4_H
#define RAMULATOR_DDR4_H

#include "../dram/DRAM.h"
#include "../dram/DDR4.h"

#include <queue>
#include "MemoryDevice.h"

using namespace ramulator;
namespace MCsim
{
	template <typename T>
	class Ramulator_DDR4: public MemoryDevice
	{	
	public:
	bool DSARP_enabled;
		Ramulator_DDR4(T* spec, unsigned int ranks): MemoryDevice(ranks)
		{
			spec->set_rank_number(ranks);
			channel = new DRAM<T>(spec, T::Level::Channel);
			for(int index = 0; index < int(T::Level::MAX); index++) {
				addr_vec.push_back(-1);
			}

			if(int(T::Level::Bank) != int(T::Level::Rank)+1) {
				bankGroups = channel->spec->org_entry.count[int(T::Level::Bank)-1];
			}

			banks = channel->spec->org_entry.count[int(T::Level::Bank)];

			if(int(T::Level::Row) != int(T::Level::Bank)+1) {
				subArrays = channel->spec->org_entry.count[int(T::Level::Row)-1];
			}
			rows = channel->spec->org_entry.count[int(T::Level::Row)];
			columns = channel->spec->org_entry.count[int(T::Level::Column)];
			
			dataBusWidth = channel->spec->org_entry.dq;
		}
		float get_constraints(const string& parameter) 
		{
			float param = 0;
			if(parameter == "tCK") {
				param = channel->spec->speed_entry.tCK;
			}
			else if(parameter == "tRCD") {
				param = channel->spec->speed_entry.nRCD;
			}
			else if(parameter == "tWL") {
				param = channel->spec->speed_entry.nCWL;
			}
			else if(parameter == "tRL") {
				param = channel->spec->speed_entry.nCL;
			}
			else if(parameter == "tWTR") {
				//param = channel->spec->speed_entry.nWTR; /// for DDR3 DEVICE - change according to the device tech  TODO
				 param = channel->spec->speed_entry.nWTRS;
			}
			else if(parameter == "tRTW") {
				param = channel->spec->speed_entry.nCL + channel->spec->speed_entry.nBL + 2 - channel->spec->speed_entry.nCWL;
			}
			else if (parameter == "tBus") {
				param = channel->spec->speed_entry.nBL;
			}
			else if (parameter == "tREFI") {
				param = channel->spec->speed_entry.nREFI;
			}
			else if (parameter == "tRFC") {
				param = channel->spec->speed_entry.nRFC;
			}
			else if (parameter == "tCCDL") {
				param = channel->spec->speed_entry.nCCDL;
			}
			else if (parameter == "tCCDS") {
				param = channel->spec->speed_entry.nCCDS;
			}
			else if (parameter == "tRRDS") {
				param = channel->spec->speed_entry.nRRDS;
			}	
			else if (parameter == "tRRDL") {
				param = channel->spec->speed_entry.nRRDL;	
			}	
			else if (parameter == "tXS") {
				param = channel->spec->speed_entry.nXS;	
			}		
			else if (parameter == "tFAW") {
				param = channel->spec->speed_entry.nFAW;
			}
			
			else {
				param = 0;
			}
			return param;		
		}
		
		unsigned int command_timing(BusPacket* command, int type) 
		{
			int time = 0;
				switch(command->busPacketType) 
				{
				case PRE:
					if(type == ACT) {
						time = channel->spec->speed_entry.nRP;
					}
					else if(type == ACT_R) {
						time = channel->spec->speed_entry.nRP;
					}
					else if(type == ACT_W) {
						time = channel->spec->speed_entry.nRP;
					}
					break;
				case PREA:
					if(type == ACT) {
						time = channel->spec->speed_entry.nRP;
					}
					else if(type == ACT_R) {
						time = channel->spec->speed_entry.nRP;
					}
					else if(type == ACT_W) {
						time = channel->spec->speed_entry.nRP;
					}
					break;	
				case ACT:
					switch(type) {
						case ACT:
							time = channel->spec->speed_entry.nRC;
							break;
						case PREA:	
						case PRE:
							time = channel->spec->speed_entry.nRAS;
							break;
						case RD:
						case RDA:
						case WR:
						case WRA:
							time = channel->spec->speed_entry.nRCD;
							break;
						default:
							time = 1;
							break;
					}
					break;
				case ACT_R:
					switch(type) {
						case ACT_R:
							time = channel->spec->speed_entry.nRC;
							break;
						case ACT_W:
							time = channel->spec->speed_entry.nRC;
							break;
						case PREA:		
						case PRE:
							time = channel->spec->speed_entry.nRAS;
							break;
						case RD:
						case RDA:
						case WR:
						case WRA:
							time = channel->spec->speed_entry.nRCD;
							break;
						default:
							time = 1;
							break;
					}
					break;	
				case ACT_W:
					switch(type) {
						case ACT_R:
							time = channel->spec->speed_entry.nRC;
							break;
						case ACT_W:
							time = channel->spec->speed_entry.nRC;
							break;
						case PREA:		
						case PRE:
							time = channel->spec->speed_entry.nRAS;
							break;
						case RD:
						case RDA:
						case WR:
						case WRA:
							time = channel->spec->speed_entry.nRCD;
							break;
						default:
							time = 1;
							break;
					}
					break;	
				case RD:
					switch(type) {
						case RD:
						case RDA:
							time = 4;
							break;
						case WR:
						case WRA:
							time = channel->spec->speed_entry.nCL + channel->spec->speed_entry.nBL 
							+ 2 - channel->spec->speed_entry.nCWL;
							break;
						case PRE:
							time = channel->spec->speed_entry.nRTP;
						case PREA:
							time = channel->spec->speed_entry.nRTP;
						default:
							time = 1;
							break;
					}	
					break;				
				case RDA:
					switch(type) {
						case ACT:
							time = channel->spec->speed_entry.nRTP + channel->spec->speed_entry.nRP;
							break;
						default:
							time = 1;
							break;
					}
					break;
				case WR:
					switch(type) {
						case RD:
						case RDA:
							time = channel->spec->speed_entry.nCWL + 4 + 6;
							break;
						case WR:
						case WRA:
							time = 4;
							break;
						case PREA:
							time = channel->spec->speed_entry.nCWL + 4 + channel->spec->speed_entry.nWR;
						case PRE:
							time = channel->spec->speed_entry.nCWL + 4 + channel->spec->speed_entry.nWR;
						default:
							time = 1;
							break;
					}
					break;
				case WRA:
					switch(type) {
						case ACT:
							time = channel->spec->speed_entry.nCWL + 4 + channel->spec->speed_entry.nWR + 
							channel->spec->speed_entry.nRP;
							break;
						default:
							time = 1;
							break;
					}
					break;
				default:
					time = 1;
					break;
				}
			
			
			
			return time;
		}

		long command_timing(BusPacket* command) 
		{
			convert_addr(command);
			return channel->get_next(convert_cmd(command), addr_vec.data()) - clockCycle;
		}

		bool command_check(BusPacket* command) 
		{
			if(command->postCommand) {
		
				return true;
			}
			else {
		
				convert_addr(command);
		
				return channel->check(convert_cmd(command), addr_vec.data(), clockCycle);				
			}

		}

		void receiveFromBus(BusPacket* busPacket) 
		{
			// busPacket->rank
			if(busPacket->postCommand) {
				postBuffer.push_back(new BusPacket(busPacket->busPacketType, busPacket->requestorID, busPacket->address, 
					busPacket->column, busPacket->row, busPacket->bank, 0, busPacket->data, busPacket->arriveTime));
				postCounter.push_back(get_constraints("tRCD"));
				return;
			}
			
			if(busPacket->busPacketType != DATA) {			
				commandTrace<<clockCycle<<" B"<<busPacket->bank<<" "<<busPacket->busPacketType<<"\n";
				convert_addr(busPacket);
				channel->update(convert_cmd(busPacket), addr_vec.data(), clockCycle);
			}
			generateData(busPacket);
		}

	private:
		DRAM<T>* channel;
		vector<int> addr_vec;
		typename T::Command convert_cmd(BusPacket* command) 
		{
			switch(command->busPacketType) 
			{
			case RD:
				return T::Command::RD;
			case WR:
				return T::Command::WR;
			case RDA:
				return T::Command::RDA;
			case WRA:
				return T::Command::WRA;
			case ACT:
				return T::Command::ACT;
			case ACT_R:		 // note that ACT_R is similar to ACT but it gives the designer capability to distinguish between ACT for a read request
				return T::Command::ACT_R;  		// note that ACT_W is similar to ACT but it gives the designer capability to distinguish between ACT for a write request
			case ACT_W:
				return T::Command::ACT_W;		
			case PRE:
				return T::Command::PRE;
			case PREA:
				return T::Command::PREA;
			case REF:
				return T::Command::REF;
			default:
				abort();
			}			
		}
		void convert_addr(BusPacket* command) 
		{
			addr_vec[int(T::Level::Channel)] = 0;
			addr_vec[int(T::Level::Rank)] =  0 ;//command->rank;
			 // No BankGroup
			if(int(T::Level::Bank) == int(T::Level::Rank)+1) {
				addr_vec[int(T::Level::Bank)] = command->bank;
			}
			else {
				addr_vec[int(T::Level::Bank)-1] = 0;//command->bankGroup;
				addr_vec[int(T::Level::Bank)] = command->bank;
			}
			if(int(T::Level::Row) == int(T::Level::Bank)+1) {
				addr_vec[int(T::Level::Row)] = command->row;
			}
			else {
				addr_vec[int(T::Level::Row)-1] = command->subArray;
				addr_vec[int(T::Level::Row)] = command->row;
			}
			addr_vec[int(T::Level::Column)] = command->column;			
		}
	};
}

#endif

