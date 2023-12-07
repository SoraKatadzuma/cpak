#pragma once

#if (defined(linux)     || \
     defined(__linux)   || \
     defined(__linux__) || \
     defined(__GNU__)   || \
     defined(__GLIBC__))
#  define CPAK_PLATFORM_CONFIG "config/linux.hpp"
#elif (defined(_WIN32)    || \
       defined(__WIN32__) || \
       defined(WIN32))
#  define CPAK_PLATFORM_CONFIG "config/windows.hpp"
#elif (defined(macintosh) || \
       defined(__APPLE__) || \
       defined(__APPLE_CC__))
#  define CPAK_PLATFORM_CONFIG "config/macintosh.hpp"
#endif

#if 0
#include "config/macintosh.hpp"
#include "config/linux.hpp"
#include "config/windows.hpp"
#endif