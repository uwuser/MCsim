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

#include "ClockDomain.h"

using namespace std;



namespace ClockDomain
{
	// "Default" crosser with a 1:1 ratio
	ClockDomainCrosser::ClockDomainCrosser(ClockUpdateCB *_callback)
		: callback(_callback), clock1(1UL), clock2(1UL), counter1(0UL), counter2(0UL)
	{
	}
	ClockDomainCrosser::ClockDomainCrosser(uint64_t _clock1, uint64_t _clock2, ClockUpdateCB *_callback) 
		: callback(_callback), clock1(_clock1), clock2(_clock2), counter1(0), counter2(0)
	{}

	ClockDomainCrosser::ClockDomainCrosser(double ratio, ClockUpdateCB *_callback)
		: callback(_callback), counter1(0), counter2(0)
	{
		double x = ratio;

		const int MAX_ITER = 15;
		size_t i;
		unsigned ns[MAX_ITER], ds[MAX_ITER];
		double zs[MAX_ITER];
		ds[0] = 0;
		ds[1] = 1;
		zs[1] = x;
		ns[1] = (int)x; 

		for (i = 1; i<MAX_ITER-1; i++)
		{
			if (fabs(x - (double)ns[i]/(double)ds[i]) < 0.00005)
			{
				break;
			}

			zs[i+1] = 1.0f/(zs[i]-(int)floor(zs[i])); // 1/(fractional part of z_i)
			ds[i+1] = ds[i]*(int)floor(zs[i+1])+ds[i-1];
			double tmp = x*ds[i+1];
			double tmp2 = tmp - (int)tmp;
			ns[i+1] = tmp2 >= 0.5 ? ceil(tmp) : floor(tmp); // ghetto implementation of a rounding function
			//printf("i=%lu, z=%20f n=%5u d=%5u\n",i,zs[i],ns[i],ds[i]);
		}

		//printf("APPROXIMATION= %u/%d\n",ns[i],ds[i]);
		this->clock1=ns[i];
		this->clock2=ds[i];
	}

	void ClockDomainCrosser::update()
	{
		//short circuit case for 1:1 ratios
		if (clock1 == clock2 && callback)
		{
			(*callback)();
			return; 
		}

		// Update counter 1.
		counter1 += clock1;

		while (counter2 < counter1)
		{
			counter2 += clock2;
			if (callback)
			{
				(*callback)();
			}
		}

		if (counter1 == counter2)
		{
			counter1 = 0;
			counter2 = 0;
		}
	}



	void TestObj::cb()
	{
			cout << "In Callback\n";
	}

	int TestObj::test()
	{
		ClockUpdateCB *callback = new Callback<TestObj, void>(this, &TestObj::cb);
		ClockDomainCrosser x(0.5, callback);
		cout <<"------------------------------------------\n";
		ClockDomainCrosser y(0.3333, callback);
		cout <<"------------------------------------------\n";
		ClockDomainCrosser z(0.9, callback);
		cout <<"------------------------------------------\n";
		for (int i=0; i<10; i++)
		{
			
			x.update();
			cout << "UPDATE: counter1= " << x.counter1 << "; counter2= " << x.counter2 << "; " << endl;
		}

		return 0;
	}


}
