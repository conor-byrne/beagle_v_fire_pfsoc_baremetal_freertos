/*******************************************************************************
 * (c) Copyright 2013 Microsemi SoC Producst Group.  All rights reserved.
 *
 *
 * SVN $Revision: 4971 $
 * SVN $Date: 2013-01-11 22:14:31 +0000 (Fri, 11 Jan 2013) $
 */

#include "httpserver-netconn.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOS_IP.h"

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_rtc/mss_rtc.h"

#include <assert.h>

#include "drivers/mss/mss_ethernet_mac/mss_ethernet_registers.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac_sw_cfg.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac_regs.h"
#include "drivers/mss/mss_ethernet_mac/mss_ethernet_mac.h"

/*------------------------------------------------------------------------------
 * Note. Size here needs to track size in mscc_logo.c
 */
extern const char mscc_png_logo[1740];

static const char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n";

static const char http_json_hdr[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n";

static const char http_png_hdr[] = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n";

static const char http_post_resp_hdr[] = "HTTP/1.1 204 No Content\r\n\r\n";
static const char http_html_ok_hdr[] = "HTTP/1.1 200 OK\r\n\r\n";

#define DEMO_PLATFORM "MPFS BeagleV-Fire Board GEM 0<br>FreeRTOS + FreeRTOS-Plus-TCP U54-1"


static const char http_index_html[] = "<!DOCTYPE html>\
<html dir=\"ltr\"><head>\
    <meta content=\"text/html; charset=UTF-8\">\
    <title>Microchip MPFS Demo</title>\
\
    <style type=\"text/css\">\
        .bodyText {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 12px;\
            color: #333333;\
        }\
        .headline2 {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 12px;\
            font-weight: bold;\
            color: #333333;\
        }\
        .headline1 {\
            font-family: \"Trebuchet MS\";\
            font-size: 18px;\
            font-weight: bold;\
            color: #666666;\
        }\
        .smallText {\
            font-family: Verdana, Tahoma, Arial, Helvetica, sans-serif;\
            font-size: 9px;\
            color: #666666;\
        }\
        .top_headline {\
            font-family: Arial, Helvetica, sans-serif;\
            font-size: 24px;\
            color: #666666;\
            padding-top: 5px;\
        }\
        </style>\
  </head>\
<body>\
<table border=\"0\" width=\"800\" align=\"center\">\
  <tbody>\
    <tr>\
        <td width=\"50%\"><img name=\"index_r1_c1\" src=\"./index_r1_c1.png\" width=\"300\" height=\"89\" border=\"0\" alt=\"\"></td>\
        <td width=\"50%\"><p class=\"top_headline\">Simple Web Server Application<br>" DEMO_PLATFORM "</p></td>\
    </tr>\
  </tbody>\
</table>\
<hr width=\"800\" />\
<table border=\"0\" width=\"800\" align=\"center\">\
      <tbody>\
        <tr>\
          <td width=\"50%\" colspan=\"1\" rowspan=\"2\"><div class=\"headline1\" style=\"text-align: center;\"><span style=\"font-weight: bold;\">Network Interface</span><br></div>\
            <table border=\"0\" width=\"60%\" align=\"center\" cellspacing=\"10\">\
              <tbody class=\"bodyText\">\
                <tr>\
                  <td class=\"headline2\" width=\"50%\" style=\"text-align: right;\">MAC Address:<br></td>\
                  <td id=\"MAC_Addr\" width=\"50%\" style=\"text-align: center;\">-<br></td>\
                </tr>\
                <tr>\
                  <td class=\"headline2\" style=\"text-align: right;\">TCP/IP Address:<br></td>\
                  <td id=\"TCPIP_Addr\" style=\"text-align: center;\">-<br></td>\
                </tr>\
                <tr>\
                  <td class=\"headline2\" style=\"text-align: right;\">Speed:<br></td>\
                  <td id=\"LinkSpeed\" style=\"text-align: center;\">-<br></td>\
                </tr>\
                <tr>\
                  <td><br></td>\
                  <td id=\"DebugStatus\"><br></td>\
                </tr>\
              </tbody>\
            </table>\
          </td>\
          <td>\
            <div class=\"headline1\" style=\"text-align: center;\"><span style=\"font-weight: bold;\">Real Time Counter</span><br>\
            </div>\
            <table border=\"0\" width=\"30%\" align=\"center\" cellspacing=\"10\">\
              <tbody class=\"bodyText\">\
                <tr>\
                  <td width=\"40%\"><div class=\"headline2\" style=\"text-align: center;\">Request Count:<br></div></td>\
                  <td width=\"20%\"><div id=\"CurrentCount\" style=\"text-align: center;\">-<br></div></td>\
                </tr>\
                <tr>\
                  <td width=\"40%\"><div class=\"headline2\" style=\"text-align: center;\">Time:<br></div></td>\
                  <td width=\"20%\"><div id=\"CurrentRTCTime\" style=\"text-align: center;\">-<br></div></td>\
                </tr>\
                <tr>\
                  <td width=\"40%\"><div class=\"headline2\" style=\"text-align: center;\">Date:<br></div></td>\
                  <td width=\"20%\"><div id=\"Current_RTC_Date\" style=\"text-align: center;\">-<br></div></td>\
                </tr>\
              </tbody>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td>\
            <hr />\
            <div class=\"headline1\" width=\"50%\" style=\"text-align: center; font-weight: bold;\">Change RTC value<br></div>\
            <table border=\"0\" width=\"65%\" align=\"center\" cellspacing=\"10\">\
              <tbody class=\"bodyText\">\
                <tr>\
                  <td>\
                    <table border=\"0\" width=\"100%\" align=\"center\">\
                      <tbody class=\"bodyText\">\
                        <tr>\
                            <td  class=\"headline2\">Time (hh:mm:ss):</td>\
                            <td><input id=\"SetTime\" type=\"text\" name=\"time\" maxlength=\"8\" size=\"8\" value=\"00:00:00\"  style=\"text-align: center;\" /></td>\
                            <td><button onclick=\"post_new_time_request();\">Set</button></td>\
                        </tr>\
                      </tbody>\
                    </table>\
                    </td>\
                </tr>\
                <tr>\
                  <td>\
                    <table border=\"0\" width=\"100%\" align=\"center\">\
                      <tbody class=\"bodyText\">\
                        <tr>\
                            <td  class=\"headline2\">Date (dd/mm/yy):</td>\
                            <td><input id=\"SetDate\" type=\"text\" name=\"time\" maxlength=\"8\" size=\"8\" value=\"00:00:00\"  style=\"text-align: center;\" /></td>\
                            <td><button onclick=\"post_new_date_request();\">Set</button></td>\
                        </tr>\
                      </tbody>\
                    </table>\
                    </td>\
                </tr>\
                <tr>\
                  <td>\
                    </td>\
                </tr>\
              </tbody>\
            </table>\
          </td>\
        </tr>\
      </tbody>\
    \
</table>\
<hr width=\"800\" />\
<div class=\"smallText\" width=\"50%\" style=\"text-align: center; font-weight: bold;\">Microchip FPGA Business Unit - v0.2<br></div>\
<p><br>\
    </p>\
<script type=\"text/javascript\">\
function update_page() {\
    var request = new XMLHttpRequest();\
    request.open(\"GET\",\"status\");\
    request.onreadystatechange = function() {\
        if(request.readyState === 4 && request.status === 200) {\
            var parsed_status = JSON.parse(request.responseText);\
            var mac_addr = document.getElementById(\"MAC_Addr\");\
            mac_addr.innerHTML = parsed_status.MAC_Addr;\
            var tcpip_addr = document.getElementById(\"TCPIP_Addr\");\
            tcpip_addr.innerHTML = parsed_status.TCPIP_Addr;\
            var link_speed = document.getElementById(\"LinkSpeed\");\
            link_speed.innerHTML = parsed_status.LinkSpeed;\
            var current_count = document.getElementById(\"CurrentCount\");\
            current_count.innerHTML = parsed_status.CurrentCount;\
            var current_time = document.getElementById(\"CurrentRTCTime\");\
            current_time.innerHTML = parsed_status.CurrentRTCTime;\
            var current_date = document.getElementById(\"Current_RTC_Date\");\
            current_date.innerHTML = parsed_status.Current_RTC_Date;\
        };\
    };\
    request.send(null);\
};\
\
function encodeFormData(data) {\
    if (!data) return \"\";\
    var pairs = [];\
    for(var name in data) {\
        if (!data.hasOwnProperty(name)) continue;\
        if (typeof data[name] === \"function\") continue;\
        var value = data[name].toString();\
        name = encodeURIComponent(name.replace(\" \", \"+\"));\
        value = encodeURIComponent(value.replace(\" \", \"+\"));\
        pairs.push(name + \"=\" + value);\
    }\
    return pairs.join('&');\
}\
\
function post_new_time_request() {\
    var req_data = { secs:0, ns:0};\
    req_data.secs = document.getElementById(\"SetTime\").value;\
    var request = new XMLHttpRequest();\
    request.open(\"GET\",\"trigger0?\" + encodeFormData(req_data));\
    request.send(null);\
};\
\
function post_new_date_request() {\
    var req_data = { secs:0, ns:0};\
    req_data.secs = document.getElementById(\"SetDate\").value;\
    var request = new XMLHttpRequest();\
    request.open(\"GET\",\"trigger1?\" + encodeFormData(req_data));\
    request.send(null);\
};\
\
window.onload = function() {\
    let now = new Date();\
    document.getElementById(\"SetTime\").value = now.toLocaleTimeString('en-IE');\
    document.getElementById(\"SetDate\").value = now.toLocaleDateString('en-IE', {day: '2-digit', month: '2-digit', year: '2-digit'});\
    setInterval(update_page, 500);\
};\
</script>\
</body></html>";

static int sock_send(Socket_t pxSock, const char* pcBufferToTransmit, BaseType_t xTotalLengthToSend)
{
    BaseType_t xAlreadyTransmitted = 0, xBytesSent = 0;
    size_t xLenToSend;

    /* Keep sending until the entire buffer has been sent. */
    while( xAlreadyTransmitted < xTotalLengthToSend )
    {
        /* How many bytes are left to send? */
        xLenToSend = (size_t)(xTotalLengthToSend - xAlreadyTransmitted);
        xBytesSent = FreeRTOS_send( pxSock, &( pcBufferToTransmit[ xAlreadyTransmitted ] ), xLenToSend, 0 );

        if( xBytesSent >= 0 )
        {
            /* Data was sent successfully. */
            xAlreadyTransmitted += xBytesSent;
        }
        else
        {
            /* Error - break out of the loop for graceful socket close. */
        }
    }
    return (int)xAlreadyTransmitted;
}

static void handle_trigger_request(char * buf, uint16_t len)
{
    unsigned int trigger_id;
    unsigned int seconds = 1;
    int idx;
    int time[3] = {0, 0, 0};
    int time_inc = 0;
    trigger_id = (unsigned int)(buf[7] - '0');
    if(trigger_id < 3)
    {
        /* Parse request for the trigger's seconds value. */
        idx = 14;
        while((buf[idx] != '&') && (idx < len) && (time_inc < 3))
        {
            if('%' == buf[idx])
            {
                idx += 3;   /* skip %3A. */
                ++time_inc;
            }
            else
            {
                if((buf[idx] >= '0') && (buf[idx] <= '9'))
                {
                    time[time_inc] = (time[time_inc] * 10) + buf[idx] - '0';
                    ++idx;
                }
                else
                {   /* Invalid character found in request. */
                    seconds = 0;
                    idx = len;
                }
            }
        }

        if(seconds != 0)
        {
#if 0
            mss_rtc_calender_t new_calendar_time;

            MSS_RTC_get_calendar_count(&new_calendar_time);
            if(0 == trigger_id)
            {
                new_calendar_time.hour = (uint8_t)time[0];
                new_calendar_time.minute = (uint8_t)time[1];
                new_calendar_time.second = (uint8_t)time[2];
            }
            else if(1 == trigger_id)
            {
                if((time[0] > 0) && (time[0] <= 31))
                {
                    new_calendar_time.day = (uint8_t)time[0];
                }
                if((time[1] > 0) && (time[1] <= 12))
                {
                    new_calendar_time.month = (uint8_t)time[1];
                }
                if((time[2] > 0) && (time[2] <= 255))
                {
                    new_calendar_time.year = (uint8_t)time[2];
                }
            }
            MSS_RTC_set_calendar_count(&new_calendar_time);
#endif
        }
    }
}

/** Serve one HTTP connection accepted in the http thread */
extern mss_mac_instance_t g_mac0;
static void http_server_netconn_serve(Socket_t pxSock, uint32_t count)
{
    char cBuf[512];
    BaseType_t uBuflen;
    uint8_t mac_addr[6];
    static char status_json[400];
    static char header[256];
    uint16_t header_length;

    const char * mac_speed_lut[2][4] =
    {
        {
            "10M half duplex",
            "100M half duplex",
            "1000M half duplex",
            "Invalid"
        },
        {
            "10M full duplex",
            "100M full duplex",
            "1000M full duplex",
            "Invalid"
        }
    };

    /* Read the data from the port, blocking if nothing yet there. 
    We assume the request (the part we care about) is in one netbuf */
    uBuflen = FreeRTOS_recv( pxSock, &cBuf, 512, 0 );

    if(uBuflen > 0)
    {
        /* Is this an HTTP GET command? (only check the first 5 chars, since
        there are other formats for GET, and we're keeping it very simple )*/
        if(uBuflen >= 5 &&
           cBuf[0] == 'G' &&
           cBuf[1] == 'E' &&
           cBuf[2] == 'T' &&
           cBuf[3] == ' ' &&
           cBuf[4] == '/' )
        {
        
            if(cBuf[5]=='s')
            {
                BaseType_t json_resp_size;
                mss_mac_speed_t mac_spd = 3;
                uint8_t fullduplex = 0;
                uint32_t ip_addr = pxSock->pxEndPoint->ipv4_settings.ulIPAddress;
                memcpy(mac_addr, pxSock->pxEndPoint->xMACAddress.ucBytes, 6);
                MSS_MAC_get_link_status(&g_mac0, &mac_spd, &fullduplex);

                json_resp_size = snprintf(status_json, sizeof(status_json),
                                          "\r\n\r\n{\"MAC_Addr\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"TCPIP_Addr\": \"%d.%d.%d.%d\",\r\n\"LinkSpeed\": \"%s\",",
                                          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
                                          (int)(ip_addr & 0x000000FFu),
                                          (int)((ip_addr >> 8u) & 0x000000FFu),
                                          (int)((ip_addr >> 16u) & 0x000000FFu),
                                          (int)((ip_addr >> 24u) & 0x000000FFu),
                                          mac_speed_lut[fullduplex][mac_spd]);

#if 0
                mss_rtc_calender_t cal_time;
                MSS_RTC_get_calendar_count(&cal_time);
#endif
                json_resp_size += snprintf(&status_json[json_resp_size], sizeof(status_json),
                                           "\"CurrentRTCTime\": \"%02u:%02u:%02u\","
                                           "\"Current_RTC_Date\": \"%02u/%02u/%02u\","
                                           "\"CurrentCount\": \"%d\" }\r\n",
#if 0
                                           cal_time.hour, cal_time.minute, cal_time.second,
                                           cal_time.day, cal_time.month, cal_time.year,
#endif
                                           0,0,0,0,0,0,
                                           count);
                assert(json_resp_size < (BaseType_t)sizeof(status_json));
                if(json_resp_size > (BaseType_t)sizeof(status_json))
                {
                    json_resp_size = sizeof(status_json);
                }

                /* Send the HTML header */
                header_length = (uint16_t)snprintf(header, 256, "%sContent-Length: %u\r\n\r\n", http_json_hdr, (uint16_t)(json_resp_size-1));
                sock_send( pxSock, header, header_length );

                /* Send JSON */
                sock_send( pxSock, status_json, json_resp_size-1 );
            }
            else if(cBuf[5]=='i')
            {
                header_length = (uint16_t)snprintf(header, 256, "%sContent-Length: %u\r\n\r\n", http_png_hdr, (uint16_t)sizeof(mscc_png_logo));
                sock_send( pxSock, header, header_length );
                sock_send( pxSock, mscc_png_logo, sizeof(mscc_png_logo) );
            }
            else if(cBuf[5]=='t')
            {
                handle_trigger_request(&cBuf[5], (uint16_t)(uBuflen - 5));
                sock_send( pxSock, http_html_ok_hdr, sizeof(http_html_ok_hdr)-1 );
            }
            else 
            {
                /* Send the HTML header */
                header_length = (uint16_t)snprintf(header, 256, "%sContent-Length: %u\r\n\r\n", http_html_hdr, (uint16_t)(strlen(http_index_html)-1));
                sock_send( pxSock, header, header_length );
              
                /* Send our HTML page */
                sock_send( pxSock, http_index_html, sizeof(http_index_html)-1 );
            }
        }
        else if (uBuflen >= 6 &&
                 cBuf[0]=='P' &&
                 cBuf[1]=='O' &&
                 cBuf[2]=='S' &&
                 cBuf[3]=='T' &&
                 cBuf[4]==' ' &&
                 cBuf[5]=='/' )
        {
        
            sock_send( pxSock, http_post_resp_hdr, sizeof(http_post_resp_hdr)-1 );
        }
    } /* if(uBuflen > 0) */

    /* Initiate graceful shutdown. */
    FreeRTOS_shutdown( pxSock, FREERTOS_SHUT_RDWR );

    /* Shutdown is complete and the socket can be safely closed. */
    FreeRTOS_closesocket( pxSock );
}

/** The main function, never returns! */
void http_server_netconn_thread(void *arg)
{
    (void)arg;
    uint32_t counter = 0;
    uint8_t status;
    struct freertos_sockaddr xClient, xBindAddress;
    Socket_t pxListeningSocket, pxConnectedSocket;
    socklen_t xSize = sizeof( struct freertos_sockaddr );
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 5;

    pxListeningSocket = FreeRTOS_socket( FREERTOS_AF_INET4,
                                         FREERTOS_SOCK_STREAM,
                                         FREERTOS_IPPROTO_TCP );

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt( pxListeningSocket,
                         0,
                         FREERTOS_SO_RCVTIMEO,
                         &xReceiveTimeOut,
                         sizeof( xReceiveTimeOut ) );

    /* Check the socket was created. */
    configASSERT( pxListeningSocket != FREERTOS_INVALID_SOCKET );

    /* Set the listening port to 80. */
    memset( &xBindAddress, 0, sizeof(xBindAddress) );
    xBindAddress.sin_port = ( uint16_t )80;
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
    xBindAddress.sin_family = FREERTOS_AF_INET4;

    /* Bind the socket to the port that the client RTOS task will send to. */
    FreeRTOS_bind( pxListeningSocket, &xBindAddress, sizeof( xBindAddress ) );

    /* Set the socket into a listening state so it can accept connections.
       The maximum number of simultaneous connections is limited to 20. */
    FreeRTOS_listen( pxListeningSocket, xBacklog );
  
    do
    {
        /* Wait for incoming connections. */
        pxConnectedSocket = FreeRTOS_accept( pxListeningSocket, &xClient, &xSize );
        status = (pxConnectedSocket != FREERTOS_INVALID_SOCKET);
        if( status == pdTRUE )
        {
            http_server_netconn_serve(pxConnectedSocket, counter);

            /* Initiate graceful shutdown. */
            FreeRTOS_shutdown( pxConnectedSocket, FREERTOS_SHUT_RDWR );

            /* Shutdown is complete and the socket can be safely closed. */
            FreeRTOS_closesocket( pxConnectedSocket );

            counter++;
        }
    }
    while( status == pdTRUE );

    /* Initiate graceful shutdown. */
    FreeRTOS_shutdown( pxListeningSocket, FREERTOS_SHUT_RDWR );

    /* Shutdown is complete and the socket can be safely closed. */
    FreeRTOS_closesocket( pxListeningSocket );
}

