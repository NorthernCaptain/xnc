/*--------------------------------*-C-*---------------------------------*
 * File:	command.h
 *
 * Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 * You can do what you like with this source code as long as you don't try
 * to make money out of it and you include an unaltered copy of this
 * message (including the copyright).
 *
 * This module has been heavily modified by R. Nation
 * <nation@rocket.sanders.lockheed.com>
 * No additional restrictions are applied.
 *
 * Additional modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions are applied.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
#ifndef _COMMAND_H
# define _COMMAND_H
# include <X11/Xfuncproto.h>
# include <stdio.h>

#define menuBar_esc	10
#define scrollBar_esc	30

_XFUNCPROTOBEGIN

extern void
init_command (char * /* argv */ []);

extern void
tt_resize (void);

extern void
tt_write (const unsigned char * /* str */,
	  unsigned int /* count */);

extern void
tt_printf (const unsigned char * /* fmt */, ...);

extern unsigned int
cmd_write (const unsigned char * /* str */,
	   unsigned int /* count */);

extern void
main_loop (void);

extern FILE *
popen_printer (void);

extern int
pclose_printer (FILE * /* stream */);

_XFUNCPROTOEND

#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

