#include "digitalPin.h"
//#include "../boardconfig.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createDigitalPin()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    const char* pin = module["Pin"];
    const char* mode = module["Mode"];
    const char* invert = module["Invert"];
    const char* modifier = module["Modifier"];
    int dataBit = module["Data Bit"];

    int mod;
    bool inv;

    if (!strcmp(invert,"True"))
    {
        inv = true;
    }
    else inv = false;

    //ptrOutputs = &pruRxData->outputs;
    //ptrInputs = &pruTxData->inputs;

    printf("Make Digital %s at pin %s\n", mode, pin);

    if (!strcmp(mode,"Output"))
    {
        Module* digitalPin = new DigitalPin(1, pin, dataBit, inv, mod);
        servoThread->registerModule(digitalPin);
    }
    else if (!strcmp(mode,"Input"))
    {
        Module* digitalPin = new DigitalPin(0, pin, dataBit, inv, mod);
        servoThread->registerModule(digitalPin);
    }
    else
    {
        printf("Error - incorrectly defined Digital Pin\n");
    }

}


/***********************************************************************
    MODULE CONFIGURATION AND CREATION FROM STATIC CONFIG - boardconfi.h   
************************************************************************/

void loadStaticIO()
{

}


/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

//DigitalPin::DigitalPin(volatile uint32_t &ptrData, int mode, std::string portAndPin, int bitNumber, bool invert, int modifier) :
DigitalPin::DigitalPin(int mode, std::string portAndPin, int bitNumber, bool invert, int modifier) :
    mode(mode),
	portAndPin(portAndPin),
	bitNumber(bitNumber),
    invert(invert),
	modifier(modifier)
{
	this->pin = new Pin(this->portAndPin, this->mode, this->modifier);		// Input 0x0, Output 0x1
	this->mask = 1 << this->bitNumber;
    //printf("ptrData = %x\n", ptrData); //can no longer just use a single pointer.
}


void DigitalPin::update()
{
	bool pinState;
    rxData_t* currentRxPacket = getCurrentRxBuffer(&rxPingPongBuffer);
	txData_t* currentTxPacket = getCurrentTxBuffer(&txPingPongBuffer);

	if (this->mode == 0x0)									// the pin is configured as an input
	{
		pinState = this->pin->get();
		if(this->invert)
		{
			pinState = !pinState;
		}

		if (pinState == 1)								// input is high
		{
			currentTxPacket->inputs |= this->mask;
		}
		else											// input is low
		{
			currentTxPacket->inputs &= ~this->mask;
		}
	}
	else												// the pin is configured as an output
	{
		pinState = currentRxPacket->outputs & this->mask;		// get the value of the bit in the data source
		if(this->invert)
		{
			pinState = !pinState;
		}
		this->pin->set(pinState);			// simple conversion to boolean
	}
}

void DigitalPin::slowUpdate()
{
	return;
}
