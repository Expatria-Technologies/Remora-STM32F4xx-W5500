#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef union {
    uint8_t value;
    struct {
        uint8_t dtr    :1,
                rts    :1,
                unused :6;
    };
} serial_linestate_t;

typedef struct {
    serial_linestate_t pin;
    uint32_t timestamp;
} usb_linestate_t;

extern volatile usb_linestate_t usb_linestate;

void usbInit (void);
