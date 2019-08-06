#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h" 


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Log Count Led"
#define LOG_LEVEL LOG_LEVEL_APP

PROCESS(address_finder_counter, "Address Finder Counter");
PROCESS(address_finder_led, "Address Finder LED");
AUTOSTART_PROCESSES(&address_finder_counter, &address_finder_led);

PROCESS_THREAD(address_finder_counter, ev, data)
{
  static struct etimer et;
  static uint8_t msg_counter;
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND/PRINT_RATE);

  while(1) {
    for (msg_counter = 1; msg_counter <= PRINT_RATE; msg_counter++) {
      //LOG_INFO("BOARD_ID: %d; Count: %d\n", BOARD_ID, msg_counter);
      LOG_INFO(";%d;%d\n", BOARD_ID, msg_counter);
      PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_TIMER);
      etimer_reset(&et);
    }
  }

  PROCESS_END();
}

PROCESS_THREAD(address_finder_led, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  etimer_set(&et, CLOCK_SECOND/LED_RATE/2);
  leds_set(LEDS_ALL);

  while(1) {
      leds_on(LEDS_ALL);
      PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_TIMER);
      etimer_reset(&et);
      leds_off(LEDS_ALL);
      PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_TIMER);
      etimer_reset(&et);
  }

  PROCESS_END();
}