/*--------------------------------*-C-*---------------------------------*
 * File:	graphics.h
 *
 * function declarations for graphics.c, only to be included by the rxvt
 * source itself.
 *
 * To use graphics in external programs only include "rxvtgrx.h"
 *----------------------------------------------------------------------*/
#ifndef _GRAPHICS_H
#define _GRAPHICS_H
#include <X11/Xfuncproto.h>

#include "rxvtgrx.h"		/* text alignment */

/*
 * number of graphics points
 * divisible by 2 (num lines)
 * divisible by 4 (num rect)
 */
#define	NGRX_PTS	1000

_XFUNCPROTOBEGIN

extern void
Gr_ButtonReport (int /* but */,
		 int /* x */,
		 int /* y */);

extern void
Gr_do_graphics (int /* cmd */,
		int /* nargs */,
		int /* args */[],
		unsigned char * /* text */);

extern void
Gr_scroll (int /* count */);

extern void
Gr_ClearScreen (void);

extern void
Gr_ChangeScreen (void);

extern void
Gr_expose (Window /* win */);

extern void
Gr_Resize (int /* w */,
	   int /* h */);

extern void
Gr_reset (void);

extern int
Gr_Displayed (void);

_XFUNCPROTOEND

#ifdef RXVT_GRAPHICS
# define Gr_ButtonPress(x,y)	Gr_ButtonReport ('P',(x),(y))
# define Gr_ButtonRelease(x,y)	Gr_ButtonReport ('R',(x),(y))
#else
# define Gr_ButtonPress(x,y)	((void)0)
# define Gr_ButtonRelease(x,y)	((void)0)
# define Gr_scroll(count)	((void)0)
# define Gr_ClearScreen()	((void)0)
# define Gr_ChangeScreen()	((void)0)
# define Gr_expose(win)		((void)0)
# define Gr_Resize(w,h)		((void)0)
# define Gr_reset()		((void)0)
# define Gr_Displayed()		(0)		/* return zero */
#endif
#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

