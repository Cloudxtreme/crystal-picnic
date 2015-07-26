#!/bin/sh

cp -a data __data.tmp__

cd __data.tmp__/areas/tiles

../../../packtiles2.exe .png *

for f in `find . -name "*_new.png"` ; do mv $f `echo $f | sed -e 's/_new//'` ; done

cd ../..

FILES=`find . -type f | sort`

echo "Writing header..."
# the big space is a tab
du -bc $FILES | grep "	total$" | cut -f1 > ../build/data.cpa

echo "Writing data..."
cat $FILES >> ../build/data.cpa

echo "Writing info..."
# sed removed "./" from beginning of filenames
du -b $FILES | sed -e 's|./||' >> ../build/data.cpa

cd ..

echo "Saving uncompressed archive..."
cp build/data.cpa build/data.cpa.uncompressed

echo "Compressing..."
gzip build/data.cpa
mv build/data.cpa.gz build/data.cpa

rm -rf __data.tmp__