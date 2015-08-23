#!/bin/bash

ROOT=$(pwd)
ATLAS_DIR=${ROOT}/libs/atlas
WRAP_DIR=${ROOT}/libs/wrap
TGUI2_DIR=${ROOT}/libs/tgui2
BASS_DIR=${ROOT}/../bass

if [ $(getconf LONG_BIT) == 64 ]; then
  BASS_LIBDIR=${BASS_DIR}/x64
else
  BASS_LIBDIR=${BASS_DIR}
fi

NODES=3

# Clone TGUI2 if its directory doesn't already exist.
if [ ! -d ${TGUI2_DIR} ]; then
  echo "### Cloning TGUI2 ###"
  git clone https://github.com/Nooskewl/tgui2.git ${TGUI2_DIR}
fi

echo "### Building wrap ###"
pushd ${WRAP_DIR}
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j${NODES}
popd

echo "### Building atlas ###"
pushd ${ATLAS_DIR}
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DUSER_INCLUDE_PATH="${WRAP_DIR}/src" \
         -DUSER_LIBRARY_PATH="${WRAP_DIR}/build"
make -j${NODES}
popd

echo "### Building tgui2 ###"
pushd ${TGUI2_DIR}
git pull --rebase
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j${NODES}
popd

echo "### Building Crystal Picnic ###"
git pull --rebase
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DUSER_INCLUDE_PATH="${WRAP_DIR}/src;${ATLAS_DIR}/src;${TGUI2_DIR};${BASS_DIR}" \
         -DUSER_LIBRARY_PATH="${WRAP_DIR}/build;${ATLAS_DIR}/build;${TGUI2_DIR}/build;${BASS_LIBDIR}" \
         $@
make -j${NODES}
