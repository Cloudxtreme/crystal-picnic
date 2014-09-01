arm-linux-androideabi-gcc -Wall -mthumb -march=armv7-a -mfloat-abi=softfp -fPIC -c amazon_extras.c -Iinclude
arm-linux-androideabi-gcc -mthumb -march=armv7-a -mfloat-abi=softfp -fPIC -shared -Wl,-soname,libamazon_extras.so -o libamazon_extras.so amazon_extras.o -lallegro -lbass
