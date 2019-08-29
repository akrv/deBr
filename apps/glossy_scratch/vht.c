#include "contiki.h"
#include "rtimer.h"
#include "ti/drivers/timer/GPTimerCC26XX.h"
#include <math.h>


#include "vht.h"

#include "ftoa.h"

#include "int-master.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy_VHT"
#define LOG_LEVEL LOG_LEVEL_APP

/*TODO this way will only work if the rtimer tasks have
  the same priority as interrupts */

//TODO how to handle overflow
//  Note:
//      - HF: there is no overflow for the HF clock as it takes 2^32/48MHZ=900,
//        and glossy needs the HF for periods less than that
//      - LF: over flow happens every 2^32/32768 / 3600 = 36 hour
//            - handle it later

//TODO i guess some of the furelegionnctin should be inline !

//TODO i also need to write some layer, to handle periods between
//      glossy, it's just an rtimer but still maybe write some
//      functions to better handle that

// TODO the freq for LF is 65536, not 32768 HOW? which clock source is that? 
#define GET_MCU_CLOCK 48000000 //TODO defnition should already exist
#define PHI_0 (float) GET_MCU_CLOCK/RTIMER_ARCH_SECOND

static struct rtimer rtimer_timer;
static GPTimerCC26XX_Handle hTimer;

static uint32_t toDelVar = 0;

typedef struct {
    const float phi; // hf lf ratio
    rtimer_clock_t l0_lf;
    uint32_t h0_hf;
    uint32_t h1_hf;
} vht_timer_t;

//static vht_timer_t vht_timer = {.phi = PHI_0};
static vht_timer_t vht_timer = {.phi = PHI_0};


// Processor won't sleep when this function is called
void vht_timer_init() {
    //HF clock ----------------------------------------------------------------
    //schedule an rtimer on the next tick
    //schedule the GP_Timer
    //TODO does the source CLK have to be explicilty specified
    GPTimerCC26XX_Params params;
    GPTimerCC26XX_Params_init(&params);
    params.width          = GPT_CONFIG_32BIT;
    params.mode           = GPT_MODE_PERIODIC_UP;
    params.debugStallMode = GPTimerCC26XX_DEBUG_STALL_OFF;
    uint8_t GPTimerCC26XX_Config = 0; //TODO handle This in a better way
    hTimer = GPTimerCC26XX_open(GPTimerCC26XX_Config, &params);
    LOG_INFO("Opened GP Timer.\n");
    if(hTimer == NULL) {
      //TODO When can this happen !!
      LOG_ERR("Failed to open GPTimer\n");
    }
    int freq = GET_MCU_CLOCK*VHT_LF_UPDATE_CYCLES;
    GPTimerCC26XX_Value loadVal = freq;
    GPTimerCC26XX_setLoadValue(hTimer, loadVal);
    GPTimerCC26XX_registerInterrupt(hTimer, NULL, GPT_INT_TIMEOUT);
    GPTimerCC26XX_start(hTimer);
    //-------------------------------------------------------------------------

    //TODO delete
    vht_timer.h0_hf= GPTimerCC26XX_getFreeRunValue(hTimer);
    LOG_DBG ("VHT h0 init value: %lu\n", vht_timer.h0_hf);
    LOG_DBG ("init l0: %lu\n", RTIMER_NOW());

    //HF clock ----------------------------------------------------------------
    rtimer_set(&rtimer_timer, RTIMER_NOW()+VHT_LF_UPDATE_CYCLES, 0, vht_timer_update_h0, NULL);
    //-------------------------------------------------------------------------

    LOG_DBG("VHT initalizeinterruptd.\n");
    LOG_DBG ("VHT phi: %s\n", ftoa(vht_timer.phi, 4));
    LOG_DBG ("VHT h0 init value: %lu\n", vht_timer.h0_hf);
}


//TODO check that this interrupt is correctly happening
//    the value of h1-h0 when vht_time_now() us calculating should not pass 1464,
//    after that the rtimer should trigger and update h1
//TODO make static
void vht_timer_update_h0() {
    vht_timer.h0_hf = GPTimerCC26XX_getFreeRunValue(hTimer);
    rtimer_set(&rtimer_timer, RTIMER_NOW() + VHT_LF_UPDATE_CYCLES, 0, vht_timer_update_h0, NULL);
    toDelVar++;
}

uint32_t vht_time_now() {
    int_master_status_t int_status = int_master_read_and_disable();
    uint32_t last_h0_hf = vht_timer.h0_hf;
    int_master_status_set(int_status);
    vht_timer.l0_lf = RTIMER_NOW();
    vht_timer.h1_hf = GPTimerCC26XX_getFreeRunValue(hTimer);

    //LOG_DBG("rtimer called %lu times.\n", toDelVar);
    LOG_INFO("rtimer called %lu times.\n", toDelVar);

    LOG_DBG("Calculating VHT time:\n");
    LOG_DBG ("VHT l0: %lu\n", vht_timer.l0_lf);
    LOG_DBG ("VHT h0: %lu\n", last_h0_hf);
    LOG_DBG ("VHT h1: %lu\n", vht_timer.h1_hf);

    double term1 = (double)vht_timer.l0_lf*vht_timer.phi;
    double term2 = fmod( (vht_timer.h1_hf-last_h0_hf), vht_timer.phi );
    uint32_t vht_val = (int) round(term1+term2);
    LOG_DBG ("rtime * phi    : %s\n", ftoa(term1, 4));
    LOG_DBG ("(h1-h0) mod phi: %s\n", ftoa(term2, 4));
    LOG_DBG ("rounded: %lu\n", vht_val);
    return vht_val;
    //return round(RTIMER_NOW() * PHI_0  
    //        + (GPTimerCC26XX_getFreeRunValue(hTimer) - h0_hf) % PHI_0);
}