#include "contiki.h"
#include "sys/etimer.h"

/* App includes */
#include "glossy.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE " Glossy Test App"
#define LOG_LEVEL LOG_LEVEL_APP

/*---------------------------------------------------------------------------*/
/***** Variable declarations *****/
uint8_t glossy_payload[GLOSSY_PAYLOAD_LEN];
static struct etimer et;
/*---------------------------------------------------------------------------*/
PROCESS(glossy_test_process, "Glossy Test Process");
AUTOSTART_PROCESSES(&glossy_test_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(glossy_test_process, ev, data)
{
  PROCESS_BEGIN();
  LOG_INFO("Glossy Test App started.\n");

  if (INITIATOR_ID == NODE_ID) {
    // set the content of the payload
    uint8_t i;
    for (i = 0; i < GLOSSY_PAYLOAD_LEN; i++) {
      glossy_payload[i] = 9;
    }
  }

  /* start the glossy thread in 1s */  
  glossy_start(INITIATOR_ID, NODE_ID, glossy_payload, GLOSSY_PAYLOAD_LEN, GLOSSY_N_TX);

  // print statistics
  #if GLOSSY_CONF_COLLECT_STATS
  if (INITIATOR_ID != NODE_ID)
  {
    while(1)
    {
      etimer_set(&et, CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      LOG_DBG("crc ok packets: %lu\n", glossy_get_n_pkts_crcok());
      LOG_DBG("no. of all packets (with corrupted ones): %lu\n",  glossy_get_n_pkts());
      LOG_DBG("packet error rate: %u\n", glossy_get_per());
    }
  }
  #endif /* GLOSSY_CONF_COLLECT_STATS */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
