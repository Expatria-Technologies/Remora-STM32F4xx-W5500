/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _W5X00_LL_DRIVER_H_
#define _W5X00_LL_DRIVER_H_

#include "stm32f4xx_hal.h"
#include "wizchip_conf.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define W5500_CS_PORT GPIOA
#define W5500_CS_PIN 15

#define W5500_RST_PORT GPIOA
#define W5500_RST_PIN 9

#define W5500_IRQ_PORT GPIOC
#define W5500_IRQ_PIN 3

#define BITBAND_PERI(x, b) (*((__IO uint8_t *) (PERIPH_BB_BASE + (((uint32_t)(volatile const uint32_t *)&(x)) - PERIPH_BASE)*32 + (b)*4)))
#define DIGITAL_OUT(port, pin, on) { BITBAND_PERI((port)->ODR, pin) = on; }
#define DIGITAL_IN(port, pin) BITBAND_PERI(port->IDR, pin)

typedef enum {
    WizChipInit_OK = 0,
    WizChipInit_MemErr = -1,
    WizChipInit_WrongChip = -2,
    WizChipInit_UnknownLinkStatus = -3
} wizchip_init_err_t;

/* GPIO */
/*! \brief Initialize w5x00 gpio interrupt callback function
 *  \ingroup w5x00_gpio_irq
 *
 *  Add a w5x00 interrupt callback.
 *
 *  \param socket socket number
 *  \param callback the gpio interrupt callback function
 */
bool wizchip_gpio_interrupt_initialize(uint8_t socket, void (*callback)(void));

/*! \brief W5x00 chip reset
 *  \ingroup w5x00_spi
 *
 *  Set a reset pin and reset.
 *
 *  \param none
 */
void wizchip_reset(void);

/*! \brief Initialize WIZchip
 *  \ingroup w5x00_spi
 *
 *  Set callback function to read/write byte using SPI.
 *  Set callback function for WIZchip select/deselect.
 *  Set memory size of W5x00 chip and monitor PHY link status.
 *
 *  \param none
 */
wizchip_init_err_t wizchip_initialize (void);

/* Network */
/*! \brief Initialize network
 *  \ingroup w5x00_spi
 *
 *  Set network information.
 *
 *  \param net_info network information.
 */
void network_initialize(wiz_NetInfo net_info);

#endif /* _W5X00_LL_DRIVER_H_s */
