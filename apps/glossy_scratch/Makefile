GLOSSY_DIR := ../..

#PLATFORMS_ONLY = simplelink
#PLATFORMS_ONLY = cc26x0-cc13x0

MAKE_MAC = MAKE_MAC_NULLMAC
MAKE_NET = MAKE_NET_NULLNET

PROJECT_SOURCEFILES += glossy.c vht.c ftoa.c

CONTIKI_PROJECT = glossy_scratch
all: $(CONTIKI_PROJECT)
CONTIKI = $(GLOSSY_DIR)/contiki-ng
include $(CONTIKI)/Makefile.include
