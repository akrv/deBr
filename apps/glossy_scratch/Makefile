GLOSSY_DIR := ../..

PLATFORMS_ONLY = simplelink

MAKE_MAC = MAKE_MAC_NULLMAC
MAKE_NET = MAKE_NET_NULLNET

MODULES_REL += ./smartrf_settings
PROJECT_SOURCEFILES += RFQueue.c

PROJECT_SOURCEFILES += glossy.c vht.c ftoa.c

CONTIKI_PROJECT = glossy_scratch rx_tester rx_meas_t
all: $(CONTIKI_PROJECT)
CONTIKI = $(GLOSSY_DIR)/contiki-ng
include $(CONTIKI)/Makefile.include
