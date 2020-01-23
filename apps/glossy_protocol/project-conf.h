#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_LEVEL_APP LOG_LEVEL_DBG

/* Glossy definitions */
#define INITIATOR_ID    1
#define NODE_ID    2

#define GLOSSY_PAYLOAD_LEN 8
#define GLOSSY_PAYLOAD_LEN_WITH_COUNT GLOSSY_PAYLOAD_LEN+1
#define GLOSSY_N_TX                     3
#define GLOSSY_T_SLOT                   (uint32_t) RF_convertMsToRatTicks(1) // in RAT ticks
#define GLOSSY_FLOOD_TIME                   (uint32_t) RF_convertMsToRatTicks(50) // in RAT ticks

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             30 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     5  // The Data Entries data field will contain: 
                                  //  1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                  //  Max 30 payload bytes
                                  //  4 Timestamp */

#endif /* PROJECT_CONF_H_ */
