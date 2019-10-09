#include "contiki.h"
#include "rtimer.h"
#include "clock.h"

#include "glossy.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy Test"
#define LOG_LEVEL LOG_LEVEL_APP

static uint8_t glossy_payload[GLOSSY_PAYLOAD_LEN];

/*---------------------------------------------------------------------------*/
PROCESS(app_process, "Application Task");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(app_process, ev, data) 
{
  PROCESS_BEGIN();
  LOG_INFO("Process start.\n");

  if (INITIATOR_ID == NODE_ID) {
    // set the content of the payload
    uint8_t i;
    for (i = 0; i < GLOSSY_PAYLOAD_LEN; i++) {
      glossy_payload[i] = 9;
    }
  }

  /* start the glossy thread in 1s */  
  //rtimer_set(&rtimer_timer, RTIMER_NOW() + RTIMER_SECOND, 0, NULL, NULL);
  glossy_start(INITIATOR_ID, NODE_ID, glossy_payload, GLOSSY_PAYLOAD_LEN, GLOSSY_N_TX);
  LOG_INFO("finished one glossy flood.\n");
  
  //LOG_INFO("Test Ended\n");

  while(1) {
      watchdog_periodic();
  }

  PROCESS_END();
}