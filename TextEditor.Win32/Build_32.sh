#Use OLD_TOOLS and USE_CRT_MAIN for older CPUs.
#Use NO_UNICODE for multibyte versions of Windows.

mingw32-make clean

#Windows 9x
#mingw32-make OLD_TOOLS=1 NO_UNICODE=1 USE_CRT_MAIN=1 WINDOWS_VERSION="0x0400" ARCH="i386" BIT="32"

#Windows NT 4.0
#mingw32-make OLD_TOOLS=1 WINDOWS_VERSION="0x0400" ARCH="i386" BIT="32"

#Windows 2000
mingw32-make OLD_TOOLS=1 WINDOWS_VERSION="0x0500" ARCH="i386" BIT="32"