/*--------------------------------*-C-*---------------------------------*
 * File:	misc.h
 *
 * miscellaneous service routines
 *
 * Copyright 1996
 * mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
/*{{{ includes */
#ifndef _MISC_H
#define _MISC_H
#include <X11/Xfuncproto.h>

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern const char *
my_basename (const char * str);

extern void
print_error (const char * fmt, ...);

extern int
escaped_string (char * str);

extern void
Draw_Shadow (Window /* win */, GC /* topShadow */, GC /* botShadow */,
	     int /* x */, int /* y */, int /* w */, int /* h */);

extern void
Draw_Triangle (Window /* win */, GC /* topShadow */, GC /* botShadow */,
	       int /* x */, int /* y */, int /* w */,
	       int /* type */);

_XFUNCPROTOEND
/*}}}*/
#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

