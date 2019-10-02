#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_LEVEL_APP LOG_LEVEL_DBG

#define INITIATOR_ID    1
#define NODE_ID    2
#define N_TX  3

#define GLOSSY_PAYLOAD_LEN 8
#define GLOSSY_PAYLOAD_LEN_WITH_COUNT GLOSSY_PAYLOAD_LEN+1
#define GLOSSY_N_TX                     3

#define VHT_LF_UPDATE_CYCLES 3 // specify after how many cycles of LF CLK does the VHT update h0
// not all the values can be really achieved as when using any of the channels
// because the compare value event works only if scheduled 4 cycles in advance
// and even value bigger than 4 (4-10) achieve faster rates but not as
// exactly specified rate. So from (3-10), the value 3 is the fastest rate
// so the rate for value 3 is around 8 ticks

#endif /* PROJECT_CONF_H_ */
