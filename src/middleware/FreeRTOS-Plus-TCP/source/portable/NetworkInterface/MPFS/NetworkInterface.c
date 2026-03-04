/*
 * FreeRTOS+TCP V4.3.3
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#include "stdlib.h"

/* MSS MAC includes */
#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_registers.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac_sw_cfg.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac.h"
#include "drivers/mss/mss_ethernet_mac/phy.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "list.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"

/*---------- Defines ---------------------------------------------------------*/
#define IFNAME_PREFIX   "eth"
#define BUFFER_USED     1u
#define BUFFER_EMPTY    0u
#define RELEASE_BUFFER  BUFFER_EMPTY
#define RX_BUFFER_COUNT MSS_MAC_RX_RING_SIZE
#define TX_BUFFER_COUNT 1

#define BUFFER_SIZE ( ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING )
#define BUFFER_SIZE_ROUNDED_UP ( ( BUFFER_SIZE + 7 ) & ~0x07UL )

/*---------- Static global variables -----------------------------------------*/
/* Buffers for Tx and Rx */
static uint8_t g_mac_tx_buffer[TX_BUFFER_COUNT][MSS_MAC_MAX_TX_BUF_SIZE] __attribute__ ((aligned (4)));
static uint8_t g_mac_rx_buffer[RX_BUFFER_COUNT][MSS_MAC_MAX_RX_BUF_SIZE] __attribute__ ((aligned (4)));

static volatile uint8_t g_mac_tx_buffer_used[TX_BUFFER_COUNT];
static volatile uint8_t g_mac_rx_buffer_data_valid[RX_BUFFER_COUNT];

/* FreeRTOS+ buffer */
static uint8_t ucBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ][ BUFFER_SIZE_ROUNDED_UP ] __attribute__ ((aligned (4)));

/* MAC configuration record */
mss_mac_cfg_t g_mac_config;
mss_mac_instance_t g_mac_instance;
SemaphoreHandle_t eth_interface_semaphore = NULL;

NetworkInterface_t g_eth_interface;
NetworkInterface_t *g_p_eth_interface = &g_eth_interface;

NetworkEndPoint_t g_eth_endpoint;
NetworkEndPoint_t *g_p_eth_endpoint = &g_eth_endpoint;

TaskHandle_t deferred_emac_rx_task;
BaseType_t g_mac_context_switch;

/* If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing. */
#if ( ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0 )
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eProcessBuffer
#else
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#endif

/*
 * https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/10-Porting/03-Embedded_Ethernet_Porting
 *
 * BufferAllocation_1.c uses pre-allocated network buffers that are normally statically allocated at compile time.
 *
 * The number of network buffers that must be allocated is set by the ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS definition
 * in FreeRTOSIPConfig.h,and the size of each buffer must be ( ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING ).
 * ipTOTAL_ETHERNET_FRAME_SIZE is calculated automatically from the value of ipconfigNETWORK_MTU, and ipBUFFER_PADDING
 * is calculated automatically from ipconfigBUFFER_PADDING.
 *
 * Networking hardware can impose strict alignment requirements on the allocated buffers, so it is recommended that
 * the buffers are allocated in the embedded Ethernet driver itself - that way the buffer's alignment can always be made
 * to match the hardware's requirements.
 *
 * The embedded TCP/IP stack allocates the network buffer descriptors, but does not know anything about the alignment of
 * the network buffers themselves. Therefore the embedded Ethernet driver must also provide a function called
 * vNetworkInterfaceAllocateRAMToBuffers() that allocates a statically declared buffer to each descriptor.
 * Note that ipBUFFER_PADDING bytes at the beginning of the buffer are left for use by the embedded TCP/IP stack itself.
 */
size_t uxNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] )
{
    BaseType_t x;
    for( x = 0; x < ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS; x++ )
    {
        /* pucEthernetBuffer is set to point ipBUFFER_PADDING bytes in from the
        beginning of the allocated buffer. */
        pxNetworkBuffers[ x ].pucEthernetBuffer = &( ucBuffers[ x ][ ipBUFFER_PADDING ] );

        /* The following line is also required, but will not be required in
        future versions. */
        *( ( uint32_t * ) &ucBuffers[ x ][ 0 ] ) = ( uint32_t ) &( pxNetworkBuffers[ x ] );
    }
    return ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS*BUFFER_SIZE_ROUNDED_UP;
}

/*
 * https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/10-Porting/03-Embedded_Ethernet_Porting
 *
 * The network interface port layer must provide a callback function called pfOutput() that sends data received from the
 * embedded TCP/IP stack to the Ethernet MAC driver for transmission in the instance of the 'network interface port layer'.
 */
static BaseType_t prvMPFS_Ethernet_NetworkInterfaceOutput( NetworkInterface_t *pxInterface,
                                                               NetworkBufferDescriptor_t * const pxDescriptor,
                                                               BaseType_t xReleaseAfterSend )
{
    int32_t tx_status;
    (void)pxInterface;

    /*--------------------------------------------------------------------------
     * Wait for packet buffer to become free.
     */
    // Block waiting for the semaphore to become available.
    if( xSemaphoreTake( eth_interface_semaphore, portMAX_DELAY ) == pdTRUE )
    {
        /* Simple network interfaces (as opposed to more efficient zero copy network
           interfaces) just use Ethernet peripheral driver library functions to copy
           data from the FreeRTOS-Plus-TCP buffer into the peripheral driver's own buffer.
           This example assumes SendData() is a peripheral driver library function that
           takes a pointer to the start of the data to be sent and the length of the
           data to be sent as two separate parameters.  The start of the data is located
           by pxDescriptor->pucEthernetBuffer.  The length of the data is located
           by pxDescriptor->xDataLength. */

        memcpy(&g_mac_tx_buffer[0][0], pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength);

        /*--------------------------------------------------------------------------
         * Initiate packet transmit. Keep retrying until there is room in the MAC Tx
         * ring.
         */
        do
        {
            tx_status = MSS_MAC_send_pkt(&g_mac0, 0, g_mac_tx_buffer[0], (uint32_t)pxDescriptor->xDataLength, (void *)&g_mac_tx_buffer_used[0]);
            if(MSS_MAC_SUCCESS != tx_status)
            {
                vTaskDelay(1);
            }
        }
        while(MSS_MAC_SUCCESS != tx_status);

        /* Call the standard trace macro to log the send event. */
        //iptraceNETWORK_INTERFACE_TRANSMIT();

        if( pdFALSE != xReleaseAfterSend )
        {
            /* It is assumed SendData() copies the data out of the FreeRTOS-Plus-TCP Ethernet
               buffer.  The Ethernet buffer is therefore no longer needed, and must be
               freed for re-use. */
            vReleaseNetworkBufferAndDescriptor(pxDescriptor);
        }
    }

    return pdTRUE;
}

static BaseType_t prvMPFS_Ethernet_GetPhyLinkStatus( struct xNetworkInterface *pxInterface )
{
    uint8_t fullduplex;
    mss_mac_speed_t speed;
    (void)pxInterface;
    return MSS_MAC_get_link_status(&g_mac0, &speed, &fullduplex);
}

static void prvMPFS_packet_tx_complete_handler(/* mss_mac_instance_t*/ void *this_mac, uint32_t queue_no, mss_mac_tx_desc_t *cdesc, void *caller_info)

{
    (void)caller_info;
    (void)this_mac;
    (void)cdesc;
    (void)queue_no;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    iptraceNETWORK_INTERFACE_TRANSMIT();

    // Unblock the task by releasing the semaphore.
    xSemaphoreGiveFromISR(eth_interface_semaphore, &xHigherPriorityTaskWoken);
    g_mac_context_switch = xHigherPriorityTaskWoken;
}

/***************************************************************************//**
 * Receive callback function.
 *
 * This is the prototype for the user function which the MSS Ethernet MAC driver
 * calls when a packet has been received. The users function is responsible for
 * processing any post receive processing required for the packets and for
 * returning the packet buffer to the receive chain so that further packets can
 * be received with this buffer.
 *
 *   - ___this_mac___    - pointer to global structure for the MAC in question.
 *   - ___queue_no___    - 0 to 3 for pMAC and always 0 for eMAC.
 *   - ___p_rx_packet___ - pointer to the buffer for the packet to be processed.
 *   - ___pckt_length___ - length of packet to be processed.
 *   - ___cdesc___       - pointer to the DMA descriptor associated with this
 *                         packet.
 *   - ___p_user_data___ - original user data pointer associated with the packet
 *                         buffer.
 */
static NetworkBufferDescriptor_t *pxBufferDescriptor = NULL;
static void prvMPFS_mac_packet_rx_handler(/* mss_mac_instance_t*/ void *this_mac,
                                       uint32_t queue_no,
                                       uint8_t *p_rx_packet,
                                       uint32_t pckt_length,
                                       mss_mac_rx_desc_t *cdesc,
                                       void *p_user_data)
{
    (void)this_mac;
    (void)queue_no;
    (void)cdesc;
    (void)p_user_data;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    NetworkBufferDescriptor_t **pxTempBuffer = NULL;

    /* Do we still have buffers awaiting processing?
     * Need to find the most recent and append to that */
    for(pxTempBuffer = &pxBufferDescriptor; *pxTempBuffer != NULL; pxTempBuffer = &((*pxTempBuffer)->pxNextBuffer))
    {
        ; // using the for loop to find the next free buffer
    }

    *pxTempBuffer = pxNetworkBufferGetFromISR(pckt_length);
    if(NULL != *pxTempBuffer)
    {
        memcpy((*pxTempBuffer)->pucEthernetBuffer, p_rx_packet, pckt_length);
        (*pxTempBuffer)->xDataLength = pckt_length;

        /* prvProcessEthernetPacket(...) drops buffers where:
         * ( pxNetworkBuffer->pxInterface == NULL ) || ( pxNetworkBuffer->pxEndPoint == NULL ) */
        (*pxTempBuffer)->pxInterface = g_p_eth_interface;
        (*pxTempBuffer)->pxEndPoint = FreeRTOS_MatchingEndpoint( g_p_eth_interface, (*pxTempBuffer)->pucEthernetBuffer );

        /* Reassign packet buffer for reception once we have copied it */
        MSS_MAC_receive_pkt(&g_mac0, 0, p_rx_packet, NULL, MSS_MAC_INT_ENABLE);
    }
    else
    {
        /* The event was lost because a network buffer was not available.
         * Call the standard trace macro to log the occurrence. */
        iptraceETHERNET_RX_EVENT_LOST();

        /* Silently drop current packet and reassign packet buffer for
         * reception */
        MSS_MAC_receive_pkt(&g_mac0, 0, p_rx_packet, NULL, MSS_MAC_INT_ENABLE);
    }

    vTaskNotifyGiveFromISR(deferred_emac_rx_task, &xHigherPriorityTaskWoken);
    g_mac_context_switch = xHigherPriorityTaskWoken;
}

static void prvMPFS_Ethernet_EMACDeferredInterruptHandlerTask( void *pvParameters )
{
    (void)pvParameters;
    IPStackEvent_t xRxEvent;
    NetworkBufferDescriptor_t *pxNextBuffer;

    for(;;)
    {
        ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
        if(NULL != pxBufferDescriptor)
        {
            portENTER_CRITICAL();
            if(eProcessBuffer == eConsiderFrameForProcessing( pxBufferDescriptor->pucEthernetBuffer))
            {
                if(NULL != pxBufferDescriptor->pxEndPoint)
                {
                    /* Used to indicate that xSendEventStructToIPTask() is being called because of an Ethernet receive event.*/
                    xRxEvent.eEventType = eNetworkRxEvent;

                    /* pvData is used to point to the network buffer descriptor that now references the received data. */
                    xRxEvent.pvData = ( void * ) pxBufferDescriptor;

                    /* Send the data to the TCP/IP stack. */
                    if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFAIL )
                    {
                        /* The buffer could not be sent to the IP task so the buffer must be released. */
                        for(pxNextBuffer = pxBufferDescriptor; pxNextBuffer != NULL; pxNextBuffer = pxNextBuffer->pxNextBuffer)
                        {
                            vReleaseNetworkBufferAndDescriptor( pxNextBuffer );
                        }
                        /* Make a call to the standard trace macro to log the occurrence. */
                        iptraceETHERNET_RX_EVENT_LOST();
                    }
                    else
                    {
                        /* The message was successfully sent to the TCP/IP stack.
                           Call the standard trace macro to log the occurrence. */
                        iptraceNETWORK_INTERFACE_RECEIVE();
                    } /* if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFAIL ) */
                }
                else
                {
                    /* The Ethernet frame can be dropped, but the Ethernet buffer must be released. */
                    for(pxNextBuffer = pxBufferDescriptor; pxNextBuffer != NULL; pxNextBuffer = pxNextBuffer->pxNextBuffer)
                    {
                        vReleaseNetworkBufferAndDescriptor( pxNextBuffer );
                    }
                } /* if(NULL!= pxBufferDescriptor->pxEndPoint) */
            }
            else
            {
                /* The Ethernet frame can be dropped, but the Ethernet buffer must be released. */
                for(pxNextBuffer = pxBufferDescriptor; pxNextBuffer != NULL; pxNextBuffer = pxNextBuffer->pxNextBuffer)
                {
                    vReleaseNetworkBufferAndDescriptor( pxNextBuffer );
                }
            }/* if(eProcessBuffer == eConsiderFrameForProcessing( pxBufferDescriptor->pucEthernetBuffer)) */
            pxBufferDescriptor = NULL;
            portEXIT_CRITICAL();
        } /* if(NULL != pxBufferDescriptor) */
    } /* for(;;) */
}

/*
 * https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/10-Porting/03-Embedded_Ethernet_Porting
 *
 * The network interface port layer must provide a callback function called pfInitialise() that initialises the MAC driver in the instance of the 'network interface port layer'.
 */
static BaseType_t prvMPFS_Ethernet_NetworkInterfaceInitialise( NetworkInterface_t *pxInterface )
{
    (void)pxInterface;

    /* rand() is used by xApplicationGetRandomNumber and ulApplicationGetNextSequenceNumber */
    srand((unsigned int)CLINT->MTIME);

    /*--------------------- Initialize packet containers ---------------------*/
    g_mac_tx_buffer_used[0] = RELEASE_BUFFER;
    g_mac_rx_buffer_data_valid[0] = RELEASE_BUFFER;

    /*-------------------------- Initialize the MAC --------------------------*/
    /*
     * The interrupt can cause a context switch, so ensure its priority is
     * between configKERNEL_INTERRUPT_PRIORITY and
     * configMAX_SYSCALL_INTERRUPT_PRIORITY.
     */
/* Todo: What is needed for G5SoC? */
    /*
     * Get the default configuration for the Ethernet MAC and change settings
     * to match the system/application. The default values typically changed
     * are:
     *  - interface:
     *      Specifies the interface used to connect the Ethernet MAC to the PHY.
     *      Example choice are MII, GMII, TBI.
     *  - phy_addr:
     *      Specifies the MII management interface address of the external PHY.
     *  - mac_addr:
     *      Specifies the MAC address of the device. This number should be
     *      unique on the network the device is connected to.
     *  - speed_duplex_select:
     *      Specifies the allowed speed and duplex mode for setting up a link.
     *      This can be used to specify the set of allowed speeds and duplex
     *      modes used during auto-negotiation or force the link speed to a
     *      specific speed and duplex mode.
     */
    MSS_MAC_cfg_struct_def_init(&g_mac_config);

    g_mac_config.speed_duplex_select   = MSS_MAC_ANEG_ALL_SPEEDS;//MSS_MAC_ANEG_ALL_SPEEDS;//get_user_eth_speed_choice();
    g_mac_config.mac_addr[0]           = 0x00;
    g_mac_config.mac_addr[1]           = 0xFC;
    g_mac_config.mac_addr[2]           = 0x00;
    g_mac_config.mac_addr[3]           = 0x12;
    g_mac_config.mac_addr[4]           = 0x34;
    g_mac_config.mac_addr[5]           = 0x58;

    g_mac_config.phy_addr              = PHY_RTL8211_MDIO_ADDR;
    g_mac_config.phy_type              = MSS_MAC_DEV_PHY_RTL8211;
    g_mac_config.pcs_phy_addr          = SGMII_MDIO_ADDR;
    g_mac_config.interface_type        = TBI;
    g_mac_config.phy_autonegotiate     = MSS_MAC_RTL8211_phy_autonegotiate;
    g_mac_config.phy_mac_autonegotiate = MSS_MAC_RTL8211_mac_autonegotiate;
    g_mac_config.phy_get_link_status   = MSS_MAC_RTL8211_phy_get_link_status;
    g_mac_config.phy_init              = MSS_MAC_RTL8211_phy_init;
    g_mac_config.phy_set_link_speed    = MSS_MAC_RTL8211_phy_set_link_speed;
    g_mac_config.phy_extended_read     = mmd_read_extended_regs;
    g_mac_config.phy_extended_write    = mmd_write_extended_regs;
    g_mac_config.use_local_ints        = MSS_MAC_ENABLE;

    vSemaphoreCreateBinary(eth_interface_semaphore);
    if( NULL == eth_interface_semaphore )
    {
        while(1);// could not create semaphore
    }

    /*
     * Initialize MAC with specified configuration. The Ethernet MAC is
     * functional after this function returns but still requires transmit and
     * receive buffers to be allocated for communications to take place.
     */
    MSS_MAC_init(&g_mac0, &g_mac_config);

    /*
     * Register MAC interrupt handler listener functions. These functions will
     * be called  by the MAC driver when a packet has been sent or received.
     * These callback functions are intended to help managing transmit and
     * receive buffers by indicating when a transmit buffer can be released or
     * a receive buffer has been filled with an rx packet.
     */
    MSS_MAC_set_tx_callback(&g_mac0, 0, prvMPFS_packet_tx_complete_handler);
    MSS_MAC_set_rx_callback(&g_mac0, 0, prvMPFS_mac_packet_rx_handler);

    /*
     * Allocate receive buffers.
     *
     * We prime the pump with a full set of packet buffers and then re use them
     * as each packet is handled.
     *
     * This function will need to be called each time a packet is received to
     * hand back the receive buffer to the MAC driver.
     */
    for(uint32_t count = 0; count < RX_BUFFER_COUNT; ++count)
    {
        /*
         * We allocate the buffers with the Ethernet MAC interrupt disabled
         * until we get to the last one. For the last one we ensure the Ethernet
         * MAC interrupt is enabled on return from MSS_MAC_receive_pkt().
         */
        if(count != (RX_BUFFER_COUNT - 1))
        {
            MSS_MAC_receive_pkt(&g_mac0, 0, g_mac_rx_buffer[count], NULL, MSS_MAC_INT_DISABLE);
        }
        else
        {
            MSS_MAC_receive_pkt(&g_mac0, 0, g_mac_rx_buffer[count], NULL, MSS_MAC_INT_ARM);
        }
    }

    BaseType_t rtos_result = xTaskCreate(prvMPFS_Ethernet_EMACDeferredInterruptHandlerTask,
                                         "prvMPFS_Ethernet_EMACDeferredInterruptHandlerTask",
                                         configMINIMAL_STACK_SIZE,
                                         NULL,
                                         ipconfigIP_TASK_PRIORITY+1,
                                         &deferred_emac_rx_task);
    if (1 != rtos_result)
    {
        int ix;
        for (;;)
            ix++;
    }

    return pdTRUE;
}

/*
 * https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/10-Porting/03-Embedded_Ethernet_Porting
 *
 * Each network interface must provide a function to initialise an instance of it, and which is
 * called px[port_name]_FillInterfaceDescriptor(). This function initialises the callback functions
 * in the instance structure and appends the instance to the linked list by calling FreeRTOS_AddNetworkInterface().
 */
NetworkInterface_t *pxMPFS_Ethernet_FillInterfaceDescriptor( BaseType_t xEMACIndex, NetworkInterface_t *pxInterface )
{
    static char ifname[6];

    memset(pxInterface, 0, sizeof(NetworkInterface_t));

    /* ethX */
    snprintf(ifname, 6, "%s%lu", IFNAME_PREFIX, xEMACIndex);
    pxInterface->pcName = ifname;

    // The initialisation function of this driver.
    pxInterface->pfInitialise = prvMPFS_Ethernet_NetworkInterfaceInitialise;

    // The output function of this driver.
    pxInterface->pfOutput = prvMPFS_Ethernet_NetworkInterfaceOutput;

    // The query status function of this driver.
    pxInterface->pfGetPhyLinkStatus = prvMPFS_Ethernet_GetPhyLinkStatus;

    // Assign to our global pointer
    g_p_eth_interface = pxInterface;

    FreeRTOS_AddNetworkInterface(pxInterface);

    return pxInterface;
}

/* This xApplicationGetRandomNumber() will set *pulNumber to a random number,
 * and return pdTRUE. When the random number generator is broken, it shall return
 * pdFALSE.
 * The function is defined in 'iot_secure_sockets.c'.
 * If that module is not included in the project, the application must provide an
 * implementation of it.
 * The macro's ipconfigRAND32() and configRAND32() are not in use anymore. */
BaseType_t xApplicationGetRandomNumber( uint32_t *pulNumber )
{

    *pulNumber = (uint32_t)rand();
    return pdTRUE;
}

const char *pcApplicationHostnameHook( void )
{
    static const char dhcp_hostname[] = "conor-testing-pfsoc";
    return dhcp_hostname;
}

/*
 * Generate a randomized TCP Initial Sequence Number per RFC.
 * This function must be provided by the application builder.
 */
/* This function is defined generally by the application. */
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    (void)ulSourceAddress;
    (void)usSourcePort;
    (void)ulDestinationAddress;
    (void)usDestinationPort;

    return (uint32_t)rand();
}
