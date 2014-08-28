#!/bin/sh

X=`echo $1 | gsed -e 's/.*_\([0-9]\+\)-.*/\\1/'`
Y=`echo $1 | gsed -e 's/.*_[0-9]\+-\([0-9]\+\).*/\\1/'`
W=`echo $1 | gsed -e 's/.*_\([0-9]\+\)x.*/\\1/'`
H=`echo $1 | gsed -e 's/.*_[0-9]\+x\([0-9]\+\).*/\\1/'`

X=$(($X+$W/2))

CLEAN=`echo $2 | sed -e 's/-/_/g'`

echo "$CLEAN = add_entity(\"$2\", $3, $X, $Y)"
