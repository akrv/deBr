#include "contiki.h"
#include "net/netstack.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* Configuration */
#define PAYLOAD_LENGTH      10
#define SEND_INTERVAL (4 * CLOCK_SECOND)

static uint8_t packetSent[PAYLOAD_LENGTH];
static uint8_t packetReceived[PAYLOAD_LENGTH];
/*---------------------------------------------------------------------------*/
PROCESS(direct_radio_process, "Direct Radio Process");
AUTOSTART_PROCESSES(&direct_radio_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(direct_radio_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  NETSTACK_RADIO.on();


  for (int i = 0; i < PAYLOAD_LENGTH; i++)
  {
      packetSent[i] = rand();
  }

  etimer_set(&periodic_timer, SEND_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    NETSTACK_RADIO.send(&packetSent, PAYLOAD_LENGTH);
    LOG_INFO("packet sent\n");
    LOG_INFO("%s\n", packetSent);
    
    while(!NETSTACK_RADIO.pending_packet()) {
      PROCESS_PAUSE();
    }
    if (NETSTACK_RADIO.read(&packetReceived,PAYLOAD_LENGTH)) {
      LOG_INFO("packet received\n");
      LOG_INFO("%s\n", packetReceived);
    }
    
    LOG_INFO("wait for the timer to expire\n");
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/