# Target system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# Compiler to build for the target

set(RPI_TOOLCHAIN_ROOT "c:/Users/trent/bin/gcc-4.6.3-eglibc-2.13/arm-linux-gnueabihf" CACHE PATH "Path to the Raspberry Pi toolchain" )
set(RPI_ARCH "arm-linux-gnueabihf")
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

SET(CMAKE_C_COMPILER   
  ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-gcc${CMAKE_EXECUTABLE_SUFFIX} CACHE PATH "gcc" FORCE)
SET(CMAKE_CXX_COMPILER 
  ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-g++${CMAKE_EXECUTABLE_SUFFIX} CACHE PATH "gcc" FORCE)
#there may be a way to make cmake deduce these TODO deduce the rest of the tools
set(CMAKE_AR
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-ar${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "archive" FORCE)
set(CMAKE_LINKER
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-ld${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "linker" FORCE)
set(CMAKE_NM
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-nm${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "nm" FORCE)
set(CMAKE_OBJCOPY
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-objcopy${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "objcopy" FORCE)
set(CMAKE_OBJDUMP
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-objdump${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "objdump" FORCE)
set(CMAKE_STRIP
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-strip${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "strip" FORCE)
set(CMAKE_RANLIB
 ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_ARCH}-ranlib${CMAKE_EXECUTABLE_SUFFIX}  CACHE PATH "ranlib" FORCE)
