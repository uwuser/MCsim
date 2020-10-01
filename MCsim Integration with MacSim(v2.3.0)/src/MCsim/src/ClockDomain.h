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

#include <iostream>

#include <cmath>
#include <stdint.h>

namespace ClockDomain
{

    template <typename ReturnT>
    class CallbackBase
    {
        public:
        virtual ReturnT operator()() = 0;
    };


    template <typename ConsumerT, typename ReturnT>
    class Callback: public CallbackBase<ReturnT>
    {
        private:
        typedef ReturnT (ConsumerT::*PtrMember)();

        public:
        Callback(ConsumerT* const object, PtrMember member) : object(object), member(member) {}

        Callback(const Callback<ConsumerT,ReturnT> &e) : object(e.object), member(e.member) {}

        ReturnT operator()()
        {
            return (const_cast<ConsumerT*>(object)->*member)();
        }

        private:
        ConsumerT* const object;
        const PtrMember  member;
    };

    typedef CallbackBase <void> ClockUpdateCB;


	class ClockDomainCrosser
	{
		public:
		ClockUpdateCB *callback;
		uint64_t clock1, clock2;
		uint64_t counter1, counter2;
		ClockDomainCrosser(ClockUpdateCB *_callback);
		ClockDomainCrosser(uint64_t _clock1, uint64_t _clock2, ClockUpdateCB *_callback);
		ClockDomainCrosser(double ratio, ClockUpdateCB *_callback);
		void update();
	};


	class TestObj
	{
		public:
		TestObj() {}
		void cb();
		int test();
	};
}
