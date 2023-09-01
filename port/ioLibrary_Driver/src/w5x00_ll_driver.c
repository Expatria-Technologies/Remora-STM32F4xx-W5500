/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "w5x00_ll_driver.h"

#include "spi.h"

#include "socket.h"
#if _WIZCHIP_ == W5500
#include "W5500/w5500.h"
#endif

#ifndef WIZCHIP_SPI_PRESCALER
#define WIZCHIP_SPI_PRESCALER SPI_BAUDRATEPRESCALER_4
#endif

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} st_gpio_t;

static struct {
    st_gpio_t cs;
    st_gpio_t rst;
} hw;
static uint32_t prescaler = 0;

static void (*irq_callback)(void);
static volatile bool spin_lock = false;

static void wizchip_select (void)
{
    if(prescaler != WIZCHIP_SPI_PRESCALER)
        prescaler = spi_set_speed(WIZCHIP_SPI_PRESCALER);

    DIGITAL_OUT(hw.cs.port, hw.cs.pin, 0);
}

static void wizchip_deselect (void)
{
    DIGITAL_OUT(hw.cs.port, hw.cs.pin, 1);

    if(prescaler != WIZCHIP_SPI_PRESCALER)
        spi_set_speed(prescaler);
}

static void wizchip_critical_section_lock(void)
{
    while(spin_lock);

    spin_lock = true;
}

static void wizchip_critical_section_unlock(void)
{
    spin_lock = false;
}

static bool wizchip_gpio_interrupt_callback (uint_fast8_t id, bool level)
{
    irq_callback();

    return true;
}

// Public functions

void wizchip_reset (void)
{
    if(hw.rst.port) {
        DIGITAL_OUT(hw.rst.port, hw.rst.pin, 0);
        wizchip_delay_ms(2);
        DIGITAL_OUT(hw.rst.port, hw.rst.pin, 1);
        wizchip_delay_ms(10);
    }
}

wizchip_init_err_t wizchip_initialize (void)
{
    
    GPIO_InitTypeDef GPIO_Init = {
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pin = 1 << W5500_CS_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP
    };

    HAL_GPIO_Init(W5500_CS_PORT, &GPIO_Init);
    DIGITAL_OUT(W5500_CS_PORT, W5500_CS_PIN, 1);


    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pin = 1 << W5500_RST_PIN;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;

    HAL_GPIO_Init(W5500_RST_PORT, &GPIO_Init);
    DIGITAL_OUT(W5500_RST_PORT, W5500_RST_PIN, 1);    

    //set up CS and RST pins
    hw.cs.port = W5500_CS_PORT;
    hw.cs.pin = W5500_CS_PIN;

    hw.rst.port = W5500_RST_PORT;
    hw.rst.pin = W5500_RST_PIN;

    // if(hw.cs.port == NULL)
    //    return error.

    wizchip_deselect();

    spi_init();
    spi_set_speed(WIZCHIP_SPI_PRESCALER);

    wizchip_reset();

    reg_wizchip_cris_cbfunc(wizchip_critical_section_lock, wizchip_critical_section_unlock);
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(spi_get_byte, spi_put_byte);
#ifndef USE_SPI_DMA
    reg_wizchip_spiburst_cbfunc(spi_read, spi_write);
#endif

    /* W5x00 initialize */

#if (_WIZCHIP_ == W5100S)
    uint8_t memsize[2][4] = {{8, 0, 0, 0}, {8, 0, 0, 0}};
#elif (_WIZCHIP_ == W5500)
    uint8_t memsize[2][8] = {{8, 0, 0, 0, 0, 0, 0, 0}, {8, 0, 0, 0, 0, 0, 0, 0}};
#endif

    if(ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
        return WizChipInit_MemErr;

#if _WIZCHIP_ == W5100 ||  _WIZCHIP_ == W5200
    int8_t link_status;
    if (ctlwizchip(CW_GET_PHYLINK, (void *)&link_status) == -1)
        return WizChipInit_UnknownLinkStatus;
#endif

#if (_WIZCHIP_ == W5100S)
    return getVER() == 0x51 ? WizChipInit_OK : WizChipInit_WrongChip;
#elif (_WIZCHIP_ == W5500)
    return getVERSIONR() == 0x04 ? WizChipInit_OK : WizChipInit_WrongChip;
#endif
}

void network_initialize (wiz_NetInfo net_info)
{
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
}

bool wizchip_gpio_interrupt_initialize (uint8_t socket, void (*callback)(void))
{
    int ret_val;
    uint16_t reg_val = SIK_RECEIVED; //(SIK_CONNECTED | SIK_DISCONNECTED | SIK_RECEIVED | SIK_TIMEOUT); // except SendOK

    if((ret_val = ctlsocket(socket, CS_SET_INTMASK, (void *)&reg_val)) == SOCK_OK) {

#if (_WIZCHIP_ == W5100S)
        reg_val = (1 << socket);
#elif (_WIZCHIP_ == W5500)
        reg_val = ((1 << socket) << 8);
#endif
        if(ctlwizchip(CW_SET_INTRMASK, (void *)&reg_val) == 0) {
            irq_callback = callback;
            //hal.irq_claim(IRQ_SPI, 0, wizchip_gpio_interrupt_callback);
        } else
            ret_val = SOCK_FATAL;
    }

    return ret_val == SOCK_OK;
}
