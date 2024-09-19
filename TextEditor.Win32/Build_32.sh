#Use OLD_TOOLS for older CPUs and NO_UNICODE for multibyte versions of Windows.

mingw32-make clean

#Windows NT 3.5
#mingw32-make OLD_TOOLS=1 WINDOWS_VERSION="0x0350" ARCH="i386" BIT="32"

#Windows 95
#mingw32-make OLD_TOOLS=1 NO_UNICODE=1 WINDOWS_VERSION="0x0350" ARCH="i386" BIT="32"

#Windows 98, Windows Me
mingw32-make OLD_TOOLS=1 NO_UNICODE=1 WINDOWS_VERSION="0x0400" ARCH="i386" BIT="32"

#Windows NT 4.0
#mingw32-make OLD_TOOLS=1 WINDOWS_VERSION="0x0400" ARCH="i386" BIT="32"

#Windows 2000
#mingw32-make WINDOWS_VERSION="0x0500" ARCH="i386" BIT="32"