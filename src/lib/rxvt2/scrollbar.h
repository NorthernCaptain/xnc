/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar.h
 *
 * Copyright 1995,1996
 * mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
#ifndef _SCROLLBAR_H
#define _SCROLLBAR_H
#include <X11/Xfuncproto.h>
#include <ctype.h>

typedef struct {
   short beg, end;	/* beg/end of slider sub-window */
   short top, bot;	/* top/bot of slider */
   short state;		/* scrollbar state */
   Window win;
} scrollBar_t;
extern scrollBar_t	scrollBar;

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern int
scrollbar_mapping (int /* map */);

extern int
scrollbar_show (int /* update */);
_XFUNCPROTOEND
/*}}}*/

/*{{{ macros */
#define scrollbar_visible()	(scrollBar.state)
#define scrollbar_isMotion()	(scrollBar.state == 'm')
#define scrollbar_isUp()	(scrollBar.state == 'U')
#define scrollbar_isDn()	(scrollBar.state == 'D')
#define scrollbar_isUpDn()	isupper (scrollBar.state)
#define isScrollbarWindow(w)	(scrollbar_visible() && (w) == scrollBar.win)

#define scrollbar_setNone()	do { scrollBar.state = 1; } while (0)
#define scrollbar_setMotion()	do { scrollBar.state = 'm'; } while (0)
#define scrollbar_setUp()	do { scrollBar.state = 'U'; } while (0)
#define scrollbar_setDn()	do { scrollBar.state = 'D'; } while (0)

#define scrollbar_upButton(y)	((y) < scrollBar.beg)
#define scrollbar_dnButton(y)	((y) > scrollBar.end)

#define scrollbar_above_slider(y)	((y) < scrollBar.top)
#define scrollbar_below_slider(y)	((y) > scrollBar.bot)
#define scrollbar_position(y)		((y) - scrollBar.beg)
#define scrollbar_size()		(scrollBar.end - scrollBar.beg)

/*}}}*/
/*{{{ defines */
#ifdef XTERM_SCROLLBAR
# undef  SB_WIDTH
# define SB_WIDTH	14
#else
# if !defined (SB_WIDTH) || (SB_WIDTH < 8)
#  undef SB_WIDTH
#  define SB_WIDTH	10	/* scrollBar width */
# endif
#endif	/* XTERM_SCROLLBAR */

/*}}}*/
#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

