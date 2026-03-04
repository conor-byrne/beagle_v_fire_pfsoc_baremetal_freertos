/**
 *  @file     bme280_spi_i2c.h
 *
 *  @brief    Using the BME280 temperature/humidity/pressure sensor
 *            I2C and SPI implementations
 *
 *  @details  Data taken from document: BST-BME280-DS001-23
 *            Using https://adafru.it/2652
 *
 *  @author   Conor - GLAS Energy Technology
 *
 *  @date     2026-02-05
 */

#include "stdint.h"

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_spi/mss_spi.h"
#include "drivers/mss/mss_i2c/mss_i2c.h"

#ifndef BME280_SPI_H_
#define BME280_SPI_H_

// write regs
#define BME280_RESET_REG         0xE0
#define BME280_CTRL_HUM_REG      0xF2
#define BME280_CTRL_MEAS_REG     0xF4

// write values - see datasheet
#define BM3280_RESET_VALUE       0xB6
#define BME280_CTRL_HUM_VALUE    0x01
#define BME280_CTRL_MEAS_VALUE   0x27 // 00100111b

// read regs
#define BME280_PRESSURE_REG    0xF7
#define BME280_TEMPERATURE_REG 0xFA
#define BME280_HUMIDITY_REG    0xFD
#define BME280_CALIB00_25_REG  0x88
#define BME280_CALIB26_41_REG  0xE1

// read sizes
#define BME280_PRESSURE_READ_BYTES    3
#define BME280_TEMPERATURE_READ_BYTES 3
#define BME280_HUMIDITY_READ_BYTES    2
#define BME280_CALIB00_25_READ_BYTES  26
#define BME280_CALIB26_41_READ_BYTES  7

#define BME280_CHIP_ID 0x60

#define BME280_I2C_ADDRESS 0x76

typedef struct bme280_instance_t {
    /* Calibration data stored in sensor */
    uint8_t calib_88_a1[BME280_CALIB00_25_READ_BYTES];
    uint8_t calib_e1_f0[BME280_CALIB26_41_READ_BYTES];

    /* Raw reading data */
    uint8_t pressure[3];
    uint8_t temperature[3];
    uint8_t humidity[2];

    int32_t  compensated_temperature;
    int32_t  compensated_pressure;
    uint32_t compensated_humidity;

    /* SPI */
    mss_spi_instance_t* spi;
    mss_spi_slave_t     slave;

    /* I2C */
    mss_i2c_instance_t* i2c;
} bme280_instance_t;

/*** SPI implementation ***/
void bme280_spi_init(bme280_instance_t* ctx, mss_spi_instance_t* spi, mss_spi_slave_t slave);
void bme280_spi_write_control_settings(bme280_instance_t* ctx);
void bme280_spi_read_calibration_data(bme280_instance_t* ctx);
void bme280_spi_read_sensors_data(bme280_instance_t* ctx);

/*** I2C implementation ***/
void bme280_i2c_init(bme280_instance_t* ctx, mss_i2c_instance_t* i2c);
int bme280_i2c_write_control_settings(bme280_instance_t* ctx);
int bme280_i2c_read_calibration_data(bme280_instance_t* ctx);
int bme280_i2c_read_sensors_data(bme280_instance_t* ctx);

#endif /* BME280_SPI_H_ */

