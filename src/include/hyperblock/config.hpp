#pragma once

#define HYPERBLOCK_VERSION_MAJOR 0
#define HYPERBLOCK_VERSION_MINOR 1
#define HYPERBLOCK_VERSION_PATCH 0

#if !defined(HYPERBLOCK_STATIC)
 #if defined(HYPERBLOCK_SYSTEM_WINDOWS)
  #define HYPERBLOCK_API_EXPORT __declspec(dllexport)
  #define HYPERBLOCK_API_IMPORT __declspec(dllimport)
 #else
  #if __GNUC__ >= 4
   #define HYPERBLOCK_API_EXPORT __attribute__((__visibility__("default")))
   #define HYPERBLOCK_API_IMPORT __attribute__((__visibility__("default")))
  #else
   #define HYPERBLOCK_API_EXPORT
   #define HYPERBLOCK_API_IMPORT
  #endif
 #endif
#else
 #define HYPERBLOCK_API_EXPORT
 #define HYPERBLOCK_API_IMPORT
#endif

#ifdef HYPERBLOCK_BUILDING_THE_LIB
 #define HYPERBLOCK_API HYPERBLOCK_API_EXPORT
#else
 #define HYPERBLOCK_API HYPERBLOCK_API_IMPORT
#endif
