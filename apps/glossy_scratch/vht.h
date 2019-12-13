/* Contiki Headers */
#include "contiki.h"

/* TI Drivers */
#include <ti/drivers/rf/RF.h>

// TODO the freq for LF is 65536, not 32768 HOW? which clock source is that? 
#define HF_CLOCK_FREQ 4000000 //4MHZ - RAT timer
#define PHI_0 (float) HF_CLOCK_FREQ/RTIMER_ARCH_SECOND

uint32_t RF_ratGetValue(void); //TODO delete

void enable_rtc_rat_sync();