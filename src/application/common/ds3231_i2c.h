/**
 *  @file     ds3231_i2c.h
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

#ifndef DS3231_I2C_H_
#define DS3231_I2C_H_

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_i2c/mss_i2c.h"

#include "stdint.h"

#define DS3231_I2C_ADDRESS     0x68

#define DS3231_TIME_REG        0x00
#define DS3231_ALARM1_REG      0x07
#define DS3231_ALARM2_REG      0x0B
#define DS3231_CONTROL_REG     0x0E
#define DS3231_STATUS_REG      0x0F
#define DS3231_TEMPERATURE_REG 0x11

static char ds3231_day_of_week_string[8][10] =
{
    "",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

typedef struct ds3231_datetime_t
{
    uint8_t second;
    uint8_t hour;
    uint8_t minute;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} ds3231_datetime_t;

typedef struct ds3231_control_t
{
    uint8_t alarm_1_int_enable : 1;
    uint8_t alarm_2_int_enable : 1;
    uint8_t interrupt_control : 1;
    uint8_t rate_select_1 : 1;
    uint8_t rate_select_2 : 1;
    uint8_t convert_temperature : 1;
    uint8_t battery_backed_square_wave_enable : 1;
    uint8_t enable_oscillator : 1;
} ds3231_control_t;

typedef struct ds3231_status_t
{
    uint8_t alarm_1_flag : 1;
    uint8_t alarm_2_flag : 1;
    uint8_t busy : 1;
    uint8_t enable_32kHz_output : 1;
    uint8_t unused_bit_4 : 1;
    uint8_t unused_bit_5 : 1;
    uint8_t unused_bit_6 : 1;
    uint8_t oscillator_stop_flag : 1;
} ds3231_status_t;

typedef struct ds3231_instance_t
{
    ds3231_datetime_t datetime;

    union
    {
        ds3231_control_t  vals;
        uint8_t           byte;
    } control;

    union
    {
        ds3231_status_t vals;
        uint8_t         byte;
    } status;

    mss_i2c_instance_t* i2c;
} ds3231_instance_t;

void ds3231_init(ds3231_instance_t* ctx, mss_i2c_instance_t* i2c);
int ds3231_read_control(ds3231_instance_t* ctx);
int ds3231_read_status(ds3231_instance_t* ctx);
int ds3231_read_time(ds3231_instance_t* ctx);
int ds3231_reset_oscillator_stop_flag(ds3231_instance_t* ctx);
int ds3231_write_time(ds3231_instance_t* ctx, uint8_t second, uint8_t minute, uint8_t hour,
                      uint8_t day_of_week, uint8_t day, uint8_t month, uint16_t year);

#endif /* DS3231_I2C_H_ */
