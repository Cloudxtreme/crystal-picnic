Example CMake command line:

cmake .. -DUSER_INCLUDE_PATH=C:\Users\trent\code\local\include -DUSER_LIBRARY_PATH=C:\Users\trent\code\local\lib -G "MinGW Makefiles"

Copy everything in data/ to the same directory as the executable.

Default level size is 480x320. Pass a different size to B_BattleCanvas constructor if you like (or make it flexible and send me a patch...)

Put your tiles in a directory called tiles/ beside the executable.

Requires: libatlas, libtgui2, libwrap.

