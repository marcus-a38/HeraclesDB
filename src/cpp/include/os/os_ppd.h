#ifndef HERACLES_OS_PPD_H
#define HERACLES_OS_PPD_H

/* 
Preprocessor directives from https://stackoverflow.com/a/41896671 
and the SQLite codebase.
*/

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) \
  || defined(__MINGW32__) || defined(__BORLANDC__)
  # define OS_WINDOWS 1
  # include "os_win.h"
# elif defined(__APPLE__) && defined(__MACH__)
  # define OS_MACOS 1
  # include "os_mac.h"
# elif defined(__linux__)
  # define OS_LINUX 1
  # include "os_unx.h"
# elif defined(unix) || defined(__unix__) || defined(__unix)
  # define OS_UNIX 1
  # include "os_unx.h"
# else
  # error Unrecognized operating system. 

#endif

#endif