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
    
    printf("    setting up timer Slice %d\r\n", this->slice);
   
    if (this->slice == 0)
        period = BASE_PERIOD;
    else if (this->slice == 1)
        period = SERVO_PERIOD;
    else
        period = 0;
    printf("    actual period = %d\r\n", period);

 // Single-shot 100 ns per tick
    //base thread
    if (this->slice == 0){
        /*TEPPER_TIMER_CLOCK_ENA();
                
        STEPPER_TIMER->CR1 &= ~TIM_CR1_CEN;
        STEPPER_TIMER->CR1 |= TIM_CR1_DIR;
        STEPPER_TIMER->SR &= ~TIM_SR_UIF;

        STEPPER_TIMER->PSC = 86;
        STEPPER_TIMER->ARR = BASE_PERIOD;        
        
        STEPPER_TIMER->CNT = 0;
        STEPPER_TIMER->DIER |= TIM_DIER_UIE;

        HAL_NVIC_SetPriority(STEPPER_TIMER_IRQn, 0, 0);
        NVIC_EnableIRQ(STEPPER_TIMER_IRQn);

        STEPPER_TIMER->EGR = TIM_EGR_UG;
        STEPPER_TIMER->CR1 |= TIM_CR1_CEN;*/       
    } 

 // Single-shot 100 ns per tick
    //servo thread
        else if (this->slice == 1){
        /*SERVO_TIMER_CLOCK_ENA();
        SERVO_TIMER->CR1 |= TIM_CR1_OPM|TIM_CR1_DIR|TIM_CR1_CKD_1|TIM_CR1_ARPE|TIM_CR1_URS;
        SERVO_TIMER->PSC = 86;
        SERVO_TIMER->ARR = SERVO_PERIOD;
        SERVO_TIMER->SR &= ~(TIM_SR_UIF|TIM_SR_CC1IF);
        SERVO_TIMER->CNT = 0;
        SERVO_TIMER->DIER |= TIM_DIER_UIE;*/

        SERVO_TIMER->CR1 &= ~TIM_CR1_CEN;
        SERVO_TIMER->CR1 |= TIM_CR1_DIR;
        SERVO_TIMER->SR &= ~TIM_SR_UIF;

        SERVO_TIMER->PSC = 86;
        SERVO_TIMER->ARR = SERVO_PERIOD;        
        
        SERVO_TIMER->CNT = 0;
        SERVO_TIMER->DIER |= TIM_DIER_UIE;

        HAL_NVIC_SetPriority(SERVO_TIMER_IRQn, 2, 2);
        NVIC_EnableIRQ(SERVO_TIMER_IRQn);

        SERVO_TIMER->EGR = TIM_EGR_UG;
        SERVO_TIMER->CR1 |= TIM_CR1_CEN;        
    } else{
        printf("	Invalid Slice\r\n");
    }

    printf("	timer started\r\n");
}

void pruTimer::stopTimer()
{
    printf("	timer stop\n\r");
}