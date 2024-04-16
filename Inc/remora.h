#ifndef REMORA_H
#define REMORA_H
#pragma pack(push, 1)

#include "main_init.h"

#include "configuration.h"

typedef union
{
  // this allow structured access to the incoming SPI data without having to move it
  struct
  {
    uint8_t rxBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    volatile int32_t jointFreqCmd[JOINTS]; 	// Base thread commands ?? - basically motion
    float setPoint[VARIABLES];		  // Servo thread commands ?? - temperature SP, PWM etc
    uint8_t jointEnable;
    uint32_t outputs;
    uint8_t spare0;
  };
} rxData_t;

typedef union
{
  // this allow structured access to the out going SPI data without having to move it
  struct
  {
    uint8_t txBuffer[BUFFER_SIZE];
  };
  struct
  {
    int32_t header;
    int32_t jointFeedback[JOINTS];	  			// Base thread feedback ??
    float processVariable[VARIABLES];		    // Servo thread feedback ??
	uint32_t inputs;
	uint16_t NVMPGinputs;
  };
} txData_t;

typedef union
{
	struct
	{
		uint8_t payload[57];
	};
	struct
	{
		int32_t	header;
		int8_t	byte0;
		int8_t	byte1;
		int32_t xPos;
		int32_t yPos;
		int32_t zPos;
		int32_t aPos;
		int32_t bPos;
		int32_t cPos;
		int8_t	byte24;
		int8_t	reset;
		int8_t	byte26;
		int32_t spindle_rpm;
		int8_t	spindle_on;
		int8_t	feed_rate_override;
		int8_t	slow_jog_rate;
		int8_t	spindle_rate_override;
		int8_t	spare35;
		int8_t	parameter_select;
		int8_t	axis_select;
		int8_t	mpg_multiplier;
		int8_t	spare39;
		int8_t	spare40;
		int8_t	spare41;
		int8_t	spare42;
		int8_t	spare43;
		int8_t	spare44;
		int8_t	spare45;
		int8_t	spare46;
		int8_t	spare47;
		int8_t	spare48;
		int8_t	spare49;
		int8_t	spare50;
	};
} mpgData_t;

#ifdef SOCAT_RS485
typedef union
{
	struct
	{
		char touartbuffer[BUFFER_SIZE];
		char fromuartbuffer[BUFFER_SIZE];
		uint8_t to_counter;
		uint8_t from_counter;	
	};

} rs485Data_t;
#endif

typedef struct {
    rxData_t rxBuffers[2]; // Two buffers for rxData_t
    int currentRxBuffer;   // Index of the current rxData_t buffer
} RxPingPongBuffer;

typedef struct {
    txData_t txBuffers[2]; // Two buffers for txData_t
    int currentTxBuffer;   // Index of the current txData_t buffer
} TxPingPongBuffer;

int remora_main(void);

extern void initRxPingPongBuffer(RxPingPongBuffer* buffer);
extern void initTxPingPongBuffer(TxPingPongBuffer* buffer);
extern void swapRxBuffers(RxPingPongBuffer* buffer);
extern void swapTxBuffers(TxPingPongBuffer* buffer);
extern rxData_t* getCurrentRxBuffer(RxPingPongBuffer* buffer);
extern txData_t* getCurrentTxBuffer(TxPingPongBuffer* buffer);

#pragma pack(pop)
#endif