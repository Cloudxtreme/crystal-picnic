libwrap wraps your ALLEGRO_BITMAPs and ALLEGRO_SHADERs so you can easily destroy and reload them all (for DirectX lost displays or Android pause/resume.)

Example CMake command line:

cmake .. -DUSER_INCLUDE_PATH=C:\Users\trent\code\local\include -G "MinGW Makefiles"

Copy libwrap.a to your library path and src/wrap.h to your include path.

