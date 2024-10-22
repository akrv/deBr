/***** Includes *****/
/* Contiki */
#include "contiki.h"
#include "watchdog.h"

/* Standard C Libraries */
#include <stdlib.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
#include DeviceFamily_constructPath(driverlib/rf_common_cmd.h)

/* RF settings */
#include "smartrf_settings/smartrf_settings.h"

/* Application Header files */
#include "RFQueue.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/***** Defines *****/

/* Packet TX Configuration */


/***** Prototypes *****/
//static void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/***** Variable declarations *****/
static RF_Object rfObject;
RF_Handle rfHandle;

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

//static uint32_t rxLastTimestamp = 0;

static uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */

//rfc_CMD_SYNC_START_RAT_t RF_cmdSyncStartRAT = {
//  .commandNo = CMD_SYNC_START_RAT,
//  .pNextOp = 0, // INSERT APPLICABLE POINTER: (uint8_t*)&xxx
//  .startTime = 0x00000000,
//  .startTrigger.triggerType = 0x0,
//  .startTrigger.bEnaCmd = 0x0,
//  .startTrigger.triggerNo = 0x0,
//  .startTrigger.pastTrig = 0x0,
//  .condition.rule = 0x1,
//  .condition.nSkip = 0x0,
//  .rat0 = 0,
//};

//TODO if thread sync needed to be done in glossy, then do it in a better way
static bool rx_callback_called = false;

/***** Function definitions *****/

/*---------------------------------------------------------------------------*/
//void fs_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//{
//  fs_callback_called = true;
//}

void rx_tester_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//TODO set the callback to the highest proprity
{
    if (e &  RF_EventCmdDone)
    {
        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        /* Handle the packet data, located at &currentDataEntry->data:
         * - Length is the first byte with the current configuration
         * - Data starts from the second byte */
        packetLength      = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        /* Copy the payload + the status byte to the packet variable */
        memcpy(packet, packetDataPointer, (packetLength + 1));
        
        memcpy(&rxTimestamp, packetDataPointer + packetLength , 4);

        RFQueue_nextEntry();

        //schedule RX again
        RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityHighest, &rx_tester_callback, RF_EventCmdDone);
        
        rx_callback_called = true;
    }
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS(direct_radio_process, "Direct Radio Process");
AUTOSTART_PROCESSES(&direct_radio_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(direct_radio_process, ev, data)
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    PROCESS_BEGIN();
    LOG_INFO("RX Process Begin.\n");

    if( RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(1);
    }

    /* Modify CMD_PROP_RX command for application needs */
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
    RF_cmdPropRx.maxPktLen = MAX_LENGTH;
    RF_cmdPropRx.pktConf.bRepeatOk = 0;
    RF_cmdPropRx.pktConf.bRepeatNok = 0;
    RF_cmdPropRx.pktConf.bFsOff = 0;
    
    RF_cmdPropRx.startTrigger.triggerType = TRIG_NOW;

    /* Request access to the radio */
    rfHandle = RF_open(&rfObject, &RF_prop,
                       (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);

    // Set highest possible priority
    uint32_t swiPriority = ~0;
    RF_Stat status = RF_control(rfHandle, RF_CTRL_SET_SWI_PRIORITY, &swiPriority);
    if (status != RF_StatBusyError) {
      LOG_INFO("Priority changed.\n");
    }

    // start the RAT timer synchronized to RTC
    //RF_postCmd(rfHandle, (RF_Op*)&RF_cmdSyncStartRAT, RF_PriorityNormal, NULL, 0);
    //LOG_INFO("Start RAT cmd posted.\n");    

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    LOG_INFO("FS cmd posted.\n");    

    /* Enter RX mode and stay forever in RX */
    //RF_EventMask terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
    //                                           RF_PriorityNormal, &callback,
    //                                           RF_EventRxEntryDone);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRx,
                                               RF_PriorityHighest , &rx_tester_callback, RF_EventCmdDone);

    while(true) {
      //LOG_INFO("------------------------------------------------\n");
      //LOG_INFO("Pausing till RX received.\n");
      while(rx_callback_called != true) {
        watchdog_periodic();
        //PROCESS_PAUSE();
      }
      rx_callback_called = false;
      //LOG_INFO("RX callback called.\n");

      //uint8_t i;
      //LOG_INFO("Packet: ");
      //for (i = 0; i < GLOSSY_PAYLOAD_LEN_WITH_COUNT ; i++)
      //{
      //  printf("%u", packet[i]);
      //}
      //printf("\n");

      printf("%u", packet[0]);
      printf("\n");

      //LOG_INFO("Timestamp: %lu.\n", rxTimestamp);
      //LOG_INFO("Last packet time diff >>>>>>>>>>>> : %lu.\n", rxTimestamp-rxLastTimestamp);
      //LOG_INFO("------------------------------------------------\n");
      //rxLastTimestamp = rxTimestamp;
    }

    //switch(terminationReason)
    //{
    //    case RF_EventCmdDone:
    //        // A radio operation command in a chain finished
    //        break;
    //    case RF_EventLastCmdDone:
    //        // A stand-alone radio operation command or the last radio
    //        // operation command in a chain finished.
    //        break;
    //    case RF_EventCmdCancelled:
    //        // Command cancelled before it was started; it can be caused
    //        // by RF_cancelCmd() or RF_flushCmd().
    //        break;
    //    case RF_EventCmdAborted:
    //        // Abrupt command termination caused by RF_cancelCmd() or
    //        // RF_flushCmd().
    //        break;
    //    case RF_EventCmdStopped:
    //        // Graceful command termination caused by RF_cancelCmd() or
    //        // RF_flushCmd().
    //        break;
    //    default:
    //        // Uncaught error event
    //        while(1);
    //}

    //uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx)->status;
    //switch(cmdStatus)
    //{
    //    case PROP_DONE_OK:
    //        // Packet received with CRC OK
    //        break;
    //    case PROP_DONE_RXERR:
    //        // Packet received with CRC error
    //        break;
    //    case PROP_DONE_RXTIMEOUT:
    //        // Observed end trigger while in sync search
    //        break;
    //    case PROP_DONE_BREAK:
    //        // Observed end trigger while receiving packet when the command is
    //        // configured with endType set to 1
    //        break;
    //    case PROP_DONE_ENDED:
    //        // Received packet after having observed the end trigger; if the
    //        // command is configured with endType set to 0, the end trigger
    //        // will not terminate an ongoing reception
    //        break;
    //    case PROP_DONE_STOPPED:
    //        // received CMD_STOP after command started and, if sync found,
    //        // packet is received
    //        break;
    //    case PROP_DONE_ABORT:
    //        // Received CMD_ABORT after command started
    //        break;
    //    case PROP_ERROR_RXBUF:
    //        // No RX buffer large enough for the received data available at
    //        // the start of a packet
    //        break;
    //    case PROP_ERROR_RXFULL:
    //        // Out of RX buffer space during reception in a partial read
    //        break;
    //    case PROP_ERROR_PAR:
    //        // Observed illegal parameter
    //        break;
    //    case PROP_ERROR_NO_SETUP:
    //        // Command sent without setting up the radio in a supported
    //        // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
    //        break;
    //    case PROP_ERROR_NO_FS:
    //        // Command sent without the synthesizer being programmed
    //        break;
    //    case PROP_ERROR_RXOVF:
    //        // RX overflow observed during operation
    //        break;
    //    default:
    //        // Uncaught error event - these could come from the
    //        // pool of states defined in rf_mailbox.h
    //        while(1);
    //}

    LOG_INFO("RX Process Finished.\n");

    PROCESS_END();
}

//void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
//{
//    if (e & RF_EventRxEntryDone)
//    {
//        /* Get current unhandled data entry */
//        currentDataEntry = RFQueue_getDataEntry();
//
//        /* Handle the packet data, located at &currentDataEntry->data:
//         * - Length is the first byte with the current configuration
//         * - Data starts from the second byte */
//        packetLength      = *(uint8_t*)(&currentDataEntry->data);
//        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);
//
//        /* Copy the payload + the status byte to the packet variable */
//        memcpy(packet, packetDataPointer, (packetLength + 1));
//
//        RFQueue_nextEntry();
//    }
//}
