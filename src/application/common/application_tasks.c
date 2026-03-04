/**
 *  @file     application_tasks.c
 *
 *  @brief    Various sample tasks
 *
 *  @details  showcase of FreeRTOS, FreeRTOS-Plus-TCP, bacnet-stack
 *            mss-i2c driver, mss-spi driver, mss-ethernet-mac driver,
 *            mss-gpio driver and mss-uart driver
 *
 *  @author   Conor - GLAS Energy Technology
 *
 *  @date     2026-01-26
 */

/* enable peripherals as needed */
//#define SPI_USE_MCP3008
//#define I2C_USE_DS3231
//#define I2C_USE_BME280

#include <stdio.h>
#include <string.h>

#include "inc/common.h"

/* Drivers */
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_gpio/mss_gpio.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "drivers/mss/mss_spi/mss_spi.h"
#include "drivers/mss/mss_i2c/mss_i2c.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* FreeRTOS-Plus-TCP */
#include "FreeRTOS_IP.h"

/* sample webserver task */
#include "httpserver-netconn.h"

/* BACnet stack */
#include "bacnet/basic/binding/address.h"
#include "bacnet/basic/client/bac-task.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/datalink/datalink.h"

/* Peripherals */
#if defined(I2C_USE_BME280) || defined(SPI_USE_BME280)
#include "bme280_spi_i2c.h"
#endif

#ifdef I2C_USE_DS3231
#include "ds3231_i2c.h"
#endif

#ifdef SPI_USE_MCP3008
#include "mcp3008_spi.h"
#endif

extern NetworkInterface_t * g_p_eth_interface;
extern NetworkEndPoint_t * g_p_eth_endpoint;

/**
 *  @brief   start freeRTOS
 *
 *  @details start some tasks and do some network initialisation
 *           before starting the FreeRTOS_IP task
 *
 *  @note    will never return
 */
void free_rtos(void)
{
    TaskHandle_t thandle_blinky;
    TaskHandle_t thandle_bacnet;
    TaskHandle_t thandle_web;

#if defined(SPI_USE_BME280) || defined(SPI_USE_MCP3008)
    TaskHandle_t thandle_spi;
#endif

#if defined(I2C_USE_BME280) || defined(I2C_USE_DS3231)
    TaskHandle_t thandle_i2c;
#endif

    uint8_t user_led = MSS_GPIO_0;
    BaseType_t rtos_result = xTaskCreate(blinky_task,
                              "blinky",
                              256,
                              &user_led,
                              uartPRIMARY_PRIORITY - 2,
                              &thandle_blinky);
    configASSERT(pdPASS == rtos_result);
    vTaskSuspend(thandle_blinky);

    rtos_result = xTaskCreate(http_server_netconn_thread,
                              "http_server_netconn",
                              256,
                              NULL,
                              uartPRIMARY_PRIORITY - 1,
                              &thandle_web);
    configASSERT(pdPASS == rtos_result);
    vTaskSuspend(thandle_web);

    rtos_result = xTaskCreate(bacnet_rtos_task,
                              "bacnet_rtos_task",
                              512,
                              NULL,
                              uartPRIMARY_PRIORITY,
                              &thandle_bacnet);
    configASSERT(pdPASS == rtos_result);
    vTaskSuspend(thandle_bacnet);

#if defined(SPI_USE_BME280) || defined(SPI_USE_MCP3008)
    rtos_result = xTaskCreate(spi_task,
                              "spi_task",
                              256,
                              NULL,
                              uartPRIMARY_PRIORITY + 1,
                              &thandle_spi);
    configASSERT(pdPASS == rtos_result);
    vTaskSuspend(thandle_spi);
#endif

#if defined(I2C_USE_BME280) || defined(I2C_USE_DS3231)
    rtos_result = xTaskCreate(i2c_task,
                              "i2c_task",
                              256,
                              NULL,
                              uartPRIMARY_PRIORITY + 2,
                              &thandle_i2c);
    configASSERT(pdPASS == rtos_result);
    vTaskSuspend(thandle_i2c);
#endif

    pxMPFS_Ethernet_FillInterfaceDescriptor(0, g_p_eth_interface);

    /* The MAC address array is not declared const as the MAC address will
       normally be read from an EEPROM and not hard coded (in real deployed
       applications).*/
    static uint8_t ucMACAddress[6] = { 0x00, 0xFC, 0x00, 0x12, 0x34, 0x58 };

    /* Define the network addressing.  These parameters will be used if either
       ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
       failed. */
    static const uint8_t ucIPAddress[4] = { 10, 0, 0, 201 };
    static const uint8_t ucNetMask[4] = { 255, 255, 248, 0 };
    static const uint8_t ucGatewayAddress[4] = { 10, 0, 0, 1 };

    /* The following is the address of an OpenDNS server. */
    static const uint8_t ucDNSServerAddress[4] = { 208, 67, 222, 222 };

    FreeRTOS_FillEndPoint(g_p_eth_interface,
                          g_p_eth_endpoint,
                          ucIPAddress,
                          ucNetMask,
                          ucGatewayAddress,
                          ucDNSServerAddress,
                          ucMACAddress);
    /* try DHCP? */
    g_p_eth_endpoint->bits.bWantDHCP = pdTRUE;

    /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
       are created in the vApplicationIPNetworkEventHook() hook function
       below.  The hook function is called when the network connects. */
    FreeRTOS_IPInit_Multi();

    vTaskResume(thandle_blinky);
    vTaskResume(thandle_web);
    vTaskResume(thandle_bacnet);

#if defined(SPI_USE_BME280) || defined(SPI_USE_MCP3008)
    vTaskResume(thandle_spi);
#endif

#if defined(I2C_USE_BME280) || defined(I2C_USE_DS3231)
    vTaskResume(thandle_i2c);
#endif

    /* Start the kernel.  From here on, only tasks and interrupts will run. */
    vTaskStartScheduler();

    volatile int ix;
    for(;;)
    {
        /* Should not reach here. */
        ix++;
    }
}

/**
 *  @brief   blinky task
 *
 *  @details Uses user LED as a link status, quick when network is down, slow when up
 *           Can also use the user button as a trigger to start
 *           user LEDs are:
 *           MSS_GPIO_0
 *           MSS_GPIO_1
 *           MSS_GPIO_2
 *           MSS_GPIO_3
 *           MSS_GPIO_4
 *           MSS_GPIO_5
 *           MSS_GPIO_6
 *           MSS_GPIO_7
 *           MSS_GPIO_8
 *           MSS_GPIO_9
 *           MSS_GPIO_11
 *
 *  @param   pvParameters user LED number
 *
 *  @note    never returns
 */
void blinky_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t blinky_value = 1;
    uint8_t user_led_no = *((uint8_t*)pvParameters);

    /* bounds test */
    if(MSS_GPIO_11 > user_led_no || MSS_GPIO_10 == user_led_no)
        user_led_no = 0;

    MSS_GPIO_init(GPIO2_LO);
    MSS_GPIO_config(GPIO2_LO, user_led_no, MSS_GPIO_OUTPUT_MODE);

#if 0
    /*
     * Quick test for the user button, don't start blinky until the button is pressed once
     */
    MSS_GPIO_init(GPIO0_LO);
    MSS_GPIO_config(GPIO0_LO, 13, MSS_GPIO_INPUT_MODE);

    uint32_t io0;
    uint8_t btn;
    do
    {
        io0 = MSS_GPIO_get_inputs(GPIO0_LO);
        btn = (io0 >> 13) & 1;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    while(btn == 1);
#endif

    for (;;)
    {
        MSS_GPIO_set_output(GPIO2_LO, user_led_no, blinky_value);
        blinky_value = !blinky_value;
        if(g_p_eth_interface->pfGetPhyLinkStatus(g_p_eth_interface) && FreeRTOS_IsEndPointUp(g_p_eth_endpoint))
            vTaskDelay(pdMS_TO_TICKS(1000));
        else
            vTaskDelay(pdMS_TO_TICKS(250));
    }
}

/**
 *  @brief   bacnet client task
 *
 *  @details Simple bacnet client using library defined bacnet_task()
 *           Use a BACnet explorer such as YABE to find the device
 *
 *  @note    never returns
 */
void bacnet_rtos_task(void *pvParameters)
{
    (void)pvParameters;
    BACNET_IP_ADDRESS bacaddr_tmp;

    /* wait for the network to come up */
    while((pdFALSE == g_p_eth_interface->pfGetPhyLinkStatus(g_p_eth_interface)) ||
          (pdFALSE == FreeRTOS_IsEndPointUp(g_p_eth_endpoint))
         )
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /*
     * Set up bacnet stack
     */
    bacnet_task_init();
    Device_Set_Object_Instance_Number(4194300);
    address_init();

    /* set ip address */
    memcpy(bacaddr_tmp.address, &g_p_eth_endpoint->ipv4_settings.ulIPAddress, sizeof(bacaddr_tmp.address));
    bip_set_addr(&bacaddr_tmp);

    /* set broadcast */
    memcpy(bacaddr_tmp.address, &g_p_eth_endpoint->ipv4_settings.ulBroadcastAddress, sizeof(bacaddr_tmp.address));
    bip_set_broadcast_addr(&bacaddr_tmp);

    /* set port */
    bip_set_port(0xBAC0);

    /* init the bacnet socket */
    if(bip_init(NULL) == 0)
    {
        configASSERT(pdFALSE);
    }

    for(;;)
    {
        bacnet_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
 * SPI buffer overflow handler
 * This function is called by SPI driver in case of buffer overflow.
 */
static void spi_overflow_handler(uint8_t mss_spi_core)
{
    if (mss_spi_core)
    {
        /* reset SPI1 */
        (void)mss_config_clk_rst(MSS_PERIPH_SPI1, (uint8_t) 1, PERIPHERAL_OFF);

        /* Take SPI1 out of reset. */
        (void)mss_config_clk_rst(MSS_PERIPH_SPI1, (uint8_t) 1, PERIPHERAL_ON);
    }
    else
    {
        /* reset SPI0 */
        (void)mss_config_clk_rst(MSS_PERIPH_SPI0, (uint8_t) 1, PERIPHERAL_OFF);

         /* Take SPI0 out of reset. */
        (void)mss_config_clk_rst(MSS_PERIPH_SPI0, (uint8_t) 1, PERIPHERAL_ON);
    }
}
/**
 *  @brief   sample SPI task
 *
 *  @details Read BME280 temperature/pressure/humidity sensor or MCP3008 ADC via SPI
 *           define SPI_USE_BME280 to enable BME280 SPI
 *           define SPI_USE_MCP3008 to enable MCP3008 SPI
 *           either or, but not both!
 *
 *  @note    never returns
 */
void spi_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t spi_uart_buffer[256];

    /* Initialize and configure SPI0 as master */
    MSS_SPI_init(&g_mss_spi0_lo);
    MSS_SPI_configure_master_mode(&g_mss_spi0_lo,
                                  MSS_SPI_SLAVE_0,
                                  MSS_SPI_MODE0,
                                  256,    // divisor for PCLK, which is LIBERO_SETTING_MSS_APB_AHB_CLK which is 150,000,000
                                  MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE,
                                  spi_overflow_handler);

#if defined(SPI_USE_BME280)
    bme280_instance_t bme280;
    bme280_spi_init(&bme280, &g_mss_spi0_lo, MSS_SPI_SLAVE_0);
    bme280_spi_write_control_settings(&bme280);
    bme280_spi_read_calibration_data(&bme280);
#elif defined(SPI_USE_MCP3008)
    mcp3008_spi_instance_t mcp3008;
    mcp3008_spi_init(&mcp3008, &g_mss_spi0_lo, MSS_SPI_SLAVE_0);
#endif

    for(;;)
    {
#if defined(SPI_USE_BME280)
        bme280_spi_read_sensors_data(&bme280);
        snprintf((char*)spi_uart_buffer, 256,
                "Temperature: %d.%u °C\n"
                "   Pressure: %u.%u Pa\n"
                "   Humidity: %u.%u %%RH\n\n",
                bme280.compensated_temperature/100, bme280.compensated_temperature%100, // needs scaling of /100
                bme280.compensated_pressure/256, bme280.compensated_pressure%256,       // needs scaling of /256
                bme280.compensated_humidity/1024, bme280.compensated_humidity%1024);    // needs scaling of /1024
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, spi_uart_buffer);
#elif defined(SPI_USE_MCP3008)
        mcp3008_spi_read_all_channels_single_ended(&mcp3008);
        snprintf((char*)spi_uart_buffer, 256,
                "ADC[0]: %u.%u\r\n"
                "ADC[1]: %u.%u\r\n"
                "ADC[2]: %u.%u\r\n"
                "ADC[3]: %u.%u\r\n"
                "ADC[4]: %u.%u\r\n"
                "ADC[5]: %u.%u\r\n"
                "ADC[6]: %u.%u\r\n"
                "ADC[7]: %u.%u\r\n\n",
                ((mcp3008.ch0*5)/1024), (mcp3008.ch0*5)%1024,
                ((mcp3008.ch1*5)/1024), (mcp3008.ch1*5)%1024,
                ((mcp3008.ch2*5)/1024), (mcp3008.ch2*5)%1024,
                ((mcp3008.ch3*5)/1024), (mcp3008.ch3*5)%1024,
                ((mcp3008.ch4*5)/1024), (mcp3008.ch4*5)%1024,
                ((mcp3008.ch5*5)/1024), (mcp3008.ch5*5)%1024,
                ((mcp3008.ch6*5)/1024), (mcp3008.ch6*5)%1024,
                ((mcp3008.ch7*5)/1024), (mcp3008.ch7*5)%1024);
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, spi_uart_buffer);

#endif /* SPI_USE_BME280 or SPI_USE_MCP3008 */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/**
 * @brief   I2C task
 *
 * @details Read DS3231 RTC and/or BME280 temperature/pressure/humidity sensor via I2C
 *          define I2C_USE_BME280 to enable BME280
 *          define I2C_USE_DS3231 to enable DS3231
 *
 * @note    never returns
 */
void i2c_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t i2c_uart_buffer[512];

    /* desynch from the spi task */
    vTaskDelay(pdMS_TO_TICKS(2000));

    MSS_I2C_init(&g_mss_i2c0_lo, 1, MSS_I2C_PCLK_DIV_256);

#if defined(I2C_USE_BME280)
    bme280_instance_t bme280;
    bme280_i2c_init(&bme280, &g_mss_i2c0_lo);

    if(bme280_i2c_write_control_settings(&bme280) != 0)
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"bme280_i2c_write_control_settings failed\r\n\n");
    else
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"bme280_i2c_write_control_settings success\r\n\n");

    if(bme280_i2c_read_calibration_data(&bme280) != 0)
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"bme280_i2c_read_calibration_data failed\r\n\n");
    else
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"bme280_i2c_read_calibration_data success\r\n\n");
#endif /* I2C_USE_BME280 */

#if defined(I2C_USE_DS3231)
    ds3231_instance_t ds3231;
    ds3231_init(&ds3231, &g_mss_i2c0_lo);

    if(0 == ds3231_read_control(&ds3231))
    {
        snprintf((char*)i2c_uart_buffer, 512,
                "DS3231 Control Register\r\n"
                "         Alarm 1 interrupt enable: %u\r\n"
                "         Alarm 2 interrupt enable: %u\r\n"
                "                Interrupt control: %u\r\n"
                "                    Rate select 1: %u\r\n"
                "                    Rate select 2: %u\r\n"
                "              Convert temperature: %u\r\n"
                "Battery backed square wave enable: %u\r\n"
                "                Enable oscillator: %u\r\n\n",
                ds3231.control.vals.alarm_1_int_enable,
                ds3231.control.vals.alarm_2_int_enable,
                ds3231.control.vals.interrupt_control,
                ds3231.control.vals.rate_select_1,
                ds3231.control.vals.rate_select_2,
                ds3231.control.vals.convert_temperature,
                ds3231.control.vals.battery_backed_square_wave_enable,
                ds3231.control.vals.enable_oscillator);
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, i2c_uart_buffer);
    }
    else
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_read_control failed\n\n");

    vTaskDelay(pdMS_TO_TICKS(500));

    if(0 == ds3231_read_status(&ds3231))
    {
        snprintf((char*)i2c_uart_buffer, 512,
                "DS3231 Status Register\r\n"
                "        Alarm 1 flag: %u\r\n"
                "        Alarm 2 flag: %u\r\n"
                "                Busy: %u\r\n"
                " Enable 32kHz output: %u\r\n"
                "Oscillator stop flag: %u\r\n\n",
                ds3231.status.vals.alarm_1_flag,
                ds3231.status.vals.alarm_2_flag,
                ds3231.status.vals.busy,
                ds3231.status.vals.enable_32kHz_output,
                ds3231.status.vals.oscillator_stop_flag);
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, i2c_uart_buffer);
    }
    else
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_read_status failed\r\n\n");

    vTaskDelay(pdMS_TO_TICKS(500));

    /*
     * oscillator_stop_flag:
     * A logic 1 in this bit indicates that the oscillator either is stopped or was
     * stopped for some period and may be used to judge the validity of the timekeeping data.
     * This bit is set to logic 1 any time that the oscillator stops.
     */
    if(1 == ds3231.status.vals.oscillator_stop_flag)
    {
        if(0 == ds3231_write_time(&ds3231, 00, 54, 17, 5, 6, 2, 2026))
        {
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_write_time success\r\n\n");
            vTaskDelay(pdMS_TO_TICKS(500));

            if(0 == ds3231_reset_oscillator_stop_flag(&ds3231))
                MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_reset_oscillator_stop_flag success\r\n\n");
            else
                MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_reset_oscillator_stop_flag failed\r\n\n");
        }
        else
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_reset_oscillator_stop_flag failed\r\n\n");

        /* read status again */
        vTaskDelay(pdMS_TO_TICKS(500));
        if(0 == ds3231_read_status(&ds3231))
        {
            snprintf((char*)i2c_uart_buffer, 512,
                    "DS3231 Status Register\r\n"
                    "        Alarm 1 flag: %u\r\n"
                    "        Alarm 2 flag: %u\r\n"
                    "                Busy: %u\r\n"
                    " Enable 32kHz output: %u\r\n"
                    "Oscillator stop flag: %u\r\n\n",
                    ds3231.status.vals.alarm_1_flag,
                    ds3231.status.vals.alarm_2_flag,
                    ds3231.status.vals.busy,
                    ds3231.status.vals.enable_32kHz_output,
                    ds3231.status.vals.oscillator_stop_flag);
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, i2c_uart_buffer);
        }
        else
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_read_status failed\n\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
#endif /* I2C_USE_DS3231 */

    for(;;)
    {
#if defined(I2C_USE_DS3231)
        if(0 == ds3231_read_time(&ds3231))
        {
            snprintf((char*)i2c_uart_buffer, 512,
                    "RTC: %s, %04u/%02u/%02u %02u:%02u:%02u\r\n\n",
                    ds3231_day_of_week_string[ds3231.datetime.day_of_week],
                    ds3231.datetime.year,
                    ds3231.datetime.month,
                    ds3231.datetime.day,
                    ds3231.datetime.hour,
                    ds3231.datetime.minute,
                    ds3231.datetime.second);
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, i2c_uart_buffer);
        }
        else
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"ds3231_read_time failed\n\n");
#endif /* I2C_USE_DS3231 */

#if defined(I2C_USE_BME280)
        if(bme280_i2c_read_sensors_data(&bme280) != 0)
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t*)"bme280_i2c_read_sensors_data failed\r\n\n");
        else
        {
            snprintf((char*)i2c_uart_buffer, 512,
                    "Temperature: %d.%u deg C\r\n"
                    "   Pressure: %u.%u Pa\r\n"
                    "   Humidity: %u.%u %%RH\r\n\n",
                    bme280.compensated_temperature/100, bme280.compensated_temperature%100, // needs scaling of /100
                    bme280.compensated_pressure/256, bme280.compensated_pressure%256,       // needs scaling of /256
                    bme280.compensated_humidity/1024, bme280.compensated_humidity%1024);    // needs scaling of /1024
            MSS_UART_polled_tx_string(&g_mss_uart0_lo, i2c_uart_buffer);
        }
#endif /* I2C_USE_BME280 */
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
