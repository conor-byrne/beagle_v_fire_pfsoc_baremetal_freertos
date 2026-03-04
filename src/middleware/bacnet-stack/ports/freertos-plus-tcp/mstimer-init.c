/*
 * mstimer-init.c
 *
 *  Created on: 29 Jan 2026
 *      Author: Conor Byrne <conor@glasetech.ie>
 */

#include "bacnet/basic/sys/mstimer.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "portmacro.h"
#include "projdefs.h"
#include "timers.h"

static TimerHandle_t mstimer_handle;
static unsigned long mstimer_value;

#define MSTIMER_INTERVAL 50

unsigned long mstimer_now(void)
{
    return mstimer_value;
}

static void bacnet_mstimer_cb(TimerHandle_t timer_handle)
{
    (void)timer_handle;
    mstimer_value += MSTIMER_INTERVAL;
}

void mstimer_init(void)
{
    mstimer_value = 0;
    mstimer_handle = xTimerCreate("BACnet stack mstimer",
                                  pdMS_TO_TICKS(MSTIMER_INTERVAL),
                                  pdTRUE, // auto reload
                                  NULL,
                                  bacnet_mstimer_cb);
    (void)xTimerStart(mstimer_handle, portMAX_DELAY);
}
