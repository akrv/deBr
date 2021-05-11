# DeBr
Decentralized Brains in low data-rate, low power networks for collaborative maneuvers

DeBr is a communication protocol that can provide decentralized information propagation between large number of agents and 
at the same time meet the bandwidth and power limitations requirments.

This implementation of DeBr protocol runs on Texas Instruments CC1350 chip, which is an Ultra-Low-Power Dual-Band Wireless MCU. 
The chip hardware capabilities meets the design requirements of DeBr protocol. DeBr utilizes Sub-1-GHz frequency band.

## Installation

DeBr protocol depend on [Contini-NG](https://github.com/contiki-ng/contiki-ng) OS.
After installing Contiki-NG and the required tool chain, intialize the Contiki-NG 
submodule included in the repo. It will pull the correct release of Contiki-NG used by DeBr.
```
cd deBr
$ git submodule update --init --recursive
```
to make sure everything installed correctly, try to compile the project:
```
cd deBr/apps/glossy_protocol
make
```

## Test application
A test application is provided in this repo in `deBr/apps/glossy_protocol`.

### Flashing the test application
Flashing can be done using [Contiki-NG](https://www.contiki-ng.org/resources/contiki-ng-cheat-sheet.pdf) with a USB-to-Serial converter or
[UNIFLASH](https://www.ti.com/tool/UNIFLASH) from Texas Instrument if you have the dev-kit. Next instuctions uses UNIFLASH.

To flash the initiator node, change the NODE_ID in `deBr/apps/glossy_protocol/project-conf.h` to 1:
```
#define NODE_ID         1
```
then run these command to flash the initiator node. `dslite.sh` is a script provided by the UNIFLASH TOOL. 
If the default installation directory for the UNIFLASH tool is changed then change it in the next command.
```
cd deBr/apps/glossy_protocol
make
~/ti/uniflash_*.*.*/dslite.sh --config=targetConfigs/CC1350F128.ccxml glossy_test_app.simplelink -O PinReset
```

To flash all of the non-initiator node, change the NODE_ID in `deBr/apps/glossy_protocol/project-conf.h` to 2:
```
#define NODE_ID         2
```
then run next commands to flash all the non-initiator node.
```
cd deBr/apps/glossy_protocol
make
~/ti/uniflash_*.*.*/dslite.sh --config=targetConfigs/CC1350F128.ccxml glossy_test_app.simplelink -O PinReset
```
DeBr protocol then should be running. To make sure that the protocol is working correctly, connect one of the non-initiator node to 
a serial terminal and it should be printing:
```
19551, 19623, 9963
```
First number is the number of packets that started to be recived. Second is the number of the packets correctly received.
Third is the success percentage (here 99.63%).

## Citation
If you use this code for your research, please consider citing this publication.

First publication explaning DeBr protocol structure and applications:
```
@INPROCEEDINGS{8637327,
  author={A. K. {Ramachandran Venkatapathy} and A. {Ekblaw} and M. {ten Hompel} and J. {Paradiso}},
  booktitle={2018 6th IEEE International Conference on Wireless for Space and Extreme Environments (WiSEE)}, 
  title={Decentralized brain in low data-rate, low power networks for collaborative manoeuvres in space}, 
  year={2018},
  volume={},
  number={},
  pages={83-88},}
```

Second publication explains the code in this repository in details. The paper test the protocol on a test bed of 345 nodes ([sensor-floor](https://github.com/akrv/sensorfloor)).

```
@InProceedings{10.1007/978-3-030-71061-3_6,
author="Ramachandran Venkatapathy, Aswin Karthik and Gouda, Anas and ten Hompel, Michael and Paradiso, Joseph",
editor="Penalver, Lourdes and Parra, Lorena",
title="Decentralized Brains: A Reference Implementation with Performance Evaluation",
booktitle="Industrial IoT Technologies and Applications",
year="2021",
publisher="Springer International Publishing",
pages="80--99",
isbn="978-3-030-71061-3"
}
```