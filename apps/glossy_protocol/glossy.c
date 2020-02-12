#include "contiki.h"
#include "watchdog.h"
#include "rtimer.h"
#include "dev/leds.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Glossy Protocol"
#define LOG_LEVEL LOG_LEVEL_APP

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

#include DeviceFamily_constructPath(inc/hw_rfc_rat.h)
#include DeviceFamily_constructPath(inc/hw_aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/rfc.h)

/* RF settings */
#include "smartrf_settings/smartrf_settings.h"

/* Application Header files */
#include "glossy.h"
#include "RFQueue.h"

/*---------------------------------------------------------------------------*/
/***** Variable declarations *****/
/* glossy */
static bool glossy_init = false;
static bool glossy_stop_flag = false;
// glossy statistics
#if GLOSSY_CONF_COLLECT_STATS
typedef struct {
  struct {
  /* --- statistics only, otherwise not relevant for Glossy --- */
  /* stats of the last flood */
  uint8_t  last_flood_relay_cnt;                    /* relay cnt on first rx */
  int8_t   last_flood_rssi_noise;
  int16_t  last_flood_rssi_sum;
  uint8_t  last_flood_rssi[3];
  uint8_t  last_flood_hops[3];
  uint8_t  last_flood_n_rx_started;            /* # preamble+sync detections */
  uint8_t  last_flood_n_rx_fail;                      /* header or CRC wrong */
  uint8_t  already_counted;
  rtimer_clock_t last_flood_duration;                /* total flood duration */
  rtimer_clock_t last_flood_t_to_rx;              /* time to first reception */
  /* global stats since last reset of the node */
  //uint32_t pkt_cnt;           /* total # of received packets (preamble+sync) */
  uint32_t pkt_cnt_crc_Nok;        /* total # of received packets with CRC error */
  uint32_t pkt_cnt_crc_ok;         /* total # of received packets with CRC ok */
  uint32_t flood_cnt;    /* total # of floods (with >=1x preamble+sync det.) */
  uint32_t flood_cnt_success;      /* total # floods with at least 1x CRC ok */
  uint16_t error_cnt;              /* total number of errors */
  } stats;
} glossy_state_t;
static glossy_state_t g;
#endif /* GLOSSY_CONF_COLLECT_STATS */
// glossy time
static uint32_t cmd_base_time_RAT;
/* RF */
static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;

static uint8_t payload_with_counter[GLOSSY_PAYLOAD_LEN_WITH_COUNT];

static uint8_t n_tx_count;

/* Buffer which contains all Data Entries for receiving data.
 * Pragmas are needed to make sure this buffer is 4 byte aligned (requirement from the RF Core) */
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  GLOSSY_PAYLOAD_LEN_WITH_COUNT,
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

#define TX_FLAGS RF_EventCmdDone
#define RX_FLAGS RF_EventCmdDone | RF_EventRxOk | RF_EventRxNOk

/***** Functions definition *****/
void schedule_next_flood();
/*---------------------------------------------------------------------------*/
/***** Timers Helper Functions *****/
uint32_t RF_ratGetValue(void)
{
    return HWREG(RFC_RAT_BASE + RFC_RAT_O_RATCNT);
}

void enable_rtc_rat_sync()
{
    /* Enable output RTC clock for Radio Timer Synchronization */
    //HWREG(AON_RTC_BASE + AON_RTC_O_CTL) |= AON_RTC_CTL_RTC_UPD_EN_M;
    HWREG(AON_RTC_BASE + AON_RTC_CTL_RTC_UPD_EN) = 1;
}
/*---------------------------------------------------------------------------*/
/***** Macros *****/
#define IS_INITIATOR() \
  (INITIATOR_ID == NODE_ID)
/*---------------------------------------------------------------------------*/
/***** Glossy helper Functions *****/
void update_payload(uint8_t *payload)
{
  static uint8_t i;
  // add glossy header (the c relay counter)
  payload_with_counter[0] = 0;
  for(i = 0; i < GLOSSY_PAYLOAD_LEN; i++) {
    payload_with_counter[i+1] = payload[i]; //first byte reserved for header (c: the relay counter)
  }
  LOG_DBG("added counter byte to payload.\n");
}
/*---------------------------------------------------------------------------*/
/***** RF Callback Functions *****/
void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventTxDone || e & RF_EventCmdDone)
    {
      n_tx_count++;
      if (n_tx_count >= GLOSSY_N_TX) {
        leds_off(LEDS_ALL);
        schedule_next_flood();
      } else {
        leds_on(LEDS_ALL);
        RF_cmdPropTx.startTime += GLOSSY_T_SLOT;
        RF_cmdPropTx.pPkt[0] += 1; // increment c (glossy relay counter)
        RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                                   RF_PriorityHighest , &tx_callback, TX_FLAGS);
      }
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
      cmd_base_time_RAT = rxTimestamp;
      RF_cmdPropTx.startTime = cmd_base_time_RAT + GLOSSY_T_SLOT;
  
      // TODO not needed right !
      RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
      RF_cmdPropTx.pPkt = payload_with_counter; //TODO this may be a mistake

      RF_cmdPropTx.pPkt[0] += 1; // increment c (glossy relay counter)
      /* start first transmission - post other tx from callbacks*/
      RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                                 RF_PriorityHighest, &tx_callback, TX_FLAGS);
    }

#if GLOSSY_CONF_COLLECT_STATS
    if (e & RF_EventRxOk)
    {
      g.stats.pkt_cnt_crc_ok++;
    }
    else if (e & RF_EventRxNOk)
    {
      g.stats.pkt_cnt_crc_Nok++;
    }
#endif
}
/*---------------------------------------------------------------------------*/
/***** Glossy Callback Functions *****/
void schedule_next_flood()
{
  // options to handle timing
  // 1- RTC wake processor up before flood, RAT wait for the extra time and handle it

  // 2- if RAT timer cannot keep consistency: Then calculate the number of RAT ticks needed
  //    for TX: it's always the same
  //    for RX: calculate the difference the first Time then add this difference later
  //            (for now just use the difference in the first time)

  // 3- Handle everything in RAT, if calculates the time passed on it's own [USED METHOD]
  //    (when RAT wake up syncrhonized to RX and rat0 parameter is correct)

  if (glossy_stop_flag) {
    glossy_stop_flag = false;
    return;
  }

  if (!glossy_init)
  {
    LOG_DBG("Init Glossy first flood time.\n");
    // INITIATOR: current time is calculated as base time
    cmd_base_time_RAT = RF_ratGetValue()+2*GLOSSY_T_SLOT; // some time that is far enough for this processing to be done
    glossy_init = true;
    // NON-INITIATOR
    RF_cmdPropRx.startTrigger.triggerType = TRIG_NOW;
  } else /* if(glossy_init) */ {
    // INITIATOR
    if (IS_INITIATOR())
    {
      cmd_base_time_RAT += GLOSSY_FLOOD_TIME;
    }
    // NON_INITIATOR
    else /* if (!IS_INITIATOR()) */
    {
      RF_cmdPropRx.startTrigger.triggerType = TRIG_ABSTIME;
      cmd_base_time_RAT += GLOSSY_FLOOD_TIME; // start listen a bit before expected TX time //TODO make this minimal
    }
  }

  n_tx_count = 0;
  RF_cmdPropTx.pPkt[0] = 0; // reset c (glossy relay counter)
  // start TX or RX if initiator no non-intiator
  if (IS_INITIATOR())
  {
    RF_cmdPropTx.startTime = cmd_base_time_RAT;
    /* start first transmission - post other tx from callbacks*/
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                               RF_PriorityHighest , &tx_callback, TX_FLAGS);
  }
  else /* if (!IS_INITIATOR()) */
  {
    //LOG_DBG("switch to RX\n");
    RF_cmdPropRx.startTime = cmd_base_time_RAT;
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityHighest , &rx_callback, RX_FLAGS);
  }
}
/*---------------------------------------------------------------------------*/
/***** Glossy Functions *****/
void
glossy_start(uint16_t initiator_id, uint16_t node_id, uint8_t *payload,
             uint8_t payload_len, uint8_t n_tx_max)
{
  LOG_DBG("glossy_start called.\n");

  /* Setup RF */
  /* Request access to the radio */
  RF_Params_init(&rfParams);
  rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);
  LOG_DBG("RF_open executed.\n");

  RF_cmdSyncStartRat.rat0 = last_r0;
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdSyncStartRat, RF_PriorityHighest , NULL, 0);
  //while (!(RF_cmdSyncStartRat.status &  RF_EventCmdDone));

  /* Set the frequency */
  RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
  while (RF_cmdFs.status & RF_EventCmdDone) {watchdog_periodic();};
  LOG_DBG("Fs_cmd poseted to radio.\n");

  LOG_DBG("Init TX and RX cmds and setup reading Queue.\n");
  /* Setting RF commands */
  // Setting TX cmd
  RF_cmdPropTx.pktLen = GLOSSY_PAYLOAD_LEN_WITH_COUNT ;
  RF_cmdPropTx.pPkt = payload_with_counter;
  RF_cmdPropTx.startTrigger.pastTrig = 0;
  RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
  RF_cmdPropTx.startTime = 0; //to be determined by each command later
  RF_cmdPropTx.pktConf.bFsOff = 0;

  // Setting RX cmd
  if( RFQueue_defineQueue(&dataQueue,
                          rxDataEntryBuffer,
                          sizeof(rxDataEntryBuffer),
                          NUM_DATA_ENTRIES,
                          GLOSSY_PAYLOAD_LEN_WITH_COUNT + NUM_APPENDED_BYTES))
  {
      /* Failed to allocate space for all data entries */
      LOG_ERR("Failed to allocate space for all data entries.\n");
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

  // set payload
  update_payload(payload);

  #if GLOSSY_CONF_COLLECT_STATS
  g.stats.pkt_cnt_crc_Nok = 0;
  g.stats.pkt_cnt_crc_ok = 0;
  #endif /* GLOSSY_CONF_COLLECT_STATS */

  // start first flood
  LOG_DBG("Scheduling first flood.\n");
  schedule_next_flood();
}

uint8_t glossy_stop(void)
{
  glossy_stop_flag = true;
  glossy_init = false;
  return 0;
}
/*---------------------------------------------------------------------------*/
/***** Glossy Statistics Functions *****/
uint16_t glossy_get_per(void)
{
  return (uint16_t) g.stats.pkt_cnt_crc_Nok/(g.stats.pkt_cnt_crc_ok+g.stats.pkt_cnt_crc_Nok);
}
uint32_t glossy_get_n_pkts(void)
{
  return g.stats.pkt_cnt_crc_ok + g.stats.pkt_cnt_crc_Nok;
}
uint32_t glossy_get_n_pkts_crcok(void)
{
  return g.stats.pkt_cnt_crc_ok;
}