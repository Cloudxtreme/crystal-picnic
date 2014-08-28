#!/bin/bash

mkdir $1.zip

LAYER=$2

X1=$3
Y1=$4
X2=$5
Y2=$6

W=$((X2-X1))
H=$((Y2-Y1))
W2=$((W/2))
H2=$((H/2))

echo $((-W2)) -$H $W $H 0 > $1.zip/base

ID=`echo $1 | sed -e 's/-/_/g'`

echo "$ID = add_entity(\"$1\", $LAYER, $((X1+W2)), $Y2)"
echo "	if (colliding(id1, id2, 0, $ID)) then"
echo "		change_areas(\"dungeon1-\", x, x, x)"
echo "	end"
