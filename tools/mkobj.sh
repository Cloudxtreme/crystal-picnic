#!/bin/sh

W=`echo $1 | gsed -e 's/.*_\([0-9]\+\)x.*/\\1/'`
H=`echo $1 | gsed -e 's/.*_[0-9]\+x\([0-9]\+\).*/\\1/'`

#convert $1 $2.png

echo "<only>" > $2.xml
echo "	<x>0</x>" >> $2.xml
echo "	<y>0</y>" >> $2.xml
echo "	<width>$W</width>" >> $2.xml
echo "	<height>$H</height>" >> $2.xml
echo "</only>" >> $2.xml

echo "$3 $4 $5 $6 $7" > base

zip $2.zip $2.png $2.xml base

rm -rf $2.png $2.xml base
