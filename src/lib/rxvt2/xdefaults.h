/*--------------------------------*-C-*---------------------------------*
 * File:	xdefaults.h
 *----------------------------------------------------------------------*/
#ifndef _XDEFAULTS_H
#define _XDEFAULTS_H
/*{{{ includes */
#include <X11/Xfuncproto.h>
#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */
#include "feature.h"
/*}}}*/

/*{{{ extern variables */
extern const char * rs_title;		/* title name for window */
extern const char * rs_iconName;	/* icon name for window */
extern const char * rs_geometry;	/* window geometry */
extern const char * rs_saveLines;	/* scrollback buffer [lines] */

#ifdef KEYSYM_RESOURCE
extern const unsigned char * KeySym_map [256];
#endif

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
extern KeySym ks_bigfont;
extern KeySym ks_smallfont;
#endif

#ifdef XPM_BACKGROUND
extern const char * rs_path;
extern const char * rs_backgroundPixmap;
#endif	/* XPM_BACKGROUND */
/*}}}*/

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern void
extract_resources (Display * /* display */,
		   const char * /* name */);

extern void
get_options (int /* argc */,
	     char * /* argv */ []);

_XFUNCPROTOEND
/*}}}*/
#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

