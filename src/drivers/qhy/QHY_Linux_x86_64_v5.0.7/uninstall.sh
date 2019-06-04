#!/bin/bash
#####################################################################################################################
# Description 	: This script uninstalls all the necessary files (libraries, firmwares, udev, include) on Linux machine. 
# Written by  	: MiaoYongQiang (davis_miao@126.com)
# Version      	: V4.0.13
# Date  		: 2019-02-18
#####################################################################################################################

# uninstall QHYCCD SDK

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

rm -f  /usr/local/udev/85-qhy.rules

rm -f  /usr/local/udev/85-qhyccd.rules

rm -f  /usr/share/usb/a3load.hex
