Introduction
============

Crystal Picnic is an action RPG by Nooskewl. The git history is incomplete. Sorry.

See http://nooskewl.ca/crystal-picnic for more info.



Building
========

The build process is not streamlined much yet. You need to know what you're doing.

Prerequisites
-------------

- Allegro 5 (git version)
- Lua 5.2
- BASS and BASSMIDI
- poly2tri
- TGUI2 from our GitHub page
- The libs in libs/, these are not compiled automatically yet

Platforms
---------

The game should work on Windows, Mac OS X, Linux, Android and iOS. 

Basic Process
-------------

- To build for Windows, you'll need the prerequisites first. Then use CMake to generate a project (MSVC and MinGW should work but we use MSVC for official builds.)
- The datafile needs to be created. tools/mkcpa.sh will create it. It requires MSYS on Windows. There is an mkcpa-linux.sh too. Run it from the root of the project and it will create data.cpa and data.cpa.uncompressed in build/ off the root.
- You'll need to put the datafile with the EXE and BASS DLLs to run the game.