import sys, os
import time
import argparse
import serial

client = None
READING_SEEING = 'A'
READING_RAW = 'B'

parser = argparse.ArgumentParser(description='Use Airylab Solar Scintillation Monitor to start/stop Planetary Imager recording based on seeing')
parser.add_argument('serial_port', help='Serial port to use for SSM communication')
parser.add_argument('-t', '--trigger', type=float, help='Seeing value in arcseconds. When seeing is lower than <trigger>, recording will start. Default: None (will just watch for SSM data)')
parser.add_argument('-p', '--planetary-imager-host', help='PlanetaryImager host (default: localhost)', default='localhost')
args = parser.parse_args()

def handle_recording(seeing, client, trigger):
    if client.capture.is_recording:
        if seeing > trigger:
            client.capture.end_recording()
    else:
        if seeing <= trigger:
            client.capture.start_recording()

if args.trigger is not None:
    from planetaryimager import PlanetaryImagerClient
    client = PlanetaryImagerClient(args.planetary_imager_host, autoconnect=True)

with serial.Serial(args.serial_port, 115000, timeout=2) as ser:
    readings = {}
    while True:
        line = ser.readline().decode()
        if not line:
            continue
        reading_type = line[0]
        reading_value = float(line[1:5])
        if reading_type == READING_SEEING:
            readings = {}
        readings[reading_type] = reading_value

        if readings.get(READING_SEEING) and readings.get(READING_RAW):
            print('seeing={}, raw={}'.format(readings[READING_SEEING], readings[READING_RAW]))
            if client and args.trigger > 0:
                handle_recording(readings[READING_SEEING], client, args.trigger)
                
        
