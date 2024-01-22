#ifndef PRU_TIMER_H
#define PRU_TIMER_H

#include <stdint.h>
#include "timer.h"
#include "pruThread.h"

class TimerInterrupt; // forward declaration
class pruThread; // forward declaration

class pruTimer
{
	friend class TimerInterrupt;

	private:

		TimerInterrupt* 	interruptPtr;
		uint8_t				slice;
		uint32_t 			frequency;
		pruThread* 			timerOwnerPtr;

		void startTimer(void);
		void timerTick();			// Private timer tiggered method

	public:

		pruTimer(uint8_t slice, uint32_t frequency, pruThread* ownerPtr);
        void stopTimer(void);

};

#endif
