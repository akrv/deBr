#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_LEVEL_APP LOG_LEVEL_DBG

/* Glossy definitions */
#define INITIATOR_ID    1
#define NODE_ID    1

#define GLOSSY_PAYLOAD_LEN 254 // Max 254, 1 byte used for Hops count and Radio max is 255
#define GLOSSY_PAYLOAD_LEN_WITH_COUNT GLOSSY_PAYLOAD_LEN+1
#define GLOSSY_N_TX                     3
#define GLOSSY_FLOOD_TIME                   (uint32_t) RF_convertMsToRatTicks(50) // Time between floods [in RAT ticks]
// ----------------------------
#define GLOSSY_T_SLOT                       (uint32_t) RF_convertMsToRatTicks(5) // Time between Pkts in Floods [in RAT ticks]
// configure Rate in smartrf_settings.c
// RATE            PKT BYTES        TIME(ms) one packet        Flood_Time
// 50kpbs 2-GFSK
//                   8                1
//                   64               1
//                   128              1
//                   256              1
// 200kpbs 2-GFSK
//                   8                1
//                   64               1
//                   128              1
//                   256              1
// 500kpbs 2-GFSK
//                   8                1
//                   64               1
//                   128              1
//                   256              1
// ----------------------------
#define GLOSSY_CONF_COLLECT_STATS 1

/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             GLOSSY_PAYLOAD_LEN_WITH_COUNT  /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     5  // Appended bytes contain: 
                                  //  1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                  //  4 Timestamp */

#endif /* PROJECT_CONF_H_ */
