/***** Includes *****/
/* Contiki */
#include "contiki.h"

#include "vht.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy_VHT"
#define LOG_LEVEL LOG_LEVEL_APP

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_rfc_rat.h)
#include DeviceFamily_constructPath(inc/hw_aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/rfc.h)

//TODO make static
uint32_t RF_ratGetValue(void)
{
    return HWREG(RFC_RAT_BASE + RFC_RAT_O_RATCNT);
}

void enable_rtc_rat_sync()
{
    /* Enable output RTC clock for Radio Timer Synchronization */
    //HWREG(AON_RTC_BASE + AON_RTC_O_CTL) |= AON_RTC_CTL_RTC_UPD_EN_M;
    HWREG(AON_RTC_BASE + AON_RTC_CTL_RTC_UPD_EN) = 1;
}