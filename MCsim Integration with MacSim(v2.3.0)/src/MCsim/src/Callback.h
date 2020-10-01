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


#include <stdint.h> // uint64_t

#ifndef CALLBACK_H
#define CALLBACK_H

namespace MCsim
{

template <typename ReturnT, typename Param1T, typename Param2T,
typename Param3T>
class CallbackBase
{
public:
	virtual ~CallbackBase() = 0;
	virtual ReturnT operator()(Param1T, Param2T, Param3T) = 0;
};

template <typename Return, typename Param1T, typename Param2T, typename Param3T>
MCsim::CallbackBase<Return,Param1T,Param2T,Param3T>::~CallbackBase() {}

template <typename ConsumerT, typename ReturnT,
typename Param1T, typename Param2T, typename Param3T >
class Callback: public CallbackBase<ReturnT,Param1T,Param2T,Param3T>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(Param1T,Param2T,Param3T);

public:
	Callback( ConsumerT* const object, PtrMember member) :
			object(object), member(member)
	{
	}

	Callback( const Callback<ConsumerT,ReturnT,Param1T,Param2T,Param3T>& e ) :
			object(e.object), member(e.member)
	{
	}

	ReturnT operator()(Param1T param1, Param2T param2, Param3T param3)
	{
		return (const_cast<ConsumerT*>(object)->*member)
		       (param1,param2,param3);
	}

private:

	ConsumerT* const object;
	const PtrMember  member;
};

typedef CallbackBase <void, unsigned, uint64_t, uint64_t> TransactionCompleteCB;
} // namespace Controller

#endif
