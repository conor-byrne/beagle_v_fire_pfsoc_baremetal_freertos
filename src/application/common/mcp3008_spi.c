/**
 *  @file     mcp3008_spi.c
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

#include "mcp3008_spi.h"

#include "string.h"

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_spi/mss_spi.h"

/**
 * @brief        MCP3008 SPI init funtion
 *
 * @param ctx    pointer to a mcp3008_spi_instance_t structure
 * @param spi    pointer to a mss_spi_instance_t structure
 * @param slave  slave enum
 */
void mcp3008_spi_init(mcp3008_spi_instance_t* ctx, mss_spi_instance_t* spi, mss_spi_slave_t slave)
{
    memset(ctx, 0, sizeof(mcp3008_spi_instance_t));
    ctx->spi = spi;
    ctx->slave = slave;
}


/**
 * @brief                   MCP3008 read single channel
 *
 * @details                 read a single channel from the MCP3008
 *                          channel selector should be one of:
 *                          MCP3008_SINGLE_ENDED_CHx
 *                          or
 *                          MCP3008_DIFFERENTIAL_CH_x_x
 *
 * @note                    the return value (RAW) needs to be scaled by:
 *                          V_in = ((RAW x V_ref) / 1024)
 *
 *                          e.g. when using a 5V reference voltage:
 *                          uint16_t raw_val = mcp3008_spi_read_channel(...);
 *                          float true_val = ((5.0 * raw_val)/1024.0);
 *
 * @param ctx               pointer to a mcp3008_spi_instance_t structure
 * @param channel_selector  channel to read
 *
 * @return                  raw 10-bit ADC value
 */
uint16_t mcp3008_spi_read_channel(mcp3008_spi_instance_t* ctx, uint8_t channel_selector)
{
    /*
     * As shown in the datasheet, 8-bit communication starts with
     * a start bit and then the channel selector:
     *   |0|0|0|0|0|0|0|1| |SGL/DIFF|D2|D1|D0|0|0|0|0|
     */
    uint8_t tx_buffer[2] = { 0x01, channel_selector };
    uint8_t rx_buffer[2] = { 0, 0 };

    MSS_SPI_set_slave_select(ctx->spi, ctx->slave);

    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           rx_buffer,
                           sizeof(rx_buffer));

    MSS_SPI_clear_slave_select(ctx->spi, ctx->slave);

    return (*((uint16_t*)&rx_buffer[0]) & 0x3ff);
}

/**
 * @brief      MCP3008 read all 8 channels in single mode
 *
 * @param ctx  pointer to a mcp3008_spi_instance_t structure
 */
void mcp3008_spi_read_all_channels_single_ended(mcp3008_spi_instance_t* ctx)
{
    ctx->ch0 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH0);
    ctx->ch1 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH1);
    ctx->ch2 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH2);
    ctx->ch3 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH3);
    ctx->ch4 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH4);
    ctx->ch5 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH5);
    ctx->ch6 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH6);
    ctx->ch7 = mcp3008_spi_read_channel(ctx, MCP3008_SINGLE_ENDED_CH7);
}
