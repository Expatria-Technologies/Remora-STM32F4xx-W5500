#if USB_SERIAL_CDC


#include "usb_serial.h"

#ifndef RX_BUFFER_SIZE
#define RX_BUFFER_SIZE 1024 // must be a power of 2
#endif

#ifndef TX_BUFFER_SIZE
#define TX_BUFFER_SIZE 512  // must be a power of 2
#endif

#ifndef BLOCK_TX_BUFFER_SIZE
#define BLOCK_TX_BUFFER_SIZE 256
#endif

#define BUFNEXT(ptr, buffer) ((ptr + 1) & (sizeof(buffer.data) - 1))
#define BUFCOUNT(head, tail, size) ((head >= tail) ? (head - tail) : (size - tail + head))

typedef struct {
    volatile uint_fast16_t head;
    volatile uint_fast16_t tail;
    volatile bool rts_state;
    bool overflow;
    bool backup;
    char data[RX_BUFFER_SIZE];
} stream_rx_buffer_t;

// double buffered tx stream
typedef struct {
    uint_fast16_t length;
    uint_fast16_t max_length;
    char *s;
    bool use_tx2data;
    char data[BLOCK_TX_BUFFER_SIZE];
    char data2[BLOCK_TX_BUFFER_SIZE];
} stream_block_tx_buffer2_t;

static stream_rx_buffer_t rxbuf = {0};
static stream_block_tx_buffer2_t txbuf = {0};

// NOTE: add a call to this function as the first line CDC_Receive_FS() in usbd_cdc_if.c
void usbBufferInput (uint8_t *data, uint32_t length)
{
    while(length--) {
                 // Check and strip realtime commands,
            uint16_t next_head = BUFNEXT(rxbuf.head, rxbuf);    // Get and increment buffer pointer
            if(next_head == rxbuf.tail)                         // If buffer full
                rxbuf.overflow = 1;                             // flag overflow
            else {
                rxbuf.data[rxbuf.head] = *data;                 // if not add data to buffer
                rxbuf.head = next_head;                         // and update pointer
            }

        data++;                                                 // next...
    }
}

// NOTE: USB interrupt priority should be set lower than stepper/step timer to avoid jitter
// It is set in HAL_PCD_MspInit() in usbd_conf.c
void usbInit (void)
{

    MX_USB_DEVICE_Init();
    
    return;
}

#endif
