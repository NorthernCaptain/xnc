#ifndef XNC_DEBUG_H
#define XNC_DEBUG_H

#include "config.h"
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" int debprintf(const char *format,...);
#else
extern int debprintf(const char *format,...);
#endif

#ifdef DEBUG_XNC
#ifdef __FILE__
#ifdef __LINE__
#ifndef __DEBUG
#define __DEBUG()                fprintf(stderr, "%-14s:%4d ", __FILE__, __LINE__)
#endif
#endif
#endif

#ifndef __DEBUG
#define __DEBUG()                ((void)0)
#endif

#  define xncdprintf(x)           do {__DEBUG(); debprintf x;} while(0)
#else
#  define xncdprintf(x)           ((void)0)
#endif

#endif
