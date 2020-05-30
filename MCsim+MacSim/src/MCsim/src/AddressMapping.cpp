#include "AddressMapping.h"
#include "global.h"

using namespace MCsim;

AddressMapping::AddressMapping(const std::string& format, const unsigned int (&memTable)[6])
{
	unsigned int memIndex = 0;
	for(unsigned index=0; index<format.size(); index++) {
		memIndex = format[index] - '0';
		if(memIndex > memOrg::Column) {
			std::cout<<"Wrong Operation"<<std::endl;
			abort();
		}
		else {
			decodeIndex.push_back(std::make_pair(memIndex,log2(memTable[memIndex])));
		}
	}
}

void AddressMapping::addressMapping(Request* request)
{
	// Case 1: Interleaved controllers such as (AMC, RTMem, PMC), normal translation 
	// Decode the address from trace according to the address mapping scheme
	unsigned tempA = request->address;
	unsigned tempB = request->address;
	unsigned tempAddr = 0;
	for(int index=decodeIndex.size()-1; index>=0; index--) {
		tempA = tempA >> decodeIndex[index].second;
		tempA = tempA << decodeIndex[index].second;
		tempAddr = tempA ^ tempB;
		tempB = tempA >> decodeIndex[index].second;
		tempA = tempB;
		switch (decodeIndex[index].first) {
			case memOrg::Rank: 
				request->addressMap[Rank] = tempAddr;				
				request->rank = tempAddr; 
				break;
			case memOrg::BankGroup:
				request->addressMap[BankGroup] = tempAddr;
				request->bankGroup = tempAddr;
				break;
			case memOrg::Bank:
				request->addressMap[Bank] = tempAddr;
				request->bank = tempAddr;
				break;
			case memOrg::SubArray: 
				request->addressMap[SubArray] = tempAddr;
				request->subArray = tempAddr;
				break;
			case memOrg::Row:
				request->addressMap[Row] = tempAddr;
				request->row = tempAddr;
				break;
			case memOrg::Column:
				request->addressMap[Column] = tempAddr;
				request->col = tempAddr;
				break;
			default:
				abort();
		}
	}

// Case 2: Single Rank Bank Privatized Controllers
//#ifdef SINGLE_RANK_BANK_PRIVATIZATION
	request->rank = 0;
	request->bank = request->requestorID;
//#endif

// Case 3: Multiple Rank Bank Privatized Controllers- Manual allocation (Required some code modification based on the needs)
#ifdef MULTI_RANK_BANK_PRIVATIZATION
	// It should be carefully modified according to the requirement of specific controllers 
	// Request->rank = request->requestorID%4;
	// ROC rank assignment
	if(request->requestorID<8) {
		request->rank = request->requestorID%3;
		// request->rank = 0;
	}
	else {
		// Request->rank = 2+ request->requestorID%2;
		request->rank = 3;// + request->requestorID%2;
	}
	request->bank = request->requestorID;
#endif
	request->addressMap[Rank] = request->rank;
	request->addressMap[BankGroup] = request->bankGroup;
	request->addressMap[Bank] = request->bank;
	request->addressMap[SubArray] = request->subArray;
}
