#!/bin/bash
#####################################################################################################################
# Description 	: This script installs all the necessary files (libraries, firmwares, udev, include) on Linux machine. 
# Written by  	: MiaoYongQiang (davis_miao@126.com)
# Version      	: V4.0.13
# Date  		: 2019-02-18
#####################################################################################################################

# install firmware

mkdir -p /lib/udev
mkdir -p /lib/udev/rules.d
cp -a lib/udev/rules.d/*  /lib/udev/rules.d

mkdir -p /etc/udev
mkdir -p /etc/udev/rules.d
cp -a etc/udev/rules.d/*  /etc/udev/rules.d

cp -a sbin/fxload /sbin/fxload

mkdir -p /usr/local/cmake_modules
cp -a usr/local/cmake_modules/*  /usr/local/cmake_modules

mkdir -p /usr/local/doc
cp -a usr/local/doc/*  /usr/local/doc

mkdir -p /usr/local/fx3load
cp -a usr/local/fx3load/*  /usr/local/fx3load

mkdir -p /usr/local/lib
cp -a usr/local/lib/*  /usr/local/lib

mkdir -p /usr/local/include
cp -a usr/local/include/*  /usr/local/include

mkdir -p /lib/firmware
mkdir -p /lib/firmware/qhy
cp -a  lib/firmware/qhy/*  /lib/firmware/qhy

mkdir -p /usr/local/testapp
cp -a usr/local/testapp/*  /usr/local/testapp

mkdir -p /usr/local/udev
cp -a usr/local/udev/*  /usr/local/udev

mkdir -p /usr/share/usb
cp -a usr/share/usb/*  /usr/share/usb

ldconfig
