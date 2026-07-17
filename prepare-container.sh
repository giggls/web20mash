#!/bin/bash

VERS=$(head -n 1 debian/changelog |cut -d \( -f 2 |cut -d \) -f 1)

PACKAGE=../web20mash_$VERS_*.deb

rm -f $(basename $PACKAGE)

sed -e "s/%PACKAGENAME%/$(basename $PACKAGE)/g" Containerfile.in >Containerfile

ln $PACKAGE .

