/*--------------------------------*-C-*---------------------------------*
 * File:	menubar.h
 *
 * Copyright 1995,1996
 * mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
#ifndef _MENUBAR_H
#define _MENUBAR_H
#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */
#include <X11/Xfuncproto.h>

typedef struct {
   short state;
   Window win;
} menuBar_t;
extern menuBar_t	menuBar;

/*{{{ defines */
#define menuBar_margin		2		/* margin below text */
/*}}}*/
/*{{{ macros */
#define menubar_visible()	(menuBar.state)
#define menuBar_height()	(TermWin.fheight + SHADOW)
#define menuBar_TotalHeight()	(menuBar_height() + SHADOW + menuBar_margin)
#define isMenuBarWindow(w)	((w) == menuBar.win)
/*}}}*/

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern void
menubar_control (XButtonEvent * /* ev */);

extern void
menubar_dispatch (char * /* str */);

extern void
menubar_expose (void);

extern int
menubar_mapping (int /* map */);

_XFUNCPROTOEND
/*}}}*/

#ifdef NO_MENUBAR
# define menubar_dispatch(str)	((void)0)
# define menubar_expose()	((void)0)
# define menubar_control(ev)	((void)0)
# define menubar_mapping(map)	(0)
# undef menubar_visible
# define menubar_visible()	(0)
# undef isMenuBarWindow
# define isMenuBarWindow(w)	(0)
#endif

#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

