/*--------------------------------*-C-*---------------------------------*
 * File:	misc.c
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
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "misc.h"
/*}}}*/

/*----------------------------------------------------------------------*/
const char *
my_basename (const char * str)
{
   const char * base = strrchr (str, '/');
   return (base ? base + 1 : str);
}

/*
 * Print an error message
 */
void
print_error (const char * fmt, ...)
{
   va_list arg_ptr;

   va_start (arg_ptr, fmt);
   fprintf (stderr, APL_NAME ": ");
   vfprintf (stderr, fmt, arg_ptr);
   fprintf (stderr, "\n");
   va_end (arg_ptr);
}

/*
 * in-place interpretation of string:
 * backslash-escaped strings:
 * Emacs-style Ctrl chars:	^@, ^?, ^A, etc.
 *
 * return the converted string length
 */
int
escaped_string (char * str)
{
   register char * p = str;
   int i = 0, len, meta_x = 0;
   /* use 'i' to increment through destination and p through source */

   if (str == NULL || (len = strlen (str)) == 0)
     return 0;

   /* Emacs convenience, replace leading `M-..' with `\E..' */
   if (!strncmp (p, "M-", 2))
     {
	str [i++] = '\033';		/* destination */
	p += 2;
	len--;
	if (toupper (*p) == 'X')
	  {
	     meta_x = 1;
	     str [i++] = 'x';		/* destination */
	     p++;
	     while (isspace (*p)) { p++; len--; }
	  }
     }

   for (/*nil*/; i < len; i++)
     {
	register char ch = *p++;
	if (ch == '\\')
	  {
	     ch = *p;
	     if (ch >= '0' && ch <= '7')	/* octal */
	       {
		  int j, num = 0;
		  for (j = 0; j < 3 && (ch >= '0' && ch <= '7'); j++)
		    {
		       num = num * 010 + (ch - '0');
		       p++; len--;
		       ch = *p;
		    }
		  ch = (unsigned char) num;
	       }
	     else
	       {
		  p++; len--;
		  switch (ch) {
		   case 'a': ch = 007;	break;	/* bell */
		   case 'b': ch = '\b';	break;	/* backspace */
		   case 'E':
		   case 'e': ch =  033;	break;	/* escape */
		   case 'n': ch = '\n';	break;	/* newline */
		   case 'r': ch = '\r';	break;	/* carriage-return */
		   case 't': ch = '\t';	break;	/* tab */
		  }
	       }
	  }
	else if (ch == '^')
	  {
	     ch = *p;
	     p++; len--;
	     ch = toupper (ch);
	     ch = (ch == '?' ? 127 : (ch - '@'));
	  }

	str [i] = ch;
     }

   /* add trailing carriage-return for `M-xcommand' */
   if (meta_x && str [len-1] != '\r')
     str [len++] = '\r';

   str [len] = '\0';

   return len;
}

/*----------------------------------------------------------------------*
 * miscellaneous drawing routines
 */

/*
 * draw bottomShadow/highlight along top/left sides of the window
 */
static void
Draw_tl (Window win, GC gc, int x, int y, int w, int h)
{
   int shadow = SHADOW;
   if (w == 0 || h == 0) shadow = 1;

   w += (x - 1);
   h += (y - 1);

   for (/*nil*/; shadow > 0; shadow--, x++, y++, w--, h--)
     {
	XDrawLine (Xdisplay, win, gc, x, y, w, y);
	XDrawLine (Xdisplay, win, gc, x, y, x, h);
     }
}

/*
 * draw bottomShadow/highlight along the bottom/right sides of the window
 */
static void
Draw_br (Window win, GC gc, int x, int y, int w, int h)
{
   int shadow = SHADOW;
   if (w == 0 || h == 0) shadow = 1;

   w += (x - 1);
   h += (y - 1);

   x++; y++;
   for (/*nil*/; shadow > 0; shadow--, x++, y++, w--, h--)
     {
	XDrawLine (Xdisplay, win, gc, w, h, w, y);
	XDrawLine (Xdisplay, win, gc, w, h, x, h);
     }
}

void
Draw_Shadow (Window win, GC topShadow, GC botShadow,
	    int x, int y, int w, int h)
{
   Draw_tl (win, topShadow, x, y, w, h);
   Draw_br (win, botShadow, x, y, w, h);
}

/* button shapes */
void
Draw_Triangle (Window win, GC topShadow, GC botShadow,
	       int x, int y, int w, int type)
{
   switch (type) {
    case 'r':			/* right triangle */
      XDrawLine (Xdisplay, win, topShadow, x, y, x, y + w);
      XDrawLine (Xdisplay, win, topShadow, x, y, x + w, y + w/2);
      XDrawLine (Xdisplay, win, botShadow, x, y + w, x + w, y + w/2);
      break;

    case 'l':			/* right triangle */
      XDrawLine (Xdisplay, win, botShadow, x + w, y + w, x + w, y);
      XDrawLine (Xdisplay, win, botShadow, x + w, y + w, x, y + w/2);
      XDrawLine (Xdisplay, win, topShadow, x, y + w/2, x + w, y);
      break;

    case 'd':			/* down triangle */
      XDrawLine (Xdisplay, win, topShadow, x, y, x + w/2, y + w);
      XDrawLine (Xdisplay, win, topShadow, x, y, x + w, y);
      XDrawLine (Xdisplay, win, botShadow, x + w, y, x + w/2, y + w);
      break;

    case 'u':			/* up triangle */
      XDrawLine (Xdisplay, win, botShadow, x + w, y + w, x + w/2, y);
      XDrawLine (Xdisplay, win, botShadow, x + w, y + w, x, y + w);
      XDrawLine (Xdisplay, win, topShadow, x, y + w, x + w/2, y);
      break;
#if 0
    case 's':			/* square */
      XDrawLine (Xdisplay, win, topShadow, x + w, y, x, y);
      XDrawLine (Xdisplay, win, topShadow, x, y, x, y + w);
      XDrawLine (Xdisplay, win, botShadow, x, y + w, x + w, y + w);
      XDrawLine (Xdisplay, win, botShadow, x + w, y + w, x + w, y);
      break;
#endif
   }
}
/*----------------------- end-of-file (C source) -----------------------*/
