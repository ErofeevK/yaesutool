#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__ 1

#ifdef MINGW32
#   include <windows.h>
typedef HANDLE SERIALPORTDESC;
#else
#   include <termios.h>
#   include <sys/stat.h>
typedef int SERIALPORTDESC;
#endif
#endif
