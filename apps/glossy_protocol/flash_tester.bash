#!/usr/bin/env bash
make
~/ti/uniflash_*.*.0/dslite.sh --config=targetConfigs/CC1350F128.ccxml rx_tester.simplelink -O PinReset
