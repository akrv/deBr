/***** Includes *****/
/* Contiki */
#include "contiki.h"

/* Standard C Libraries */
#include <stdlib.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* RF settings */
#include "smartrf_settings/smartrf_settings.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/***** Defines *****/

/* Packet TX Configuration */
#define PAYLOAD_LENGTH      30
#define PACKET_INTERVAL     (uint32_t)(4000000*0.5f) /* Set packet interval to 500ms */

/* Do power measurement */
//#define POWER_MEASUREMENT

/***** Prototypes *****/



/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

static uint8_t packet[PAYLOAD_LENGTH];
static uint16_t seqNumber;

static bool fs_callback_called = false;
static bool tx_callback_called = false;
/***** Function definitions *****/

/*---------------------------------------------------------------------------*/
void fs_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    fs_callback_called = true;
    if (e & RF_EventCmdDone)
    {
      // Do something
    }
}

void tx_callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    tx_callback_called = true;
    if (e & RF_EventTxDone)
    {
      // Do something
    }
    if (e & RF_EventError)
    {
      // Do something
    }
}
/*---------------------------------------------------------------------------*/
PROCESS(direct_radio_process, "Direct Radio Process");
AUTOSTART_PROCESSES(&direct_radio_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(direct_radio_process, ev, data)
{
    static uint32_t curtime;
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    //static RF_EventMask terminationReason;

    PROCESS_BEGIN();
    LOG_INFO("TX Process Begin.\n");

    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTx.startTrigger.pastTrig = 1;
    RF_cmdPropTx.startTime = 0;

    /* Request access to the radio */
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup, &rfParams);

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, &fs_callback, RF_EventCmdDone);

    LOG_INFO("Pausing till FS finish.\n");
    while(fs_callback_called != true) {
      PROCESS_PAUSE();
    }
    LOG_INFO("FS callback called.\n");

    /* Get current time */
    curtime = RF_getCurrentTime();
    //while(1)
    //{
        /* Create packet with incrementing sequence number and random payload */
        packet[0] = (uint8_t)(seqNumber >> 8);
        packet[1] = (uint8_t)(seqNumber++);
        uint8_t i;
        for (i = 2; i < PAYLOAD_LENGTH; i++)
        {
            //packet[i] = rand();
            packet[i] = i; 
        }
        LOG_INFO("Packet setup done.\n");
        LOG_INFO("Packet: ");
        for (i = 0; i < PAYLOAD_LENGTH; i++)
        {
            printf("%u", packet[i]);
        }
        printf("\n");

        /* Set absolute TX time to utilize automatic power management */
        curtime += PACKET_INTERVAL;
        RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW; 

        /* Send packet */
        RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                                   RF_PriorityNormal, &tx_callback, RF_EventTxDone);
        //terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
        //                                           RF_PriorityNormal, &tx_callback, RF_EventTxDone);


        LOG_INFO("Pausing till TX finish.\n");
        while(tx_callback_called != true) {
          PROCESS_PAUSE();
         }
        LOG_INFO("TX callback called.\n");

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
        //    // by RF_cancelCmd() or RF_flushCmd().
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

        //uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropTx)->status;
        //switch(cmdStatus)
        //{
        //    case PROP_DONE_OK:
        //        // Packet transmitted successfully
        //        break;
        //    case PROP_DONE_STOPPED:
        //        // received CMD_STOP while transmitting packet and finished
        //        // transmitting packet
        //        break;
        //    case PROP_DONE_ABORT:
        //        // Received CMD_ABORT while transmitting packet
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
        //    case PROP_ERROR_TXUNF:
        //        // TX underflow observed during operation
        //        break;
        //    default:
        //        // Uncaught error event - these could come from the
        //        // pool of states defined in rf_mailbox.h
        //        while(1);
        //}

    //}
    
    LOG_INFO("TX Process Finished.\n");

    PROCESS_END();
}
