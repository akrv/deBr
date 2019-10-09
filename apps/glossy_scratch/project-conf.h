#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_LEVEL_APP LOG_LEVEL_DBG

#define INITIATOR_ID    1
#define NODE_ID    1
#define N_TX  3

#define GLOSSY_PAYLOAD_LEN 8
#define GLOSSY_PAYLOAD_LEN_WITH_COUNT GLOSSY_PAYLOAD_LEN+1
#define GLOSSY_N_TX                     3
#define GLOSSY_T_SLOT                   (uint32_t) RF_convertMsToRatTicks(50) //TODO this should mean 50 ms check that


/* Packet TX Configuration */
/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             30 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     5  /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * Max 30 payload bytes
                                   * 4 Timestamp */

#define VHT_LF_UPDATE_CYCLES 3 // specify after how many cycles of LF CLK does the VHT update h0
// not all the values can be really achieved as when using any of the channels
// because the compare value event works only if scheduled 4 cycles in advance
// and even value bigger than 4 (4-10) achieve faster rates but not as
// exactly specified rate. So from (3-10), the value 3 is the fastest rate
// so the rate for value 3 is around 8 ticks

#endif /* PROJECT_CONF_H_ */
