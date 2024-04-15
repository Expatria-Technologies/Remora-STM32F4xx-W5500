#include "spindlePwm.h"

// Module for NVMEM spindle RPM on pin PA_0

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON
************************************************************************/

void createSpindlePWM()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    int sp = module["SP[i]"];

	//cannot use a constant pointer
    //ptrSetPoint[sp] = &rxData.setPoint[sp];
    Module* spindle = new SpindlePWM(sp);
    servoThread->registerModule(spindle);

	printf("Make Spindle PWM %d\n", sp);

}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

SpindlePWM::SpindlePWM(int setpoint)
{
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	sp = setpoint;

	__HAL_RCC_TIM1_CLK_ENABLE();

	this->prescaler = 60;
	this->period = 10000;

	this->htim.Instance = TIM1;
	this->htim.Init.Prescaler = this->prescaler-1;
	this->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
	this->htim.Init.Period = this->period-1;
	this->htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	this->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_PWM_Init(&this->htim);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	HAL_TIMEx_MasterConfigSynchronization(&this->htim, &sMasterConfig);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	//sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	HAL_TIM_PWM_ConfigChannel(&this->htim, &sConfigOC, TIM_CHANNEL_1);

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**TIM1 GPIO Configuration
	PA8     ------> TIM1_CH1
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_TIM_PWM_Start(&this->htim, TIM_CHANNEL_1);
	TIM1->CCR1 = 0;
}



void SpindlePWM::update()
{

    rxData_t* currentRxPacket = getCurrentRxBuffer(&rxPingPongBuffer);
	this->ptrPwmPulseWidth = &currentRxPacket->setPoint[sp];
	
	if (*(this->ptrPwmPulseWidth) != this->pwmPulseWidth)
    {
        // PWM duty has changed
        this->pwmPulseWidth = *(this->ptrPwmPulseWidth);

        if (this->pwmPulseWidth > 100.0)
        {
        	this->pwmPulseWidth = 100.0;
        }

        if (this->pwmPulseWidth < 0.0)
        {
        	this->pwmPulseWidth = 0.0;
        }

        this->pulse = (uint32_t)((float)this->period*(this->pwmPulseWidth / 100.0));
        if (this->pulse == 0)
        {
        	TIM5->CCR1 = 0;
        }
        else
        {
        	TIM5->CCR1 = this->pulse-1;
        }
    }

    return;
}


void SpindlePWM::slowUpdate()
{
	return;
}
