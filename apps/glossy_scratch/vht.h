/* Contiki Headers */
#include "contiki.h"
#include "rtimer.h"

/* TI Drivers */
#include <ti/drivers/rf/RF.h>

void vht_timer_init(RF_Handle rfHandle);

void vht_timer_update_h0();

uint32_t vht_time_now(uint32_t h1_hf);

uint32_t RF_ratGetValue(void); //TODO delete