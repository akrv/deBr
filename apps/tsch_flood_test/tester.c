/*
Based on null-net example in contiki-ng examples
*/

#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include <string.h>
#include <stdio.h> /* For printf() */

#include <sys/rtimer.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (5*CLOCK_SECOND) //TODO increase this period so there is enough time for all kits to start sending

/*---------------------------------------------------------------------------*/
/* Global variables*/
uint32_t rx_start, rx_end;

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet unicast example");
AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  rx_end = RTIMER_NOW();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;

  PROCESS_BEGIN();

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
    /* send a packet so all tx start sending */
    LOG_INFO("Sending start packet %u to all (", count);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_(")\n");

    NETSTACK_NETWORK.output(NULL);
    rx_start = RTIMER_NOW();
    etimer_reset(&periodic_timer);
    count++;
    
    /* measure how long it took to receive all packets*/
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("RX duration: %lu \n", rx_end-rx_start);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
