#pragma once

#if !defined(CPAK_NO_PLATFORM_CONFIG) && \
    !defined(CPAK_PLATFORM_CONFIG)
#  include "platform.hpp"
#endif

#ifdef CPAK_PLATFORM_CONFIG
#  include CPAK_PLATFORM_CONFIG
#endif

#if defined(CPAK_BUILD_SHARED) && \
    defined(CPAK_BUILD_STATIC)
#  error "Only specify one of CPAK_BUILD_SHARED or CPAK_BUILD_STATIC."
#endif

#ifdef CPAK_SYMBOL_EXPORT
#  if defined(CPAK_BUILD_SHARED)
#    define CPAKAPI CPAK_SYMBOL_EXPORT
#  elif !defined(CPAK_BUILD_STATIC)
#    define CPAKAPI CPAK_SYMBOL_IMPORT
#  else
#    define CPAKAPI
#  endif
#endif

#ifndef CPAKCALL
#  define CPAKCALL
#endif

#define CPAK_STRINGIZE(x) CPAK_DO_STRINGIZE(x)
#define CPAK_DO_STRINGIZE(x) #x

#define CPAK_CONCAT(x, y) CPAK_DO_CONCAT(x, y)
#define CPAK_DO_CONCAT(x, y) CPAK_DO_CONCAT2(x, y)
#define CPAK_DO_CONCAT2(x, y) x##y
