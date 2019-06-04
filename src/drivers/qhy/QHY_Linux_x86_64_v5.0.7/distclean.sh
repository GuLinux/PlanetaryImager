#!/bin/bash
#####################################################################################################################
# Description 	: This script uninstalls all the necessary files (libraries, firmwares, udev, include) on Linux machine. 
# Written by  	: MiaoYongQiang (davis_miao@126.com)
# Version      	: V4.0.13
# Date  		: 2019-02-18
#####################################################################################################################

# uninstall QHYCCD SDK



find /lib -name "libqhyccd*" -exec rm -f {} \;
find /usr/lib -name "libqhyccd*" -exec rm -f {} \;
find /usr/local/lib -name "libqhyccd*" -exec rm -f {} \;

find /lib -name "QHY*.HEX" -exec rm -f {} \;
find /lib -name IC16200A.HEX  -exec rm -f {} \;
find /lib -name IC16803.HEX  -exec rm -f {} \;
find /lib -name IC8300.HEX -exec rm -f {} \;
find /lib -name IC90A.HEX -exec rm -f {} \;
find /lib -name IMG0H.HEX -exec rm -f {} \;
find /lib -name IMG2P.HEX -exec rm -f {} \;
find /lib -name IMG2S.HEX -exec rm -f {} \;
find /lib -name IMG50.HEX -exec rm -f {} \;
find /lib -name POLEMASTER.HEX -exec rm -f {} \;
find /lib -name miniCam5.HEX -exec rm -f {} \;
find /lib -name "QHY*.img" -exec rm -f {} \;

find /usr/lib -name "QHY*.HEX" -exec rm -f {} \;
find /usr/lib -name IC16200A.HEX  -exec rm -f {} \;
find /usr/lib -name IC16803.HEX  -exec rm -f {} \;
find /usr/lib -name IC8300.HEX -exec rm -f {} \;
find /usr/lib -name IC90A.HEX -exec rm -f {} \;
find /usr/lib -name IMG0H.HEX -exec rm -f {} \;
find /usr/lib -name IMG2P.HEX -exec rm -f {} \;
find /usr/lib -name IMG2S.HEX -exec rm -f {} \;
find /usr/lib -name IMG50.HEX -exec rm -f {} \;
find /usr/lib -name POLEMASTER.HEX -exec rm -f {} \;
find /usr/lib -name miniCam5.HEX -exec rm -f {} \;
find /usr/lib -name "QHY*.img" -exec rm -f {} \;

find /usr/local -name "QHY*.HEX" -exec rm -f {} \;
find /usr/local -name IC16200A.HEX  -exec rm -f {} \;
find /usr/local -name IC16803.HEX  -exec rm -f {} \;
find /usr/local -name IC8300.HEX -exec rm -f {} \;
find /usr/local -name IC90A.HEX -exec rm -f {} \;
find /usr/local -name IMG0H.HEX -exec rm -f {} \;
find /usr/local -name IMG2P.HEX -exec rm -f {} \;
find /usr/local -name IMG2S.HEX -exec rm -f {} \;
find /usr/local -name IMG50.HEX -exec rm -f {} \;
find /usr/local -name POLEMASTER.HEX -exec rm -f {} \;
find /usr/local -name miniCam5.HEX -exec rm -f {} \;
find /usr/local -name "QHY*.img" -exec rm -f {} \;



rm -f  /lib/udev/rules.d/85-qhy.rules

rm -f  /etc/udev/rules.d/85-qhy.rules

rm -f  /lib/udev/rules.d/85-qhyccd.rules

rm -f  /etc/udev/rules.d/85-qhyccd.rules

rm -f  /sbin/fxload

rm -rf  /usr/local/cmake_modules

rm -f  /usr/local/doc/HowTo_QHYCCD_SDK_CrossPlatform.pdf

rm -rf /usr/local/fx3load

rm -f  /usr/local/include/config.h
rm -f  /usr/local/include/qhyccd*.h

rm -f  /usr/local/lib/libqhyccd*
rm -rf  /lib/firmware/qhy/*

rm -rf /usr/local/testapp/*

rm -f  /usr/local/udev/85-qhyccd.rules

rm -f  /usr/local/udev/85-qhy.rules

rm -f  /usr/share/usb/a3load.hex
