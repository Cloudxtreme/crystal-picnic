#!/bin/sh

if [ ! -d data ]; then
  echo "No ./data directory, make sure to run this script from the root of the repo."
  exit 1
fi

ROOT=$(pwd)
mkdir -p ${ROOT}/build

if [ ! -x "${ROOT}/tools/packtiles2" ]; then
  echo "The tools/packtiles2 script seems missing, trying to build it with GCC."
  gcc ${ROOT}/tools/packtiles2.cpp -lallegro -lallegro_image -lstdc++ -o tools/packtiles2
fi

cp -a data __data.tmp__
cd __data.tmp__/areas/tiles
${ROOT}/tools/packtiles2 .png *

for f in `find . -name "*_new.png"` ; do mv $f `echo $f | sed -e 's/_new//'` ; done

cd ${ROOT}/__data.tmp__

FILES=`find . -type f | sort`

echo "Writing header..."
# the big space is a tab
du -bc $FILES | grep "	total$" | cut -f1 > ${ROOT}/build/data.cpa

echo "Writing data..."
cat $FILES >> ${ROOT}/build/data.cpa

echo "Writing info..."
# sed removed "./" from beginning of filenames
du -b $FILES | sed -e 's|./||' >> ${ROOT}/build/data.cpa

cd ${ROOT}

echo "Saving uncompressed archive..."
cp build/data.cpa build/data.cpa.uncompressed

echo "Compressing..."
gzip build/data.cpa
mv build/data.cpa.gz build/data.cpa

rm -rf __data.tmp__