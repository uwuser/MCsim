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
