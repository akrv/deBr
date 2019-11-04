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
static RF_Params rfParams;

static uint8_t payload_with_counter[GLOSSY_PAYLOAD_LEN_WITH_COUNT];

static uint8_t n_tx_count;
static uint32_t initiator_cmd_base_time;

/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)]
                                                  __attribute__((aligned(4)));

/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;
static uint32_t rxTimestamp;
static uint32_t currentTimestamp; //TODO delete

//static uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */

static bool tx_callback_called = false; //TODO delete
static bool rx_callback_called = false; //TODO delete

/*---------------------------------------------------------------------------*/
//void fs_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e) //TODO delete
//{
//    fs_callback_called = true;
//    if (e & RF_EventCmdDone)
//    {
//      // Do something
//    }
//}

void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    tx_callback_called = true;

    // TODO move this code under RF_EventTxDone
    n_tx_count++;
    if (n_tx_count == GLOSSY_N_TX) {
      return;
    }
    initiator_cmd_base_time += GLOSSY_T_SLOT_WITH_ERROR;
    RF_cmdPropTx.startTime = initiator_cmd_base_time;
    RF_cmdPropTx.pPkt[0] = n_tx_count; // increment c (glossy relay counter)
    //int_master_status_t int_status = int_master_read_and_disable();
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
    //                                           RF_PriorityHighest , NULL, 0);
                                               RF_PriorityHighest , &tx_callback, RF_EventTxDone);
    //int_master_status_set(int_status);


    //if (e & RF_EventTxDone)
    //{
    //  // Do something
    //}
    //if (e & RF_EventError)
    //{
    //  // Do something
    //}
}

void rx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//TODO not sure if the callback is set to the highest proprity
{

    // TODO move this code under RF_EventRxEntryDone
    /* Extract packet and timestamp (sfd time in RAT time) from incoming data */
    /* Get current unhandled data entry */
    currentDataEntry = RFQueue_getDataEntry();

    /* Handle the packet data, located at &currentDataEntry->data:
     * - Length is the first byte with the current configuration
     * - Data starts from the second byte */
    //TODO use next line instead of defined value
    //packetLength      = *(uint8_t*)(&currentDataEntry->data);
    packetLength      = (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT;
    packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

    /* Copy the payload + the status byte to the packet variable */
    memcpy(payload_with_counter, packetDataPointer, (packetLength + 1));
    
    memcpy(&rxTimestamp, packetDataPointer + packetLength , 4);

    RFQueue_nextEntry();


    // setup the first retransmission - next retransmisions will be handled by tx_callback
    initiator_cmd_base_time = rxTimestamp;
    initiator_cmd_base_time += GLOSSY_T_SLOT_WITH_ERROR;
    RF_cmdPropTx.startTime = initiator_cmd_base_time;

    // TODO not needed right !
    RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
    RF_cmdPropTx.pPkt = payload_with_counter; //TODO this may be a mistake
    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW; //TODO delete - test
    
    n_tx_count = 0;
    RF_cmdPropTx.pPkt[0] = n_tx_count; // increment c (glossy relay counter)
    /* start first transmission - post other tx from callbacks*/
    //int_master_status_t int_status = int_master_read_and_disable();
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
    //                                           RF_PriorityNormal, NULL, 0);
                                               RF_PriorityNormal, &tx_callback, RF_EventTxDone | RF_EventError | RF_EventCmdPreempted);
    //int_master_status_set(int_status);


    if (e & RF_EventRxEntryDone)
    {
      // Do something, for instance post a semaphore.
    }
    if (e & RF_EventNDataWritten)
    {
      //TODO: how to specify the number of bytes
      // Do something
    }
    if (e & RF_EventRxOk)
    {
      //rx_callback_called = true;
      // Do something
    }
    if (e & RF_EventCmdDone)
    {
      rx_callback_called = true;
    }

    currentTimestamp = RF_getCurrentTime();
}
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

  //TODO to be deleted - just for test
  //RF_cmdPropTX.startTrigger.pastTrig = 1;

  // Setting RX cmd
  if( RFQueue_defineQueue(&dataQueue,
                          rxDataEntryBuffer,
                          sizeof(rxDataEntryBuffer),
                          NUM_DATA_ENTRIES,
                          MAX_LENGTH + NUM_APPENDED_BYTES))
  {
      /* Failed to allocate space for all data entries */
      while(1);
  }
  /* Set the Data Entity queue for received data */
  RF_cmdPropRx.pQueue = &dataQueue;
  /* Discard ignored packets from Rx queue */
  RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
  //RF_cmdPropRx.rxConf.bAppendRssi = 1;
  RF_cmdPropRx.rxConf.bAppendTimestamp = 1;
  //RF_cmdPropRx.rxConf.bIncludeHdr = 1;
  RF_cmdPropRx.rxConf.bAppendStatus = 0;
  //RF_cmdPropRX.rxConf.bAppendTimestamp = 0;
  /* Discard packets with CRC error from Rx queue */
  RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
  /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
  //RF_cmdPropRx.maxPktLen = MAX_LENGTH;
  RF_cmdPropRx.maxPktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT;
  RF_cmdPropRx.pktConf.bRepeatOk = 0;
  RF_cmdPropRx.pktConf.bRepeatNok = 0;

  /*TODO IMPORTANT, regarding RF cmds execution time
         - first issue the commands separately but with the same time
         - if that didn't work maybe somehow i should relate the time of the commands */

  // checking packets
  //LOG_INFO("payload: ");
  //for (i = 0; i < GLOSSY_PAYLOAD_LEN ; i++)
  //{
  //    printf("%u", payload[i]);
  //}
  //printf("\n");

  //// add glossy header (the c relay counter)
  //for(i = 0; i < (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++) {
  //  payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
  //}
  //LOG_INFO("payload with glossy: ");
  //for (i = 0; i < GLOSSY_PAYLOAD_LEN_WITH_COUNT  ; i++)
  //{
  //    printf("%u", payload_with_counter[i]);
  //}
  //printf("\n");

  //// add glossy header (the c relay counter)
  //for(i = 0; i < (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++) {
  //  payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
  //}

  if(IS_INITIATOR()) {
  // Initiator
  
    LOG_DBG("Node is initiator, Start Tx\n");

    //TODO: check syncing

    // add glossy header (the c relay counter)
    for(i = 0; i < (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++) {
      payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
    }

    initiator_cmd_base_time = RF_ratGetValue();
    initiator_cmd_base_time += GLOSSY_T_SLOT_WITH_ERROR;
    RF_cmdPropTx.startTime = initiator_cmd_base_time;
    n_tx_count = 0;
    RF_cmdPropTx.pPkt[0] = n_tx_count; // increment c (glossy relay counter)
    LOG_INFO("start time of %u TX cmd: %lu.\n", n_tx_count, initiator_cmd_base_time);
    /* start first transmission - post other tx from callbacks*/
    //int_master_status_t int_status = int_master_read_and_disable();
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
    //                                           RF_PriorityHighest , NULL, 0);
                                               RF_PriorityHighest , &tx_callback, RF_EventTxDone);
    //int_master_status_set(int_status);

  } else {
  // Normal node not Initiator
    LOG_DBG("Node is not initiator, Start RX\n");
    ///* Glossy Receiver */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityNormal, &rx_callback,
      //RF_EventRxOk);
      //RF_EventRxEntryDone | RF_EventNDataWritten | RF_EventRxOk);
      RF_EventRxOk | RF_EventCmdDone);
    // Retransmitting the packet will be handled by the callback
    while(rx_callback_called != true) { //TODO delete
      watchdog_periodic();
    }
    LOG_DBG("Received a packet.\n");
    LOG_DBG("time of sfd: %lu\n", rxTimestamp);
    LOG_DBG("callback time: %lu\n", currentTimestamp);
    //LOG_DBG("current time: %lu\n", RF_getCurrentTime());
    LOG_DBG("time of %u TX cmd should be: %lu.\n", n_tx_count, initiator_cmd_base_time);
    uint8_t i;
    LOG_INFO("Packet: ");
    for (i = 0; i < GLOSSY_PAYLOAD_LEN_WITH_COUNT  ; i++)
    {
        printf("%u", payload_with_counter[i]);
    }
    printf("\n");
    
    while(tx_callback_called != true) { //TODO delete
      watchdog_periodic();
    }
    LOG_DBG("Tx callback called.\n");
  }
  
  // TODO this probably should be placed somewhere else, or even not needed at all
  while (n_tx_count != n_tx_max) {
    watchdog_periodic();
    //LOG_DBG("N_TX: %u\n", n_tx_count);
  }
  LOG_INFO("start time of %u TX cmd: %lu.\n", n_tx_count, initiator_cmd_base_time);

}
/*---------------------------------------------------------------------------*/