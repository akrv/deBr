#!/bin/bash

# This script copies the collected data from the sensor-floor RPI

cd ~/sensor_floor/deBr/apps/glossy_sensorfloor_test
mkdir -p data_all/data_N_6

sshpass -p "raspberry" scp pi@129.217.152.74:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_1.txt
sshpass -p "raspberry" scp pi@129.217.152.111:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_2.txt
sshpass -p "raspberry" scp pi@129.217.152.79:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_3.txt
sshpass -p "raspberry" scp pi@129.217.152.54:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_4.txt
sshpass -p "raspberry" scp pi@129.217.152.86:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_5.txt
sshpass -p "raspberry" scp pi@129.217.152.89:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_6.txt
sshpass -p "raspberry" scp pi@129.217.152.84:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_7.txt
sshpass -p "raspberry" scp pi@129.217.152.119:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_8.txt
sshpass -p "raspberry" scp pi@129.217.152.77:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_9.txt
sshpass -p "raspberry" scp pi@129.217.152.118:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_10.txt
sshpass -p "raspberry" scp pi@129.217.152.69:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_11.txt
sshpass -p "raspberry" scp pi@129.217.152.59:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_12.txt
sshpass -p "raspberry" scp pi@129.217.152.85:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_13.txt
sshpass -p "raspberry" scp pi@129.217.152.48:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_14.txt
sshpass -p "raspberry" scp pi@129.217.152.63:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_15.txt
sshpass -p "raspberry" scp pi@129.217.152.50:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_16.txt
sshpass -p "raspberry" scp pi@129.217.152.37:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_17.txt
sshpass -p "raspberry" scp pi@129.217.152.60:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_18.txt
sshpass -p "raspberry" scp pi@129.217.152.64:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_19.txt
sshpass -p "raspberry" scp pi@129.217.152.62:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_20.txt
sshpass -p "raspberry" scp pi@129.217.152.51:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_21.txt
sshpass -p "raspberry" scp pi@129.217.152.87:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_22.txt
sshpass -p "raspberry" scp pi@129.217.152.33:/home/pi/sensorfloor/floor_flasher/data.txt data_all/data_N_6/data_23.txt
