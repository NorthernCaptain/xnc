/*--------------------------------*-C-*---------------------------------*
 * File:        main.h
 */
/*{{{ notes: */
/*----------------------------------------------------------------------*
 * Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 * You can do what you like with this source code as long as you don't try
 * to make money out of it and you include an unaltered copy of this
 * message (including the copyright).
 *
 * This module has been heavily modified by R. Nation
 * <nation@rocket.sanders.lockheed.com>
 * No additional restrictions are applied
 *
 * Additional modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions are applied.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
/*}}}*/
#ifndef _MAIN_H
#define _MAIN_H
/*{{{ includes */
#include "VERSION.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "feature.h"
#include <X11/Xfuncproto.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

/* STDC_HEADERS
 * don't check for these using configure, since we need them regardless.
 * if you don't have them -- figure a workaround.
 *
 * Sun is often reported as not being STDC_HEADERS, but it's not true
 * for our purposes and only generates spurious bug reports.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXIT_SUCCESS                /* missing from <stdlib.h> */
# define EXIT_SUCCESS        0        /* exit function success */
# define EXIT_FAILURE        1        /* exit function failure */
#endif

#include <X11/Intrinsic.h>        /* Xlib, Xutil, Xresource, Xfuncproto */
#include "misc.h"
/*}}}*/

typedef struct {
   short width,  height;        /* window size [pixels] */
   short fwidth, fheight;        /* font width and height [pixels] */
   short ncol, nrow;        /* window size [characters] */
   short focus;                /* window has focus */
   short saveLines;        /* number of lines that fit in scrollback */
   short nscrolled;        /* number of line actually scrolled */
   short view_start;        /* scrollback view starts here */
   Window parent, vt;        /* parent (main) and vt100 window */
   GC gc;                /* GC for drawing text */
   XFontStruct        * font;        /* main font structure */
#ifndef NO_BOLDFONT
   XFontStruct        * boldFont;        /* bold font */
#endif
#ifdef KANJI
   XFontStruct        * kanji;        /* Kanji font structure */
#endif
#ifdef XPM_BACKGROUND
   Pixmap pixmap;
# ifdef XPM_BUFFERING
   Pixmap buf_pixmap;
# endif
#endif
} TermWin_t;

extern TermWin_t TermWin;

/* gap between text and window edges (could be configurable) */
#define TermWin_internalBorder        2
#define MAX_COLS        200
#define MAX_ROWS        128

/* width of scrollBar, menuBar shadow ... don't change! */
#define SHADOW        2

/* convert pixel dimensions to row/column values */
#define Pixel2Width(x)        ((x) / TermWin.fwidth)
#define Pixel2Height(y)        ((y) / TermWin.fheight)
#define Pixel2Col(x)        Pixel2Width((x) - TermWin_internalBorder)
#define Pixel2Row(y)        Pixel2Height((y) - TermWin_internalBorder)

#define Width2Pixel(n)        ((n) * TermWin.fwidth)
#define Height2Pixel(n)        ((n) * TermWin.fheight)
#define Col2Pixel(col)        (Width2Pixel(col) + TermWin_internalBorder)
#define Row2Pixel(row)        (Height2Pixel(row) + TermWin_internalBorder)

#define TermWin_TotalWidth()        (TermWin.width  + 2 * TermWin_internalBorder)
#define TermWin_TotalHeight()        (TermWin.height + 2 * TermWin_internalBorder)

extern Display                * Xdisplay;

#define Xscreen                DefaultScreen(Xdisplay)
#define Xcmap                DefaultColormap(Xdisplay,Xscreen)
#define Xdepth                DefaultDepth(Xdisplay,Xscreen)
#define Xroot                DefaultRootWindow(Xdisplay)
#ifdef DEBUG_DEPTH
# undef Xdepth
# define Xdepth                DEBUG_DEPTH
#endif

#define        Opt_console        (1LU<<0)
#define Opt_loginShell (1LU<<1)
#define Opt_iconic        (1LU<<2)
#define Opt_visualBell        (1LU<<3)
#define Opt_mapAlert        (1LU<<4)
#define Opt_reverseVideo (1LU<<5)
#define Opt_utmpInhibit        (1LU<<6)
#define Opt_scrollBar        (1LU<<7)
#define Opt_meta8        (1LU<<8)
#define Opt_showIntro  (1LU<<9)
#define Opt_fastLoad  (1LU<<10)
#define Opt_termOff   (1LU<<11)
#define Opt_systemUse (1LU<<12)
#define Opt_lookFive  (1LU<<13)
#define Opt_backLoad  (1LU<<14)
#define Opt_sharedLoad  (1LU<<15)

/* place holder used for parsing command-line options */
#define Opt_Boolean        (1LU<<31)
extern unsigned long Options;

extern const char * display_name;
extern const char * rs_name;        /* client instance (resource name) */

/*
 * XTerm escape sequences: ESC ] Ps;Pt BEL
 */
#define XTerm_name        0
#define XTerm_iconName        1
#define XTerm_title        2
#define XTerm_logfile        46        /* not implemented */
#define XTerm_font        50

/*
 * rxvt extensions of XTerm escape sequences: ESC ] Ps;Pt BEL
 */
#define XTerm_Menu        10        /* set menu item */
#define XTerm_Pixmap        20        /* new bg pixmap */
#define XTerm_restoreFG        39        /* change default fg color */
#define XTerm_restoreBG        49        /* change default bg color */

/*----------------------------------------------------------------------*/

#define restoreFG        39        /* restore default fg color */
#define restoreBG        49        /* restore default bg color */

#define fgColor                0
#define bgColor                1
/* 0-7: black, red, green, yellow, blue, magenta, cyan, white */
#define minColor        2
#define maxColor        (minColor+7)
#define blackColor        (minColor)

/* 10-17: Bright black, red, green, yellow, blue, magenta, cyan, white */
#ifdef NO_BRIGHTCOLOR
# define whiteColor        (maxColor)
#else
# define minBright        (maxColor+1)
# define maxBright        (minBright+7)
# define whiteColor        (maxBright)
#endif
#define NCOLORS                (whiteColor+1)

#ifdef NO_CURSORCOLOR
# define NCURSOR        0
#else
# define cursorColor        (NCOLORS)
# define cursorColor2        (cursorColor+1)
# define NCURSOR        2
#endif

#ifdef NO_BOLDUNDERLINE
# define NBOLDULINE        0
#else
# define colorBD        (NCOLORS + NCURSOR)
# define colorUL        (colorBD+1)
# define NBOLDULINE        2
#endif

#ifdef XTERM_SCROLLBAR
# define NSCROLLCOLORS        0
# define NSHADOWCOLORS        0
#else
# define scrollColor                (NCOLORS + NCURSOR + NBOLDULINE)
# define topShadowColor                (scrollColor + 1)
# define bottomShadowColor        (scrollColor + 2)
# define NSCROLLCOLORS        1
# define NSHADOWCOLORS        2
#endif

#define NRS_COLORS        (NCOLORS + NCURSOR + NBOLDULINE + NSCROLLCOLORS)
extern const char * rs_color [NRS_COLORS];
extern Pixel PixColors [NRS_COLORS + NSHADOWCOLORS];

#define NFONTS                5
extern const char * rs_font [NFONTS];
#ifdef KANJI
extern const char * rs_kfont [NFONTS];
#endif
#ifndef NO_BOLDFONT
extern const char * rs_boldFont;
#endif

#ifdef PRINTPIPE
extern const char * rs_print_pipe;
#endif

extern const char * rs_cutchars;

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern void
map_menuBar (int /* map */);

extern void
map_scrollBar (int /* map */);

extern void
xterm_seq (int /* op */,
           char * /* str */);

/* special (internal) prefix for font commands */
#define FONT_CMD        '#'
#define FONT_DN                "#-"
#define FONT_UP                "#+"

extern void
change_font (int /* init */,
             const char * /* fontname */);
             
extern int
is_term_on();

extern void
set_width (unsigned short /* ncol */);

extern void
resize_window (void);

_XFUNCPROTOEND
/*}}}*/
#endif        /* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

