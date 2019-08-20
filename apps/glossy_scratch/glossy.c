#include "contiki.h"
#include "net/netstack.h"
#include "rtimer.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy"
#define LOG_LEVEL LOG_LEVEL_APP

/*---------------------------------------------------------------------------*/
#define IS_INITIATOR() \
  (initiator_id == node_id)
/*---------------------------------------------------------------------------*/
void
glossy_start(uint16_t initiator_id, uint16_t node_id, uint8_t *payload,
             uint8_t payload_len, uint8_t n_tx_max)
{
    //TODO: fill the packet fields here

    //TODO: disable interrupts

    NETSTACK_RADIO.on();

    if(IS_INITIATOR()) {
        LOG_DBG("Node is initiator, Start Tx\n");
        //TODO: check syncing

        /* start first transmission */
        //TODO: set proper radio parameters
        //TODO: add glossy header
        NETSTACK_RADIO.send(&payload, payload_len);
        //NETSTACK_RADIO.channel_clear();
    } else {
        LOG_DBG("Node is not initiator, Start RX\n");
        /* Glossy Receiver */

        while(!NETSTACK_RADIO.pending_packet());
        //TODO stamp the vht HF time
        LOG_DBG("Waiting for first Packet to be received\n");
        NETSTACK_RADIO.read(&payload, payload_len);
        LOG_DBG("A packet is received\n");
          //TODO should the payload variable needs to be changed
        //TODO check if the packet is not corrupted, how !
    }
}
/*---------------------------------------------------------------------------*/