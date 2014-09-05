arm-linux-androideabi-gcc -Wall -mthumb -march=armv7-a -mfloat-abi=softfp -fPIC -c android_extras.c -Iinclude
arm-linux-androideabi-gcc -mthumb -march=armv7-a -mfloat-abi=softfp -fPIC -shared -Wl,-soname,libandroid_extras.so -o libandroid_extras.so android_extras.o -lallegro -lbass
