#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

/*
 * From:
 * https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/06-Configuration
 * Updated Aug 2025
 *
 */

/*
 * Used to standardize macro checks since ( MACRO == 1 ) and ( MACRO != 0 )
 * are used inconsistently.
 */

#ifndef ipconfigENABLE
    #define ipconfigENABLE    ( 1 )
#endif

#ifndef ipconfigDISABLE
    #define ipconfigDISABLE    ( 0 )
#endif

#ifndef ipconfigIS_ENABLED
    #define ipconfigIS_ENABLED( x )    ( ( x ) != ipconfigDISABLE )
#endif

#ifndef ipconfigIS_DISABLED
    #define ipconfigIS_DISABLED( x )    ( ( x ) == ipconfigDISABLE )
#endif

/*========== Constants Effecting the TCP/IP Stack Task Execution Behaviour ===========================================*/

/*
 * ipconfigEVENT_QUEUE_LENGTH
 *
 * A FreeRTOS queue is used to send events from application tasks to the IP stack.
 * ipconfigEVENT_QUEUE_LENGTH sets the maximum number of events that can be queued for processing at any one time.
 * The event queue must be a minimum of 5 greater than the total number of network buffers.
 *
 * default: ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 )
 */
//#define ipconfigEVENT_QUEUE_LENGTH

/*
 * ipconfigIP_TASK_PRIORITY
 *
 * The TCP/IP stack executes it its own RTOS task (although any application RTOS task can make use of its
 * services through the published sockets API). ipconfigIP_TASK_PRIORITY sets the priority of the RTOS task
 * that executes the TCP/IP stack. The priority is a standard FreeRTOS task priority so it can take any value
 * from 0 (the lowest priority) to (configMAX_PRIORITIES - 1) (the highest priority). configMAX_PRIORITIES is
 * a standard FreeRTOS configuration parameter defined in FreeRTOSConfig.h, not FreeRTOSIPConfig.h.
 * Consideration needs to be given as to the priority assigned to the RTOS task executing the TCP/IP stack
 * relative to the priority assigned to tasks that use the TCP/IP stack.
 *
 * default: ( configMAX_PRIORITIES - 2 )
 */
//#define ipconfigIP_TASK_PRIORITY

/*
 * ipconfigIP_TASK_STACK_SIZE_WORDS
 *
 * The size, in words (not bytes), of the stack allocated to the FreeRTOS-Plus-TCP RTOS task.
 * FreeRTOS includes optional stack overflow detection.
 *
 * default: configMINIMALSTACKSIZE
 */
#define ipconfigIP_TASK_STACK_SIZE_WORDS 512

/*
 * ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES
 *
 * If ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES is set to 1, then the TCP/IP stack will call
 * eApplicationProcessCustomFrameHook to process any unknown frame, that is,
 * any frame that expects ARP or IP.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigPROCESS_CUSTOM_ETHERNET_FRAMES ipconfigDISABLE

/*
 * ipconfigUSE_NETWORK_EVENT_HOOK
 *
 * If ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 then FreeRTOS-Plus-TCP will call the network event
 * hook at the appropriate times. If ipconfigUSE_NETWORK_EVENT_HOOK is not set to 1 then the network
 * event hook will never be called.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_NETWORK_EVENT_HOOK ipconfigDISABLE

/*========== Debug, Trace and Logging Settings =======================================================================*/

/*
 * ipconfigCHECK_IP_QUEUE_SPACE
 *
 * A FreeRTOS queue is used to send events from application tasks to the IP stack.
 * ipconfigEVENT_QUEUE_LENGTH sets the maximum number of events that can be queued for processing
 * at any one time. If ipconfigCHECK_IP_QUEUE_SPACE is set to 1 then the uxGetMinimumIPQueueSpace()
 * function can be used to query the minimum amount of free space that has existed in the queue
 * since the system booted.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigCHECK_IP_QUEUE_SPACE ipconfigDISABLE

/*
 * ipconfigHAS_DEBUG_PRINTF and FreeRTOS_debug_printf
 *
 * The TCP/IP stack outputs debugging messages by calling the FreeRTOS_debug_printf macro.
 * To obtain debugging messages set ipconfigHAS_DEBUG_PRINTF to 1, then define FreeRTOS_debug_printf()
 * to a function that takes a printf() style format string and variable number of inputs,
 * and sends the formatted messages to an output of your choice.
 * Do not define FreeRTOS_debug_printf if ipconfigHAS_DEBUG_PRINTF is set to 0.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigHAS_DEBUG_PRINTF ipconfigENABLE

#if ipconfigIS_ENABLED( ipconfigHAS_DEBUG_PRINTF )
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "stdio.h"
static char freertosip_debug_printstr[256];
#define FREERTOS_DEBUG_PRINT_STRING(...)\
    do{\
        snprintf(freertosip_debug_printstr, 256, __VA_ARGS__);\
        MSS_UART_polled_tx_string(&g_mss_uart0_lo, (uint8_t *)freertosip_debug_printstr);\
    }\
    while(0)
#define FreeRTOS_debug_printf(MSG) FREERTOS_DEBUG_PRINT_STRING MSG

#endif /* ipconfigIS_ENABLED ipconfigHAS_DEBUG_PRINTF */

/*
 * ipconfigHAS_PRINTF and FreeRTOS_printf
 *
 * Some of the TCP/IP stack demo applications generate output messages. The TCP/IP stack
 * outputs these messages by calling the FreeRTOS_printf macro. To obtain the demo application
 * messages set ipconfigHAS_PRINTF to 1, then define FreeRTOS_printf() to a function that takes a
 * printf() style format string and variable number of inputs, and sends the formatted messages
 * to an output of your choice.
 *
 * Do not define FreeRTOS_printf if ipconfigHAS_PRINTF is set to 0.
 *
 * ipconfigHAS_PRINTF default: ipconfigDISABLE
 * FreeRTOS_printf default: do {} while( ipFALSE_BOOL )
 */
//#define ipconfigHAS_PRINTF ipconfigENABLE

#if ipconfigIS_ENABLED( ipconfigHAS_PRINTF )
#define FreeRTOS_printf(MSG) FREERTOS_DEBUG_PRINT_STRING MSG
#endif

/*
 * ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS
 *
 * The macro configINCLUDE_TRACE_RELATED_CLI_COMMANDS can be defined in FreeRTOSConfig.h.
 * When defined, it will be assigned to ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS.
 * It allows the inclusion of a CLI for tracing purposes.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigINCLUDE_EXAMPLE_FREERTOS_PLUS_TRACE_CALLS ipconfigDISABLE

/*
 * ipconfigTCP_IP_SANITY
 *
 * The name of this macro is a bit misleading: it only checks the behaviour of the module
 * BufferAllocation_1.c. It issues warnings when irregularities are detected.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigTCP_IP_SANITY ipconfigDISABLE

/*
 * ipconfigTCP_MAY_LOG_PORT( x )
 *
 * ipconfigTCP_MAY_LOG_PORT( x ) can be defined to specify which port numbers should
 * or should not be logged by FreeRTOS_lprintf().
 *
 * default: ( ( x ) != 23U )
 */
//#define ipconfigTCP_MAY_LOG_PORT

/*
 * ipconfigWATCHDOG_TIMER()
 *
 * ipconfigWATCHDOG_TIMER() is a macro that is called on each iteration of the IP task and may be useful
 * if the application included watchdog type functionality that needs to know the IP task is still cycling
 * (although the fact that the IP task is cycling does not necessarily indicate it is functioning correctly).
 * ipconfigWATCHDOG_TIMER() can be defined to perform any action desired by the application writer.
 * If ipconfigWATCHDOG_TIMER() is left undefined then it will be removed completely by the pre-processor
 * (it will default to an empty macro).
 *
 * default: do {} while( ipFALSE_BOOL )
 */
//#define ipconfigWATCHDOG_TIMER

/*========== Hardware and Driver Specific Settings ===================================================================*/

/*
 * ipconfigBUFFER_PADDING and ipconfigPACKET_FILLER_SIZE
 *
 * Advanced driver implementation use only.
 * When the application requests a network buffer, the size of the network buffer is specified by the
 * application writer, but the size of the network buffer actually obtained is increased by
 * ipconfigBUFFER_PADDING bytes. The first ipconfigBUFFER_PADDING bytes of the buffer is then used to
 * hold metadata about the buffer, and the area that actually stores the data follows the metadata.
 * This mechanism is transparent to the user as the user only see a pointer to the area within the
 * buffer actually used to hold network data.
 *
 * Some network hardware has very specific byte alignment requirements, so ipconfigBUFFER_PADDING
 * is provided as a configurable parameter to allow the writer of the network driver to influence the
 * alignment of the start of the data that follows the metadata.
 *
 * ipconfigBUFFER_PADDING default: 0
 * ipconfigPACKET_FILLER_SIZE default: 2
 */
//#define ipconfigBUFFER_PADDING
//#define ipconfigPACKET_FILLER_SIZE

/*
 * ipconfigBYTE_ORDER
 *
 * If the microcontroller on which FreeRTOS-Plus-TCP is running is big endian then ipconfigBYTE_ORDER
 * must be set to pdFREERTOS_BIG_ENDIAN. If the microcontroller is little endian then ipconfigBYTE_ORDER
 * must be set to pdFREERTOS_LITTLE_ENDIAN.
 *
 * default: no default, must be defined here
 */
#define ipconfigBYTE_ORDER pdFREERTOS_LITTLE_ENDIAN

/*
 * ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM
 *
 * If the network driver or network hardware is calculating the IP, TCP and UDP checksums of
 * incoming packets, and discarding packets that are found to contain invalid checksums, then set
 * ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM to 1, otherwise set ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM
 * to 0.
 *
 * Throughput and processor load are greatly improved by implementing drivers that make use of
 * hardware checksum calculations.
 *
 * Note: From FreeRTOS-Plus-TCP V2.3.0, the length is checked in software even when it has already
 * been checked in hardware.
 *
 * Note: If hardware supports checking TCP checksum only, the network interface layer should handle
 * the same for other protocols, such as IP/UDP/ICMP/etc, and give the checksum verified packets to
 * the FreeRTOS-plus-TCP stack.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM

/*
 * ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM
 *
 * If the network driver or network hardware is calculating the IP, TCP and UDP checksums of outgoing
 * packets then set ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM to 1, otherwise set
 * ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM to 0.
 *
 * Throughput and processor load are greatly improved by implementing drivers that make use of
 * hardware checksum calculations.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM

/*
 * ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES
 *
 * Ethernet/hardware MAC addresses are used to address Ethernet frames.
 * If the network driver or hardware is discarding packets that do not contain a MAC address
 * of interest then set ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES to 1. Otherwise set
 * ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES to 0.
 *
 * Throughput and processor load are greatly improved by implementing network address
 * filtering in hardware. Most network interfaces allow multiple MAC addresses to be
 * defined so filtering can allow through the unique hardware address of the node,
 * the broadcast address, and various multicast addresses.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES

/*
 * ipconfigETHERNET_DRIVER_FILTERS_PACKETS
 *
 * For expert users only.
 *
 * Whereas ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is used to specify whether or
 * not the network driver or hardware filters Ethernet frames,
 * ipconfigETHERNET_DRIVER_FILTERS_PACKETS is used to specify whether or not the network
 * driver filters the IP, UDP or TCP data within the Ethernet frame.
 *
 * The TCP/IP stack is only interested in receiving data that is either addresses to a socket
 * (IP address and port number) on the local node, or is a broadcast or multicast packet.
 * Throughput and process load can be greatly improved by preventing packets that do not meet
 * these criteria from being sent to the TCP/IP stack.
 * FreeRTOS provides some features that allow such filtering to take place in the network driver.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigETHERNET_DRIVER_FILTERS_PACKETS

/*
 * ipconfigETHERNET_MINIMUM_PACKET_BYTES
 *
 * When the device is connected to a LAN, it is strongly recommended to give each outgoing packet a
 * minimum length of 60 bytes (plus 4 bytes CRC). The macro ipconfigETHERNET_MINIMUM_PACKET_BYTES
 * determines the minimum length.
 * By default, it is defined as zero, meaning that packets will be sent as they are.
 *
 * default: 0
 */
//#define ipconfigETHERNET_MINIMUM_PACKET_BYTES

/*
 * ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES
 *
 * If ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES is set to 1 then Ethernet frames that are not
 * in Ethernet II format will be dropped.
 * This option is included for potential future IP stack developments.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES

/*
 * ipconfigNETWORK_MTU
 *
 * The MTU is the maximum number of bytes the payload of a network frame can contain.
 * For normal Ethernet V2 frames the maximum MTU is 1500 (although a lower number may be
 * required for Internet routing). Setting a lower value can save RAM, depending on the
 * buffer management scheme used.
 *
 * default: 1500
 */
//#define ipconfigNETWORK_MTU

/*
 * ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS
 *
 * ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS defines the total number of network buffer that are available
 * to the TCP/IP stack. The total number of network buffers is limited to ensure the total amount of RAM
 * that can be consumed by the TCP/IP stack is capped to a pre-determinable value.
 * How the storage area is actually allocated to the network buffer structures is not fixed,
 * but part of the portable layer. The simplest scheme simply allocates the exact amount of
 * storage as it is required.
 *
 * default: 45
 */
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS 10

/*
 * ipconfigUSE_LINKED_RX_MESSAGES
 *
 * Advanced users only.
 *
 * When ipconfigUSE_LINKED_RX_MESSAGES is set to 1 it is possible to reduce CPU load during periods of heavy
 * network traffic by linking multiple received packets together, then passing all the linked packets to the
 * IP RTOS task in one go.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_LINKED_RX_MESSAGES ipconfigENABLE

/*
 * ipconfigZERO_COPY_RX_DRIVER
 *
 * Advanced users only.
 *
 * If ipconfigZERO_COPY_RX_DRIVER is set to 1 then the network interface will assign network buffers
 * NetworkBufferDescriptor_t::pucEthernetBuffer to the DMA of the EMAC. When a packet is received, no data is copied.
 * Instead, the buffer is sent directly to the IP-task. If the TX zero-copy option is disabled,
 * every received packet will be copied from the DMA buffer to the network buffer of type NetworkBufferDescriptor_t.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigZERO_COPY_RX_DRIVER

/*
 * ipconfigZERO_COPY_TX_DRIVER
 *
 * Advanced users only.
 *
 * If ipconfigZERO_COPY_TX_DRIVER is set to 1 then the driver function xNetworkInterfaceOutput() will always be called with its
 * bReleaseAfterSend parameter set to pdTRUE - meaning it is always the driver that is responsible for freeing the network buffer
 * and network buffer descriptor. This is useful if the driver implements a zero-copy scheme whereby the packet data is sent
 * directly from within the network buffer (for example by pointing a DMA descriptor at the data within the network buffer),
 * instead of copying the data out of the network buffer before the data is sent (for example by copying the data into a separate
 * pre-allocated DMA descriptor). In such cases the driver needs to take ownership of the network buffer because the network buffer
 * can only be freed after the data has actually been transmitted - which might be some time after the xNetworkInterfaceOutput()
 * function returns. See the examples on the Porting FreeRTOS to a Different Microcontroller documentation page for worked examples.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigZERO_COPY_TX_DRIVER

/*
 * ipconfigSUPPORT_NETWORK_DOWN_EVENT
 *
 * Set to 1 if you want to receive eNetworkDown notification via the vApplicationIPNetworkEventHook_Multi() callback.
 * Note: Not all drivers support this feature.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSUPPORT_NETWORK_DOWN_EVENT

/*========== TCP Specific Constants ==================================================================================*/

/*
 * ipconfigIGNORE_UNKNOWN_PACKETS
 *
 * Normally TCP packets that have a bad or unknown destination will result in a RESET being sent back to the remote host.
 * If ipconfigIGNORE_UNKNOWN_PACKETS is set to 1 then such resets will be suppressed (not sent).
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigIGNORE_UNKNOWN_PACKETS

/*
 * ipconfigTCP_HANG_PROTECTION
 *
 * If ipconfigTCP_HANG_PROTECTION is set to 1 then FreeRTOS-Plus-TCP will mark a socket as closed if there is no
 * status change on the socket within the period of time specified by ipconfigTCP_HANG_PROTECTION_TIME.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigTCP_HANG_PROTECTION

/*
 * ipconfigTCP_HANG_PROTECTION_TIME
 *
 * If ipconfigTCP_HANG_PROTECTION is set to 1 then ipconfigTCP_HANG_PROTECTION_TIME sets the interval in seconds
 * between the status of a socket last changing and the anti-hang mechanism marking the socket as closed.
 *
 * default: 30
 */
//#define ipconfigTCP_HANG_PROTECTION_TIME

/*
 * ipconfigTCP_KEEP_ALIVE
 *
 * Sockets that are connected but do not transmit any data for an extended period can be disconnected by routers or
 * firewalls that time out. This can be avoided at the application level by ensuring the application periodically
 * sends a packet. Alternatively FreeRTOS-Plus-TCP can be configured to automatically send keep alive messages when it
 * detects that a connection is dormant. Note that, while having FreeRTOS-Plus-TCP automatically send keep alive
 * messages is the more convenient method, it is also the least reliable method because some routers will discard
 * keep alive messages.
 *
 * Set ipconfigTCP_KEEP_ALIVE to 1 to have FreeRTOS-Plus-TCP periodically send keep alive messages on connected
 * but dormant sockets. Set ipconfigTCP_KEEP_ALIVE to 0 to prevent the automatic transmission of keep alive messages.
 *
 * If FreeRTOS-Plus-TCP does not receive a reply to a keep alive message then the connection will be broken and
 * the socket will be marked as closed. Subsequent FreeRTOS_recv() calls on the socket will return
 * -pdFREERTOS_ERRNO_ENOTCONN.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigTCP_KEEP_ALIVE

/*
 * ipconfigTCP_KEEP_ALIVE_INTERVAL
 *
 * If ipconfigTCP_KEEP_ALIVE is set to 1 then ipconfigTCP_KEEP_ALIVE_INTERVAL sets the interval in seconds between
 * successive keep alive messages. Keep alive messages are not sent at all unless ipconfigTCP_KEEP_ALIVE_INTERVAL
 * seconds have passed since the last packet was sent or received.
 *
 * default: 20
 */
//#define ipconfigTCP_KEEP_ALIVE_INTERVAL

/*
 * ipconfigTCP_MSS
 *
 * Sets the MSS value (in bytes) for all TCP packets.
 * Note that FreeRTOS-Plus-TCP contains checks that the defined ipconfigNETWORK_MTU
 * and ipconfigTCP_MSS values are consistent with each other.
 *
 * The default is derived from MTU - ( ipconfigNETWORK_MTU + ipSIZE_OF_TCP_HEADER )
 * Where ipconfigNETWORK_MTU + ipSIZE_OF_TCP_HEADER is 40 bytes.
 *
 * default: ( ipconfigNETWORK_MTU - 40U )
 */
//#define ipconfigTCP_MSS

/*
 * ipconfigTCP_RX_BUFFER_LENGTH and ipconfigTCP_TX_BUFFER_LENGTH
 *
 * Each TCP socket has a buffer for reception and a separate buffer for transmission.
 * The default buffer size is (4 * ipconfigTCP_MSS). FreeRTOS_setsockopt() can be used with the
 * FREERTOS_SO_RCVBUF and FREERTOS_SO_SNDBUF parameters to set the receive and send buffer sizes respectively
 * - but this must be done between the time that the socket is created and the buffers used by the socket are created.
 * The receive buffer is not created until data is actually received, and the transmit buffer is not created
 * until data is actually sent to the socket for transmission.
 * Once the buffers have been created their sizes cannot be changed.
 *
 * If a listening socket creates a new socket in response to an incoming connect request then the
 * new socket will inherit the buffers sizes of the listening socket.
 *
 * ipconfigTCP_RX_BUFFER_LENGTH default: ( 4 * ipconfigTCP_MSS )
 * ipconfigTCP_TX_BUFFER_LENGTH default: ( 4 * ipconfigTCP_MSS )
 */
//#define ipconfigTCP_RX_BUFFER_LENGTH
//#define ipconfigTCP_TX_BUFFER_LENGTH

/*
 * ipconfigTCP_TIME_TO_LIVE
 *
 * Defines the Time To Live (TTL) values used in outgoing TCP packets.
 *
 * default: 128
 */
//define ipconfigTCP_TIME_TO_LIVE

/*
 * ipconfigTCP_WIN_SEG_COUNT
 *
 * If ipconfigUSE_TCP_WIN is set to 1 then each socket will use a sliding window.
 * Sliding windows allow messages to arrive out-of order, and FreeRTOS-Plus-TCP uses window descriptors
 * to track information about the packets in a window. A pool of descriptors is allocated when the first
 * TCP connection is made. The descriptors are shared between all the sockets.
 * ipconfigTCP_WIN_SEG_COUNT sets the number of descriptors in the pool, and each descriptor is approximately 64 bytes.
 *
 * As an example: If a system will have at most 16 simultaneous TCP connections,
 * and each connection will have an Rx and Tx window of at most 8 segments, then the worst case maximum number
 * of descriptors that will be required is 256 ( 16 * 2 * 8 ).
 * However, the practical worst case is normally much lower than this as most packets will arrive in order.
 *
 * default: 256
 */
//#define ipconfigTCP_WIN_SEG_COUNT

/*
 * ipconfigUSE_TCP
 *
 * Set ipconfigUSE_TCP to 1 to enable TCP. If ipconfigUSE_TCP is set to 0 then only UDP is available.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_TCP ipconfigENABLE

/*
 * ipconfigUSE_TCP_WIN
 *
 * Sliding Windows allows messages to arrive out-of-order.
 *
 * Set ipconfigUSE_TCP_WIN to 1 to include sliding window behaviour in TCP sockets.
 * Set ipconfigUSE_TCP_WIN to 0 to exclude sliding window behaviour in TCP sockets.
 *
 * Sliding windows can increase throughput while minimising network traffic at the expense of consuming more RAM.
 * The size of the sliding window can be changed from its default using the FREERTOS_SO_WIN_PROPERTIES
 * parameter to FreeRTOS_setsockopt(). The sliding window size is specified in units of MSS
 * (so if the MSS is set to 200 bytes then a sliding window size of 2 is equal to 400 bytes) and must
 * always be smaller than or equal to the size of the internal buffers in both directions.
 *
 * If a listening socket creates a new socket in response to an incoming connect request then the
 * new socket will inherit the sliding window sizes of the listening socket.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigUSE_TCP_WIN

/*
 * ipconfigTCP_SRTT_MINIMUM_VALUE_MS
 *
 * The minimum value of TCP Smoothed Round Trip Time (SRTT).
 * When measuring the Smoothed Round Trip Time (SRTT), the result will be rounded up to a minimum value.
 * The default has always been 50 ms, but a value of 1000 ms is recommended (see RFC6298) because hosts
 * often delay the sending of ACK packets with 200 ms.
 *
 * default: 50
 */
//#define ipconfigTCP_SRTT_MINIMUM_VALUE_MS

/*========== UDP Specific Constants ==================================================================================*/

/*
 * ipconfigUDP_MAX_RX_PACKETS
 *
 * ipconfigUDP_MAX_RX_PACKETS defines the maximum number of packets that can exist in the Rx queue of a UDP socket.
 * For example, if ipconfigUDP_MAX_RX_PACKETS is set to 5 and there are already 5 packets queued on the UDP socket
 * then subsequent packets received on that socket will be dropped until the queue length is less than 5 again.
 *
 * default: 0
 */
#define ipconfigUDP_MAX_RX_PACKETS 10

/*
 * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS
 *
 * Sockets have a send block time attribute. If FreeRTOS_sendto() is called but a network buffer cannot be obtained,
 * then the calling RTOS task is held in the Blocked state (so other tasks can continue to executed) until either a
 * network buffer becomes available or the send block time expires.
 * If the send block time expires then the send operation is aborted.
 *
 * The maximum allowable send block time is capped to the value set by ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS.
 * Capping the maximum allowable send block time prevents prevents a deadlock occurring when all the network buffers
 * are in use and the tasks that process (and subsequently free) the network buffers are themselves blocked waiting
 * for a network buffer.
 *
 * ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS is specified in RTOS ticks. A time in milliseconds can be converted to a time
 * in ticks by dividing the time in milliseconds by portTICK_PERIOD_MS.
 *
 * default: pdMS_TO_TICKS( 20 )
 */
//#define ipconfigUDP_MAX_SEND_BLOCK_TIME_TICKS

/*
 * ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS
 *
 * If ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS is set to 1 then FreeRTOS-Plus-TCP will accept UDP packets that have
 * their checksum value set to 0, which is in compliance with the UDP specification.
 *
 * If ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS is set to 0 then FreeRTOS-Plus-TCP will drop UDP packets that have
 * their checksum value set to 0, which deviates from the UDP specification, but is safer.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigUDP_PASS_ZERO_CHECKSUM_PACKETS

/*
 * ipconfigUDP_TIME_TO_LIVE
 *
 * Defines the Time To Live (TTL) values used in outgoing UDP packets.
 *
 * default: 128
 */
//define ipconfigUDP_TIME_TO_LIVE

/*=========== Other Constants Effecting Socket Behaviour =============================================================*/

/*
 * ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND
 *
 * The address of a socket is the combination of its IP address and its port number. FreeRTOS_bind() is used
 * to manually allocate a port number to a socket (to 'bind' the socket to a port), but manual binding is not
 * normally necessary for client sockets (those sockets that initiate outgoing connections rather than wait for
 * incoming connections on a known port number). If ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 then calling
 * FreeRTOS_sendto() on a socket that has not yet been bound will result in the IP stack automatically binding
 * the socket to a port number from the range socketAUTO_PORT_ALLOCATION_START_NUMBER to 0xffff.
 * If ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 0 then calling FreeRTOS_sendto() on a socket that has not
 * yet been bound will result in the send operation being aborted.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND

/*
 * ipconfigINCLUDE_FULL_INET_ADDR
 *
 * Implementing FreeRTOS_inet_addr() necessitates the use of string handling routines, which are relatively large.
 * To save code space, the full FreeRTOS_inet_addr() implementation is made optional, and a smaller and faster
 * alternative called FreeRTOS_inet_addr_quick() is provided. FreeRTOS_inet_addr() takes an IP in decimal dot
 * format (for example, "192.168.0.1") as its parameter. FreeRTOS_inet_addr_quick() takes an IP address as
 * four separate numerical octets (for example, 192, 168, 0, 1) as its parameters. If ipconfigINCLUDE_FULL_INET_ADDR
 * is set to 1, then both FreeRTOS_inet_addr() and FreeRTOS_indet_addr_quick() are available.
 * If ipconfigINCLUDE_FULL_INET_ADDR is not set to 1, then only FreeRTOS_indet_addr_quick() is available.
 *
 * default: no default, must be set here
 */
#define ipconfigINCLUDE_FULL_INET_ADDR ipconfigENABLE

/*
 * ipconfigSELECT_USES_NOTIFY
 *
 * This option is only used in case the socket-select functions are activated (when ipconfigSUPPORT_SELECT_FUNCTION
 * is non-zero). When calling select() for a given socket from the same task, this macro is not required.
 * Only when there are multiple tasks using select on the same sockets, this option may prevent a dead-lock.
 * The problem is that the event bit eSELECT_CALL_IP is waited for and cleared by multiple tasks. The macro
 * ipconfigSELECT_USES_NOTIFY defaults to zero, meaning not active.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSELECT_USES_NOTIFY

/*
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME
 *
 * API functions used to read data from a socket can block to wait for data to become available.
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME sets the default block time defined in RTOS ticks. If
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME is not defined then the default block time will be set to
 * portMAX_DELAY - meaning an RTOS task that is blocked on a socket read will not leave the Blocked state
 * until data is available. Note that tasks in the Blocked state do not consume any CPU time.
 *
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME is specified in ticks. The macros pdMS_TO_TICKS() and
 * portTICK_PERIOD_MS can both be used to convert a time specified in milliseconds to a time specified in ticks.
 *
 * The timeout time can be changed at any time using the FREERTOS_SO_RCVTIMEO parameter with FreeRTOS_setsockopt().
 * Note: Infinite block times should be used with extreme care in order to avoid a situation where all tasks are
 * blocked indefinitely to wait for another RTOS task (which is also blocked indefinitely) to free a network buffer.
 *
 * A socket can be set to non-blocking mode by setting both the send and receive block time to 0.
 * This might be desirable when an RTOS task is using more than one socket - in which case blocking can instead
 * by performed on all the sockets at once using FreeRTOS_select(), or the RTOS task can set
 * ipconfigSOCKET_HAS_USER_SEMAPHORE to one, then block on its own semaphore.
 *
 * default: portMAX_DELAY
 */
//#define ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME

/*
 * ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
 *
 * When writing to a socket, the write may not be able to proceed immediately. For example, depending on the
 * configuration, a write might have to wait for a network buffer to become available. API functions used to
 * write data to a socket can block to wait for the write to succeed. ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME
 * sets the default block time (defined in RTOS ticks). If ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME is not defined,
 * then the default block time will be set to portMAX_DELAY - meaning an RTOS task that is blocked on a socket
 * read will not leave the Blocked state until data is available. Note that tasks in the Blocked state do not
 * consume any CPU time.
 *
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME is specified in ticks. The macros pdMS_TO_TICKS()
 * and portTICK_PERIOD_MS can both be used to convert a time specified in milliseconds to a time specified in ticks.
 *
 * The timeout time can be changed at any time using the FREERTOS_SO_SNDTIMEO parameter with FreeRTOS_setsockopt().
 * Note: Infinite block times should be used with extreme care in order to avoid a situation where all tasks are blocked
 * indefinitely to wait for another RTOS task (which is also blocked indefinitely) to free a network buffer.
 *
 * A socket can be set to non-blocking mode by setting both the send and receive block time to 0. This might be desirable
 * when an RTOS task is using more than one socket - in which case blocking can instead by performed on all the
 * sockets at once using FreeRTOS_select(), or the RTOS task can set ipconfigSOCKET_HAS_USER_SEMAPHORE to one,
 * then block on its own semaphore.
 *
 * A socket can be set to non-blocking mode by setting both the send and receive block time to 0.
 *
 * default: portMAX_DELAY
 */
//#define ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME

/*
 * ipconfigSOCKET_HAS_USER_SEMAPHORE
 *
 * By default, sockets will block on a send or receive that cannot complete immediately. See the description of the
 * ipconfigSOCK_DEFAULT_RECEIVE_BLOCK_TIME and ipconfigSOCK_DEFAULT_SEND_BLOCK_TIME parameters.
 *
 * If an RTOS task is using multiple sockets and cannot block on one socket at a time, then the sockets can be set
 * into non-blocking mode, and the RTOS task can block on all the sockets at once by either using the FreeRTOS_select()
 * function or by setting ipconfigSOCKET_HAS_USER_SEMAPHORE to 1, using the FREERTOS_SO_SET_SEMAPHORE parameter with
 * FreeRTOS_setsockopt() to provide a semaphore to the socket, and then blocking on the semaphore.
 * The semaphore will be given when any of the sockets are able to proceed - at which time the RTOS task can inspect
 * all the sockets individually using non blocking API calls to determine which socket caused it to unblock.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSOCKET_HAS_USER_SEMAPHORE

/*
 * ipconfigSOCKET_HAS_USER_WAKE_CALLBACK
 *
 * It is possible to install an application hook that will be called after every essential socket event.
 * The hook has one parameter: the socket, and it has no return value:
 * typedef void (* SocketWakeupCallback_t)( Socket_t pxSocket );
 *
 * The reason for calling the hook can be one or more of these events:
 *
 * eSOCKET_RECEIVE = 0x0001, // Reception of new data.
 * eSOCKET_SEND    = 0x0002, // Some data has been sent.
 * eSOCKET_ACCEPT  = 0x0004, // A new TCP client was detected, please call accept().
 * eSOCKET_CONNECT = 0x0008, // A TCP connect has succeeded or timed-out.
 * eSOCKET_BOUND   = 0x0010, // A socket got bound.
 * eSOCKET_CLOSED  = 0x0020, // A TCP connection got closed.
 * eSOCKET_INTR    = 0x0040, // A blocking API call got interrupted, because
 *                           // the function FreeRTOS\_SignalSocket() was called.
 *
 * Normally the hook will only notify the task that owns the socket so that the socket gets
 * immediate attention.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSOCKET_HAS_USER_WAKE_CALLBACK

/*
 * ipconfigSUPPORT_SELECT_FUNCTION
 *
 * Set ipconfigSUPPORT_SELECT_FUNCTION to 1 to include support for the FreeRTOS_select() and associated
 * API functions, or 0 to exclude FreeRTOS_select() and associated API functions from the build.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSUPPORT_SELECT_FUNCTION

/*
 * ipconfigSUPPORT_SIGNALS
 *
 * If ipconfigSUPPORT_SIGNALS is set to 1 then the FreeRTOS_SignalSocket() API function is included
 * in the build. FreeRTOS_SignalSocket() can be used to send a signal to a socket, so that any task
 * blocked on a read from the socket will leave the Blocked state (abort the blocking read operation).
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSUPPORT_SIGNALS

/*
 * ipconfigUSE_CALLBACKS
 *
 * When this macro is defined as non-zero, it is possible to bind specific application hooks (callbacks)
 * to a socket. There is a different application hook for every type of event:
 *
 * FREERTOS_SO_TCP_CONN_HANDLER // Callback for (dis) connection events.
 *                              // Supply pointer to 'F_TCP_UDP_Handler_t'
 *
 * FREERTOS_SO_TCP_RECV_HANDLER // Callback for receiving TCP data.
 *                              // Supply pointer to 'F_TCP_UDP_Handler_t'
 *
 * FREERTOS_SO_TCP_SENT_HANDLER // Callback for sending TCP data.
 *                              // Supply pointer to 'F_TCP_UDP_Handler_t'
 *
 * FREERTOS_SO_UDP_RECV_HANDLER // Callback for receiving UDP data.
 *                              // Supply pointer to 'F_TCP_UDP_Handler_t'
 *
 * FREERTOS_SO_UDP_SENT_HANDLER // Callback for sending UDP data.
 *                              // Supply pointer to 'F_TCP_UDP_Handler_t'
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigUSE_CALLBACKS

/*========== Constants Effecting ARP Behaviour =======================================================================*/

/*
 * ipconfigARP_CACHE_ENTRIES
 *
 * The ARP cache is a table that maps IP addresses to MAC addresses.
 *
 * The IP stack can only send a UDP message to a remove IP address if it knows the MAC address associated with the
 * IP address, or the MAC address of the router used to contact the remote IP address. When a UDP message is received
 * from a remote IP address, the MAC address and IP address are added to the ARP cache. When a UDP message is sent
 * to a remote IP address that does not already appear in the ARP cache, then the UDP message is replaced by a ARP
 * message that solicits the required MAC address information.
 *
 * ipconfigARP_CACHE_ENTRIES defines the maximum number of entries that can exist in the ARP table at any one time.
 *
 * default: 10
 */
//#define ipconfigARP_CACHE_ENTRIES

/*
 * ipconfigARP_STORES_REMOTE_ADDRESSES
 *
 * Advanced users only.
 *
 * ipconfigARP_STORES_REMOTE_ADDRESSES is provided for the case when a message that requires a reply arrives from
 * the Internet, but from a computer attached to a LAN rather than via the defined gateway. Before replying to the
 * message, the TCP/IP stack RTOS task will loop up the message's IP address in the ARP table - but if
 * ipconfigARP_STORES_REMOTE_ADDRESSES is set to 0, then ARP will return the MAC address of the defined gateway,
 * because the destination address is outside of the netmask.
 * That might prevent the reply reaching its intended destination.
 *
 * If ipconfigARP_STORES_REMOTE_ADDRESSES is set to 1, then remote addresses will also be stored in the ARP table,
 * along with the MAC address from which the message was received.
 * This can allow the message in the scenario above to be routed and delivered correctly.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigARP_STORES_REMOTE_ADDRESSES

/*
 * ipconfigARP_USE_CLASH_DETECTION
 *
 * When a link-layer address is assigned, the driver will test if it is already taken by a different device by
 * sending ARP requests. Therefore, ipconfigARP_USE_CLASH_DETECTION must be defined as non-zero.
 *
 * default: ( ipconfigIS_ENABLED( ipconfigUSE_DHCP ) && ipconfigIS_ENABLED( ipconfigDHCP_FALL_BACK_AUTO_IP )
 */
//#define ipconfigARP_USE_CLASH_DETECTION

/*
 * ipconfigMAX_ARP_AGE
 *
 * ipconfigMAX_ARP_AGE defines the maximum time between an entry in the ARP table being created or refreshed and the
 * entry being removed because it is stale. New ARP requests are sent for ARP cache entries that are nearing their
 * maximum age.
 *
 * ipconfigMAX_ARP_AGE is specified in tens of seconds, so a value of 150 is equal to 1500 seconds (or 25 minutes).
 *
 * default: 150
 */
//#define ipconfigMAX_ARP_AGE


/*
 * ipconfigMAX_ARP_RETRANSMISSIONS
 *
 * ARP requests that do not result in an ARP response will be re-transmitted a maximum of
 * ipconfigMAX_ARP_RETRANSMISSIONS times before the ARP request is aborted.
 *
 * default: 5
 */
//#define ipconfigMAX_ARP_RETRANSMISSIONS

/*
 * ipconfigUSE_ARP_REMOVE_ENTRY
 *
 * Advanced users only.
 *
 * If ipconfigUSE_ARP_REMOVE_ENTRY is set to 1 then ulARPRemoveCacheEntryByMac() is included in the build.
 * ulARPRemoveCacheEntryByMac() uses a MAC address to look up, and then remove, an entry from the ARP cache.
 * If the MAC address is found in the ARP cache, then the IP address associated with the MAC address is returned.
 * If the MAC address is not found in the ARP cache, then 0 is returned.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigUSE_ARP_REMOVE_ENTRY

/*
 * ipconfigUSE_ARP_REVERSED_LOOKUP
 *
 * Advanced users only.
 *
 * Normally ARP will look up an IP address from a MAC address. If ipconfigUSE_ARP_REVERSED_LOOKUP
 * is set to 1 then a function that does the reverse is also available. eARPGetCacheEntryByMac() looks up
 * a MAC address from an IP address.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigUSE_ARP_REVERSED_LOOKUP

/*========== Constants Effecting DHCP and Name Service Behaviour =====================================================*/

/*
 * ipconfigDHCP_FALL_BACK_AUTO_IP
 *
 * Only applicable when DHCP is in use. If no DHCP server responds, use "Auto-IP";
 * the device will allocate a random LinkLayer IP address, and test if it is still available.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigDHCP_FALL_BACK_AUTO_IP ipconfigENABLE

/*
 * ipconfigDHCP_REGISTER_HOSTNAME
 *
 * Often DHCP servers can show the names of devices that have leased IP addresses. When ipconfigDHCP_REGISTER_HOSTNAME
 * is set to 1, the device running FreeRTOS-Plus-TCP can identify itself to a DHCP server with a human readable name by
 * returning the name from an application provided hook (or 'callback') function called pcApplicationHostnameHook().
 *
 * When ipconfigDHCP_REGISTER_HOSTNAME is set to 1 the application must provide a hook (callback) function with the
 * following name and prototype:
 *
 * const char *pcApplicationHostnameHook( void );
 *
 * default: ipconfigDISABLE
 */
#define ipconfigDHCP_REGISTER_HOSTNAME 1


/*
 * ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY
 *
 * When looking up a URL, multiple answers (IP-addresses) may be received.
 * This macro determines how many answers will be stored per URL.
 *
 * default: 1
 */
//#define ipconfigDNS_CACHE_ADDRESSES_PER_ENTRY

/*
 * ipconfigDNS_CACHE_ENTRIES
 *
 * If ipconfigUSE_DNS_CACHE is set to 1 then ipconfigDNS_CACHE_ENTRIES
 * defines the number of entries in the DNS cache.
 *
 * default: 1
 */
#define ipconfigDNS_CACHE_ENTRIES 5

/*
 * ipconfigDNS_CACHE_NAME_LENGTH
 *
 * The maximum number of characters a DNS host name can take, including the NULL terminator.
 *
 * default: 254U
 */
//#define ipconfigDNS_CACHE_NAME_LENGTH

/*
 * ipconfigDNS_REQUEST_ATTEMPTS
 *
 * When looking up a host, the library has to send a DNS request and wait for a result. This process
 * will be repeated at most ipconfigDNS_REQUEST_ATTEMPTS times. The macro ipconfigDNS_SEND_BLOCK_TIME_TICKS
 * determines how long the function FreeRTOS_sendto() may block.
 *
 * When sending, by default, the function will block for at most 500 milliseconds. When waiting for a reply,
 * FreeRTOS_recvfrom() will wait for at most 5000 milliseconds.
 *
 * default: 5
 */
//#define ipconfigDNS_REQUEST_ATTEMPTS

/*
 * ipconfigDNS_USE_CALLBACKS
 *
 * When defined, the function FreeRTOS_gethostbyname_a() becomes available. This function will start a
 * DNS-lookup and set an application 'hook'. This user function (or 'hook') will be called when either
 * the URL has been found, or when a time-out has been reached. Note that the function
 * FreeRTOS_gethostbyname_a() will not make use of the macros ipconfigDNS_SEND_BLOCK_TIME_TICKS and
 * ipconfigDNS_RECEIVE_BLOCK_TIME_TICKS.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigDNS_USE_CALLBACKS

/*
 * ipconfigMAXIMUM_DISCOVER_TX_PERIOD
 *
 * When ipconfigUSE_DHCP is set to 1, DHCP requests will be sent out at increasing time intervals
 * until either a reply is received from a DHCP server and accepted, or the interval between
 * transmissions reaches ipconfigMAXIMUM_DISCOVER_TX_PERIOD. The TCP/IP stack will revert to using
 * the static IP address passed as a parameter to FreeRTOS_IPInit() if the re-transmission time
 * interval reaches ipconfigMAXIMUM_DISCOVER_TX_PERIOD without a DHCP reply being received.
 *
 * default: pdMS_TO_TICKS( 30000 )
 */
//#define ipconfigMAXIMUM_DISCOVER_TX_PERIOD

/*
 * ipconfigUSE_DHCP
 *
 * If ipconfigUSE_DHCP is 1 then FreeRTOS-Plus-TCP will attempt to retrieve an IP address,
 * netmask, DNS server address and gateway address from a DHCP server - and revert to using
 * the defined static address if an IP address cannot be obtained.
 *
 * If ipconfigUSE_DHCP is 0 then FreeRTOS-Plus-TCP will not attempt to obtain its address
 * information from a DHCP server. Instead, it will immediately use the defined static address
 * information.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_DHCP ipconfigENABLE

/*
 * ipconfigUSE_DHCPv6
 *
 * If ipconfigUSE_DHCPv6 is 1 then FreeRTOS-Plus-TCP will attempt to retrieve an IPv6 address,
 * netmask, DNS server address and gateway address from a DHCPv6 server - and revert to using
 * the defined static address if an IPv6 address cannot be obtained when an end-point is set
 * to enable DHCPv6 flow.
 *
 * If ipconfigUSE_DHCPv6 is 0 then FreeRTOS-Plus-TCP will not attempt to obtain its IPv6 address
 * information from a DHCPv6 server. Instead, it will immediately use the defined static address
 * information.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_DHCPv6 ipconfigDISABLE

/*
 * ipconfigUSE_DHCP_HOOK
 *
 * A normal DHCP transaction involves the following sequence:
 *   1. The client sends a DHCP discovery packet to request an IP address from the DHCP server.
 *   2. The DHCP server responds with an offer packet that contains the offered IP address.
 *   3. The client sends a DHCP request packet in order to claim the offered IP address
 *   4. The DHCP server sends an acknowledgement packet to grant the client use of the offered IP address,
 *      and to send additional configuration information to the client. Additional configuration information
 *      typically includes the IP address of the gateway, the IP address of the DNS server, and the
 *      IP address lease length.
 *
 * If ipconfigUSE_DHCP_HOOK is set to 1 then FreeRTOS-Plus-TCP will call an application provided hook
 * (or 'callback') function called xApplicationDHCPUserHook() both before the initial discovery packet
 * is sent, and after a DHCP offer has been received - the hook function can be used to terminate the
 * DHCP process at either one of these two phases in the DHCP sequence. For example, the application
 * writer can effectively disable DHCP, even when ipconfigUSE_DHCP is set to 1, by terminating the DHCP
 * process before the initial discovery packet is sent. As another example, the application writer can
 * check a static IP address is compatible with the network to which the device is connected by receiving
 * an IP address offer from a DHCP server, but then terminating the DHCP process without sending a request
 * packet to claim the offered IP address.
 *
 * If ipconfigUSE_DHCP_HOOK is set to 1, then the application writer must provide a hook (callback)
 * function with the following name and prototype:
 *
 * eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress );
 *
 * Where eDHCPCallbackQuestion_t and eDHCPCallbackAnswer_t are defined as follows
 * typedef enum eDHCP_QUESTIONS
 * {
 *     // About to send discover packet.
 *     eDHCPPhasePreDiscover,
 *     // About to send a request packet.
 *     eDHCPPhasePreRequest,
 * } eDHCPCallbackQuestion_t;
 *
 * typedef enum eDHCP_ANSWERS
 * {
 *     // Continue the DHCP process as normal.
 *     eDHCPContinue,
 *     // Stop the DHCP process, and use the static defaults.
 *     eDHCPUseDefaults,
 *     // Stop the DHCP process, and continue with current settings.
 *     eDHCPStopNoChanges,
 * } eDHCPCallbackAnswer_t;
 *
 * When the eDHCPPhase parameter is set to eDHCPPhasePreDiscover, the ulIPAddress parameter is set
 * to the IP address already in use. When the eDHCPPhase parameter is set to eDHCPPhasePreRequest,
 * the ulIPAddress parameter is set to the IP address offered by the DHCP server.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_DHCP_HOOK ipconfigDISABLE

/*
 * ipconfigUSE_DNS
 *
 * Set ipconfigUSE_DNS to 1 to include a basic DNS client/resolver.
 * DNS is used through the FreeRTOS_gethostbyname() API function.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_DNS ipconfigENABLE

/*
 * ipconfigUSE_DNS_CACHE
 *
 * If ipconfigUSE_DNS_CACHE is set to 1, then the DNS cache will be enabled.
 * If ipconfigUSE_DNS_CACHE is set to 0, then the DNS cache will be disabled.
 * Note that if DNS cache is enabled (ipconfigUSE_DNS_CACHE), then the maximum length of
 * hostnames that can be resolved is capped by ipconfigDNS_CACHE_NAME_LENGTH since all DNS
 * queries are cached.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_DNS_CACHE ipconfigENABLE

/*
 * ipconfigUSE_LLMNR
 *
 * Set ipconfigUSE_LLMNR to 1 to include LLMNR (Link-Local Multicast Name Resolution).
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_LLMNR ipconfigDISABLE

/*
 * ipconfigUSE_NBNS
 *
 * Set ipconfigUSE_NBNS to 1 to include NBNS (NetBIOS Name Service).
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_NBNS ipconfigDISABLE

/*
 * ipconfigUSE_MDNS
 *
 * Set ipconfigUSE_MDNS to 1 to include Multicast DNS.
 *
 * default: ipconfigDISABLE
 */
#define ipconfigUSE_MDNS ipconfigDISABLE

/*========== Constants Effecting IP and ICMP Behaviour ===============================================================*/

/*
 * ipconfigUSE_IPv4
 *
 * This macro is about IPv4. The FreeRTOS-Plus-TCP stack supports handling IPv4 packets
 * (including handling IPv4 header, ARP, DHCP, and so on) when it’s set to 1.
 * Otherwise, the stack will drop all IPv4 packets on the RX side and be unable to transmit
 * any IPv4 packets.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_IPv4 ipconfigENABLE

/*
 * ipconfigUSE_IPv6
 *
 * This macro is about IPv6. The FreeRTOS-Plus-TCP stack supports handling IPv6 packets
 * (including handling IPv6 header, ND, RA, and so on) when it’s set to 1.
 * Otherwise, the stack will drop all IPv6 packets on the RX side and be unable to transmit
 * any IPv6 packets.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_IPv6 ipconfigDISABLE

/*
 * ipconfigFORCE_IP_DONT_FRAGMENT
 *
 * This macro is about IP-fragmentation. When sending an IP-packet over the Internet, a big packet
 * may be split up into smaller parts which are then combined by the receiver. The sender can
 * determine if this fragmentation is allowed or not. ipconfigFORCE_IP_DONT_FRAGMENT is zero by
 * default, which means that fragmentation is allowed.
 *
 * Note that the FreeRTOS-Plus-TCP stack does not accept received fragmented packets.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigFORCE_IP_DONT_FRAGMENT

/*
 * ipconfigICMP_TIME_TO_LIVE
 *
 * When replying to an ICMP packet, the TTL field will be set to the value of this macro.
 * The default value is 64 (as recommended by RFC 1700).
 * The minimum value is 1, the maximum value is 255.
 *
 * default: 64
 */
//#define ipconfigICMP_TIME_TO_LIVE

/*
 * ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS
 *
 * If ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS is set to 1, then FreeRTOS-Plus-TCP accepts IP packets
 * that contain IP options, but does not process the options (IP options are not supported).
 *
 * If ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS is set to 0, then FreeRTOS-Plus-TCP will drop IP packets
 * that contain IP options.
 *
 * default: ipconfigENABLE
 */
//#define ipconfigIP_PASS_PACKETS_WITH_IP_OPTIONS

/*
 * ipconfigREPLY_TO_INCOMING_PINGS
 *
 * If ipconfigREPLY_TO_INCOMING_PINGS is set to 1, then the TCP/IP stack will generate replies
 * to incoming ICMP echo (ping) requests.
 *
 * default: ipconfigENABLE
 */
#define ipconfigREPLY_TO_INCOMING_PINGS ipconfigENABLE

/*
 * ipconfigSUPPORT_OUTGOING_PINGS
 *
 * If ipconfigSUPPORT_OUTGOING_PINGS is set to 1 then the FreeRTOS_SendPingRequest()
 * API function is available.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigSUPPORT_OUTGOING_PINGS

/*========== Constants Effecting ND Behaviour ========================================================================*/

/*
 * ipconfigND_CACHE_ENTRIES
 *
 * The ND cache is a table that maps IP addresses to MAC addresses. The IP stack can only send a TCP/UDP message
 * to a remote IPv6 address if it knows the MAC address associated with the IPv6 address, or the MAC address of
 * the router used to contact the remote IPv6 address. When a message is received from a remote IPv6 address,
 * the MAC address and IPv6 address are added to the ND cache. When a TCP/UDP message is sent to a remote IPv6
 * address that does not already appear in the ND cache, then the TCP/UDP message is replaced by a
 * Neighbor Solicitation message that solicits the required MAC address information.
 *
 * ipconfigND_CACHE_ENTRIES defines the maximum number of entries that can exist in the ND table at any one time.
 *
 * default: 24
 */
//#define ipconfigND_CACHE_ENTRIES

/*========== Constants Effecting RA Behaviour ========================================================================*/

/*
 * ipconfigUSE_RA
 *
 * If ipconfigUSE_RA is 1 then FreeRTOS-Plus-TCP will attempt to retrieve an IPv6 address, prefix address,
 * and gateway address from a IPv6 router by SLAAC flow - and revert to using the defined static address
 * if an IPv6 address cannot be obtained when an end-point is set to enable RA flow.
 *
 * If ipconfigUSE_RA is 0 then FreeRTOS-Plus-TCP will not attempt to obtain its address information from
 * a DHCP server. Instead, it will immediately use the defined static address information.
 *
 * default: ipconfigENABLE
 */
#define ipconfigUSE_RA ipconfigDISABLE

/*
 * ipconfigRA_SEARCH_COUNT and ipconfigRA_IP_TEST_COUNT
 *
 * RA or Router Advertisement/SLAAC: see end-point flag 'bWantRA'. A Router Solicitation will be sent.
 * It will wait for ipconfigRA_SEARCH_TIME_OUT_MSEC ms. When there is no response, it will be repeated
 * ipconfigRA_SEARCH_COUNT times. Then it will check if the chosen IP-address already exists, and repeat
 * this ipconfigRA_IP_TEST_COUNT times, each time with a timeout of ipconfigRA_IP_TEST_TIME_OUT_MSEC ms.
 * Finally, the end-point will enter the UP state.
 *
 * ipconfigRA_SEARCH_COUNT default: 3
 * ipconfigRA_IP_TEST_COUNT default: 3
 */
//#define ipconfigRA_SEARCH_COUNT
//#define ipconfigRA_IP_TEST_COUNT

/*========== Constants Providing Target Support ======================================================================*/

/*
 * ipconfigIS_VALID_PROG_ADDRESS
 *
 * In cases where installable application hooks are used, this macro is called to check if a given address
 * refers to valid (instruction) memory.
 *
 * default: ipconfigIS_VALID_PROG_ADDRESS( pxAddress ) ( ( pxAddress ) != NULL )
 */
//#define ipconfigIS_VALID_PROG_ADDRESS

/*
 * ipconfigPORT_SUPPRESS_WARNING
 *
 * For some use cases, users set the configurations that issue warning messages.
 * This configuration is used to suppress warnings in portable layers to make compilation clean.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigPORT_SUPPRESS_WARNING

/*========== Backward Compatibility ==================================================================================*/

/*
 * ipconfigCOMPATIBLE_WITH_SINGLE
 *
 * If ipconfigCOMPATIBLE_WITH_SINGLE is set to 1, then FreeRTOS-Plus-TCP assumes there are no multiple
 * end-points/interfaces in the program. Some routing functions can be simplified to return the first
 * end-point/interface directly.
 *
 * If ipconfigCOMPATIBLE_WITH_SINGLE is set to 0, which is the default value, then FreeRTOS-Plus-TCP
 * assumes multiple end-points/interfaces are allowed in the program.
 *
 * default: ipconfigDISABLE
 */
//#define ipconfigCOMPATIBLE_WITH_SINGLE

/*
 * ipconfigIPv4_BACKWARD_COMPATIBLE
 *
 * If ipconfigIPv4_BACKWARD_COMPATIBLE is set to 1, then FreeRTOS-Plus-TCP supports the original feature
 * before V4.0.0, and the stack is not able to support IPv6. All functions prototypes are reset to original ones.
 *
 * If ipconfigIPv4_BACKWARD_COMPATIBLE is set to 0, which is the default value, then FreeRTOS-Plus-TCP
 * applies all new features introduced by V4.0.0.
 *
 * default: ipconfigENABLE
 */
#define ipconfigIPv4_BACKWARD_COMPATIBLE ipconfigDISABLE

#include "FreeRTOSIPTraceMacroCustom.h"

#endif /* FREERTOS_IP_CONFIG_H */

