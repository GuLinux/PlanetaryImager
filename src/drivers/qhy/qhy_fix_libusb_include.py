#!/usr/bin/python
import sys
import fileinput

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, 'r') as file:
  with open(output_file, 'w') as output:
    for line in file:
      output.write(line.replace('#include <libusb-1.0/libusb.h>', '#include <libusb.h>'))
