mingw32-make clean

#Windows 9x
mingw32-make OLD_TOOLS=1 NO_UNICODE=1 USE_CRT_MAIN=1 ARCH="i386" BIT="32" -j
#mingw32-make ARCH="i386" BIT="32" -j