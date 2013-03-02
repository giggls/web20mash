#!/bin/bash

TMP_CFG=/tmp/mashctld.conf
TARGET_CFG=/etc/mashctld.conf

# make shure we run as root
if [ $(id -u) != 0 ]; then
  exec sudo $0
fi

mount -o remount,rw /
cp $TMP_CFG $TARGET_CFG
mount -o remount,ro /


  