#include "contiki.h"
#include "watchdog.h"
#include "net/netstack.h"

/* std lib */
#include <math.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* RF settings */
#include "smartrf_settings/smartrf_settings.h"

/* Application Header files */
#include "glossy.h"
#include "vht.h"
#include "RFQueue.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy"
#define LOG_LEVEL LOG_LEVEL_APP

/***** Variable declarations *****/
/* RF */
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;

static uint8_t payload_with_counter[GLOSSY_PAYLOAD_LEN_WITH_COUNT];

static uint8_t n_tx_count;
static uint32_t initiator_cmd_base_time;

extern uint8_t glossy_payload[GLOSSY_PAYLOAD_LEN];

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

//static uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */

static ratmr_t last_r0 = 0; 


static bool glossy_init_flag = false;
static bool glossy_stop_flag = false;

static bool tx_callback_called = false; //TODO delete
static bool rx_callback_called = false; //TODO delete
//static bool rtc_interrupt_called = false; //TODO delete
//static bool rat_start_callback_called = false; //TODO delete


/*---------------------------------------------------------------------------*/
/***** TODO general *****/
/* TODO check disabiling interrupts. In cc1350, Maybe there is no need to
   disable the interrupts as the RF core is a separate processor and everything
   is implemented as interrupts or normal processes  */
   // lines that may be added before and after rf cmds:
     //int_master_status_t int_status = int_master_read_and_disable();
     //RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
     //                                           RF_PriorityHighest , &tx_callback, RF_EventTxDone);
     //int_master_status_set(int_status);

//TODO check if callbacks can be set to higher priotiy

//TODO remove global variables for callbacks and use thread sync

/*---------------------------------------------------------------------------*/
/***** RF Callback Functions *****/
//void rat_start_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//{
//    if (e & RF_EventCmdDone)
//    {
//      rat_start_callback_called = true;
//    }
//}
/*---------------------------------------------------------------------------*/
void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventTxDone || e & RF_EventCmdDone)
    {
      n_tx_count++;
      if (n_tx_count == GLOSSY_N_TX) {
        return;
      }
      initiator_cmd_base_time += GLOSSY_T_SLOT;
      RF_cmdPropTx.startTime = initiator_cmd_base_time;
      RF_cmdPropTx.pPkt[0] = RF_cmdPropTx.pPkt[0] + 1; // increment c (glossy relay counter)
      RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                                 RF_PriorityHighest , &tx_callback, RF_EventCmdDone);
      tx_callback_called = true;
    }
}
/*---------------------------------------------------------------------------*/
void rx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventCmdDone)
    {
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
      initiator_cmd_base_time += GLOSSY_T_SLOT;
      RF_cmdPropTx.startTime = initiator_cmd_base_time;
  
      // TODO not needed right !
      RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
      RF_cmdPropTx.pPkt = payload_with_counter; //TODO this may be a mistake

      n_tx_count = 0;
      RF_cmdPropTx.pPkt[0] = payload_with_counter[0] + 1; // increment c (glossy relay counter)
      /* start first transmission - post other tx from callbacks*/
      RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                                 RF_PriorityHighest, &tx_callback, RF_EventCmdDone);
  
      rx_callback_called = true;
    }
}

/*---------------------------------------------------------------------------*/
/***** Macros *****/
#define IS_INITIATOR() \
  (initiator_id == node_id)
/*---------------------------------------------------------------------------*/
/***** Glossy Functions *****/
void
glossy_start(uint16_t initiator_id, uint16_t node_id, uint8_t *payload,
             uint8_t payload_len, uint8_t n_tx_max)
{
  LOG_DBG("----------------------------------------------------------------\n");
  LOG_DBG("print definitions.\n");
  LOG_DBG("GLOSSY_FLOOD_TIME_PERIOD_IN_RTC : %lu\n", GLOSSY_FLOOD_TIME_PERIOD_IN_RTC);
  LOG_DBG("PROCESSING_TIME_IN_RTC          : %lu\n", PROCESSING_TIME_IN_RTC);
  LOG_DBG("RTC_TICKS_BETWEEN_FLOODS        : %lu\n", RTC_TICKS_BETWEEN_FLOODS);
  LOG_DBG("GLOSSY_FLOOD_TIME_PERIOD_IN_RAT : %lu\n", (uint32_t)(GLOSSY_FLOOD_TIME_PERIOD_IN_RAT));
  LOG_DBG("----------------------------------------------------------------\n");

  LOG_DBG("glossy_start called.\n");

  if (glossy_stop_flag) {
    LOG_DBG("glossy stopped.\n");
    return;
  }

  static uint8_t i;

  /* Setup RF */
  /* Request access to the radio */
  RF_Params_init(&rfParams);
  rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
  LOG_INFO("RF_open executed.\n");

  RF_cmdSyncStartRat.rat0 = last_r0;
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdSyncStartRat, RF_PriorityHighest , NULL, 0);
  //while (!(RF_cmdSyncStartRat.status &  RF_EventCmdDone));

  /* Set the frequency */
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
  while (RF_cmdFs.status & RF_EventCmdDone) {watchdog_periodic();};
  LOG_DBG("Fs_cmd poseted to radio.\n");

  if (!glossy_init_flag) {
    LOG_DBG("Init TX and RX cmds and setup reading Queue.\n");
    /* Setting RF commands */
    // Setting TX cmd
    RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
    RF_cmdPropTx.pPkt = payload_with_counter; //TODO this may be a mistake
    RF_cmdPropTx.startTrigger.pastTrig = 0;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTime = 0; //to be determined by each command later
    RF_cmdPropTx.pktConf.bFsOff = 0;

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
    RF_cmdPropRx.pktConf.bFsOff = 0;

    // Setup RTC and RAT sync cmd
    RF_cmdSyncStartRat.startTrigger.pastTrig = 1;
    RF_cmdSyncStartRat.startTrigger.triggerType = TRIG_NOW;

    /* Enable output RTC clock for Radio Timer Synchronization */
    enable_rtc_rat_sync(); //TODO maybe not needed
  }

  if(IS_INITIATOR()) {
  // Initiator
  
    LOG_DBG("Initiator Node, start flood\n");

    //TODO check for new payload - for now assume payload doesn't change
    // add glossy header (the c relay counter)
    for(i = 0; i < (uint8_t)GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++) {
      payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
    }
    LOG_DBG("added counter byte to payload.\n");

    n_tx_count = 0;
    RF_cmdPropTx.pPkt[0] = n_tx_count; // increment c (glossy relay counter)

    if (!glossy_init_flag) {
      initiator_cmd_base_time =  RF_ratGetValue()+2*GLOSSY_T_SLOT; //TODO change GLOSSY_T_SLOT to least possible number (but adequate enough to process)
    } else /* if(glossy_init_flag) */ {
      initiator_cmd_base_time += GLOSSY_FLOOD_TIME_PERIOD_IN_RAT;
    }
    RF_cmdPropTx.startTime = initiator_cmd_base_time;
    LOG_DBG("init_cmd_base_time: %lu\n", initiator_cmd_base_time);

    /* start first transmission - post other tx from callbacks*/
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                               RF_PriorityHighest , &tx_callback, RF_EventCmdDone);
    LOG_DBG("First TX cmd posted.\n");

  } else {
  // Normal node not Initiator
    LOG_DBG("Node is not initiator, Start RX\n");
    ///* Glossy Receiver */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityHighest , &rx_callback, RF_EventCmdDone);
    // Retransmitting the packet will be handled by the callback
    while(rx_callback_called != true) {
      watchdog_periodic();
    }
    LOG_DBG("Received a packet.\n");
    LOG_DBG("time of sfd: %lu\n", rxTimestamp);

    while(tx_callback_called != true) { //TODO delete
      watchdog_periodic();
    }
    LOG_DBG("Tx callback called once.\n");

    // Print packet
    uint8_t i;
    LOG_INFO("Packet: ");
    for (i = 0; i < GLOSSY_PAYLOAD_LEN_WITH_COUNT  ; i++)
    {
        printf("%u", payload_with_counter[i]);
    }
    printf("\n");
    
  }
  
  // TODO do this with events
  LOG_DBG("Wait for N TX.\n");
  while (n_tx_count < n_tx_max) {
    LOG_DBG("n_tx_count: %u.\n", n_tx_count);
    LOG_DBG("n_tx_max: %u.\n", n_tx_max);
    watchdog_periodic();
  }
  LOG_DBG("One flood finished.\n");

  glossy_init_flag = true;

  LOG_DBG("RAT time: %lu\n", RF_ratGetValue());
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdSyncStopRat, RF_PriorityHighest , NULL, 0);
  while (!(RF_cmdSyncStopRat.status &  (DONE_OK | DONE_COUNTDOWN | DONE_TIMEOUT | DONE_STOPPED | DONE_ABORT | DONE_FAILED))) {
    watchdog_periodic();
    LOG_DBG("Waiting RAT to stop.\n");
  }
  LOG_DBG("RAT stopped, code: %x.\n", RF_cmdSyncStopRat.status);
  last_r0 = RF_cmdSyncStopRat.rat0;
  LOG_DBG("rat0 returned: %lu\n", RF_cmdSyncStopRat.rat0);
  RF_close(rfHandle);
  LOG_DBG("rfHandle closed.\n");

  //while(1) {watchdog_periodic();}
}
/*---------------------------------------------------------------------------*/