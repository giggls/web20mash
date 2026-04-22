#!/bin/bash
#
# Persist configuration in readonly mode for
# Raspberry Pi OS Trixie overlayroot system
#
# Do this by:
# mounting fs rw, copy mashctld.conf and mounting fs rw again
#

TMP_CFG=/tmp/mashctld.conf
TARGET_CFG=/etc/mashctld.conf

# exit if not mounted ro
if ! mount |grep -q -e '^/dev/mmcblk0p2.* (ro'; then
  exit 0
fi

# make shure we run as root
if [ $(id -u) != 0 ]; then
  exec sudo $0
fi

mount -o remount,rw /dev/mmcblk0p2
cp $TMP_CFG /media/root-ro$TARGET_CFG
mount -o remount,ro /dev/mmcblk0p2

