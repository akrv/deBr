#!/bin/sh
GLOSSY_DIR := ../..

PLATFORMS_ONLY = simplelink

MAKE_MAC = MAKE_MAC_NULLMAC
MAKE_NET = MAKE_NET_NULLNET

PROJECT_SOURCEFILES += glossy.c RFQueue.c
MODULES_REL += ./smartrf_settings

CONTIKI_PROJECT = glossy_test_app
all: $(CONTIKI_PROJECT)
CONTIKI = $(GLOSSY_DIR)/contiki-ng
include $(CONTIKI)/Makefile.include
