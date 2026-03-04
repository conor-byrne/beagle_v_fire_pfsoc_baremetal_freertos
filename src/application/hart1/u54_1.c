#include <stdio.h>
#include <string.h>

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"

#include "inc/common.h"

extern void freertos_risc_v_trap_handler( void );

void u54_1(void)
{
#if (MPFS_HAL_LAST_HART == 1 )
    write_csr(mscratch, 0);  // Machine Scratch - holds one XLEN of data for temporary storage for trap handlers
    write_csr(mcause, 0);    // Machine Exception Cause - indicates which exception occurred
    write_csr(mepc, 0);      // Machine Exception PC - points to the instruction where the exception occurred


    /* Reset clocks */
    SYSREG->SOFT_RESET_CR = 0U;

    PLIC_init();

    /* reset peripherals used elsewhere */
    mss_config_clk_rst(MSS_PERIPH_MAC0,  1, PERIPHERAL_ON);
    mss_config_clk_rst(MSS_PERIPH_GPIO2, 1, PERIPHERAL_ON);

    /*
     * Enable mac local interrupts to hart 1, U54 1
     * Fabric interrupts enable
     */
    SYSREG->FAB_INTEN_MISC = FAB_INTEN_MAC0_U54_1_EN_MASK;
    SYSREG->FAB_INTEN_U54_1 = (1U << MAC0_INT_U54_INT) | (1U << MAC0_QUEUE1_U54_INT) |
                              (1U << MAC0_QUEUE2_U54_INT) | (1U << MAC0_QUEUE3_U54_INT) |
                              (1U << MAC0_EMAC_U54_INT) | (1U << MAC0_MMSL_U54_INT);
    /*
     * Init UART
     */
    mss_config_clk_rst(MSS_PERIPH_MMUART0, 1, PERIPHERAL_ON);
    MSS_UART_init(&g_mss_uart0_lo,
                  MSS_UART_115200_BAUD,
                  MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT);

    /* SPI */
    mss_config_clk_rst(MSS_PERIPH_SPI0, 1, PERIPHERAL_ON);
    PLIC_SetPriority(SPI0_PLIC, 2);
    PLIC_EnableIRQ(SPI0_PLIC);

    /* I2C */
    mss_config_clk_rst(MSS_PERIPH_I2C0, 1, PERIPHERAL_ON);
    PLIC_SetPriority(I2C0_MAIN_PLIC, 2);
    PLIC_EnableIRQ(I2C0_MAIN_PLIC);

    __enable_irq();

    /*
     * Use the FreeRTOS trap handler instead of the default
     */
    __asm__ volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    /* Look at banner Michael! */
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"+==============================================================================+\r\n");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"| PolarFire SoC (BeagleV-Fire)                                                 |\r\n");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"|   - FreeRTOS-Kernel......v11.1.0                                             |\r\n");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"|   - FreeRTOS-Plus-TCP....v4.3.3                                              |\r\n");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"|   - BACnet Stack.........v1.4.2                                              |\r\n");
    MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)"+==============================================================================+\r\n\n");

    /* start things off */
    free_rtos();
#endif

    volatile int ix;
    while (1)
    {
        ix++;
    }
    /* Never return */
}
