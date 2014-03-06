#!/bin/sh

VERSION=$(head -n 1 ../debian/changelog |cut -d \( -f 2 |cut -d \) -f 1)

sed -e "s/VERSION/$VERSION/g" $1.in >$1

