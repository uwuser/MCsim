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

#ifdef CMD_TRACE_ENABLED
#define TRACE_CMD(str) std::cerr<< str <<std::endl;
#else
#define TRACE_CMD(str) 
#endif

#ifdef REQ_TRACE_ENABLED
#define TRACE_REQ(str) std::cerr<< str <<std::endl;
#else
#define TRACE_REQ(str) 
#endif

#ifdef DEBUG_ENABLED
#define DEBUG(str) std::cerr<< str <<std::endl;
#else
#define DEBUG(str) 
#endif

#define PRINT(str) std::cerr<< str <<std::endl;
#define ERROR(str) std::cerr<<"[ERROR ("<<__FILE__<<":"<<__LINE__<<")]: "<<str<<std::endl;

namespace MCsim
{
	// Memory Organization Table
	enum memOrg
	{
		Rank, BankGroup, Bank, SubArray, Row, Column
	};
}