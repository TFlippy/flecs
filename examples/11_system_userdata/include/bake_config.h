/*
                                   )
                                  (.)
                                  .|.
                                  | |
                              _.--| |--._
                           .-';  ;`-'& ; `&.
                          \   &  ;    &   &_/
                           |"""---...---"""|
                           \ | | | | | | | /
                            `---.|.|.|.---'

 * This file is generated by bake.lang.c for your convenience. Headers of
 * dependencies will automatically show up in this file. Include bake_config.h
 * in your main project file. Do not edit! */

#ifndef SYSTEMUSERDATA_BAKE_CONFIG_H
#define SYSTEMUSERDATA_BAKE_CONFIG_H

/* Headers of public dependencies */
#include <reflecs>

/* Headers of private dependencies */
#ifdef SYSTEMUSERDATA_IMPL
/* No dependencies */
#endif

/* Convenience macro for exporting symbols */
#if SYSTEMUSERDATA_IMPL && defined _MSC_VER
#define SYSTEMUSERDATA_EXPORT __declspec(dllexport)
#elif SYSTEMUSERDATA_IMPL
#define SYSTEMUSERDATA_EXPORT __attribute__((__visibility__("default")))
#elif defined _MSC_VER
#define SYSTEMUSERDATA_EXPORT __declspec(dllimport)
#else
#define SYSTEMUSERDATA_EXPORT
#endif

#endif
