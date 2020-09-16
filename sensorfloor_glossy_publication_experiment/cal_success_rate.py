#!/usr/bin/python3
import csv

'''
This script reads the collected data from the nodes then calculate the statistics
'''

dir_name = 'data_all/data_N_6/after_4_days' # CHOOSE data directory
files = list()
for i in range(1,23):
    files.append(dir_name + '/data_' + str(i) + '.txt')

rates = list()
received_pkts = list()
for file in files:
    with open(file, 'r') as csvfile:
        reader = csv.reader(csvfile, skipinitialspace=True)
        for row in reader:
            try:
                rates.append(int(row[2]))
                received_pkts.append(int(row[1]))
            except:
                pass
print('no of nodes:', len(rates))
print(received_pkts)
print('received pkts max:', max(received_pkts))
print('received pkts avg:', sum(received_pkts)/len(received_pkts))
print('success rate avg:', sum(rates)/len(rates))
