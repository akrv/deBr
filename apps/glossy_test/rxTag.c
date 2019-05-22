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
//#define SEND_INTERVAL (2 * CLOCK_SECOND)

static uint8_t packet[PAYLOAD_LENGTH];
/*---------------------------------------------------------------------------*/
PROCESS(direct_radio_process, "Direct Radio Process");
AUTOSTART_PROCESSES(&direct_radio_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(direct_radio_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_RADIO.on();

  while(1) {
    while(!NETSTACK_RADIO.pending_packet()) {
      PROCESS_PAUSE();
    }
    if (NETSTACK_RADIO.read(&packet,PAYLOAD_LENGTH)) {
      LOG_INFO("packet received\n");
      LOG_INFO("%s\n", packet);
    }

    NETSTACK_RADIO.send(&packet, PAYLOAD_LENGTH);
    LOG_INFO("packet sent\n");
    LOG_INFO("%s\n", packet);

    LOG_INFO("wait for a msg from TX\n");
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
