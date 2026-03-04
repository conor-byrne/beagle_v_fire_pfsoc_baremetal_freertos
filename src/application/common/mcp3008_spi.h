/**
 *  @file     mcp3008_spi.h
 *
 *  @brief    Using the MCP3008 10-bit ADC
 *
 *  @details  Data taken from document: DS20001295E
 *            Using https://adafru.it/856
 *
 *  @author   Conor - GLAS Energy Technology
 *
 *  @date     2026-02-07
 */

#ifndef MCP3008_SPI_H_
#define MCP3008_SPI_H_

#include "stdint.h"

#include "drivers/mss/mss_spi/mss_spi.h"

/*
 * channel selection
 * 8 single channels or 4 differential
 */

#define MCP3008_SINGLE_ENDED_CH0 0x80
#define MCP3008_SINGLE_ENDED_CH1 0x90
#define MCP3008_SINGLE_ENDED_CH2 0xA0
#define MCP3008_SINGLE_ENDED_CH3 0xB0
#define MCP3008_SINGLE_ENDED_CH4 0xC0
#define MCP3008_SINGLE_ENDED_CH5 0xD0
#define MCP3008_SINGLE_ENDED_CH6 0xE0
#define MCP3008_SINGLE_ENDED_CH7 0xF0

#define MCP3008_DIFFERENTIAL_CH_0_1 0x00  // CH0 = IN+, CH1 = IN-
#define MCP3008_DIFFERENTIAL_CH_1_0 0x10  // CH0 = IN-, CH1 = IN+
#define MCP3008_DIFFERENTIAL_CH_2_3 0x20  // CH2 = IN+, CH3 = IN-
#define MCP3008_DIFFERENTIAL_CH_3_2 0x30  // CH2 = IN-, CH3 = IN+
#define MCP3008_DIFFERENTIAL_CH_4_5 0x40  // CH4 = IN+, CH5 = IN-
#define MCP3008_DIFFERENTIAL_CH_5_4 0x50  // CH4 = IN-, CH5 = IN+
#define MCP3008_DIFFERENTIAL_CH_6_7 0x60  // CH6 = IN+, CH7 = IN-
#define MCP3008_DIFFERENTIAL_CH_7_6 0x70  // CH6 = IN-, CH7 = IN+

typedef struct mcp3008_spi_instance_t
{
    /* SPI */
    mss_spi_instance_t*     spi;
    mss_spi_slave_t         slave;
    mss_spi_protocol_mode_t mode;

    /* ADC channel values */
    uint16_t ch0 : 10;
    uint16_t ch1 : 10;
    uint16_t ch2 : 10;
    uint16_t ch3 : 10;
    uint16_t ch4 : 10;
    uint16_t ch5 : 10;
    uint16_t ch6 : 10;
    uint16_t ch7 : 10;
} mcp3008_spi_instance_t;

void mcp3008_spi_init(mcp3008_spi_instance_t* ctx, mss_spi_instance_t* spi, mss_spi_slave_t slave);
uint16_t mcp3008_spi_read_channel(mcp3008_spi_instance_t* ctx, uint8_t channel);
void mcp3008_spi_read_all_channels_single_ended(mcp3008_spi_instance_t* ctx);

#endif /* MCP3008_SPI_H_ */
