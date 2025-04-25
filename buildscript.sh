#!/bin/bash
set -ex

echo "## Cloning repos"
git clone --recursive https://github.com/Rubberduckycooly/Sonic-Mania-Decompilation.git
git clone --recursive https://github.com/SaturnSH2x2/RSDKv5-Decompilation.git Sonic-Mania-Decompilation/dependencies/v5_3DS

echo "## Switching to 3ds-main branch"
cd Sonic-Mania-Decompilation/dependencies/v5_3DS
cd ../../

echo "## Creating build directory"
mkdir -p build
cd build

echo "## Building"
cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake -DRSDK_PATH="dependencies/v5_3DS" -DGAME_STATIC=ON
make -j$(nproc)

echo "## Done building 3DSX, build is in $(pwd)/dependencies/v5_3DS/RetroEngine.3dsx"

echo "## Now building CIA from ELF"
export ELF_PATH=$(pwd)/dependencies/v5_3DS/RSDKv5U.elf
cd ../dependencies/v5_3DS/3ds
./build_cia.sh $ELF_PATH
echo "## Done! CIA is in $(pwd)/out/"
