#!/bin/sh

cp -a data __data.tmp__

cd data
DATA_FILES=`/c/mingw/msys/1.0/bin/find . -type f | sort`
cd ../__data.tmp__
FILES=`/c/mingw/msys/1.0/bin/find . -type f | sort`

if [ ! "$DATA_FILES" == "$FILES" ] ; then echo "cp on Windows sucks." ; exit ; fi

cd areas/tiles

../../../packtiles2.exe .png *

for f in `/c/mingw/msys/1.0/bin/find . -name "*_new.png"` ; do mv $f `echo $f | sed -e 's/_new//'` ; done

cd ../..


echo "Writing header..."
# the big space is a tab
du -bc $FILES | dos2unix | grep "	total$" | cut -f1 > ../build/data.cpa

echo "Writing data..."
cat $FILES >> ../build/data.cpa

echo "Writing info..."
# sed removed "./" from beginning of filenames
du -b $FILES | dos2unix | sed -e 's|./||' >> ../build/data.cpa

cd ..

echo "Saving uncompressed archive..."
cp build/data.cpa build/data.cpa.uncompressed

echo "Compressing..."
gzip build/data.cpa
mv build/data.cpa.gz build/data.cpa

rm -rf __data.tmp__
