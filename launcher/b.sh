#!/bin/sh

fluid -c LAUNCHER.fl
g++ -DBUILDING_LAUNCHER ../src/config.cpp LAUNCHER.cxx -o CrystalPicnicLauncher -I../include -lfltk -lallegro-static -lm -lpthread -lSM -lICE -lX11 -lXext -lXcursor -lXinerama -lXrandr -lXpm -lGL -lGLU -lXft
strip CrystalPicnicLauncher
upx CrystalPicnicLauncher
