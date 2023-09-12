/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdbool.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "spi.h"

#define USE_SPI_DMA 1

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

#ifndef WIZCHIP_SPI_PRESCALER
#define WIZCHIP_SPI_PRESCALER SPI_BAUDRATEPRESCALER_2
#endif

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} st_gpio_t;

static struct {
    st_gpio_t cs;
    st_gpio_t rst;
} hw;
static uint32_t prescaler = WIZCHIP_SPI_PRESCALER;

static void (*irq_callback)(void);
static volatile bool spin_lock = false;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
static inline void wizchip_select(void)
{
    if(prescaler != WIZCHIP_SPI_PRESCALER)
        prescaler = spi_set_speed(WIZCHIP_SPI_PRESCALER);

    HAL_GPIO_WritePin(hw.cs.port, hw.cs.pin, 0);
}

static inline void wizchip_deselect(void)
{
    HAL_GPIO_WritePin(hw.cs.port, hw.cs.pin, 1);

    if(prescaler != WIZCHIP_SPI_PRESCALER)
        spi_set_speed(prescaler);
}

void wizchip_reset()
{
    if(hw.rst.port) {
        HAL_GPIO_WritePin(hw.rst.port, hw.rst.pin, 0);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
        HAL_Delay(250);
        HAL_GPIO_WritePin(hw.rst.port, hw.rst.pin, 1);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
        HAL_Delay(250);
    }
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

void wizchip_spi_initialize(void)
{    

    GPIO_InitTypeDef    GPIO_InitStruct = {0};

    hw.cs.port = SPI_CS_PORT;
    hw.cs.pin = SPI_CS_PIN;

    hw.rst.port = SPI_RST_PORT;
    hw.rst.pin = SPI_RST_PIN;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS_PIN, GPIO_PIN_RESET);

    // Configure the GPIO pin
    GPIO_InitStruct.Pin = hw.cs.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hw.cs.port, &GPIO_InitStruct);  

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(SPI_RST_PORT,SPI_RST_PIN, GPIO_PIN_RESET);

    // Configure the GPIO pin
    GPIO_InitStruct.Pin = hw.rst.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hw.rst.port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(SPI_CS_PORT,SPI_CS_PIN, GPIO_PIN_RESET);



    spi_init();
    wizchip_deselect();
    spi_set_speed(WIZCHIP_SPI_PRESCALER);
}

void wizchip_cris_initialize(void)
{
    spin_lock = false;
    reg_wizchip_cris_cbfunc(wizchip_critical_section_lock, wizchip_critical_section_unlock);
}

void wizchip_initialize(void)
{
    wizchip_reset();

    reg_wizchip_cris_cbfunc(wizchip_critical_section_lock, wizchip_critical_section_unlock);
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(spi_get_byte, spi_put_byte);
    reg_wizchip_spiburst_cbfunc(spi_read, spi_write);

    /* W5x00 initialize */
    uint8_t temp;
#if (_WIZCHIP_ == W5100S)
    uint8_t memsize[2][4] = {{8, 0, 0, 0}, {8, 0, 0, 0}};
#elif (_WIZCHIP_ == W5500)
    uint8_t memsize[2][8] = {{8, 0, 0, 0, 0, 0, 0, 0}, {8, 0, 0, 0, 0, 0, 0, 0}};
#endif

    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
        printf(" W5x00 initialized fail\n");

        return;
    }

    /* Check PHY link status */
    do
    {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
        {
            printf(" Unknown PHY link status\n");

            return;
        }
    } while (temp == PHY_LINK_OFF);
}

void wizchip_check(void)
{
#if (_WIZCHIP_ == W5100S)
    /* Read version register */
    if (getVER() != 0x51)
    {
        printf(" ACCESS ERR : VERSION != 0x51, read value = 0x%02x\n", getVER());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W5500)
    /* Read version register */
    if (getVERSIONR() != 0x04)
    {
        printf(" ACCESS ERR : VERSION != 0x04, read value = 0x%02x\n", getVERSIONR());

        while (1)
            ;
    }
#endif
}

/* Network */
void network_initialize(wiz_NetInfo net_info)
{
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
}

void print_network_information(wiz_NetInfo net_info)
{
    uint8_t tmp_str[8] = {
        0,
    };

    ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);

    if (net_info.dhcp == NETINFO_DHCP)
    {
        printf("====================================================================================================\n");
        printf(" %s network configuration : DHCP\n\n", (char *)tmp_str);
    }
    else
    {
        printf("====================================================================================================\n");
        printf(" %s network configuration : static\n\n", (char *)tmp_str);
    }

    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    printf("====================================================================================================\n\n");
}
