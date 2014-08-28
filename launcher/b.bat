fluid -c LAUNCHER.fl
g++ -static-libgcc -static-libstdc++ -DLAUNCHER ..\src\config.cpp LAUNCHER.cxx -o LAUNCHER.exe -I..\include -I"c:\Documents and Settings\Trent Gamblin\code\local\include" -L"c:\Documents and Settings\Trent Gamblin\code\local\lib" -lfltk -mwindows -luuid -lcomctl32 -lallegro-static -lopengl32 -lwinmm -lkernel32 -lpsapi -lshlwapi -lole32 icon.res
strip LAUNCHER.exe
upx LAUNCHER.exe
