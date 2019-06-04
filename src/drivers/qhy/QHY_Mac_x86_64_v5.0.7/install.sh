#!/bin/bash
#####################################################################################################################
# Description 	: This script uninstalls all the necessary files (libraries, firmwares, udev, include) on MacOS machine. 
# Written by  	: MiaoYongQiang (davis_miao@126.com)
# Version      	: V4.0.13
# Date  		: 2019-02-18
#####################################################################################################################

# install firmware

mkdir -p /usr/local/cmake_modules
cp -a usr/local/cmake_modules/*  /usr/local/cmake_modules

mkdir -p /usr/local/doc
cp -a usr/local/doc/*  /usr/local/doc

mkdir -p /usr/local/include
cp -a usr/local/include/*  /usr/local/include

mkdir -p /usr/local/lib 
cp -a  usr/local/lib/* /usr/local/lib

mkdir -p /usr/local/testapp
cp -a usr/local/testapp/*  /usr/local/testapp


