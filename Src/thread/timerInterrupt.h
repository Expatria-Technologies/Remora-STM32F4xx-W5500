#ifndef TIMERINTERRUPT_H
#define TIMERINTERRUPT_H

#include <stdint.h>
#include "timer.h"
#include "pruThread.h"
#include "pruTimer.h"

class TimerInterrupt; // forward declaration

// Derived class for timer interrupts

class TimerInterrupt : public Interrupt
{
	private:
	    
		pruTimer* InterruptOwnerPtr;
	
	public:

		TimerInterrupt(int interruptNumber, pruTimer* ownerptr);
    
		void ISR_Handler(void);
};

#endif
