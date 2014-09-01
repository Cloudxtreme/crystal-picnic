#!/bin/sh

fluid -c LAUNCHER.fl
arm-linux-gnueabihf-g++ -DBUILDING_LAUNCHER ../src/config.cpp LAUNCHER.cxx -o CrystalPicnicLauncher -I../include -I/c/Users/trent/bin/gcc-4.6.3-eglibc-2.13/arm-linux-gnueabihf/usr/local/include -L/c/Users/trent/bin/gcc-4.6.3-eglibc-2.13/arm-linux-gnueabihf/opt/vc/lib -L/c/Users/trent/bin/gcc-4.6.3-eglibc-2.13/arm-linux-gnueabihf/usr/lib/arm-linux-gnueabihf -L/c/Users/trent/bin/gcc-4.6.3-eglibc-2.13/arm-linux-gnueabihf/usr/local/lib -lfltk -lallegro-static -lm -lpthread -lSM -lICE -lX11 -lXext -lXcursor -lXinerama -lXrandr -lXpm -lxcb -ldl -lbcm_host -lGLESv2 -lEGL -lXrender -lXfixes -lXau -lXdmcp -lvcos -lvchiq_arm
arm-linux-gnueabihf-strip CrystalPicnicLauncher

