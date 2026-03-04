/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include <string.h>
#include "bacnet/bacdcode.h"
#include "bacnet/bacint.h"
#include "bacnet/datalink/bip.h"
#include "bacnet/datalink/bvlc.h"
#include "bacnet/basic/services.h"  // replaces handlers.h
#include <bacnet/basic/bbmd/h_bbmd.h>

/* TCP/IP stack */
#include "FreeRTOS_IP.h"

#if PRINT_ENABLED
#include <stdio.h>      /* for standard i/o, like printing */
#endif

/** @file bip.c  Configuration and Operations for BACnet/IP */
static bool BIP_Debug = false;
static int BIP_Socket = -1;
Socket_t pxBIP_socket;

/* port to use - stored in network byte order */
static uint16_t BIP_Port = 0;   /* this will force initialization in demos */

/* IP Address */
static BACNET_IP_ADDRESS BIP_Address;

/* Broadcast Address */
static BACNET_IP_ADDRESS BIP_Broadcast_Address;

/** Getter for the BACnet/IP socket handle.
 *
 * @return The handle to the BACnet/IP socket.
 */
int bip_get_socket(void)
{
    return BIP_Socket;
}

bool bip_valid(void)
{
    return (BIP_Socket != -1);
}

bool bip_set_addr(const BACNET_IP_ADDRESS *addr)
{
    memcpy(&BIP_Address, addr, sizeof(BACNET_IP_ADDRESS));
    return true;
}

bool bip_get_addr(BACNET_IP_ADDRESS *addr)
{
    memcpy(addr, &BIP_Address, sizeof(BACNET_IP_ADDRESS));
    return true;
}

bool bip_set_broadcast_addr(const BACNET_IP_ADDRESS *addr)
{
    memcpy(&BIP_Broadcast_Address, addr, sizeof(BACNET_IP_ADDRESS));
    return true;
}

bool bip_get_broadcast_addr(BACNET_IP_ADDRESS *addr)
{
    memcpy(addr, &BIP_Broadcast_Address, sizeof(BACNET_IP_ADDRESS));
    return true;
}

/**
 * @brief Get the IPv4 address for my interface. Used for sending src address.
 * @param addr - BACnet datalink address
 */
void bip_get_my_address(BACNET_ADDRESS *addr)
{
    unsigned int i = 0;

    if (addr) {
        addr->mac_len = 6;
        memcpy(&addr->mac[0], BIP_Address.address, 4);
        memcpy(&addr->mac[4], &BIP_Port, 2);
        /* local only, no routing */
        addr->net = 0;
        /* no SLEN */
        addr->len = 0;
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            addr->adr[i] = 0;
        }
    }
}

/**
 * Get the IPv4 broadcast address for my interface.
 *
 * @param addr - BACnet datalink address
 */
void bip_get_broadcast_address(BACNET_ADDRESS *dest)
{
    int i = 0; /* counter */

    if (dest) {
        dest->mac_len = 6;
        memcpy(&dest->mac[0], BIP_Broadcast_Address.address, 4);
        memcpy(&dest->mac[4], &BIP_Port, 2);
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0; /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            dest->adr[i] = 0;
        }
    }
    return;
}

/**
 * @brief Get the BACnet/IP subnet mask CIDR prefix
 * @return subnet mask CIDR prefix 1..32
 */
uint8_t bip_get_subnet_prefix(void)
{
    uint32_t address = 0;
    uint32_t broadcast = 0;
    uint32_t mask = 0xFFFFFFFE;
    uint8_t prefix = 0;

    memcpy(&address, BIP_Address.address, 4);
    memcpy(&address, BIP_Broadcast_Address.address, 4);
    /* calculate the subnet prefix from the broadcast address */
    for (prefix = 1; prefix <= 32; prefix++) {
        if ((address | mask) == broadcast) {
            break;
        }
        mask = mask << 1;
    }

    return prefix;
}


void bip_set_port(uint16_t port)
{   /* in network byte order */
    BIP_Port = port;
}

/* returns network byte order */
uint16_t bip_get_port(void)
{
    return BIP_Port;
}

/** Send the Original Broadcast or Unicast messages
 *
 * @param dest [in] Destination address (may encode an IP address and port #).
 * @param npdu_data [in] The NPDU header (Network) information (not used).
 * @param pdu [in] Buffer of data to be sent - may be null (why?).
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 *
 * @return number of bytes sent
 */
int bip_send_pdu(
    BACNET_ADDRESS *dest, /* destination address */
    BACNET_NPDU_DATA *npdu_data, /* network information */
    uint8_t *pdu, /* any data to be sent - may be null */
    unsigned pdu_len)
{
    return bvlc_send_pdu(dest, npdu_data, pdu, pdu_len);
}

/**
 * The send function for BACnet/IP driver layer
 *
 * @param dest - Points to a BACNET_IP_ADDRESS structure containing the
 *  destination address.
 * @param mtu - the bytes of data to send
 * @param mtu_len - the number of bytes of data to send
 *
 * @return Upon successful completion, returns the number of bytes sent.
 *  Otherwise, -1 shall be returned and errno set to indicate the error.
 */
int bip_send_mpdu(const BACNET_IP_ADDRESS *dest, const uint8_t *mtu, uint16_t mtu_len)
{
    struct freertos_sockaddr bip_dest = { 0 };

    /* assumes that the driver has already been initialized */
    if (BIP_Socket < 0) {
        if (BIP_Debug) {
            fprintf(stderr, "BIP: driver not initialized!\n");
            fflush(stderr);
        }
        return BIP_Socket;
    }

    /* load destination IP address */
    bip_dest.sin_family = FREERTOS_AF_INET4;
    memcpy(&bip_dest.sin_address, &dest->address[0], 4);
    bip_dest.sin_port = FreeRTOS_htons(BIP_Port);

    /* Send the packet */
    int32_t bytes = FreeRTOS_sendto(pxBIP_socket,
                           (const char *)mtu,
                           mtu_len,
                           0,
                           &bip_dest,
                           sizeof(struct freertos_sockaddr));

    return bytes;
}

/**
 * BACnet/IP Datalink Receive handler.
 *
 * @param src - returns the source address
 * @param npdu - returns the NPDU buffer
 * @param max_npdu -maximum size of the NPDU buffer
 * @param timeout - number of milliseconds to wait for a packet
 *
 * @return Number of bytes received, or 0 if none or timeout.
 */
uint16_t bip_receive(BACNET_ADDRESS *src, uint8_t *npdu, uint16_t max_npdu, unsigned timeout)
{
    uint16_t npdu_len = 0; /* return value */
    int max = 0;
    TickType_t timeout_ticks;
    BACNET_IP_ADDRESS addr = { 0 };
    int received_bytes = 0;
    uint16_t offset = 0;
    uint16_t i = 0;
    struct freertos_sockaddr xClient;
    uint32_t xClientLength = sizeof( struct freertos_sockaddr );

    /* Make sure the socket is open */
    if (BIP_Socket < 0) {
        return 0;
    }

    /* Set a time out */
    timeout_ticks = pdMS_TO_TICKS(timeout);
    FreeRTOS_setsockopt( pxBIP_socket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &timeout_ticks,
                         sizeof( timeout_ticks ));

    /* see if there is a packet for us */
    received_bytes = FreeRTOS_recvfrom(pxBIP_socket,
                                       npdu,
                                       max_npdu,
                                       0,
                                       &xClient,
                                       &xClientLength);

    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }
    /* no problem, just no bytes */
    if (received_bytes == 0) {
        return 0;
    }
    /* the signature of a BACnet/IPv packet */
    if (npdu[0] != BVLL_TYPE_BACNET_IP) {
        return 0;
    }

    /* Erase up to 16 bytes after the received bytes as safety margin to
     * ensure that the decoding functions will run into a 'safe field'
     * of zero, if for any reason they would overrun, when parsing the
     * message. */
    max = (int)max_npdu - received_bytes;
    if (max > 0) {
        if (max > 16) {
            max = 16;
        }
        memset(&npdu[received_bytes], 0, (size_t)max);
    }
    /* Data link layer addressing between B/IPv4 nodes consists of a 32-bit
       IPv4 address followed by a two-octet UDP port number (both of which
       shall be transmitted with the most significant octet first). This
       address shall be referred to as a B/IPv4 address.
    */
    memcpy(&addr.address[0], &xClient.sin_address , 4);
    addr.port = FreeRTOS_ntohs(xClient.sin_port);

    /* pass the packet into the BBMD handler */
    offset = (uint16_t)bvlc_handler(&addr, src, npdu, (uint16_t)received_bytes);

    if (offset > 0)
    {
        npdu_len = (uint16_t)(received_bytes - offset);
        if (npdu_len <= max_npdu)
        {
            /* shift the buffer to return a valid NPDU */
            for (i = 0; i < npdu_len; i++)
            {
                npdu[i] = npdu[offset + i];
            }
        }
        else
            npdu_len = 0;
    }
    return npdu_len;
}

/** Initialize the BACnet/IP services at the given interface.
 * @ingroup DLBIP
 * -# Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 * -# Opens a UDP socket
 * -# Configures the socket for sending and receiving
 * -# Configures the socket so it can send broadcasts
 * -# Binds the socket to the local IP address at the specified port for
 *    BACnet/IP (by default, 0xBAC0 = 47808).
 *
 * @note For Linux, ifname is eth0, ath0, arc0, and others.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the default interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(char *ifname)
{
    (void)ifname;
    struct freertos_sockaddr bip_source = { 0 };

    if(BIP_Address.address == 0)
        return false;

    bip_source.sin_family = FREERTOS_AF_INET4;
    bip_source.sin_port = FreeRTOS_htons(BIP_Port);
    memcpy(&bip_source.sin_address.ulIP_IPv4, BIP_Address.address, 4);
    BIP_Socket = 1;

    pxBIP_socket = FreeRTOS_socket(FREERTOS_AF_INET4,
                                   FREERTOS_SOCK_DGRAM,
                                   FREERTOS_IPPROTO_UDP);

    if(pxBIP_socket == FREERTOS_INVALID_SOCKET)
        return false;

    if(FreeRTOS_bind(pxBIP_socket, &bip_source, sizeof(struct freertos_sockaddr)) != 0)
        return false;

    bvlc_init();
    return true;
}
