#ifndef XNC_GLOBALS_H
#define XNC_GLOBALS_H

#include "config.h"

#include "globdef.h"
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include <sys/param.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <signal.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#include <ctype.h>

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifndef major
#include <sys/sysmacros.h>
#endif

#include "types.h"


#include "commonfuncs.h"
#include "xncdebug.h"

#define MAX2(x1,x2)            (x1<x2 ? x2 : x1)

/* -- defines for gettext localization package -- */
#ifdef ENABLE_NLS

#include <libintl.h>
#define _(STRING)          gettext(STRING)

#else

#define _(STRING)          STRING

#endif

#define _noop(STRING)      STRING
#define _cant(STRING)      STRING

/* ---------------------------------------------- */


#endif

/* ------------ End of file -------------- */

