from planetaryimager import PlanetaryImagerClient
from functools import partial
import time
import os
import sys

planetary_imager = PlanetaryImagerClient()



#print(planetary_imager.imager.controls)

#print(planetary_imager.configuration.entries)


# Set save directory and filename scheme

profiles = [
    { 'filter': 'R', 'exposure': 25000 },
    { 'filter': 'G', 'exposure': 35000 },
    { 'filter': 'B', 'exposure': 65000 },
]

planetary_imager.configuration.save_directory.value = '/home/marco/PlanetaryImager'
os.makedirs(planetary_imager.configuration.save_directory.value, exist_ok=True)

# Set recording limit to infinite (see configuration.h in PlanetaryImager sources for enum values)
planetary_imager.configuration.recording_limit_type.value = 2
planetary_imager.configuration.recording_seconds_limit.value = 60

planetary_imager.configuration.save_file_prefix.value = 'Mars'

shot_finished = False
# Setup capture callbacks

def on_finished(*args, **kwargs):
    global shot_finished
    shot_finished = True

planetary_imager.capture.callbacks['on_recording_finished'] = on_finished

current_index = 0
if len(sys.argv) > 1:
    current_name = sys.argv[1]
    current_index = [index for index, profile in profiles if profile['name'] == current_name][0]
should_stop = False
while True:
    profile = profiles[current_index % 3]
    current_index += 1
    print('{:04d} - Start exposure for profile {}? [y/n]'.format(current_index+1, profile['filter']))


    # Get a control by name
    exposure = planetary_imager.imager.find_control(name='Exposure`')
#    print(exposure)

    # Change control
    exposure['val'] = profile['exposure']
    planetary_imager.imager.set_control(exposure)


    should_stop = input() != 'y'
    if should_stop:
        break
    planetary_imager.configuration.save_file_suffix.value = profile['filter']
    shot_finished = False

    planetary_imager.capture.start_recording()
    print("Recording to: {}".format(planetary_imager.capture.recording_filename))

    while not shot_finished:
        time.sleep(0.5)



