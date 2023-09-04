#include <stdio.h>

#include "timer.h"

#include "configuration.h"
#include "../interrupt/interrupt.h"
#include "timerInterrupt.h"
#include "pruThread.h"

extern "C" void PWM_Wrap_Handler();
extern "C" void PWM_Wrap_Handler0();
extern "C" void PWM_Wrap_Handler1();

// Timer constructor
pruTimer::pruTimer(uint8_t slice, uint32_t frequency, pruThread* ownerPtr):
	slice(slice),
	frequency(frequency),
	timerOwnerPtr(ownerPtr)
{
	interruptPtr = new TimerInterrupt(this->slice, this);	// Instantiate a new Timer Interrupt object and pass "this" pointer

	this->startTimer();
}


void pruTimer::timerTick(void)
{
	//base thread is run from interrupt context.  Servo thread is not and can get interrupted.
    this->timerOwnerPtr->execute = true;
    if (this->slice == 0)
	    this->timerOwnerPtr->run();
}



void pruTimer::startTimer(void)
{
    uint32_t period;
    
    printf("    setting up timer Slice %d\n", this->slice);
   
    if (this->slice == 0)
        period = BASE_PERIOD;
    else if (this->slice == 1)
        period = SERVO_PERIOD;
    else
        period = 0;
    printf("    actual period = %d\n", period);

 // Single-shot 1 us per tick
    //base thread
    if (this->slice == 0){
        STEPPER_TIMER_CLOCK_ENA();
        STEPPER_TIMER->CR1 |= TIM_CR1_OPM|TIM_CR1_DIR|TIM_CR1_CKD_1|TIM_CR1_ARPE|TIM_CR1_URS;
        STEPPER_TIMER->PSC = 45;
        STEPPER_TIMER->ARR = BASE_PERIOD;
        STEPPER_TIMER->SR &= ~TIM_SR_UIF;
        STEPPER_TIMER->CNT = 0;
        STEPPER_TIMER->DIER |= TIM_DIER_UIE;

        HAL_NVIC_SetPriority(STEPPER_TIMER_IRQn, 0, 0);
        NVIC_EnableIRQ(STEPPER_TIMER_IRQn);
    } 

 // Single-shot 1 us per tick
    //servo thread
        else if (this->slice == 1){
        PULSE_TIMER_CLOCK_ENA();
        PULSE_TIMER->CR1 |= TIM_CR1_OPM|TIM_CR1_DIR|TIM_CR1_CKD_1|TIM_CR1_ARPE|TIM_CR1_URS;
        PULSE_TIMER->PSC = 45;
        PULSE_TIMER->ARR = SERVO_PERIOD;
        PULSE_TIMER->SR &= ~(TIM_SR_UIF|TIM_SR_CC1IF);
        PULSE_TIMER->CNT = 0;
        PULSE_TIMER->DIER |= TIM_DIER_UIE;

        HAL_NVIC_SetPriority(PULSE_TIMER_IRQn, 1, 1);
        NVIC_EnableIRQ(PULSE_TIMER_IRQn);   
    } else{
        printf("	Invalid Slice\n");
    }

    printf("	timer started\n");
}

void pruTimer::stopTimer()
{
    printf("	timer stop\n\r");
}