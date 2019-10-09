/***** Includes *****/
/* Contiki */
#include "contiki.h"
#include "rtimer.h"

/* TI Drivers */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/rf/RF.h>

/* Standard C Libraries */
#include <math.h>
#include <stdlib.h>

/* TI Drivers - RAT timer */
#include <ti/drivers/rf/RF.h>
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)

#include "vht.h"

#include "int-master.h"

#include "ftoa.h" //TODO for debug, delete later

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy_VHT"
#define LOG_LEVEL LOG_LEVEL_APP


#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/SwiP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/utils/List.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_rfc_rat.h)
#include DeviceFamily_constructPath(inc/hw_rfc_dbell.h)
#include DeviceFamily_constructPath(driverlib/rfc.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#include DeviceFamily_constructPath(driverlib/aon_ioc.h)
#include DeviceFamily_constructPath(driverlib/rf_mailbox.h)
#include DeviceFamily_constructPath(driverlib/adi.h)
#include DeviceFamily_constructPath(driverlib/aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/chipinfo.h)


/*TODO rtimer callback should be fast enough to update
       h0, This must be checked */

//TODO how to handle overflow
//  Note:
//      - HF: there is no overflow for the HF clock as it takes 2^32/4MHZ =~1073 sec,
//        and glossy flood needs the HF for periods much less than that
//      - LF: over flow happens every 2^32/32768 / 3600 = 36 hour
//            - handle it later

//TODO i guess some of the function should be inline !

//TODO i also need to write some layer, to handle periods between
//      glossy, it's just an rtimer but still maybe write some
//      functions to better handle that

// TODO the freq for LF is 65536, not 32768 HOW? which clock source is that? 
#define LF_CLOCK_FREQ 4000000 //4MHZ - RAT timer
#define PHI_0 (float) LF_CLOCK_FREQ/RTIMER_ARCH_SECOND

static struct rtimer rtimer_timer;
static uint32_t last_h0_hf;
static uint32_t last_l0_lf;

static uint32_t toDelVar = 0; //TODO delete after debug

//TODO Align memory
typedef struct {
    const float phi; // hf lf ratio
    rtimer_clock_t l0_lf;
    uint32_t h0_hf;
    uint32_t h1_hf;
} vht_timer_t;

//static vht_timer_t vht_timer = {.phi = PHI_0};
static vht_timer_t vht_timer = {.phi = PHI_0};

  // Command to start RAT timer synced with RTC
  rfc_CMD_SYNC_START_RAT_t RF_cmdSyncStartRAT = {
  .commandNo = CMD_SYNC_START_RAT,
  .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
  .startTime = 0x00000000,
  .startTrigger.triggerType = 0x0,
  .startTrigger.bEnaCmd = 0x0,
  .startTrigger.triggerNo = 0x0,
  .startTrigger.pastTrig = 0x0,
  .condition.rule = 0x1,
  .condition.nSkip = 0x0,
  .rat0 = 0,
};

//TODO make static
uint32_t RF_ratGetValue(void)
{
    return HWREG(RFC_RAT_BASE + RFC_RAT_O_RATCNT);
}

// Radio core must not sleep when this function is called
//TODO maybe add code that prevent RF core sleep here
void vht_timer_init(RF_Handle rfHandle) {
    //HF clock ----------------------------------------------------------------
    //schedule an rtimer on the next tick
    //configure RAT timer
    // RF core must be active
    //TODO add this check
    //TODO recheck if this section is really critical
    ///* Enter critical section */
    //uint32_t key = HwiP_disable();
    //if (RF_core.status == RF_CoreStatusActive)
    //{
    //  LOG_INFO("RF core must be active");
    //  while(1); //lock
    //}
    ///* Exit critical section */
    //HwiP_restore(key);
    // start the RAT timer synchronized to RTC
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdSyncStartRAT, RF_PriorityNormal, NULL, 0);
    LOG_DBG("Start RAT cmd posted.\n");    
    //TODO wait for RAT to work, as it's waiting for the next RTC tick

    //LF clock ----------------------------------------------------------------
    rtimer_set(&rtimer_timer, RTIMER_NOW()+VHT_LF_UPDATE_CYCLES, 0, vht_timer_update_h0, NULL);
    //-------------------------------------------------------------------------

    LOG_DBG ("VHT phi: %s\n", ftoa(vht_timer.phi, 4));
    LOG_DBG("VHT initalized.\n");
}


//TODO check that this interrupt is correctly happening
//    the value of h1-h0 when vht_time_now() us calculating should not pass 1464,
//    after that the rtimer should trigger and update h1
//TODO make static
void vht_timer_update_h0() {
    int_master_status_t int_status = int_master_read_and_disable();
    last_h0_hf = RF_ratGetValue();
    last_l0_lf = RTIMER_NOW();
    rtimer_set(&rtimer_timer, RTIMER_NOW() + VHT_LF_UPDATE_CYCLES, 0, vht_timer_update_h0, NULL);
    int_master_status_set(int_status);
    toDelVar++;
}

//TODO no need for the mod as RAT is synchronized with the RTC, right?
//    if so also delete the header file for fmod()
uint32_t vht_time_now(uint32_t h1_hf) {
    int_master_status_t int_status = int_master_read_and_disable();
    vht_timer.h0_hf = last_h0_hf;
    vht_timer.l0_lf = last_l0_lf;
    int_master_status_set(int_status);
    vht_timer.h1_hf = h1_hf; 

    LOG_DBG("--------------Init values--------------\n");
    LOG_DBG("rtimer called %lu times.\n", toDelVar);

    LOG_DBG("Calculating VHT time:\n");
    LOG_DBG ("VHT l0: %lu\n", vht_timer.l0_lf);
    LOG_DBG ("VHT h0: %lu\n", vht_timer.h0_hf);
    LOG_DBG ("VHT h1: %lu\n", vht_timer.h1_hf);

    double term1 = (double)vht_timer.l0_lf*vht_timer.phi;
    //double term2 = fmod( (vht_timer.h1_hf-vht_timer.h0_hf), vht_timer.phi );
    double term2 = vht_timer.h1_hf-vht_timer.h0_hf;
    uint32_t vht_val = (int) round(term1+term2);
    //LOG_DBG ("rtime * phi    : %s\n", ftoa(term1, 4));
    //LOG_DBG ("(h1-h0) mod phi: %s\n", ftoa(term2, 4));
    //LOG_DBG ("h1 - h0: %s\n", ftoa(term2, 4));
    LOG_DBG ("vht val rounded: %lu\n", vht_val);
    LOG_DBG("----------------------------------------\n");
    return vht_val;
    //return round(RTIMER_NOW() * PHI_0  
    //        + (h1_hf - h0_hf) % PHI_0);
}