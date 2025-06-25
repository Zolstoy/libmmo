#pragma once

#define MMO_VERSION_MAJOR 0
#define MMO_VERSION_MINOR 1
#define MMO_VERSION_PATCH 0

#if !defined(MMO_STATIC)
 #if defined(_WIN32)
  #define MMO_API_EXPORT __declspec(dllexport)
  #define MMO_API_IMPORT __declspec(dllimport)
 #else
  #if __GNUC__ >= 4
   #define MMO_API_EXPORT __attribute__((__visibility__("default")))
   #define MMO_API_IMPORT __attribute__((__visibility__("default")))
  #else
   #define MMO_API_EXPORT
   #define MMO_API_IMPORT
  #endif
 #endif
#else
 #define MMO_API_EXPORT
 #define MMO_API_IMPORT
#endif

#ifdef MMO_BUILDING_THE_LIB
 #define MMO_API MMO_API_EXPORT
#else
 #define MMO_API MMO_API_IMPORT
#endif
