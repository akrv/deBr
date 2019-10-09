#include "contiki.h"
#include "watchdog.h"
#include "net/netstack.h"
#include "rtimer.h"

//#include "int-master.h"

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* RF settings */
#include "smartrf_settings/smartrf_settings.h"

/* Application Header files */
#include "vht.h"
#include "RFQueue.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy"
#define LOG_LEVEL LOG_LEVEL_APP

/***** Variable declarations *****/
// RF
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Handle rfHandle;
static RF_Params rfParams;

static uint8_t payload_with_counter[GLOSSY_PAYLOAD_LEN_WITH_COUNT];

///* Buffer which contains all Data Entries for receiving data.
// * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
//static uint8_t
//rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
//                                                  MAX_LENGTH,
//                                                  NUM_APPENDED_BYTES)]
//                                                  __attribute__((aligned(4)));
//
///* Receive dataQueue for RF Core to fill in data */
//static dataQueue_t dataQueue;
//static rfc_dataEntryGeneral_t* currentDataEntry;
//static uint8_t packetLength;
//static uint8_t* packetDataPointer;
//static uint32_t rxTimestamp;
//static uint32_t currentTimestamp;

//static uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */

//static bool tx_callback_called = false; //TODO delete

/*---------------------------------------------------------------------------*/
//void fs_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//{
//    fs_callback_called = true;
//    if (e & RF_EventCmdDone)
//    {
//      // Do something
//    }
//}

//void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//{
//    tx_callback_called = true;
//    if (e & RF_EventTxDone)
//    {
//      // Do something
//    }
//    if (e & RF_EventError)
//    {
//      // Do something
//    }
//}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define IS_INITIATOR() \
  (initiator_id == node_id)
/*---------------------------------------------------------------------------*/
void
glossy_start(uint16_t initiator_id, uint16_t node_id, uint8_t *payload,
             uint8_t payload_len, uint8_t n_tx_max)
{
  static uint8_t i;
  // Packet
  uint8_t n_tx_count;
  uint32_t cmd_time;

  //static uint32_t RAT_time_first_cmd;

  /* Setup RF */
  /* Request access to the radio */
  RF_Params_init(&rfParams);
  rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);

  /* Setup VHT */
  //vht_timer_init(rfHandle); //TODO enable VHT after finishing one flood code

  /* Set the frequency */
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

  /* TODO: disable interrupts !!
           glossy state in the paper that interrupt should be disables
           , but RF callbacks and VHT update are implemented as interrupts */

  /* Setting RF commands */
  // Setting TX cmd
  RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
  RF_cmdPropTx.pPkt = payload_with_counter; //TODO this may be a mistake
  RF_cmdPropTx.startTrigger.pastTrig = 1;
  RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
  RF_cmdPropTx.startTime = 0; //to be determined by each command later
  // Setting RX cmd
  ///* Set the Data Entity queue for received data */
  //RF_cmdPropRx.pQueue = &dataQueue;
  ///* Discard ignored packets from Rx queue */
  //RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
  ////RF_cmdPropRx.rxConf.bAppendRssi = 1;
  //RF_cmdPropRx.rxConf.bAppendTimestamp = 1;
  ////RF_cmdPropRx.rxConf.bIncludeHdr = 1;
  //RF_cmdPropRx.rxConf.bAppendStatus = 0;
  ////RF_cmdPropRX.rxConf.bAppendTimestamp = 0;
  ///* Discard packets with CRC error from Rx queue */
  //RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
  ///* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
  //RF_cmdPropRx.maxPktLen = MAX_LENGTH;
  //RF_cmdPropRx.pktConf.bRepeatOk = 1;
  //RF_cmdPropRx.pktConf.bRepeatNok = 1;

  /*TODO IMPORTANT, regarding RF cmds execution time
         - first issue the commands separately but with the same time
         - if that didn't work maybe somehow i should relate the time of the commands */


  if(IS_INITIATOR()) {
  // Initiator
    // add glossy header (the c relay counter)
    for(i = 0; i < (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++) {
      payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
    }
  
    LOG_DBG("Node is initiator, Start Tx\n");

    //TODO: check syncing

    /* start first transmission */

    cmd_time = RF_ratGetValue();
    for(n_tx_count=0; n_tx_count<n_tx_max; n_tx_count++) {
      /* Send packet */
      cmd_time += GLOSSY_T_SLOT;
      //cmd_time = GLOSSY_T_SLOT;
      RF_cmdPropTx.startTime = cmd_time;
      LOG_INFO("start time: %lu.\n", cmd_time);
      //int_master_status_t int_status = int_master_read_and_disable();
      RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
      //                                           RF_PriorityNormal, &tx_callback, RF_EventTxDone);
                                                 RF_PriorityNormal, NULL, 0);
      //int_master_status_set(int_status);
      LOG_DBG("sent packet %u.\n", n_tx_count);
      payload_with_counter[0] +=1; // increment c (glossy relay counter)
    }
  } else {
  // Normal node not Initiator
    LOG_DBG("Node is not initiator, Start RX\n");
    ///* Glossy Receiver */

    //while(!NETSTACK_RADIO.pending_packet());
    ////TODO stamp the vht HF time
    //LOG_DBG("Waiting for first Packet to be received\n");
    //while(!NETSTACK_RADIO.pending_packet()) {
    //  watchdog_periodic();
    //}
    //NETSTACK_RADIO.read(&payload_with_counter, GLOSSY_PAYLOAD_LEN_WITH_COUNT);
    //LOG_DBG("A packet is received\n");
    //  //TODO should the payload variable needs to be changed
    ////TODO check if the packet is not corrupted, why, how !
    //for(n_tx_count=0; n_tx_count<GLOSSY_N_TX ; n_tx_count++) {
    //  payload_with_counter[0] +=1; // increment c (glossy relay counter)
    //  NETSTACK_RADIO.send(&payload_with_counter, GLOSSY_PAYLOAD_LEN_WITH_COUNT);
    //  LOG_DBG("sent packet %u.\n", n_tx_count);
    //}
  }
  
  //NETSTACK_RADIO.off();

}
/*---------------------------------------------------------------------------*/