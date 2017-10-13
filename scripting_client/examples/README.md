# Planetary Imager Scripting examples

Planetary Imager now offers a Python scripting API capable of controlling controls, recording and many other internal parameters.

This is a collection of sample scripts that you can use as it is, or modify to any need.

## Running

To run these scripts you need to have the `planetaryimager` module in your `PYTHONPATH` environment variable.

To achieve this, you can install it in your python modules directory, or simply run them using the provided script:
```
./examples/run_script.sh examples/script_name.py <script_arguments>
``` 

The `planetaryimager` module and these scripts were all developed and tested using python3. Compatibility with python2 should be possible, but it's not currently tested or supported.



## airylab_ssm

Simple script that connects to an [Airylab SSM](https://airylab.com/) device, and starts/stops recording based on the detected seeing.

### Usage:
```
usage: airylab_ssm.py [-t TRIGGER] [-p PLANETARY_IMAGER_HOST] serial_port
```

 - `serial_port` should point to the device associated with your SSM (usually /dev/ttyUSB0)
 - `PLANETARY_IMAGER_HOST` is the hostname for remote connection to Planetary Imager. Can be omitted, defaults to `localhost`.
 - `TRIGGER`: seeing value in arcseconds to control Planetary Imager:
   - if `TRIGGER` is not specified, the script will only print the values detected by the SSM device
   - if seeing is lower or equal than `TRIGGER`, recording will start
   - if seeing is higher than `TRIGGER`, recording will stop



