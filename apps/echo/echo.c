#include "contiki.h"
#include "sys/log.h"
#include "dev/serial-line.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/*---------------------------------------------------------------------------*/
PROCESS(ping_process, "ping_process");
AUTOSTART_PROCESSES(&ping_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ping_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Wating for Ping\n");

  while(1) {
    PROCESS_YIELD();
    if (ev == serial_line_event_message) {
      LOG_INFO("received: %s\n", (char *)data);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
