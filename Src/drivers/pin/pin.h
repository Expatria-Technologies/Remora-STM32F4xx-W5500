#ifndef PIN_H
#define PIN_H

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

#include "stm32f4xx_hal.h"

#define BITBAND_PERI(x, b) (*((__IO uint8_t *) (PERIPH_BB_BASE + (((uint32_t)(volatile const uint32_t *)&(x)) - PERIPH_BASE)*32 + (b)*4)))
#define DIGITAL_OUT(port, pin, on) { BITBAND_PERI((port)->ODR, pin) = on; }
#define DIGITAL_IN(port, pin) BITBAND_PERI(port->IDR, pin)

#define INPUT 0x0
#define OUTPUT 0x1

#define NONE        0b000
#define OPENDRAIN   0b001
#define PULLUP      0b010
#define PULLDOWN    0b011
#define PULLNONE    0b100

class Pin
{
    private:

        std::string         portAndPin;
        uint8_t             dir;
        uint8_t             modifier;
        uint8_t             portIndex;
        uint16_t            pinNumber;
        uint16_t            pin;
        uint32_t            mode;
        uint32_t            pull;
        uint32_t            speed;
        GPIO_TypeDef*       GPIOx;
        GPIO_InitTypeDef    GPIO_InitStruct = {0};

    public:

        Pin(std::string, int);
        Pin(std::string, int, int);

        void configPin();
        void initPin();
        void setAsOutput();
        void setAsInput();
        void pull_none();
        void pull_up();
        void pull_down();

        inline bool get()
        {
            return HAL_GPIO_ReadPin(this->GPIOx, this->pin);
        }

        inline void set(bool value)
        {
            if (value)
            {
                HAL_GPIO_WritePin(this->GPIOx, this->pin, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(this->GPIOx, this->pin, GPIO_PIN_RESET);
            }
        }
};

#endif
