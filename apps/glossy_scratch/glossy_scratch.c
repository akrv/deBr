#include "contiki.h"
#include "rtimer.h"
#include "clock.h"

#include "glossy.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy Test"
#define LOG_LEVEL LOG_LEVEL_APP

uint8_t glossy_payload[GLOSSY_PAYLOAD_LEN];

static struct rtimer glossy_rtimer;

/*---------------------------------------------------------------------------*/
/***** Callback Functions *****/
//extern void  update_packet_call_glossy_start();
/*---------------------------------------------------------------------------*/
PROCESS(second_process, "SecondTask");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(second_process, ev, data) 
{
  PROCESS_BEGIN();
  LOG_INFO("second flood.\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void   start_next_flood() {
  //rtc_interrupt_called = true;
  //process_poll(&second_process);
}
/*---------------------------------------------------------------------------*/
PROCESS(app_process, "Application Task");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
//void  update_packet_call_glossy_start() {
//  //rtc_interrupt_called = true;
//  process_poll(&app_process);
//}
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

  //schedule next flood - schedules AON RTC callback to wake up the radio before next flood
  if (rtimer_set(&glossy_rtimer, RTIMER_NOW()+100, 0, start_next_flood, NULL)) {
    LOG_DBG("RTC interrupt for next flood was set.\n");
  } else {
    LOG_ERR("RTC interrupt for next flood was not set.\n");
  }
  
  //LOG_INFO("Test Ended\n");

  //while(1) {
  //    watchdog_periodic();
  //}

  PROCESS_END();
}