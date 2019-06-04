#!/bin/bash
#####################################################################################################################
# Description 	: This script uninstalls all the necessary files (libraries, firmwares, udev, include) on MacOS machine. 
# Written by  	: MiaoYongQiang (davis_miao@126.com)
# Version      	: V4.0.13
# Date  		: 2019-02-18
#####################################################################################################################

# uninstall QHYCCD SDK


rm -rf  /usr/local/cmake_modules

rm -f  /usr/local/doc/HowTo_QHYCCD_SDK_CrossPlatform.pdf


rm -f  /usr/local/include/config.h
rm -f  /usr/local/include/qhyccd*.h

rm -f  /usr/local/lib/libqhyccd*
rm -rf  /lib/firmware/qhy/*

rm -rf /usr/local/testapp/*

