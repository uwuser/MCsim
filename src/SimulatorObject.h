#ifndef SIMULATOROBJ_H
#define SIMULATOROBJ_H

#include <stdint.h>

namespace MCsim
{
	class SimulatorObject
	{
	public:
		uint64_t clockCycle;

		void step();
		virtual void update() = 0;
	};
} // namespace MCsim

#endif
