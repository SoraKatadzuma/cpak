#pragma once
#define CPAK_PLATFORM_WINDOWS 1
#define CPAK_PLATFORM "Windows"

// Provides MinGW runtime information.
#ifdef __MINGW32__
#  include <_mingw.h>
#endif

#ifndef CPAK_SYMBOL_EXPORT
#  define CPAK_HAS_DECLSPEC
#  define CPAK_SYMBOL_EXPORT __declspec(dllexport)
#  define CPAK_SYMBOL_IMPORT __declspec(dllimport)
#endif

#if defined(__MINGW32__) && \
    ((__MINGW32_MAJOR_VERSION > 2)  || \
     ((__MINGW32_MAJOR_VERSION == 2) && \
      (__MINGW32_MINOR_VERSION >= 0)))
#  define CPAK_HAS_STDINT_H
#  ifndef __STDC_LIMIT_MACROS
#    define __STDC_LIMIT_MACROS
#  endif
#  define CPAK_HAS_DIRENT_H
#  define CPAK_HAS_UNISTD_H
#endif


#ifndef CPAKCALL
#  define CPAKCALL __stdcall
#endif

#if defined(WINAPI_FAMILY) && \
    (WINAPI_FAMILY == WINAPI_FAMILY_APP || \
     WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#  define CPAK_NO_ANSI_APIS
#endif
