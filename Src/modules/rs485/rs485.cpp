#include "rs485.h"

#include <stdio.h>


/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON
************************************************************************/

void createRS485()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

	int baud_rate = module["Baud Rate"];

	printf("Make RS485 baud rate %d\n", baud_rate);

    Modbus = new ModbusRS485(*ptrRs485Data, baud_rate);
    servoThread->registerModule(Modbus);
}

/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

static stream_rx_buffer_t rxbuf2 = {0};
static stream_tx_buffer_t txbuf2 = {0};

static bool serial2PutC (const char c)
{
    uint32_t next_head = BUFNEXT(txbuf2.head, txbuf2);   // Set and update head pointer

    while(txbuf2.tail == next_head) {           // While TX buffer full
        UART2->CR1 |= USART_CR1_TXEIE;          // Enable TX interrupts???
    }

    txbuf2.data[txbuf2.head] = c;               // Add data to buffer
    txbuf2.head = next_head;                    // and update head pointer

    UART2->CR1 |= USART_CR1_TXEIE;              // Enable TX interrupts

    return true;
}

// Writes a number of characters from a buffer to the serial output stream, blocks if buffer full
//
static void serial2Write (volatile char *s, uint16_t length)
{
    char *ptr = (char *)s;

    while(length--)
        serial2PutC(*ptr++);
}

//
// serialGetC - returns -1 if no data available
//
static int16_t serial2GetC (void)
{
    uint_fast16_t tail = rxbuf2.tail;       // Get buffer pointer

    if(tail == rxbuf2.head)
        return -1; // no data available

    char data = rxbuf2.data[tail];          // Get next character
    rxbuf2.tail = BUFNEXT(tail, rxbuf2);    // and update pointer

    return (int16_t)data;
}

extern "C" {
	void UART2_IRQHandler (void)
	{
		if(UART2->SR & USART_SR_RXNE) {
			uint32_t data = UART2->DR;

				uint16_t next_head = BUFNEXT(rxbuf2.head, rxbuf2);  // Get and increment buffer pointer
				if(next_head == rxbuf2.tail)                        // If buffer full
					rxbuf2.overflow = 1;                            // flag overflow
				else {
					rxbuf2.data[rxbuf2.head] = (char)data;          // if not add data to buffer
					rxbuf2.head = next_head;                        // and update pointer
				}
			
		}

		if((UART2->SR & USART_SR_TXE) && (UART2->CR1 & USART_CR1_TXEIE)) {
			uint_fast16_t tail = txbuf2.tail;           // Get buffer pointer
			UART2->DR = txbuf2.data[tail];              // Send next character
			txbuf2.tail = tail = BUFNEXT(tail, txbuf2); // and increment pointer
			if(tail == txbuf2.head)                     // If buffer empty then
				UART2->CR1 &= ~USART_CR1_TXEIE;         // disable UART TX interrupt
	}
	}
}

ModbusRS485::ModbusRS485(volatile rs485Data_t &ptrData, int baud_rate) :
	modrs485Data(&ptrData)
{

	modrs485Data = &ptrData;

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	printf("Creating RS485 module\n");

    UART2_CLK_En();

        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Pin       = (1 << UART2_RX_PIN)|(1 << UART2_TX_PIN);
        GPIO_InitStruct.Alternate = UART2_AF;
    
    HAL_GPIO_Init(UART2_PORT, &GPIO_InitStruct);

    UART2->CR1 = USART_CR1_RE|USART_CR1_TE;
    UART2->BRR = UART_BRR_SAMPLING16(UART2_CLK, baud_rate);
    UART2->CR1 |= (USART_CR1_UE|USART_CR1_RXNEIE);

    HAL_NVIC_SetPriority(UART2_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(UART2_IRQ);

	printf("RS485 Interface configured\n");	

}


void ModbusRS485::update()
{
	//if there are bytes in the buffer mark them for transmission
	if(modrs485Data->to_counter){
		serial2Write(modrs485Data->touartbuffer, modrs485Data->to_counter);
		modrs485Data->to_counter = 0;
	}

	if(modrs485Data->from_counter == 0) //if previous transmissions are finished
	{
		int_fast16_t c;
		while(c != -1){
		c = serial2GetC();
			if(c!=-1) {
				modrs485Data->fromuartbuffer[modrs485Data->from_counter] = (char) c;
				modrs485Data->from_counter++;
			}
		}
	}
	
}


void ModbusRS485::slowUpdate()
{
	return;
}

void ModbusRS485::configure()
{
	// use standard module configure method to set payload flag
	this->payloadReceived = true;
}

void ModbusRS485::handleInterrupt()
{
	//this->serialReceived = true;
	//HAL_DMA_IRQHandler(&this->hdma_usart1_rx);
	//HAL_UART_Receive_DMA(&this->uartHandle, (uint8_t*)&this->recvChar, 1);
	//HAL_UARTEx_ReceiveToIdle_DMA(&this->uartHandle, (uint8_t*)&this->modrs485Data->fromuartbuffer,64);
}



