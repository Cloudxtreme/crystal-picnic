#!/bin/sh

FILES=`find . -type f | sort`

echo "Writing header..."
# the big space is a tab
du -bc $FILES | grep "	total$" | cut -f1 > $1

echo "Writing data..."
cat $FILES >> $1

echo "Writing info..."
# sed removed "./" from beginning of filenames
du -b $FILES | sed -e 's|./||' >> $1

echo "Saving uncompressed archive..."
cp $1 $1.uncompressed

echo "Compressing..."
gzip $1
mv $1.gz $1
