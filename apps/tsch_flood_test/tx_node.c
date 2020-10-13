/*
Based on null-net example in contiki-ng examples
  */

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
static linkaddr_t dest_addr =         {{ 0x00, 0x12, 0x4b, 0x00, 0x0f, 0x2a, 0x4b, 0x83 }};
#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x0f, 0x2a, 0x4b, 0x83 }};
#endif /* MAC_CONF_WITH_TSCH */

/*---------------------------------------------------------------------------*/
/* Global variables*/
static process_event_t start_event;

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  process_post(&nullnet_example_process,start_event,NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static unsigned count = 0;

  PROCESS_BEGIN();

  start_event = process_alloc_event();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(start_event);
    LOG_INFO("Sending %u to ", count);
    LOG_INFO_LLADDR(&dest_addr);
    LOG_INFO_("\n");

    NETSTACK_NETWORK.output(&dest_addr);
    count++;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
