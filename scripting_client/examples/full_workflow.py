from planetaryimager import PlanetaryImagerClient
from functools import partial
import time
import os

planetary_imager = PlanetaryImagerClient()


def generic_callback(event, *args):
    print("-callback for event {}: {}".format(event, args))


# display available cameras
print(planetary_imager.cameras)

planetary_imager.imager.callbacks['on_camera_connected'] = partial(generic_callback, 'on_camera_connected')
planetary_imager.imager.callbacks['on_fps'] = partial(generic_callback, 'on_fps')
planetary_imager.imager.callbacks['on_temperature'] = partial(generic_callback, 'on_temperature')
planetary_imager.imager.callbacks['on_control_changed'] = partial(generic_callback, 'on_control_changed')
planetary_imager.imager.callbacks['on_camera_disconnected'] = partial(generic_callback, 'on_camera_disconnected')

# connect the first available camera
planetary_imager.imager.open(planetary_imager.cameras[0])
time.sleep(5)

# List controls
print(planetary_imager.imager.controls)

# Get a control by name
exposure = planetary_imager.imager.find_control(name='exposure')
print(exposure)

# Change control
exposure['val'] = 11.2
planetary_imager.imager.set_control(exposure)
print(planetary_imager.imager.controls)

# Set ROI

planetary_imager.imager.set_roi(2, 2, 100, 100)

time.sleep(10)

# Clear ROI

planetary_imager.imager.clear_roi()
time.sleep(2)


# Configuration

# Print configuration entries

print(planetary_imager.configuration.entries)


# Set save directory and filename scheme

planetary_imager.configuration.save_directory.value = '/tmp/'
planetary_imager.configuration.save_file_prefix.value = 'Moon'
planetary_imager.configuration.save_file_suffix.value = 'test'

# Set recording limit to infinite (see configuration.h in PlanetaryImager sources for enum values)
planetary_imager.configuration.recording_limit_type.value = 0

# Setup capture callbacks
planetary_imager.capture.callbacks['on_recording_started'] = partial(generic_callback, 'on_recording_started')
planetary_imager.capture.callbacks['on_recording_finished'] = partial(generic_callback, 'on_recording_finished')
planetary_imager.capture.callbacks['on_save_fps'] = partial(generic_callback, 'on_save_fps')
planetary_imager.capture.callbacks['on_save_mean_fps'] = partial(generic_callback, 'on_save_mean_fps')
planetary_imager.capture.callbacks['on_saved_frames'] = partial(generic_callback, 'on_saved_frames')
planetary_imager.capture.callbacks['on_dropped_frames'] = partial(generic_callback, 'on_dropped_frames')

# Start recording to the previously configured directory

planetary_imager.capture.start_recording()
time.sleep(1)
# Print the recording filename

print("Recording to: {}".format(planetary_imager.capture.recording_filename))

time.sleep(5)

# Pause recording

planetary_imager.capture.pause()

time.sleep(5)

# Resume recording

planetary_imager.capture.resume()

time.sleep(5)

# Stop recording

planetary_imager.capture.end_recording()

time.sleep(2)

# Close imager

planetary_imager.imager.close()

time.sleep(2)
