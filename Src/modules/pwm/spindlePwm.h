#ifndef SPINDLEPWM_H
#define SPINDLEPWM_H

#include "stm32f4xx_hal.h"
#include <sys/errno.h>

#include "../module.h"

#include "extern.h"

void createSpindlePWM(void);

class SpindlePWM : public Module
{
	private:

		TIM_HandleTypeDef htim;

        float *ptrPwmPulseWidth; 	// pointer to the data source
        float pwmPulseWidth;                // Pulse width (%)

        uint32_t	prescaler;
        uint32_t	period;
        uint32_t	pulse;
		int 		sp;

	public:

		SpindlePWM(int setpoint);

		virtual void update(void);          // Module default interface
		virtual void slowUpdate(void);      // Module default interface
};

#endif

