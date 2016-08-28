#!/usr/bin/python3
import json
import sys
import os

file = sys.argv[1]
json_file = open(file + ".txt")
file_info = json.load(json_file)

width = file_info["width"]
height = file_info["height"]
frames = file_info["total-frames"]
Bpp = file_info["bpp"] / 8
planes = file_info["channels"]
single_frame_size = width * height * Bpp * planes

print("Image size: {}x{}, frames: {}".format(width, height, frames))

expected_file_size = single_frame_size*frames + 178 + (frames*8)
current_size = os.path.getsize(file)

print("Size comparison: \nE={}\nA={}".format(expected_file_size, current_size))
difference = current_size - expected_file_size
print("Bytes difference: {}\nFrames difference: {}".format(difference, difference/single_frame_size))
