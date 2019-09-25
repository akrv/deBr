#!/usr/bin/env bash
make distclean
make
~/ti/uniflash_*.*.0/dslite.sh --config=targetConfigs/CC1350F128.ccxml tx.simplelink -O PinReset
# sleep 1 
# screen /dev/ttyACM0 115200
