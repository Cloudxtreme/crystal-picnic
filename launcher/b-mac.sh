#!/bin/sh

/Users/trent/code/local/bin/fluid -c LAUNCHER.fl
g++ -arch "i386" -DBUILDING_LAUNCHER ../src/config.cpp LAUNCHER.cxx -o CrystalPicnicLauncher -I/Users/trent/code/local/include -L/Users/trent/code/local/lib -I../include -lfltk -lallegro-static -lallegro_main-static -framework Foundation -framework AppKit -framework OpenGL -framework IOKit
strip CrystalPicnicLauncher

