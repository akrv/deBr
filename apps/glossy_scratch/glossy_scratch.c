#include "contiki.h"
#include "rtimer.h"
#include "clock.h"

#include "glossy.h"

// TESTING VHT
#include "vht.h"
//#include DeviceFamily_constructPath(driverlib/aon_rtc.h)

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy Test"
#define LOG_LEVEL LOG_LEVEL_APP

//static struct rtimer rtimer_timer;
//static uint8_t glossy_payload[GLOSSY_PAYLOAD_LEN];
/*---------------------------------------------------------------------------*/
PROCESS(app_process, "Application Task");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(app_process, ev, data) 
{
  PROCESS_BEGIN();

  //if (INITIATOR_ID == NODE_ID) {
  //  // set the content of the payload
  //  uint8_t i;
  //  for (i = 0; i < GLOSSY_PAYLOAD_LEN; i++) {
  //    glossy_payload[i] = i;
  //  }
  //}
  ///* start the glossy thread in 1s */  
  ////rtimer_set(&rtimer_timer, RTIMER_NOW() + RTIMER_SECOND, 0, glossy_thread, NULL);
  //glossy_start(INITIATOR_ID, NODE_ID, glossy_payload, GLOSSY_PAYLOAD_LEN, GLOSSY_N_TX);
  
  //LOG_INFO("Test Ended\n");

  // TESTING VHT
  vht_timer_init();

  //uint32_t rtimer_before, rtimer_after;
  //rtimer_before = RTIMER_NOW();
  LOG_DBG("just doing anything to waste some random cycles.\n");
  clock_delay_usec(3000);
  //rtimer_after = RTIMER_NOW();
  //LOG_DBG("rtimer before wait: %lu\n", rtimer_before);
  //LOG_DBG("rtimer after  wait: %lu\n", rtimer_after);

  uint32_t rtime = RTIMER_NOW();
  uint32_t vht_time = vht_time_now();
  LOG_INFO("rtime   : %lu\n", rtime);
  LOG_INFO("vht time: %lu\n", vht_time);

  while(1) {
    PROCESS_PAUSE();
  }

  PROCESS_END();
}