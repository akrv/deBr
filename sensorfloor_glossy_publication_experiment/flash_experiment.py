#!/usr/bin/python3

import os, requests, sys, json
import asyncio
import argparse
import random

RPi_IPs = [
            {"column_num": 1, "ip_addr": "129.217.152.74", "mac_id": "b8:27:eb:41:99:a0", "hostname": "raspberrypi"},
            {"column_num": 2, "ip_addr": "129.217.152.111", "mac_id": "b8:27:eb:c0:fd:6a", "hostname": "raspberrypi"},
            {"column_num": 3, "ip_addr": "129.217.152.79", "mac_id": "b8:27:eb:18:92:c7", "hostname": "raspberrypi"},
            {"column_num": 4, "ip_addr": "129.217.152.54", "mac_id": "b8:27:eb:53:f2:33", "hostname": "raspberrypi"},
            {"column_num": 5, "ip_addr": "129.217.152.86", "mac_id": "b8:27:eb:e7:6f:dc", "hostname": "raspberrypi"},
            {"column_num": 6, "ip_addr": "129.217.152.89", "mac_id": "b8:27:eb:38:4b:07", "hostname": "raspberrypi"},
            {"column_num": 7, "ip_addr": "129.217.152.84", "mac_id": "b8:27:eb:1b:cf:26", "hostname": "raspberrypi"},
            {"column_num": 8, "ip_addr": "129.217.152.119", "mac_id": "b8:27:eb:6d:0e:53", "hostname": "raspberrypi"},
            {"column_num": 9, "ip_addr": "129.217.152.77", "mac_id": "b8:27:eb:b7:a3:b7", "hostname": "raspberrypi"},
            {"column_num": 10, "ip_addr": "129.217.152.118", "mac_id": "b8:27:eb:be:dc:32", "hostname": "raspberrypi"},
            {"column_num": 11, "ip_addr": "129.217.152.69", "mac_id": "b8:27:eb:ff:a4:48", "hostname": "raspberrypi"},
            {"column_num": 12, "ip_addr": "129.217.152.59", "mac_id": "b8:27:eb:a9:7d:4d", "hostname": "raspberrypi"},
            {"column_num": 13, "ip_addr": "129.217.152.85", "mac_id": "b8:27:eb:c4:f8:c7", "hostname": "raspberrypi"},
            {"column_num": 14, "ip_addr": "129.217.152.48", "mac_id": "b8:27:eb:e4:43:6d", "hostname": "raspberrypi"},
            {"column_num": 15, "ip_addr": "129.217.152.63", "mac_id": "b8:27:eb:98:69:6e", "hostname": "raspberrypi"},
            {"column_num": 16, "ip_addr": "129.217.152.50", "mac_id": "b8:27:eb:75:c7:a2", "hostname": "raspberrypi"},
            {"column_num": 17, "ip_addr": "129.217.152.37", "mac_id": "b8:27:eb:09:3d:77", "hostname": "raspberrypi"},
            {"column_num": 18, "ip_addr": "129.217.152.60", "mac_id": "b8:27:eb:05:d8:4d", "hostname": "raspberrypi"},
            {"column_num": 19, "ip_addr": "129.217.152.64", "mac_id": "b8:27:eb:36:da:22", "hostname": "raspberrypi"},
            {"column_num": 20, "ip_addr": "129.217.152.62", "mac_id": "b8:27:eb:f5:5d:04", "hostname": "raspberrypi"},
            {"column_num": 21, "ip_addr": "129.217.152.51", "mac_id": "b8:27:eb:88:8d:56", "hostname": "raspberrypi"},
            {"column_num": 22, "ip_addr": "129.217.152.87", "mac_id": "b8:27:eb:00:be:93", "hostname": "raspberrypi"},
            {"column_num": 23, "ip_addr": "129.217.152.33", "mac_id": "b8:27:eb:c0:10:ae", "hostname": "raspberrypi"},
            ]

def send_flash_req(ip_addr, filename, devices):
    if filename:
        url = "http://" + ip_addr
        payload = {'device': devices}
        headers = {}
        files = [('file', open(filename, 'rb'))]
        response = requests.request("POST", url, headers=headers, data=payload, files=files)
        if response.status_code == 200:
            return (['success: ', ip_addr])
        else:
            return (['failed: ', ip_addr])
    else:
        return (['error', 'Please provide absolute path file name as arg'])

if __name__ == '__main__':
    nodes = list()
    for i in range(1,24):
        for j in range(1,16):
            if i==6: continue # broken strip
            elif i==7 and j==12: continue  # broken node
            elif i==23 and j==10: continue # broken node
            elif i==12 and j==8: continue # initiator
            elif i==12 and j==9: continue # tester
            nodes.append((i,j))
    random.shuffle(nodes)
    # flash initiator
    firmware_path = os.path.dirname(os.path.realpath(__file__)) + '/test_firmware/initiator.bin'
    strip_id, node_id = (12,8)
    print('Flashing initiator node strip_id:', strip_id, 'ip:', RPi_IPs[strip_id-1]['ip_addr'], 'node_id:', node_id)
    send_flash_req(RPi_IPs[strip_id-1]['ip_addr'], firmware_path, devices=node_id)

    # flash tester node
    firmware_path = os.path.dirname(os.path.realpath(__file__)) + '/test_firmware/tester.bin'
    strip_id, node_id = (12,9)
    print('Flashing tester node strip_id:', strip_id, 'ip:', RPi_IPs[strip_id-1]['ip_addr'], 'node_id:', node_id)
    send_flash_req(RPi_IPs[strip_id-1]['ip_addr'], firmware_path, devices=node_id)

    # flash 100 test node
    for i in range(10):
        print('group:', i)
        firmware_path = os.path.dirname(os.path.realpath(__file__)) + '/test_firmware/node_' + str(i) + '.bin'
        for j in range(10):
            strip_id, node_id = nodes.pop()
            print('Flashing node strip_id:', strip_id, 'ip:', RPi_IPs[strip_id-1]['ip_addr'], 'node_id:', node_id)
            send_flash_req(RPi_IPs[strip_id-1]['ip_addr'], firmware_path, devices=node_id)
