#!/bin/sh

# This version runs on Windows using MSYS tools.

# You may need to change the following path... Windows has a built-in find.exe
# that doesn't work with this script.

FIND="/c/mingw/msys/1.0/bin/find"

xcopy //e //y data __data__.tmp\\

cd __data__.tmp/areas/tiles

# You need to build packtiles2.exe and put it in the root of this project

../../../packtiles2.exe .png *

for f in `$FIND . -name "*_new.png"` ; do mv $f `echo $f | sed -e 's/_new//'` ; done

cd ../..

FILES=`$FIND * -type f | grep -v README.txt | sort`

echo "Writing header..."
# the big space is a tab
du -bc $FILES | dos2unix | grep "	total$" | cut -f1 > ../build/data.cpa

echo "Writing data..."
cat $FILES >> ../build/data.cpa

echo "Writing info..."
du -b $FILES | dos2unix >> ../build/data.cpa

echo "Saving uncompressed archive..."
cp ../build/data.cpa ../build/data.cpa.uncompressed

echo "Compressing..."
gzip ../build/data.cpa
mv ../build/data.cpa.gz ../build/data.cpa

cd ..

rm -rf __data__.tmp
