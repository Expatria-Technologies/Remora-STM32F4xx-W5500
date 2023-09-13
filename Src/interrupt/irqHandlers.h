#include "interrupt.h"
#include "../thread/timer.h"

extern "C" {
	// Main stepper driver
	void STEPPER_TIMER_IRQHandler (void)
	{
		STEPPER_TIMER->SR &= ~TIM_SR_UIF;                 // Clear UIF flag
		//STEPPER_TIMER->ARR = BASE_PERIOD;				  //schedule next IRQ
		//STEPPER_TIMER->CNT = 0;
		//STEPPER_TIMER->EGR = TIM_EGR_UG;
		//STEPPER_TIMER->CR1 |= TIM_CR1_CEN;	

		Interrupt::SLICE0_Wrapper();		
	}

	void PULSE_TIMER_IRQHandler (void)
	{
		PULSE_TIMER->SR &= ~TIM_SR_UIF;                 // Clear UIF flag
		//PULSE_TIMER->ARR = SERVO_PERIOD;				//schedule next IRQ
		//PULSE_TIMER->CNT = 0;
		//PULSE_TIMER->EGR = TIM_EGR_UG;
		//PULSE_TIMER->CR1 |= TIM_CR1_CEN;	

		Interrupt::SLICE1_Wrapper();		
	}
	
}
