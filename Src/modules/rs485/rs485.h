#ifndef NVMPG_H
#define NVMPG_H

#include "stm32f4xx_hal.h"

#include <cstdint>
#include <sys/errno.h>

#include "configuration.h"
#include "remora.h"
#include "../module.h"
#include "../moduleinterrupt.h"

#include "extern.h"

#define RX_BUFFER_SIZE BUFFER_SIZE
#define TX_BUFFER_SIZE BUFFER_SIZE

typedef struct {
    volatile uint_fast16_t head;
    volatile uint_fast16_t tail;
    volatile bool rts_state;
    bool overflow;
    bool backup;
    char data[RX_BUFFER_SIZE];
} stream_rx_buffer_t;

typedef struct {
    volatile uint_fast16_t head;
    volatile uint_fast16_t tail;
    char data[TX_BUFFER_SIZE];
} stream_tx_buffer_t;

#define BUFNEXT(ptr, buffer) ((ptr + 1) & (sizeof(buffer.data) - 1))
#define BUFCOUNT(head, tail, size) ((head >= tail) ? (head - tail) : (size - tail + head))

void createRS485(void);

class ModbusRS485 : public Module
{
	friend class ModuleInterrupt;

	private:
	
		volatile rs485Data_t *modrs485Data;

		//bool serialReceived = false;
		bool payloadReceived = false;
		char recvChar;
		//int baud_rate;

	public:

		ModbusRS485(volatile rs485Data_t&, int baud_rate);
		virtual void update(void);
		virtual void slowUpdate(void);
		virtual void configure(void);
		void handleInterrupt(void);

		void serialWrite(const char *s, uint16_t length);
};

#endif
