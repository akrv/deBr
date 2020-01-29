import serial
import csv
import string

def main():
    ser = serial.Serial('/dev/ttyACM0', 115200)

    filename = 'glossy_data.csv'
    fields = ['no_pkts_all', 'no_pkts_crc_ok', 'per']
    with open(filename, 'w') as csvfile: 
        csvwriter = csv.writer(csvfile) 
        csvwriter.writerow(fields)
        ser.flushInput()
        line = ser.readline() # ignore first flood
        for i in range(100): # collect measurements of 100 glossy floods
            ser.flushInput()
            line = ser.readline() 
            line = line[:-1]
            data = list(line.split(","))
            csvwriter = csv.writer(csvfile) 
            csvwriter.writerow(data)

if __name__ == "__main__":
    main()