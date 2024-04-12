#include <cstdio>

#include "pruThread.h"
#include "../modules/module.h"

#include "stm32f4xx_hal.h"
GPIO_TypeDef       GPIO0, GPIO1;
GPIO_InitTypeDef   GPIO_InitStruct = {0};


using namespace std;

// Thread constructor
pruThread::pruThread(uint8_t slice, uint32_t frequency) :
	slice(slice),
	frequency(frequency)
{
	printf("Creating thread %d\n", this->frequency);
	
	if (this->slice == 0){
		__HAL_RCC_GPIOB_CLK_ENABLE();
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10, GPIO_PIN_RESET);

		// Configure the GPIO pin
		GPIO_InitStruct.Pin = GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);  
	}

	if (this->slice == 1){
		__HAL_RCC_GPIOC_CLK_ENABLE();
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6, GPIO_PIN_RESET);

		// Configure the GPIO pin
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); 
	}	

	this->semaphore = false;
	this->execute = false;
}

void pruThread::startThread(void)
{
	TimerPtr = new pruTimer(this->slice, this->frequency, this);
}

void pruThread::stopThread(void)
{
    this->TimerPtr->stopTimer();
}


void pruThread::registerModule(Module* module)
{
	this->vThread.push_back(module);
}


void pruThread::registerModulePost(Module* module)
{
	this->vThreadPost.push_back(module);
	this->hasThreadPost = true;
}


void pruThread::run(void)
{

	if(!this->execute)
		return;	
	
	while (this->semaphore == true);	
	this->semaphore = true;	
	
	if (this->slice == 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	}

	if (this->slice == 1){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	}

	// iterate over the Thread pointer vector to run all instances of Module::runModule()
	for (iter = vThread.begin(); iter != vThread.end(); ++iter) (*iter)->runModule();

	//need a check here on the base thread to ensure the pulse width is correct.
	if(this->frequency > 10000 ){
		for (int j = 67; j > 0; j--) {
			__ASM volatile ("nop");
		}
	}

	// iterate over the second vector that contains module pointers to run after (post) the main vector
	if (hasThreadPost)
	{
		for (iter = vThreadPost.begin(); iter != vThreadPost.end(); ++iter) (*iter)->runModulePost();
	}
	
	if (this->slice == 0){
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	} 

	if (this->slice == 1){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	}

	this->execute = false;
	this->semaphore = false;
}
