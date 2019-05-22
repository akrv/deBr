#include "contiki.h"
#include "net/netstack.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <int-master.h>
#include <gpio-hal.h>
#include <gpio-hal-arch.h>
#include <Board.h>
//#include <sys/critical.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* Configuration */
#define PAYLOAD_LENGTH      10
#define SEND_INTERVAL (2 * CLOCK_SECOND)

//#define INT_PIN CC1350STK_KEY_LEFT
#define INT_PIN CC1350STK_DP1

static uint8_t packetSent[PAYLOAD_LENGTH];
static uint8_t packetReceived[PAYLOAD_LENGTH];

/*---------------------------------------------------------------------------*/
PROCESS(direct_radio_process, "Direct Radio Process");
AUTOSTART_PROCESSES(&direct_radio_process);
/*---------------------------------------------------------------------------*/
static void send_callback(gpio_hal_pin_mask_t pin_mask) 
{            
  //gpio_hal_arch_interrupt_disable(INT_PIN);
  //LOG_INFO("interrupt was triggered \n");  
  process_poll(&direct_radio_process);
}

PROCESS_THREAD(direct_radio_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_RADIO.on();

  LOG_DBG("Debugging --------------- \n");

  //int_master_enable(); 
  //gpio_hal_init();
  //gpio_hal_arch_init();

  gpio_hal_pin_cfg_t cfg;
  cfg = GPIO_HAL_PIN_CFG_INT_ENABLE | GPIO_HAL_PIN_CFG_PULL_DOWN |
        GPIO_HAL_PIN_CFG_EDGE_BOTH;
  gpio_hal_arch_interrupt_enable(INT_PIN);
  gpio_hal_arch_pin_cfg_set(INT_PIN, cfg);
  gpio_hal_arch_pin_set_input(INT_PIN);

  gpio_hal_event_handler_t int_handler;
  int_handler.handler = send_callback;
  int_handler.pin_mask = gpio_hal_pin_to_mask(INT_PIN);
  gpio_hal_register_handler(&int_handler);
  //gpio_hal_event_handler(gpio_hal_pin_to_mask(INT_PIN));


  LOG_INFO("pin : %i\n",  INT_PIN);
  LOG_INFO("mask: %lu\n",  gpio_hal_pin_to_mask(INT_PIN));
  
  // check if pin gpio is working manually
  //while(1) {
  //LOG_INFO("GPIO read: %i\n", gpio_hal_arch_read_pin(INT_PIN));
  //static struct etimer timer;
  //etimer_set(&timer, 1 * CLOCK_SECOND);
  //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  //}

  while(1) {
    LOG_INFO("waiting for interrupt\n");
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
    LOG_INFO("Process Polled\n");

    for (int i = 0; i < PAYLOAD_LENGTH; i++)
    {
        packetSent[i] = rand();
    }

    PROCESS_PAUSE();


    NETSTACK_RADIO.send(&packetSent, PAYLOAD_LENGTH);
    LOG_INFO("packet Tx: ");
    for (int i = 0; i < PAYLOAD_LENGTH; i++)
    {
      printf("%c ", packetSent[i]);
    }
    printf("\n");    

    while(!NETSTACK_RADIO.pending_packet()) {
      PROCESS_PAUSE();
    }
    if (NETSTACK_RADIO.read(&packetReceived,PAYLOAD_LENGTH)) {
      LOG_INFO("packet received\n");
      LOG_INFO("packet Rx: ");
      for (int i = 0; i < PAYLOAD_LENGTH; i++)
      {
        printf("%c ", packetReceived[i]);
      }
      printf("\n");
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/