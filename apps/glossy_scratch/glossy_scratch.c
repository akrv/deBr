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
  static uint32_t rtime_now=0,rtime_after=0;
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

  LOG_DBG("wait for 200 rtimer ticks.\n");
  rtime_now = (uint32_t)RTIMER_NOW(); 
  rtime_after = rtime_now+200;
  LOG_INFO("rtime now   : %lu\n", rtime_now);
  LOG_INFO("rtime after   : %lu\n", rtime_after);

  while(rtime_after >= rtime_now) {
    rtime_now = (uint32_t)RTIMER_NOW(); 
    //LOG_INFO("rtime now   : %lu\n", rtime_now);
    //PROCESS_PAUSE();
  }
    LOG_INFO("rtime now last: %lu\n", rtime_now);
    LOG_INFO("waited for : %lu\n", rtime_now-rtime_after-200);
  //clock_delay_usec(10000);
  //PROCESS_PAUSE();


  uint32_t rtime = RTIMER_NOW();
  uint32_t vht_time = vht_time_now();
  LOG_INFO("rtime   : %lu\n", rtime);
  LOG_INFO("vht time: %lu\n", vht_time);

  while(1) {
    PROCESS_PAUSE();
  }

  PROCESS_END();
}