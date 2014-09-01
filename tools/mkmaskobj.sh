#!/bin/sh

W=`echo $1 | gsed -e 's/.*_\([0-9]\+\)x.*/\\1/'`
H=`echo $1 | gsed -e 's/.*_[0-9]\+x\([0-9]\+\).*/\\1/'`

mkdir $2

mkmask $1 $2/mask
echo "$W $H 0" > $2/base

rm -rf base mask
