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

#ifndef FREERTOS_IP_TRACE_MACRO_CUSTOM_H
#define FREERTOS_IP_TRACE_MACRO_CUSTOM_H

#ifdef FREERTOS_IP_TRACE_ENABLE

#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "stdio.h"
static char freertosip_trace_printstr[256];
#define IP_TRACE_PRINT_STRING(...)\
    do{\
        snprintf(freertosip_trace_printstr, 256, __VA_ARGS__);\
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)freertosip_trace_printstr);\
    }\
    while(0)

/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           NETWORK TRACE MACROS                            */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER
 *
 * Called when a task attempts to obtain a network buffer, but a buffer was
 * not available even after any defined block period.
 */
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: FAILED_TO_OBTAIN_NETWORK_BUFFER\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR
 *
 * Called when an interrupt service routine attempts to obtain a network
 * buffer, but a buffer was not available.
 */
#define iptraceFAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: FAILED_TO_OBTAIN_NETWORK_BUFFER_FROM_ISR\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_OBTAINED
 *
 * Called when the network buffer at address pxBufferAddress is obtained from
 * the TCP/IP stack by an RTOS task.
 */
#define iptraceNETWORK_BUFFER_OBTAINED( pxBufferAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_BUFFER_OBTAINED, pxBufferAddress = %p\n", __func__, (void *)pxBufferAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR
 *
 * Called when the network buffer at address pxBufferAddress is obtained from
 * the TCP/IP stack by an interrupt service routine.
 */
#define iptraceNETWORK_BUFFER_OBTAINED_FROM_ISR( pxBufferAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_BUFFER_OBTAINED_FROM_ISR, pxBufferAddress = %p\n", __func__, (void *)pxBufferAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_BUFFER_RELEASED
 *
 * Called when the network buffer at address pxBufferAddress is released back
 * to the TCP/IP stack.
 */
#define iptraceNETWORK_BUFFER_RELEASED( pxBufferAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_BUFFER_RELEASED, pxBufferAddress = %p\n", __func__, (void *)pxBufferAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_DOWN
 *
 * Called when the network driver indicates that the network connection has
 * been lost (not implemented by all network drivers).
 */
#define iptraceNETWORK_DOWN()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_DOWN\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_EVENT_RECEIVED
 *
 * Called when the TCP/IP stack processes an event previously posted to the
 * network event queue. eEvent will be one of the following values:
 *
 * eNetworkDownEvent - The network interface has been lost and/or needs
 *                     [re]connecting.
 * eNetworkRxEvent - The network interface has queued a received Ethernet
 *                   frame.
 * eARPTimerEvent - The Resolution timer expired.
 * eStackTxEvent - The software stack has queued a packet to transmit.
 * eDHCPEvent - Process the DHCP state machine.
 *
 * Note the events are defined by the private eIPEvent_t type which is not
 * generally accessible.
 */

static char eIPEvent_t_strings[16][22] = {
    "eNoEvent",             /* -1 */
    "eNetworkDownEvent",    /* 0: The network interface has been lost and/or needs [re]connecting. */
    "eNetworkRxEvent",      /* 1: The network interface has queued a received Ethernet frame. */
    "eNetworkTxEvent",      /* 2: Let the IP-task send a network packet. */
    "eARPTimerEvent",       /* 3: The ARP timer expired. */
    "eNDTimerEvent",        /* 4: The ND timer expired. */
    "eStackTxEvent",        /* 5: The software stack has queued a packet to transmit. */
    "eDHCPEvent",           /* 6: Process the DHCP state machine. */
    "eTCPTimerEvent",       /* 7: See if any TCP socket needs attention. */
    "eTCPAcceptEvent",      /* 8: Client API FreeRTOS_accept() waiting for client connections. */
    "eTCPNetStat",          /* 9: IP-task is asked to produce a netstat listing. */
    "eSocketBindEvent",     /*10: Send a message to the IP-task to bind a socket to a port. */
    "eSocketCloseEvent",    /*11: Send a message to the IP-task to close a socket. */
    "eSocketSelectEvent",   /*12: Send a message to the IP-task for select(). */
    "eSocketSignalEvent",   /*13: A socket must be signalled. */
    "eSocketSetDeleteEvent" /*14: A socket set must be deleted. */
};
#define iptraceNETWORK_EVENT_RECEIVED( eEvent )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_EVENT_RECEIVED, eEvent = %s\n", __func__, eIPEvent_t_strings[eEvent+1]);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_INPUT
 *
 * Called when a packet of length uxDataLength and with the contents at
 * address pucEthernetBuffer has been received.
 */
#define iptraceNETWORK_INTERFACE_INPUT( uxDataLength, pucEthernetBuffer )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_INTERFACE_INPUT, uxDataLength = %lu, pucEthernetBuffer = %p\n", __func__, uxDataLength, pucEthernetBuffer);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_OUTPUT
 *
 * Called when a packet of length uxDataLength and with the contents at
 * address pucEthernetBuffer has been sent.
 */
#define iptraceNETWORK_INTERFACE_OUTPUT( uxDataLength, pucEthernetBuffer )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_INTERFACE_OUTPUT, uxDataLength = %lu, pucEthernetBuffer = %p\n", __func__, uxDataLength, (void *)pucEthernetBuffer);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_RECEIVE
 *
 * Called when a packet is received from the network by the network driver.
 * Note this macro is called by the network driver rather than the TCP/IP stack
 * and may not be called at all by drivers provided by third parties.
 */
#define iptraceNETWORK_INTERFACE_RECEIVE()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_INTERFACE_RECEIVE\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceNETWORK_INTERFACE_TRANSMIT
 *
 * Called when a packet is sent to the network by the network driver. Note this
 * macro is called by the network driver rather than the TCP/IP stack and may
 * not be called at all by drivers provided by third parties.
 */
#define iptraceNETWORK_INTERFACE_TRANSMIT()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NETWORK_INTERFACE_TRANSMIT\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceSTACK_TX_EVENT_LOST
 *
 * Called when a packet generated by the TCP/IP stack is dropped because there
 * is insufficient space in the network event queue (see the
 * ipconfigEVENT_QUEUE_LENGTH setting in FreeRTOSIPConfig.h).
 */
#define iptraceSTACK_TX_EVENT_LOST( xEvent )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: STACK_TX_EVENT_LOST, xEvent = %zd\n", __func__, xEvent);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                           NETWORK TRACE MACROS                            */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                            DRIVER TRACE MACROS                            */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceETHERNET_RX_EVENT_LOST
 *
 * Called when a packet received by the network driver is dropped for one of
 * the following reasons: There is insufficient space in the network event
 * queue (see the ipconfigEVENT_QUEUE_LENGTH setting in FreeRTOSIPConfig.h),
 * the received packet has an invalid data length, or there are no network
 * buffers available (see the ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS setting
 * in FreeRTOSIPConfig.h). Note this macro is called by the network driver
 * rather than the TCP/IP stack and may not be called at all by drivers
 * provided by third parties.
 */
#define iptraceETHERNET_RX_EVENT_LOST()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ETHERNET_RX_EVENT_LOST\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceWAITING_FOR_TX_DMA_DESCRIPTOR
 *
 * Called when a transmission at the network driver level cannot complete
 * immediately because the driver is having to wait for a DMA descriptor to
 * become free. Try increasing the configNUM_TX_ETHERNET_DMA_DESCRIPTORS
 * setting in FreeRTOSConfig.h (if it exists for the network driver being
 * used).
 */
#define iptraceWAITING_FOR_TX_DMA_DESCRIPTOR()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: WAITING_FOR_TX_DMA_DESCRIPTOR\n", __func__);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            DRIVER TRACE MACROS                            */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             UDP TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_UDP_PACKET
 *
 * Called when a UDP packet is sent to the IP address ulIPAddress. ulIPAddress
 * is expressed as a 32-bit number in network byte order.
 */
#define iptraceSENDING_UDP_PACKET( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_UDP_PACKET, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             UDP TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           SOCKET TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceBIND_FAILED
 *
 * A call to FreeRTOS_bind() failed. usPort is the port number the socket
 * xSocket was to be bound to.
 */
#define iptraceBIND_FAILED( xSocket, usPort )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: BIND_FAILED, xSocket = %zd, usPort = %u\n", __func__, xSocket, usPort);
/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_CREATE_EVENT_GROUP
 *
 * Called when an event group could not be created, possibly due to
 * insufficient heap space, during new socket creation.
 */
#define iptraceFAILED_TO_CREATE_EVENT_GROUP()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: FAILED_TO_CREATE_EVENT_GROUP\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_CREATE_SOCKET
 *
 * A call to FreeRTOS_socket() failed because there was insufficient FreeRTOS
 * heap memory available for the socket structure to be created.
 */
#define iptraceFAILED_TO_CREATE_SOCKET()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: FAILED_TO_CREATE_SOCKET\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceFAILED_TO_NOTIFY_SELECT_GROUP
 */
#define iptraceFAILED_TO_NOTIFY_SELECT_GROUP( xSocket )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: FAILED_TO_NOTIFY_SELECT_GROUP, xSocket = %zd\n", __func__, xSocket);
/*-----------------------------------------------------------------------*/

/*
 * iptraceNO_BUFFER_FOR_SENDTO
 *
 * Called when a call to FreeRTOS_sendto() tries to allocate a buffer, but a
 * buffer was not available even after any defined block period.
 */
#define iptraceNO_BUFFER_FOR_SENDTO()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: NO_BUFFER_FOR_SENDTO\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_DISCARDING_BYTES
 *
 * FreeRTOS_recvfrom() is discarding xNumberOfBytesDiscarded bytes because the
 * number of bytes received is greater than the number of bytes that will fit
 * in the user supplied buffer (the buffer passed in as a FreeRTOS_recvfrom()
 * function parameter).
 */
#define iptraceRECVFROM_DISCARDING_BYTES( xNumberOfBytesDiscarded )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: RECVFROM_DISCARDING_BYTES, xNumberOfBytesDiscarded = %zd\n", __func__, xNumberOfBytesDiscarded);
/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_INTERRUPTED
 *
 * Called when a blocking call to FreeRTOS_recvfrom() is interrupted through a
 * call to FreeRTOS_SignalSocket().
 */
#define iptraceRECVFROM_INTERRUPTED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: RECVFROM_INTERRUPTED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceRECVFROM_TIMEOUT
 *
 * Called when FreeRTOS_recvfrom() gets no data on the given socket even after
 * any defined block period.
 */
#define iptraceRECVFROM_TIMEOUT()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: RECVFROM_TIMEOUT\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDTO_DATA_TOO_LONG
 *
 * Called when the data requested to be sent using a call to FreeRTOS_sendto()
 * is too long and could not be sent.
 */
#define iptraceSENDTO_DATA_TOO_LONG()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDTO_DATA_TOO_LONG\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDTO_SOCKET_NOT_BOUND
 *
 * Called when the socket used in the call to FreeRTOS_sendto() is not already
 * bound to a port.
 */
#define iptraceSENDTO_SOCKET_NOT_BOUND()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDTO_SOCKET_NOT_BOUND\n", __func__);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                           SOCKET TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ARP TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_PACKET_RECEIVED
 *
 * Called when an ARP packet is received, even if the local network node is not
 * involved in the ARP transaction.
 */
#define iptraceARP_PACKET_RECEIVED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ARP_PACKET_RECEIVED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_CREATED
 *
 * Called when a new entry in the ARP table is created to map the IP address
 * ulIPAddress to the MAC address ucMACAddress. ulIPAddress is expressed as a
 * 32-bit number in network byte order. ucMACAddress is a pointer to an
 * MACAddress_t structure.
 */
#define iptraceARP_TABLE_ENTRY_CREATED( ulIPAddress, ucMACAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ARP_TABLE_ENTRY_CREATED, ulIPAddress = %u, ucMACAddress = %u\n", __func__, ulIPAddress, ucMACAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_EXPIRED
 *
 * Called when the entry for the IP address ulIPAddress in the ARP cache is
 * removed. ulIPAddress is expressed as a 32-bit number in network byte order.
 */
#define iptraceARP_TABLE_ENTRY_EXPIRED( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ARP_TABLE_ENTRY_EXPIRED, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceARP_TABLE_ENTRY_WILL_EXPIRE
 *
 * Called when an ARP request is about to be sent because the entry for the IP
 * address ulIPAddress in the ARP cache has become stale. ulIPAddress is
 * expressed as a 32-bit number in network byte order.
 */
#define iptraceARP_TABLE_ENTRY_WILL_EXPIRE( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ARP_TABLE_ENTRY_WILL_EXPIRE, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceCREATING_ARP_REQUEST
 *
 * Called when the IP generates an ARP request packet.
 */
#define iptraceCREATING_ARP_REQUEST( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: CREATING_ARP_REQUEST, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_BUFFER_FULL
 *
 * A packet has come in from an unknown IPv4 address. An ARP request has been
 * sent, but the queue is still filled with a different packet.
 */
#define iptraceDELAYED_ARP_BUFFER_FULL()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ARP_BUFFER_FULL\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptrace_DELAYED_ARP_REQUEST_REPLIED
 *
 * An ARP request has been sent, and a matching reply is received. Now the
 * original packet will be processed by the IP-task.
 */
#define iptrace_DELAYED_ARP_REQUEST_REPLIED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: _DELAYED_ARP_REQUEST_REPLIED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_REQUEST_STARTED
 *
 * A packet came in from an unknown IPv4 address. An ARP request has been sent
 * and the network buffer is stored for processing later.
 */
#define iptraceDELAYED_ARP_REQUEST_STARTED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ARP_REQUEST_STARTED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ARP_TIMER_EXPIRED
 *
 * A packet was stored for delayed processing, but there is no ARP reply. The
 * network buffer will be released without being processed.
 */
#define iptraceDELAYED_ARP_TIMER_EXPIRED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ARP_TIMER_EXPIRED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDROPPED_INVALID_ARP_PACKET
 *
 * Called when an ARP packet is dropped due to invalid protocol and hardware
 * fields in the header at address pxARPHeader.
 */
#define iptraceDROPPED_INVALID_ARP_PACKET( pxARPHeader )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DROPPED_INVALID_ARP_PACKET, pxARPHeader = %p\n", __func__, pxARPHeader);
/*---------------------------------------------------------------------------*/

/*
 * iptracePACKET_DROPPED_TO_GENERATE_ARP
 *
 * Called when a packet destined for the IP address ulIPAddress is dropped
 * because the ARP cache does not contain an entry for the IP address. The
 * packet is automatically replaced by an ARP packet. ulIPAddress is expressed
 * as a 32-bit number in network byte order.
 */
#define iptracePACKET_DROPPED_TO_GENERATE_ARP( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: PACKET_DROPPED_TO_GENERATE_ARP, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptracePROCESSING_RECEIVED_ARP_REPLY
 *
 * Called when the ARP cache is about to be updated in response to the
 * reception of an ARP reply. ulIPAddress holds the ARP message's target IP
 * address (as a 32-bit number in network byte order), which may not be the
 * local network node (depending on the FreeRTOSIPConfig.h settings).
 */
#define iptracePROCESSING_RECEIVED_ARP_REPLY( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: PROCESSING_RECEIVED_ARP_REPLY, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_ARP_REPLY
 *
 * An ARP reply is being sent in response to an ARP request from the IP address
 * ulIPAddress. ulIPAddress is expressed as a 32-bit number in network byte
 * order.
 */
#define iptraceSENDING_ARP_REPLY( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_ARP_REPLY, ulIPAddress = %u\n", __func__, ulIPAddress);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             ARP TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ND TRACE MACROS                               */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceND_TABLE_ENTRY_EXPIRED
 */
#define iptraceND_TABLE_ENTRY_EXPIRED( pxIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ND_TABLE_ENTRY_EXPIRED, pxIPAddress = %p\n", __func__, pxIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceND_TABLE_ENTRY_WILL_EXPIRE
 */
#define iptraceND_TABLE_ENTRY_WILL_EXPIRE( pxIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ND_TABLE_ENTRY_WILL_EXPIRE, pxIPAddress = %p\n", __func__, pxIPAddress);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_BUFFER_FULL
 *
 * A packet has come in from an unknown IPv6 address. An ND request has been
 * sent, but the queue is still filled with a different packet.
 */
#define iptraceDELAYED_ND_BUFFER_FULL()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ND_BUFFER_FULL\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptrace_DELAYED_ND_REQUEST_REPLIED
 */
#define iptrace_DELAYED_ND_REQUEST_REPLIED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: _DELAYED_ND_REQUEST_REPLIED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_REQUEST_STARTED
 *
 * A packet came in from an unknown IPv6 address. An ND request has been sent
 * and the network buffer is stored for processing later.
 */
#define iptraceDELAYED_ND_REQUEST_STARTED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ND_REQUEST_STARTED\n", __func__);
/*---------------------------------------------------------------------------*/

/*
 * iptraceDELAYED_ND_TIMER_EXPIRED
 *
 * A packet was stored for delayed processing, but there is no ND reply. The
 * network buffer will be released without being processed.
 */
#define iptraceDELAYED_ND_TIMER_EXPIRED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DELAYED_ND_TIMER_EXPIRED\n", __func__);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                              ND TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             DHCP TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
 *
 * Called when the default IP address is used because an IP address could not
 * be obtained from a DHCP. ulIPAddress is expressed as a 32-bit number in
 * network byte order.
 */
#define iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DHCP_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS, ulIPAddress = %u\n", __func__, ulIPAddress);
/*-----------------------------------------------------------------------*/

/*
 * iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS
 */
#define iptraceDHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS( xIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DHCP_REQUESTS_FAILED_USING_DEFAULT_IPv6_ADDRESS, xIPAddress = %zd\n", __func__, xIPAddress);
/*-----------------------------------------------------------------------*/

/*
 * iptraceDHCP_SUCCEEDED
 *
 * Called when DHCP negotiation is complete and the IP address in
 * ulOfferedIPAddress is offered to the device.
 */
#define iptraceDHCP_SUCCEEDED( ulOfferedIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DHCP_SUCCEEDED, ulOfferedIPAddress = %u\n", __func__, ulOfferedIPAddress);
/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_DHCP_DISCOVER
 *
 * Called when a DHCP discover packet is sent.
 */
#define iptraceSENDING_DHCP_DISCOVER()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_DHCP_DISCOVER\n", __func__);
/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_DHCP_REQUEST
 *
 * Called when a DHCP request packet is sent.
 */
#define iptraceSENDING_DHCP_REQUEST()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_DHCP_REQUEST\n", __func__);
/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             DHCP TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             DNS TRACE MACROS                              */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceSENDING_DNS_REQUEST
 *
 * Called when a DNS request is sent.
 */
#define iptraceSENDING_DNS_REQUEST()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_DNS_REQUEST\n", __func__);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             DNS TRACE MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             ICMP TRACE MACROS                             */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceICMP_PACKET_RECEIVED
 *
 * Called when an ICMP packet is received.
 */
#define iptraceICMP_PACKET_RECEIVED()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: ICMP_PACKET_RECEIVED\n", __func__);
/*-----------------------------------------------------------------------*/

/*
 * iptraceSENDING_PING_REPLY
 *
 * Called when an ICMP echo reply (ping reply) is sent to the IP address
 * ulIPAddress in response to an ICMP echo request (ping request) originating
 * from the same address. ulIPAddress is expressed as a 32-bit number in
 * network byte order.
 */
#define iptraceSENDING_PING_REPLY( ulIPAddress )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: SENDING_PING_REPLY, ulIPAddress = %u\n", __func__, ulIPAddress);
/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             ICMP TRACE MACROS                             */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                      ROUTER ADVERTISEMENT TRACE MACROS                    */
/*===========================================================================*/

/*---------------------------------------------------------------------------*/

/*
 * iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS
 */
#define iptraceRA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS( ipv6_address )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: RA_REQUESTS_FAILED_USING_DEFAULT_IP_ADDRESS, ipv6_address = %lu\n", __func__, ipv6_address);
/*---------------------------------------------------------------------------*/

/*
 * iptraceRA_SUCCEEDED
 */
#define iptraceRA_SUCCEEDED( ipv6_address )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: RA_SUCCEEDED, ipv6_address = %lu\n", __func__, ipv6_address);
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                      ROUTER ADVERTISEMENT TRACE MACROS                    */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                             MEM STATS MACROS                              */
/*===========================================================================*/

/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_CLOSE
 *
 * Should be called by the application when the collection of memory
 * statistics should be stopped.
 */
#define iptraceMEM_STATS_CLOSE()\
    IP_TRACE_PRINT_STRING("[iptrace] %s: MEM_STATS_CLOSE\n", __func__);
/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_CREATE
 *
 * Called when an object at address pxObject of type xMemType and size
 * uxSize has been allocated from the heap.
 */
#define iptraceMEM_STATS_CREATE( xMemType, pxObject, uxSize )//\
    //IP_TRACE_PRINT_STRING("[iptrace] %s: MEM_STATS_CREATE, xMemType = %zd, pxObject = %p, uxSize = %lu\n", __func__, xMemType, pxObject, uxSize);
/*-----------------------------------------------------------------------*/

/*
 * iptraceMEM_STATS_DELETE
 *
 * Called when an object at address pxObject has been deallocated and the
 * memory has been returned to the heap.
 */
#define iptraceMEM_STATS_DELETE( pxObject )//\
    //IP_TRACE_PRINT_STRING("[iptrace] %s: MEM_STATS_DELETE, pxObject = %p\n", __func__, pxObject);
/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                             MEM STATS MACROS                              */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/*                           TCP DUMP TRACE MACROS                           */
/*===========================================================================*/

/*-----------------------------------------------------------------------*/

/*
 * iptraceDUMP_INIT
 */
#define iptraceDUMP_INIT( pcFileName, pxEntries )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DUMP_INIT, pcFileName = %p, pxEntries = %p\n", __func__, pcFileName, pxEntries);
/*-----------------------------------------------------------------------*/

/*
 * iptraceDUMP_PACKET
 */
#define iptraceDUMP_PACKET( pucBuffer, uxLength, xIncoming )\
    IP_TRACE_PRINT_STRING("[iptrace] %s: DUMP_PACKET, pucBuffer = %p, uxLength = %lu, xIncoming = %zd\n", __func__, pucBuffer, uxLength, xIncoming);
/*-----------------------------------------------------------------------*/

/*===========================================================================*/
/*                           TCP DUMP TRACE MACROS                           */
/*===========================================================================*/
/*---------------------------------------------------------------------------*/
/*===========================================================================*/

#endif /* FREERTOS_IP_TRACE_ENABLE */

#endif /* FREERTOS_IP_TRACE_MACRO_CUSTOM_H */
