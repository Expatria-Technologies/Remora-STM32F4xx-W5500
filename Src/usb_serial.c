/*

  usb_serial.c - USB serial port implementation for STM32F103C8 ARM processors

  Part of grblHAL

  Copyright (c) 2019-2021 Terje Io

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "main_init.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "usb_serial.h"

#define BUFNEXT(ptr, buffer) ((ptr + 1) & (sizeof(buffer.data) - 1))
#define BUFCOUNT(head, tail, size) ((head >= tail) ? (head - tail) : (size - tail + head))

typedef struct {
    volatile uint_fast16_t head;
    volatile uint_fast16_t tail;
    volatile bool rts_state;
    bool overflow;
    bool backup;
    char data[16];
} stream_rx_buffer_t;

static stream_rx_buffer_t rxbuf = {0};

volatile usb_linestate_t usb_linestate = {0};

static bool is_connected (void)
{
    return usb_linestate.pin.dtr && uwTick - usb_linestate.timestamp >= 15;
}

//
// Writes a single character to the USB output stream, blocks if buffer full
//
static bool usbPutC (const char c)
{
    static uint8_t buf[1];
    uint32_t ms = uwTick;  //50 ms timeout
    uint32_t timeout_ms = ms + 50;    

    *buf = c;

    while(CDC_Transmit_FS(buf, 1) == USBD_BUSY) {
        if (ms > timeout_ms){
            return false;
        }        
        ms = uwTick;            
    }

    return true;
}

#if USB_DEBUG

int _write(int file, char *ptr, int len) { 
    while(CDC_Transmit_FS((uint8_t*) ptr, len) == USBD_BUSY);     
    return len; 
}

#endif

// NOTE: add a call to this function as the first line CDC_Receive_FS() in usbd_cdc_if.c
void usbBufferInput (uint8_t *data, uint32_t length)
{
    while(length--) {

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

}