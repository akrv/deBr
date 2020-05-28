#!/usr/bin/python3
import csv

dir_name = 'data_all/data_N_2/'
files = list()
for i in range(1,23):
    files.append(dir_name + '/data_' + str(i) + '.txt')
print files

rates = list()
for file in files:
    with open(file, 'r') as csvfile:
        reader = csv.reader(csvfile, skipinitialspace=True)
        print reader
        for row in reader:
            rates.append(int(row[2]))
print('lens:', len(rates))
print('avg:', sum(rates)/len(rates))