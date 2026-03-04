/**
 *  @file     ds3231_i2c.c
 *
 *  @brief    Using the DS3231 RTC
 *
 *  @details  Data taken from document: 19-5170; Rev 10; 3/15
 *            Using https://adafru.it/3013
 *
 *  @author   Conor - GLAS Energy Technology
 *
 *  @date     2026-02-06
 */

#include "ds3231_i2c.h"

#include "string.h"

#include "mpfs_hal/mss_hal.h"
#include "mpfs_hal/common/mss_assert.h"
#include "drivers/mss/mss_i2c/mss_i2c.h"

static uint8_t bcd8_to_bin(uint8_t byte)
{
    return (uint8_t)( ((byte >> 4) * 10) + (byte & 0x0F) );
}

static uint8_t bin_to_bcd8(uint8_t byte)
{
    return (uint8_t)( ((byte/10) << 4) + (byte % 10) );
}

/**
 * @brief      init ds3231 strcuture
 *
 * @details    wipes the passed ds3231_instance_t structure for a fresh start
 *             and then assigns the passed mss_i2c_instance_t to it
 *
 * @pre        i2c is expected to be already initialised with MSS_I2C_init(...)
 *
 * @param ctx  pointer to ds3231_instance_t structure
 * @param i2c  pointer to mss_i2c_instance_t structure
 */
void ds3231_init(ds3231_instance_t* ctx, mss_i2c_instance_t* i2c)
{
    memset(ctx, 0, sizeof(ds3231_instance_t));
    ctx->i2c = i2c;
}

/**
 * @brief      read ds3231 control register
 *
 * @details    reads the control register from the ds3231 and stores it in
 *             the ds3231_instance_t structure as a union of the byte value
 *             and a named bitfiedld for ease of use
 *
 * @param ctx  pointer to ds3231_instance_t structure
 *
 * @return     0 on success
 */
int ds3231_read_control(ds3231_instance_t* ctx)
{
    uint8_t buff[1] = { DS3231_CONTROL_REG };

    MSS_I2C_write_read(ctx->i2c,
                       DS3231_I2C_ADDRESS,
                       buff, sizeof(buff),
                       buff, sizeof(buff),
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) == MSS_I2C_SUCCESS)
    {
        ctx->control.byte = buff[0];
        return 0;
    }
    else
        return -1;
}

/**
 * @brief      read ds3231 status register
 *
 * @details    reads the status register from the ds3231 and stores it in
 *             the ds3231_instance_t structure as a union of the byte value
 *             and a named bitfiedld for ease of use
 *
 * @param ctx  pointer to ds3231_instance_t structure
 *
 * @return     0 on success
 */
int ds3231_read_status(ds3231_instance_t* ctx)
{
    uint8_t buff[1] = { DS3231_STATUS_REG };

    MSS_I2C_write_read(ctx->i2c,
                       DS3231_I2C_ADDRESS,
                       buff, sizeof(buff),
                       buff, sizeof(buff),
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) == MSS_I2C_SUCCESS)
    {
        ctx->status.byte = buff[0];
        return 0;
    }
    else
        return -1;
}

/**
 * @brief      read ds3231 time registers
 *
 * @details    reads the time registers from the ds3231, parses the values and
 *             stores the calander time in the ds3231_instance_t structure
 *
 * @param ctx  pointer to ds3231_instance_t structure
 *
 * @return     0 on success
 */
int ds3231_read_time(ds3231_instance_t* ctx)
{
    uint8_t buff[7] = { DS3231_TIME_REG, 0, 0, 0, 0, 0, 0 };

    MSS_I2C_write_read(ctx->i2c,
                       DS3231_I2C_ADDRESS,
                       buff, 1,
                       buff, sizeof(buff),
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) == MSS_I2C_SUCCESS)
    {
        ctx->datetime.second      = bcd8_to_bin(buff[0] & 0x7F);
        ctx->datetime.minute      = bcd8_to_bin(buff[1]);
        ctx->datetime.hour        = bcd8_to_bin(buff[2]);
        ctx->datetime.day_of_week = bcd8_to_bin(buff[3]);
        ctx->datetime.day         = bcd8_to_bin(buff[4]);
        ctx->datetime.month       = bcd8_to_bin(buff[5] & 0x7F);
        ctx->datetime.year        = (uint16_t)(bcd8_to_bin(buff[6]) + 2000U);
        return 0;
    }
    else
        return -1;
}

/**
 * @brief      reset ds3231 oscillator stop flag
 *
 * @details    The stop flag is not written to directly, instead we write to the
 *             enable oscillator control bit.
 *
 * @param ctx  pointer to ds3231_instance_t structure
 *
 * @return     0 on success
 */
int ds3231_reset_oscillator_stop_flag(ds3231_instance_t* ctx)
{
    uint8_t buff[2] = { DS3231_STATUS_REG, 0 };
    ctx->control.vals.enable_oscillator = 0; // 0 is enable for some reason
    buff[1] = ctx->control.byte;

    MSS_I2C_write(ctx->i2c,
                  DS3231_I2C_ADDRESS,
                  buff, sizeof(buff),
                  MSS_I2C_RELEASE_BUS);

    return (MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS);
}

/**
 * @brief              write time to ds3231
 *
 * @details            sets the time in the ds3231
 *
 * @param ctx          pointer to ds3231_instance_t structure
 * @param second       0-59
 * @param minute       0-59
 * @param hour         0-23
 * @param day_of_week  1-7
 * @param day          1-31
 * @param month        1-12
 * @param year         >=2000
 *
 * @note               the oscillator stop flag needs to be reset after calling
 *                     this function
 *
 * @see                ds3231_reset_oscillator_stop_flag
 *
 * @return             0 on success
 */
int ds3231_write_time(ds3231_instance_t* ctx, uint8_t second, uint8_t minute, uint8_t hour,
                         uint8_t day_of_week, uint8_t day, uint8_t month, uint16_t year)
{
    uint8_t buff[8] = { DS3231_TIME_REG,
                        bin_to_bcd8(second) & 0x7f,
                        bin_to_bcd8(minute) & 0x7f,
                        bin_to_bcd8(hour) & 0x7f,
                        bin_to_bcd8(day_of_week) & 0x07,
                        bin_to_bcd8(day) & 0x3F,
                        bin_to_bcd8(month) & 0x1F,
                        bin_to_bcd8((uint8_t)(year-2000))
    };

    MSS_I2C_write(ctx->i2c,
                  DS3231_I2C_ADDRESS,
                  buff, sizeof(buff),
                  MSS_I2C_RELEASE_BUS);

    return (MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS);
}
