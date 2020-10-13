CONTIKI_PROJECT = tester tx_node
all: $(CONTIKI_PROJECT)

CONTIKI = ../../contiki-ng

PLATFORMS_EXCLUDE = nrf52dk

#use this to enable TSCH: MAKE_MAC = MAKE_MAC_TSCH
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
