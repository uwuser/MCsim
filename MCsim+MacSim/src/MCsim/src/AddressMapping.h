#ifndef ADDRESS_MAPPING_H
#define ADDRESS_MAPPING_H

#include <vector>
#include <map>
#include <iostream>
#include "Request.h"

namespace MCsim
{
	class AddressMapping
	{
	public:
		AddressMapping(const std::string& format, const unsigned int (&memTable)[6]);
		void addressMapping(Request* request);

	private:
		std::vector<std::pair<unsigned int, unsigned int>> decodeIndex; 
		unsigned log2(unsigned num_value) {
			unsigned logbase2 = 0;
			unsigned value = num_value;
			unsigned orig = value;
			value>>=1;
			while (value>0) {
				value >>= 1;
				logbase2++;
			}
			if ((unsigned)1<<logbase2<orig)logbase2++;
			return logbase2;	
		}
	};
}

#endif /* ADDRESS_MAPPING_H */
