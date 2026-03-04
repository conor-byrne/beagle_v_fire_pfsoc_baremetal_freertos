/**
 *  @file     bme280_spi_i2c.c
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

#include "bme280_spi_i2c.h"

#include "string.h"

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_spi/mss_spi.h"
#include "drivers/mss/mss_i2c/mss_i2c.h"

/*
 * Compenstaion formulae - don't blame me for how crazy they look.
 * Uses the calibration data to convert the raw readings into correct data.
 */

static void bme280_compensate_temperature(bme280_instance_t* ctx)
{
    int32_t adc_T;
    int32_t var1;
    int32_t var2;
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    ctx->compensated_temperature = 0;

    data_msb = (uint32_t)ctx->temperature[0] << 12;
    data_lsb = (uint32_t)ctx->temperature[1] << 4;
    data_xlsb = (uint32_t)ctx->temperature[2] >> 4;
    adc_T = (int32_t)(data_msb | data_lsb | data_xlsb);

    uint16_t dig_T1 = *((uint16_t*)&ctx->calib_88_a1[0]);
    int16_t  dig_T2 = *((int16_t*)&ctx->calib_88_a1[2]);
    int16_t  dig_T3 = *((int16_t*)&ctx->calib_88_a1[4]);

    var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1)))>> 12) * ((int32_t)dig_T3)) >> 14;
    ctx->compensated_temperature = (((var1+var2) * 5 + 128) >> 8);
}

static void bme280_compensate_humidity(bme280_instance_t* ctx)
{
    int32_t adc_H;
    uint32_t data_lsb;
    uint32_t data_msb;
    int32_t var1;
    int32_t var2;
    int32_t var3;
    int32_t var4;
    int32_t var5;
    uint32_t humidity;
    uint32_t humidity_max = 102400;

    ctx->compensated_humidity = 0;

    data_msb = (uint32_t)ctx->humidity[0] << 8;
    data_lsb = (uint32_t)ctx->humidity[1];
    adc_H = (int32_t)(data_msb | data_lsb);

    uint8_t dig_H1 = ctx->calib_88_a1[24];
    int16_t dig_H2 = *((int16_t*)&ctx->calib_e1_f0[0]); // e1 e2
    uint8_t dig_H3 = ctx->calib_e1_f0[2];               // e3
    int16_t dig_H4 =  (((int16_t)ctx->calib_e1_f0[3]) << 4) | ((int16_t)(ctx->calib_e1_f0[4] & 0x0F));
    int16_t dig_H5 = *((int16_t*)&ctx->calib_e1_f0[4]); // e5 e6
    int8_t dig_H6 = (int8_t)ctx->calib_e1_f0[6];        // e7

    var1 = ctx->compensated_temperature - ((int32_t)76800);
    var2 = (int32_t)(adc_H * 16384);
    var3 = (int32_t)(((int32_t)dig_H4) * 1048576);
    var4 = ((int32_t)dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)dig_H6)) / 1024;
    var3 = (var1 * ((int32_t)dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    humidity = (uint32_t)(var5 / 4096);

    if (humidity > humidity_max)
    {
        humidity = humidity_max;
    }

    ctx->compensated_humidity = humidity;
}

static void bme280_compensate_pressure(bme280_instance_t* ctx)
{
    int32_t adc_P;
    int64_t var1;
    int64_t var2;
    int64_t p;
    uint32_t data_xlsb;
    uint32_t data_lsb;
    uint32_t data_msb;

    data_msb = (uint32_t)ctx->pressure[0] << 12;
    data_lsb = (uint32_t)ctx->pressure[1] << 4;
    data_xlsb = (uint32_t)ctx->pressure[2] >> 4;
    adc_P = (int32_t)(data_msb | data_lsb | data_xlsb);

    ctx->compensated_pressure = 0;

    uint16_t dig_P1 = *((uint16_t*)&ctx->calib_88_a1[6]);
    int16_t  dig_P2 = *((int16_t*)&ctx->calib_88_a1[8]);
    int16_t  dig_P3 = *((int16_t*)&ctx->calib_88_a1[10]);
    int16_t  dig_P4 = *((int16_t*)&ctx->calib_88_a1[12]);
    int16_t  dig_P5 = *((int16_t*)&ctx->calib_88_a1[14]);
    int16_t  dig_P6 = *((int16_t*)&ctx->calib_88_a1[16]);
    int16_t  dig_P7 = *((int16_t*)&ctx->calib_88_a1[18]);
    int16_t  dig_P8 = *((int16_t*)&ctx->calib_88_a1[20]);
    int16_t  dig_P9 = *((int16_t*)&ctx->calib_88_a1[22]);

    var1 = ((int64_t)ctx->compensated_temperature) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
    var2 = var2 + (((int64_t)dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
    if (var1 == 0)
    {
        ctx->compensated_pressure = 0;
        return; // avoid exception caused by division by zero
    }
    p = 1048576-adc_P;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
    ctx->compensated_pressure = (int32_t)p;
}

/*** SPI implementation ***/

/**
 * @brief      bme280 SPI init funtion
 *
 * @details    wipes the passed bme280_instance_t structure for a fresh start
 *             and then assigns the passed mss_spi_instance_t to it
 *
 * @pre        spi is expected to be already initialised with MSS_SPI_init(...)
 *             and MSS_SPI_configure_master_mode(...)
 *
 * @param ctx  pointer to a bme280_instance_t structure
 * @param i2c  pointer to a mss_spi_instance_t structure
 */
void bme280_spi_init(bme280_instance_t* ctx, mss_spi_instance_t* spi, mss_spi_slave_t slave)
{
    memset(ctx, 0, sizeof(bme280_instance_t));
    ctx->spi = spi;
    ctx->slave = slave;
}

/**
 * @brief      bm280 write control settings
 *
 * @details    sensor needs to be configured with control settings
 *             before it will start reading properly
 *
 * @param ctx  pointer to a bme280_instance_t structure
 *
 * @note       using fixed control values, change BME280_CTRL_HUM_VALUE
 *             and/or BME280_CTRL_MEAS_VALUE for different control settings
 */
void bme280_spi_write_control_settings(bme280_instance_t* ctx)
{
    /* write data can be sent as reg/value pairs
     * write commands must have MSB of reg set to 0, hence the & 0x7F */
    uint8_t tx_buffer[4] = { BME280_CTRL_HUM_REG & 0x7F,
                             BME280_CTRL_HUM_VALUE,
                             BME280_CTRL_MEAS_REG & 0x7F,
                             BME280_CTRL_MEAS_VALUE };

    MSS_SPI_set_slave_select(ctx->spi, ctx->slave);
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           NULL,
                           0);
    MSS_SPI_clear_slave_select(ctx->spi, ctx->slave);
}

/**
 * @brief      bm280 read calibration data
 *
 * @details    raw sensor data needs to be compensated by the sensor's
 *             internal calibration data
 *
 * @param ctx  pointer to a bme280_instance_t structure
 */
void bme280_spi_read_calibration_data(bme280_instance_t* ctx)
{
    uint8_t tx_buffer[1];

    MSS_SPI_set_slave_select(ctx->spi, ctx->slave);

    /* read commands must have MSB of reg set to 1, hence the | 0x80 */
    tx_buffer[0] = BME280_CALIB00_25_REG | 0x80;
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           ctx->calib_88_a1,
                           BME280_CALIB00_25_READ_BYTES);

    tx_buffer[0] = BME280_CALIB26_41_REG | 0x80;
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           ctx->calib_e1_f0,
                           BME280_CALIB26_41_READ_BYTES);

    MSS_SPI_clear_slave_select(ctx->spi, ctx->slave);
}

/**
 * @brief      bme280 read sensor data
 *
 * @details    reads raw sensor data from bme280 and applies
 *             the required compensation, final values are stored
 *             in the bme280_instance_t structure
 *
 * @param ctx  pointer to a bme280_instance_t structure
 */
void bme280_spi_read_sensors_data(bme280_instance_t* ctx)
{
    uint8_t tx_buffer[1];

    MSS_SPI_set_slave_select(ctx->spi, ctx->slave);

    /* read commands must have MSB of reg set to 1, hence the | 0x80 */

    /* read pressure */
    tx_buffer[0] = BME280_PRESSURE_REG | 0x80;
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           ctx->pressure,
                           BME280_PRESSURE_READ_BYTES);

    /* read temperature */
    tx_buffer[0] = BME280_TEMPERATURE_REG | 0x80;
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           ctx->temperature,
                           BME280_TEMPERATURE_READ_BYTES);

    /* read humidity */
    tx_buffer[0] = BME280_HUMIDITY_REG | 0x80;
    MSS_SPI_transfer_block(ctx->spi,
                           tx_buffer,
                           sizeof(tx_buffer),
                           ctx->humidity,
                           BME280_HUMIDITY_READ_BYTES);

    MSS_SPI_clear_slave_select(ctx->spi, ctx->slave);

    // turn the raw data into the compensated values
    bme280_compensate_temperature(ctx);
    bme280_compensate_humidity(ctx);
    bme280_compensate_pressure(ctx);
}

/*** I2C implementation ***/

/**
 * @brief      bme280 I2C init funtion
 *
 * @details    wipes the passed bme280_instance_t structure for a fresh start
 *             and then assigns the passed mss_i2c_instance_t to it
 *
 * @pre        i2c is expected to be already initialised with MSS_I2C_init(...)
 *
 * @param ctx  pointer to a bme280_instance_t structure
 * @param i2c  pointer to a mss_i2c_instance_t structure
 */
void bme280_i2c_init(bme280_instance_t* ctx, mss_i2c_instance_t* i2c)
{
    memset(ctx, 0, sizeof(bme280_instance_t));
    ctx->i2c = i2c;
}

/**
 * @brief      bm280 write control settings
 *
 * @details    sensor needs to be configured with control settings
 *             before it will start reading properly
 *
 * @param ctx  pointer to a bme280_instance_t structure
 *
 * @note       using fixed control values, change BME280_CTRL_HUM_VALUE
 *             and/or BME280_CTRL_MEAS_VALUE for different control settings
 *
 * @return     0 on success
 */
int bme280_i2c_write_control_settings(bme280_instance_t* ctx)
{
    /* write data can be sent as reg/value pairs */
    uint8_t tx_buffer[4] = { BME280_CTRL_HUM_REG,
                             BME280_CTRL_HUM_VALUE,
                             BME280_CTRL_MEAS_REG,
                             BME280_CTRL_MEAS_VALUE };

    MSS_I2C_write(ctx->i2c,
                  BME280_I2C_ADDRESS,
                  tx_buffer,
                  sizeof(tx_buffer),
                  MSS_I2C_RELEASE_BUS);

    return (MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS);
}

/**
 * @brief      bm280 read calibration data
 *
 * @details    raw sensor data needs to be compensated by the sensor's
 *             internal calibration data
 *
 * @param ctx  pointer to a bme280_instance_t structure
 *
 * @return     0 on success
 */
int bme280_i2c_read_calibration_data(bme280_instance_t* ctx)
{
    uint8_t tx_buffer[1];

    /* read commands must have MSB of reg set to 1, hence the | 0x80 */
    tx_buffer[0] = BME280_CALIB00_25_REG;
    MSS_I2C_write_read(ctx->i2c,
                       BME280_I2C_ADDRESS,
                       tx_buffer, sizeof(tx_buffer),
                       ctx->calib_88_a1, BME280_CALIB00_25_READ_BYTES,
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS)
        return -1;

    tx_buffer[0] = BME280_CALIB26_41_REG;
    MSS_I2C_write_read(ctx->i2c,
                       BME280_I2C_ADDRESS,
                       tx_buffer, sizeof(tx_buffer),
                       ctx->calib_e1_f0, BME280_CALIB26_41_READ_BYTES,
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS)
        return -1;

    return 0;
}

/**
 * @brief      bme280 read sensor data
 *
 * @details    reads raw sensor data from bme280 and applies
 *             the required compensation, final values are stored
 *             in the bme280_instance_t structure
 *
 * @param ctx  pointer to a bme280_instance_t structure
 *
 * @return     0 on success
 */
int bme280_i2c_read_sensors_data(bme280_instance_t* ctx)
{
    uint8_t tx_buffer[1];

    /* read pressure */
    memset(ctx->pressure, 0, sizeof(ctx->pressure));
    tx_buffer[0] = BME280_PRESSURE_REG;
    MSS_I2C_write_read(ctx->i2c,
                       BME280_I2C_ADDRESS,
                       tx_buffer, sizeof(tx_buffer),
                       ctx->pressure, BME280_PRESSURE_READ_BYTES,
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS)
        return -1;

    /* read temperature */
    memset(ctx->temperature, 0, sizeof(ctx->temperature));
    tx_buffer[0] = BME280_TEMPERATURE_REG;
    MSS_I2C_write_read(ctx->i2c,
                       BME280_I2C_ADDRESS,
                       tx_buffer, sizeof(tx_buffer),
                       ctx->temperature, BME280_TEMPERATURE_READ_BYTES,
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS)
        return -1;

    /* read humidity */
    memset(ctx->humidity, 0, sizeof(ctx->humidity));
    tx_buffer[0] = BME280_HUMIDITY_REG;
    MSS_I2C_write_read(ctx->i2c,
                       BME280_I2C_ADDRESS,
                       tx_buffer, sizeof(tx_buffer),
                       ctx->humidity, BME280_HUMIDITY_READ_BYTES,
                       MSS_I2C_RELEASE_BUS);

    if(MSS_I2C_wait_complete(ctx->i2c, MSS_I2C_NO_TIMEOUT) != MSS_I2C_SUCCESS)
        return -1;

    // turn the raw data into the compensated values
    bme280_compensate_temperature(ctx);
    bme280_compensate_humidity(ctx);
    bme280_compensate_pressure(ctx);

    return 0;
}
