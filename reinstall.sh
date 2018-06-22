#!/bin/bash


rmmod pnpipcinc
make -C /lib/modules/`uname -r`/build M=`pwd` modules

mkdir -p /lib/modules/`uname -r`/extra/
cp pnpipcinc.ko /lib/modules/`uname -r`/extra/
depmod -a
modprobe pnpipcinc
