/*--------------------------------*-C-*---------------------------------*
 * File:        main.c
 *
 * Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 * You can do what you like with this source code as long as
 * you don't try to make money out of it and you include an
 * unaltered copy of this message (including the copyright).
 *
 * This module has been heavily modified by R. Nation
 * <nation@rocket.sanders.lockheed.com>
 * No additional restrictions are applied
 *
 * Additional modifications by Garrett D'Amore <garrett@netcom.com>
 * No additional restrictions are applied.
 *
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions are applied.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
/*{{{ includes */
#include "main.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#ifdef XPM_BACKGROUND
# include <X11/xpm.h>
#endif

#include "command.h"
#include "debug.h"
#include "graphics.h"
#include "scrollbar.h"
#include "menubar.h"
#include "screen.h"
#include "xdefaults.h"
#include "defaults.h"
/*}}}*/

/*{{{ extern functions referenced */
#ifdef DISPLAY_IS_IP
extern char * network_display (const char * display);
#endif
/*}}}*/

/* extern variables referenced */
/*{{{ extern variables declared here */
TermWin_t        TermWin;
Display                * Xdisplay;        /* display */
Pixmap        vt_pixmap=0;

extern int bool_term_on;         /*in command.c*/
const char * rs_color [NRS_COLORS];
Pixel PixColors [NRS_COLORS + NSHADOWCOLORS];

unsigned long Options = (Opt_scrollBar) | (Opt_loginShell) | (Opt_sharedLoad);

const char * display_name = NULL;
const char * rs_name = NULL;        /* client instance (resource name) */

#ifndef NO_BOLDFONT
const char * rs_boldFont = NULL;
#endif
const char * rs_font [NFONTS];
#ifdef KANJI
const char * rs_kfont [NFONTS];
#endif

#ifdef PRINTPIPE
const char * rs_print_pipe = NULL;
#endif

const char * rs_cutchars = NULL;
/*}}}*/

/*{{{ local variables */
static Cursor TermWin_cursor;        /* cursor for vt window */

char l_bg_string[30]="#419ebd";
extern int Mainl,Mainh,Mainx,Mainy;
Window vt_win;

static XSizeHints szHint = {
   PMinSize | PResizeInc | PBaseSize | PWinGravity,
   0, 0, 80, 24,        /* x, y, width, height */
   1, 1,                /* Min width, height */
   0, 0,                /* Max width, height - unused*/
   1, 1,                /* increments: width, height */
   {1, 1},                /* increments: x, y */
   {0, 0},                /* Aspect ratio - unused */
   0, 0,                /* base size: width, height */
   NorthWestGravity        /* gravity */
};

static const char * def_colorName [] = {
   "Black", "White",                /* fg/bg */
   "Black",                        /* 0: black                (#000000) */
#ifndef NO_BRIGHTCOLOR
   /* low-intensity colors */
   "Red3",                        /* 1: red                (#CD0000) */
   "Green3",                        /* 2: green                (#00CD00) */
   "Yellow3",                        /* 3: yellow                (#CDCD00) */
   "Blue3",                        /* 4: blue                (#0000CD) */
   "Magenta3",                        /* 5: magenta                (#CD00CD) */
   "Cyan3",                        /* 6: cyan                (#00CDCD) */
   "AntiqueWhite",                /* 7: white                (#FAEBD7) */
   /* high-intensity colors */
   "Grey25",                        /* 8: bright black        (#404040) */
#endif        /* NO_BRIGHTCOLOR */
   "Red",                        /* 1/9: bright red        (#FF0000) */
   "Green",                        /* 2/10: bright green        (#00FF00) */
   "Yellow",                        /* 3/11: bright yellow        (#FFFF00) */
   "Blue",                        /* 4/12: bright blue        (#0000FF) */
   "Magenta",                        /* 5/13: bright magenta        (#FF00FF) */
   "Cyan",                        /* 6/14: bright cyan        (#00FFFF) */
   "White",                        /* 7/15: bright white        (#FFFFFF) */
#ifndef NO_CURSORCOLOR
   NULL, NULL,
#endif        /* NO_CURSORCOLOR */
#ifndef NO_BOLDUNDERLINE
   NULL, NULL,
#endif        /* NO_BOLDUNDERLINE */
   "#B2B2B2"                        /* scrollColor: match Netscape color */
};

#ifdef KANJI
/* Kanji font names, roman fonts sized to match */
static const char * def_kfontName [] = {
   KFONT0, KFONT1, KFONT2, KFONT3, KFONT4
};
#endif        /* KANJI */
static const char * def_fontName [] = {
   FONT0, FONT1, FONT2, FONT3, FONT4
};
/*}}}*/

/*{{{ local functions referenced */
#ifdef XPM_BACKGROUND
static Pixmap set_bgPixmap (const char * /* file */);
static struct {
   short w, h, x, y;
   Pixmap pixmap;
} bgPixmap = { 100, 100, 50, 50, None };
/* the originally loaded pixmap and its scaling */
static XpmAttributes xpmAttr;
#endif        /* XPM_BACKGROUND */
static void resize (void);
/*}}}*/

/*----------------------------------------------------------------------*/
static XErrorHandler
xerror_handler (Display * display, XErrorEvent * event)
{
   clean_exit();
   exit (EXIT_FAILURE);
   return 0;
}

/*{{{ color aliases, fg/bg bright-bold */
static inline void
color_aliases (int idx)
{
   if (rs_color [idx] && isdigit (*rs_color [idx]))
     {
        int i = atoi (rs_color [idx]);
        if (i >= 8 && i <= 15)        /* bright colors */
          {
             i -= 8;
#ifndef NO_BRIGHTCOLOR
             rs_color [idx] = rs_color [minBright + i];
             return;
#endif
          }
        if (i >= 0 && i <= 7)                /* normal colors */
          rs_color [idx] = rs_color [minColor + i];
     }
}

/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
#ifndef NO_BRIGHTCOLOR
extern unsigned long keyscol[];
static inline void
set_colorfgbg (void)
{
   unsigned int i;
   static char colorfgbg_env [] = "COLORFGBG=default;default;bg";
   char * p;
   int fg = -1, bg = -1;
   PixColors[bgColor]=keyscol[1];
   for (i = blackColor; i <= whiteColor; i++)
     {
        if (PixColors [fgColor] == PixColors [i])
          {
             fg = (i - blackColor);
             break;
          }
     }
   for (i = blackColor; i <= whiteColor; i++)
     {
        if (PixColors [bgColor] == PixColors [i])
          {
             bg = (i - blackColor);
             break;
          }
     }

   p = strchr (colorfgbg_env, '=');
   p++;
   if (fg >= 0)
     sprintf (p, "%d;", fg);
   else
     strcpy (p, "default;");
   p = strchr (p, '\0');
   if (bg >= 0)
     sprintf (p,
# ifdef XPM_BACKGROUND
              "default;"
# endif
              "%d", bg);
   else
     strcpy (p, "default");
   putenv (colorfgbg_env);

   colorfgbg = DEFAULT_RSTYLE;
   for (i = minColor; i <= maxColor; i++)
     {
        if (PixColors [fgColor] == PixColors [i]
# ifndef NO_BOLDUNDERLINE
            && PixColors [fgColor] == PixColors [colorBD]
# endif        /* NO_BOLDUNDERLINE */
            /* if we wanted boldFont to have precedence */
# if 0        /* ifndef NO_BOLDFONT */
            && TermWin.boldFont == NULL
# endif        /* NO_BOLDFONT */
            )
          colorfgbg = SET_FGCOLOR (colorfgbg, i);
        if (PixColors [bgColor] == PixColors [i])
          colorfgbg = SET_BGCOLOR (colorfgbg, i);
     }
}
#else        /* NO_BRIGHTCOLOR */
# define set_colorfgbg() ((void)0)
#endif
/*}}}*/

#ifdef XPM_BACKGROUND
/*
 * These GEOM strings indicate absolute size/position:
 * @ `WxH+X+Y'
 * @ `WxH+X'        -> Y = X
 * @ `WxH'        -> Y = X = 50
 * @ `W+X+Y'        -> H = W
 * @ `W+X'        -> H = W, Y = X
 * @ `W'        -> H = W, X = Y = 50
 * @ `0xH'        -> H *= H/100, X = Y = 50 (W unchanged)
 * @ `Wx0'        -> W *= W/100, X = Y = 50 (H unchanged)
 * @ `=+X+Y'        -> (H, W unchanged)
 * @ `=+X'        -> Y = X (H, W unchanged)
 *
 * These GEOM strings adjust position relative to current position:
 * @ `+X+Y'
 * @ `+X'        -> Y = X
 *
 * And this GEOM string is for querying current scale/position:
 * @ `?'
 */
static int
scale_pixmap (const char * geom)
{
   static char str [] = "[1000x1000+100+100]";   /* should be big enough */
   int w = 0, h = 0, x = 0, y = 0;
   int flags;
   int changed = 0;

   if (geom == NULL) return 0;
   if (!strcmp (geom, "?"))
     {
        sprintf (str,
                 "[%dx%d+%d+%d]",
                 bgPixmap.w,
                 bgPixmap.h,
                 bgPixmap.x,
                 bgPixmap.y);
        xterm_seq (XTerm_title, str);
        return 0;
     }

     {
        char * p;
        int n;

        if ((p = strchr (geom, ';')) == NULL) p = strchr (geom, '\0');
        n = (p - geom);
        if (n >= sizeof(str) - 1) return 0;
        strncpy (str, geom, n);
        str [n] = '\0';
     }

   flags = XParseGeometry (str, &x, &y, &w, &h);
   if (!flags) { flags |= WidthValue; w = 100; }

   if (flags & WidthValue)
     {
        if (!(flags & XValue)) { x = 50; }
        if (!(flags & HeightValue)) h = w;

        if (w && !h)
          {
             w = bgPixmap.w * ((float)w/100);
             h = bgPixmap.h;
          }
        else if (h && !w)
          {
             w = bgPixmap.w;
             h = bgPixmap.h * ((float)h/100);
          }

        if (w > 1000) w = 1000;
        if (h > 1000) h = 1000;

        if (bgPixmap.w != w)
          {
             bgPixmap.w = w;
             changed++;
          }
        if (bgPixmap.h != h)
          {
             bgPixmap.h = h;
             changed++;
          }
     }

   if (!(flags & YValue))
     {
        if (flags & XNegative) flags |= YNegative;
        y = x;
     }
   if (!(flags & WidthValue) && geom [0] != '=')
     {
        x += bgPixmap.x;
        y += bgPixmap.y;
     }
   else
     {
        if (flags & XNegative) x += 100;
        if (flags & YNegative) y += 100;
     }

   x = (x <= 0 ? 0 : (x >= 100 ? 100 : x));
   y = (y <= 0 ? 0 : (y >= 100 ? 100 : y));;
   if (bgPixmap.x != x)
     {
        bgPixmap.x = x;
        changed++;
     }
   if (bgPixmap.y != y)
     {
        bgPixmap.y = y;
        changed++;
     }
   return changed;
}

static void
resize_pixmap (void)
{
   XGCValues gcvalue;
   GC gc;
   unsigned int width = TermWin_TotalWidth ();
   unsigned int height = TermWin_TotalHeight ();

   if (TermWin.pixmap) XFreePixmap (Xdisplay, TermWin.pixmap);
   TermWin.pixmap = XCreatePixmap (Xdisplay, TermWin.vt,
                                   width, height, Xdepth);

   gcvalue.foreground = PixColors [bgColor];
   gc = XCreateGC (Xdisplay, TermWin.vt, GCForeground, &gcvalue);

   if (bgPixmap.pixmap)
     {
        int w = bgPixmap.w;
        int h = bgPixmap.h;
        int x = bgPixmap.x;
        int y = bgPixmap.y;

        /*
         * don't zoom pixmap too much nor expand really small pixmaps
         */
        if (w > 1000 || h > 1000)
          w = 1;
        else if (width > (10 * xpmAttr.width) ||
                 height > (10 * xpmAttr.height))
          w = 0;                /* tile */

        if (w)
          {
             float p, incr;
             Pixmap tmp;
             tmp = XCreatePixmap (Xdisplay, TermWin.vt,
                                  width, xpmAttr.height,
                                  Xdepth);

             XFillRectangle (Xdisplay, tmp, gc, 0, 0, width, xpmAttr.height);
             /*
              * horizontal scaling
              */
             incr = (float)xpmAttr.width;
             p = 0;

             if (w == 1)
               {
                  /* display image directly - no scaling at all */
                  incr = width;
                  if (xpmAttr.width <= width)
                    {
                       w = xpmAttr.width;
                       x = (width - w) / 2;
                       w += x;
                    }
                  else
                    {
                       x = 0;
                       w = width;
                    }
               }
             else if (w < 10)
               {
                  incr *= w;        /* fit W images across screen */
                  x = 0;
                  w = width;
               }
             else
               {
                  incr *= 100.0 / w;
                  if (w < 100)        /* contract */
                    {
                       w = (w * width) / 100;
                       if (x >= 0)        /* position */
                         {
                            float pos;
                            pos = (float)x / 100 * width - (w/2);

                            x = (width - w);
                            if (pos <= 0) x = 0; else if (pos < x) x = pos;
                         }
                       else
                         {
                            x = (width - w) / 2;
                         }
                       w += x;
                    }
                  else if (w >= 100)                /* expand */
                    {
                       if (x > 0)        /* position */
                         {
                            float pos;
                            pos = (float)x / 100 * xpmAttr.width - (incr/2);
                            p = xpmAttr.width - (incr);
                            if (pos <= 0) p = 0; else if (pos < p) p = pos;
                         }
                       x = 0;
                       w = width;
                    }
               }
             incr /= width;

             for (/*nil*/; x < w; x++, p += incr)
               {
                  if (p >= xpmAttr.width) p = 0;
                  XCopyArea (Xdisplay, bgPixmap.pixmap, tmp, gc,
                             (int)p, 0, 1, xpmAttr.height, x, 0);
               }

             /*
              * vertical scaling
              */
             incr = (float)xpmAttr.height;
             p = 0;

             if (h == 1)
               {
                  /* display image directly - no scaling at all */
                  incr = height;
                  if (xpmAttr.height <= height)
                    {
                       h = xpmAttr.height;
                       y = (height - h) / 2;
                       h += y;
                    }
                  else
                    {
                       y = 0;
                       h = height;
                    }
               }
             else if (h < 10)
               {
                  incr *= h;        /* fit H images across screen */
                  y = 0;
                  h = height;
               }
             else
               {
                  incr *= 100.0 / h;
                  if (h < 100)                /* contract */
                    {
                       h = (h * height) / 100;
                       if (y >= 0)        /* position */
                         {
                            float pos;
                            pos = (float)y / 100 * height - (h/2);

                            y = (height - h);
                            if (pos < 0.0f) y = 0; else if (pos < y) y = pos;
                         }
                       else
                         {
                            y = (height - h) / 2;
                         }
                       h += y;
                    }
                  else if (h >= 100)                /* expand */
                    {
                       if (y > 0)                /* position */
                         {
                            float pos;
                            pos = (float)y / 100 * xpmAttr.height - (incr/2);
                            p = xpmAttr.height - (incr);
                            if (pos < 0) p = 0; else if (pos < p) p = pos;
                         }
                       y = 0;
                       h = height;
                    }
               }
             incr /= height;

             for (/*nil*/; y < h; y++, p += incr)
               {
                  if (p >= xpmAttr.height) p = 0;
                  XCopyArea (Xdisplay, tmp, TermWin.pixmap, gc,
                             0, (int)p, width, 1, 0, y);
               }
             XFreePixmap (Xdisplay, tmp);
          }
        else
          {
             /* tiled */
             for (y = 0; y < height; y += xpmAttr.height)
               {
                  unsigned int h = (height - y);
                  if (h > xpmAttr.height) h = xpmAttr.height;
                  for (x = 0; x < width; x += xpmAttr.width)
                    {
                       unsigned int w = (width - x);
                       if (w > xpmAttr.width) w = xpmAttr.width;
                       XCopyArea (Xdisplay,
                                  bgPixmap.pixmap,
                                  TermWin.pixmap,
                                  gc,
                                  0, 0, w, h, x, y);
                    }
               }
          }
     }
   else
     XFillRectangle (Xdisplay, TermWin.pixmap, gc,
                     0, 0, width, height);

#ifdef XPM_BUFFERING
   if (TermWin.buf_pixmap) XFreePixmap (Xdisplay, TermWin.buf_pixmap);
   TermWin.buf_pixmap = XCreatePixmap (Xdisplay, TermWin.vt,
                                       width, height, Xdepth);
   XCopyArea (Xdisplay,
              TermWin.pixmap,
              TermWin.buf_pixmap,
              gc,
              0, 0,
              width, height,
              0, 0);
   XSetWindowBackgroundPixmap (Xdisplay, TermWin.vt, TermWin.buf_pixmap);
#else        /* XPM_BUFFERING */
   XSetWindowBackgroundPixmap (Xdisplay, TermWin.vt, TermWin.pixmap);
#endif        /* XPM_BUFFERING */
   XFreeGC (Xdisplay, gc);

   XClearWindow (Xdisplay, TermWin.vt);
   XFlush (Xdisplay);

   XSync (Xdisplay, 0);
}
#else        /* XPM_BACKGROUND */
# define scale_pixmap(str)        ((void)0)
# define resize_pixmap()        ((void)0)
#endif        /* XPM_BACKGROUND */
Window main_win=0;
extern unsigned long keyscol[];
extern Status (*pim_AllocColor)(Display*, Colormap, XColor*);
/*{{{ Create_Windows() - Open and map the window */
static void
Create_Windows (int argc, char * argv [])
{
   Cursor cursor;
   XClassHint classHint;
   XWMHints wmHint;
   int i, x, y, flags;
   unsigned int width, height;

   /*
    * grab colors before netscape does
    */
   for (i = 0;
        i < (Xdepth <= 2 ? 2 : NRS_COLORS);
        i++)
     {
        const char * const msg = "can't load color \"%s\"";
        XColor xcol;

        if (!rs_color [i]) continue;

        if (!XParseColor (Xdisplay, Xcmap, rs_color [i], &xcol) ||
            !pim_AllocColor (Xdisplay, Xcmap, &xcol))
          {
             print_error (msg, rs_color [i]);
             rs_color [i] = def_colorName [i];
             if (!rs_color [i]) continue;
             if (!XParseColor (Xdisplay, Xcmap, rs_color [i], &xcol) ||
                 !XAllocColor (Xdisplay, Xcmap, &xcol))
               {
                  print_error (msg, rs_color [i]);
                  switch (i) {
                   case fgColor:
                   case bgColor:
                     /* fatal: need bg/fg color */
                     print_error ("aborting");
                     exit (EXIT_FAILURE);
                     break;
#ifndef NO_CURSORCOLOR
                   case cursorColor:
                     xcol.pixel = PixColors [bgColor];
                     break;
                   case cursorColor2:
                     xcol.pixel = PixColors [fgColor];
                     break;
#endif        /* NO_CURSORCOLOR */
                   default:
                     xcol.pixel = PixColors [bgColor];        /* None */
                     break;
                  }
               }
          }
        PixColors [i] = xcol.pixel;
     }

#ifndef NO_CURSORCOLOR
   if (Xdepth <= 2 || !rs_color [cursorColor])
     PixColors [cursorColor] = PixColors [bgColor];
   if (Xdepth <= 2 || !rs_color [cursorColor2])
     PixColors [cursorColor2] = PixColors [fgColor];
#endif        /* NO_CURSORCOLOR */

#ifndef NO_BOLDUNDERLINE
   if (Xdepth <= 2 || !rs_color [colorBD])
     PixColors [colorBD] = PixColors [fgColor];
   if (Xdepth <= 2 || !rs_color [colorUL])
     PixColors [colorUL] = PixColors [fgColor];
#endif        /* NO_BOLDUNDERLINE */

   /*
    * get scrollBar/menuBar shadow colors
    *
    * The calculations of topShadow/bottomShadow values are adapted
    * from the fvwm window manager.
    */
#ifndef XTERM_SCROLLBAR
   if (Xdepth <= 2)                /* Monochrome */
     {
        PixColors [scrollColor] = PixColors [bgColor];
        PixColors [topShadowColor] = PixColors [fgColor];
        PixColors [bottomShadowColor] = PixColors [fgColor];
     }
   else
     {
        XColor xcol, white;

        /* bottomShadowColor */
        xcol.pixel = PixColors [scrollColor];
        XQueryColor (Xdisplay, Xcmap, &xcol);

        xcol.red   = ((xcol.red)   / 2);
        xcol.green = ((xcol.green) / 2);
        xcol.blue  = ((xcol.blue)  / 2);

        if (!pim_AllocColor (Xdisplay, Xcmap, &xcol))
          {
             print_error ("can't allocate %s", "bottomShadowColor");
             xcol.pixel = PixColors [minColor];
          }
        PixColors [bottomShadowColor] = xcol.pixel;

        /* topShadowColor */
        white.pixel = WhitePixel (Xdisplay, Xscreen);
        XQueryColor (Xdisplay, Xcmap, &white);

        xcol.pixel = PixColors [scrollColor];
        XQueryColor (Xdisplay, Xcmap, &xcol);

# ifndef min
#  define min(a,b) (((a)<(b)) ? (a) : (b))
#  define max(a,b) (((a)>(b)) ? (a) : (b))
# endif
        xcol.red   = max ((white.red   / 5), xcol.red);
        xcol.green = max ((white.green / 5), xcol.green);
        xcol.blue  = max ((white.blue  / 5), xcol.blue);

        xcol.red   = min (white.red,   (xcol.red   * 7) / 5);
        xcol.green = min (white.green, (xcol.green * 7) / 5);
        xcol.blue  = min (white.blue,  (xcol.blue  * 7) / 5);

        if (!pim_AllocColor (Xdisplay, Xcmap, &xcol))
          {
             print_error ("can't allocate %s", "topShadowColor");
             xcol.pixel = PixColors [whiteColor];
          }
        PixColors [topShadowColor] = xcol.pixel;
     }
#endif        /* XTERM_SCROLLBAR */

   szHint.base_width  = (2 * TermWin_internalBorder +
                         (Options & Opt_scrollBar ? SB_WIDTH : 0));
   szHint.base_height = (2 * TermWin_internalBorder);

   flags = (rs_geometry ?
            XParseGeometry (rs_geometry, &x, &y, &width, &height) : 0);

   if (flags & WidthValue)
     {
        szHint.width = width;
        szHint.flags |= USSize;
     }
   if (flags & HeightValue)
     {
        szHint.height = height;
        szHint.flags |= USSize;
     }
    
   TermWin.ncol = szHint.width;
   TermWin.nrow = szHint.height;

   change_font (1, NULL);

   if (flags & XValue)
     {
        if (flags & XNegative)
          {
             x += (DisplayWidth (Xdisplay, Xscreen)
                   - (szHint.width + TermWin_internalBorder));
             szHint.win_gravity = NorthEastGravity;
          }
        szHint.x = x;
        szHint.flags |= USPosition;
     }
   if (flags & YValue)
     {
        if (flags & YNegative)
          {
             y += (DisplayHeight (Xdisplay, Xscreen)
                   - (szHint.height + TermWin_internalBorder));
             szHint.win_gravity = (szHint.win_gravity == NorthEastGravity ?
                                   SouthEastGravity : SouthWestGravity);
          }
        szHint.y = y;
        szHint.flags |= USPosition;
     }

   /* parent window - reverse video so we can see placement errors
    * sub-window placement & size in resize_subwindows()
    */

  szHint.x=Mainx;
  szHint.y=Mainy;
  szHint.flags |= USPosition;
  PixColors[bgColor]=keyscol[1];
  PixColors[cursorColor]=PixColors[fgColor];
  if(main_win==0)
      TermWin.parent = XCreateSimpleWindow (Xdisplay, Xroot,
					    szHint.x, szHint.y,
					    szHint.width, szHint.height,
					    BORDERWIDTH,
					    PixColors [bgColor],
					    PixColors [fgColor]);
  else
      TermWin.parent = XCreateSimpleWindow (Xdisplay, main_win,
					    0, 0,
					    szHint.width, szHint.height,
					    0,
					    PixColors [bgColor],
					    PixColors [fgColor]);

   main_win=TermWin.parent;
   xterm_seq ((int)XTerm_title,(char*) rs_title);
   xterm_seq ((int)XTerm_iconName,(char*) rs_iconName);
   /* ignore warning about discarded `const' */
   classHint.res_name  = (char*)rs_name;
   classHint.res_class = APL_CLASS;
   wmHint.input = True;
   wmHint.initial_state = (Options & Opt_iconic ? IconicState : NormalState);
   wmHint.flags = (InputHint | StateHint);

   XSetWMProperties (Xdisplay, TermWin.parent, NULL, NULL, argv, argc,
                     &szHint, &wmHint, &classHint);

   XSelectInput (Xdisplay, TermWin.parent,
                 (KeyPressMask|FocusChangeMask|
                  StructureNotifyMask|VisibilityChangeMask)
                 );

   /* vt cursor: Black-on-White is standard, but this is more popular */
   TermWin_cursor = XCreateFontCursor (Xdisplay, XC_xterm);
     {
        XColor fg, bg;
        fg.pixel = PixColors [fgColor]; XQueryColor (Xdisplay, Xcmap, &fg);
        bg.pixel = PixColors [bgColor]; XQueryColor (Xdisplay, Xcmap, &bg);
        XRecolorCursor (Xdisplay, TermWin_cursor, &fg, &bg);
     }

   /* cursor (menuBar/scrollBar): Black-on-White */
   cursor = XCreateFontCursor (Xdisplay, XC_left_ptr);

   /* the vt window */
   TermWin.vt = XCreateSimpleWindow (Xdisplay, TermWin.parent,
                                     0, 0,
                                     szHint.width, szHint.height,
                                     0,
                                     PixColors [fgColor],
                                     PixColors [bgColor]);

   vt_win=TermWin.vt;

   XDefineCursor (Xdisplay, TermWin.vt, TermWin_cursor);
   XSelectInput (Xdisplay, TermWin.vt,
                 (ExposureMask|ButtonPressMask|ButtonReleaseMask|
                  Button1MotionMask|Button3MotionMask));
   /* scrollBar: size doesn't matter */
   scrollBar.win = XCreateSimpleWindow (Xdisplay, TermWin.parent,
                                        0, 0,
                                        1, 1,
                                        0,
                                        PixColors [fgColor],
                                        PixColors [bgColor]);

   XDefineCursor (Xdisplay, scrollBar.win, cursor);
   XSelectInput (Xdisplay, scrollBar.win,
                 (ExposureMask|ButtonPressMask|ButtonReleaseMask|
                  Button1MotionMask|Button2MotionMask|Button3MotionMask)
                 );

#ifndef NO_MENUBAR
   /* menuBar: size doesn't matter */
   menuBar.win = XCreateSimpleWindow (Xdisplay, TermWin.parent,
                                      0, 0,
                                      1, 1,
                                      0,
                                      PixColors [fgColor],
                                      PixColors [scrollColor]);
   XDefineCursor (Xdisplay, menuBar.win, cursor);
   XSelectInput (Xdisplay, menuBar.win,
                 (ExposureMask|ButtonPressMask|ButtonReleaseMask|
                  Button1MotionMask)
                 );
#endif        /* NO_MENUBAR */

#ifdef XPM_BACKGROUND
   if (rs_backgroundPixmap != NULL)
     {
        char * p = rs_backgroundPixmap;
        if ((p = strchr (p, ';')) != NULL)
          {
             p++;
             scale_pixmap (p);
          }

        set_bgPixmap (rs_backgroundPixmap);
     }
#endif        /* XPM_BACKGROUND */

   /* graphics context for the vt window */
     {
        XGCValues gcvalue;
        gcvalue.font = TermWin.font->fid;
        gcvalue.foreground = PixColors [fgColor];
        gcvalue.background = PixColors [bgColor];
        TermWin.gc = XCreateGC (Xdisplay, TermWin.vt,
                                GCForeground | GCBackground | GCFont ,
                                &gcvalue);
     }
}
/*}}}*/

void map_vt_windows(Pixmap pix)
{
   if(is_term_on())
   {
       if(pix)
       {
	   XSetWindowBackgroundPixmap(Xdisplay, TermWin.vt, pix);
	   vt_pixmap=pix;
       }
       XMapWindow (Xdisplay, TermWin.vt);
       XMapWindow (Xdisplay, TermWin.parent);
       map_scrollBar(Options & Opt_scrollBar);
   }
}

/*{{{ window resizing - assuming the parent window is the correct size */
static void
resize_subwindows (int width, int height)
{
#define SWITCH_BUTTON_H   20
   int x = 0, y = 0;
   int old_width  = TermWin.width;
   int old_height = TermWin.height;

   TermWin.width  = TermWin.ncol * TermWin.fwidth;
   TermWin.height = TermWin.nrow * TermWin.fheight;

   /* size and placement */
   if (scrollbar_visible ())
     {
        scrollBar.beg = 0;
        scrollBar.end = height-SWITCH_BUTTON_H;
#ifndef XTERM_SCROLLBAR
        /* arrows are as high as wide - leave 1 pixel gap */
        scrollBar.beg += (SB_WIDTH + 1);
        scrollBar.end -= (SB_WIDTH + 1);
#endif

        width -= SB_WIDTH;
        XMoveResizeWindow (Xdisplay, scrollBar.win,
#ifdef SCROLLBAR_RIGHT
                           width, SWITCH_BUTTON_H,
#else
                           x, 0,
#endif
                           SB_WIDTH, height-SWITCH_BUTTON_H);

#ifndef SCROLLBAR_RIGHT
        x = SB_WIDTH;                /* placement of vt window */
#endif
     }             

#ifndef NO_MENUBAR
   if (menubar_visible ())
     {
        y = menuBar_TotalHeight ();        /* for placement of vt window */
        XMoveResizeWindow (Xdisplay, menuBar.win,
                           x, 0,
                           width, y);
     }
#endif        /* NO_MENUBAR */

   XMoveResizeWindow (Xdisplay, TermWin.vt,
                      x, y,
                      width, height + 1);

   if (old_width) Gr_Resize (old_width, old_height);
   XClearWindow (Xdisplay, TermWin.vt);
   resize_pixmap ();
   XSync (Xdisplay, 0);
}

static void
resize (void)      
{
   szHint.base_width  = (2 * TermWin_internalBorder);
   szHint.base_height = (2 * TermWin_internalBorder);

   szHint.base_width += (scrollbar_visible () ? SB_WIDTH : 0);
#ifndef NO_MENUBAR
   szHint.base_height += (menubar_visible () ? menuBar_TotalHeight () : 0);
#endif

   szHint.min_width  = szHint.base_width  + szHint.width_inc;
   szHint.min_height = szHint.base_height + szHint.height_inc;

   szHint.width  = szHint.base_width  + TermWin.width;
   szHint.height = szHint.base_height + TermWin.height;

   szHint.flags = PMinSize|PResizeInc|PBaseSize|PWinGravity;

   XSetWMNormalHints (Xdisplay, TermWin.parent, &szHint);
   XResizeWindow (Xdisplay, TermWin.parent, szHint.width, szHint.height);

   resize_subwindows (szHint.width, szHint.height);
}

#ifndef NO_MENUBAR
void
map_menuBar (int map)
{
   if (menubar_mapping (map)) resize ();
}
#endif        /* NO_MENUBAR */

void
map_scrollBar (int map)
{
   if (scrollbar_mapping (map))
     {
        scr_touch ();
        resize ();
     }
}

/*
 * Redraw window after exposure or size change
 */
static void
resize_window1 (unsigned int width, unsigned int height)
{
   static short first_time = 1;
   int new_ncol = (width  - szHint.base_width)  / TermWin.fwidth;
   int new_nrow = (height - szHint.base_height) / TermWin.fheight;

   if (first_time ||
       (new_ncol != TermWin.ncol) ||
       (new_nrow != TermWin.nrow))
     {
        int curr_screen = -1;
             
        /* scr_reset only works on the primary screen */
        if (!first_time)        /* this is not the first time thru */
          {
             selection_clear ();
             curr_screen = scr_change_screen (PRIMARY);
          }

        TermWin.ncol = new_ncol;
        TermWin.nrow = new_nrow;

        resize_subwindows (width, height);
        scr_reset ();

        if (curr_screen >= 0)        /* this is not the first time thru */
          scr_change_screen (curr_screen);
        first_time = 0;
     }
}

/*
 * good for toggling 80/132 columns
 */
void
set_width (unsigned short width)
{
   unsigned short height = TermWin.nrow;

   if (width != TermWin.ncol)
     {
        width  = szHint.base_width  + width  * TermWin.fwidth;
        height = szHint.base_height + height * TermWin.fheight;

        XResizeWindow (Xdisplay, TermWin.parent, width, height);
        resize_window1 (width, height);
     }
}

/*
 * Redraw window after exposure or size change
 */
extern int resize_done;
extern int resize_done2;
extern Window Main, base_win;
extern int term_sticky;
void
resize_window (void)
{
   Window root;
   XEvent dummy;
   int x, y;
   static Bool first = True;
   unsigned int border, depth, width, height;

   while (XCheckTypedWindowEvent (Xdisplay, TermWin.parent,
                                  ConfigureNotify, &dummy));
   /* OUTERWIN
   XGetGeometry (Xdisplay, TermWin.parent,
                 &root, &x, &y, &width, &height, &border, &depth);
   */
   XGetGeometry (Xdisplay, base_win,
                 &root, &x, &y, &width, &height, &border, &depth);
   XResizeWindow(Xdisplay,main_win,width,height);


  if(resize_done==0 && term_sticky && first==False) 
  {
      XResizeWindow(Xdisplay,Main,width,height);
      resize_done2=1;
  };
  resize_done=0;
  /* parent already resized */
  resize_window1 (width, height);
  first=False;
}
/*}}}*/
/*{{{ xterm sequences - title, iconName, color (exptl) */
#ifdef SMART_WINDOW_TITLE
static void
set_title (const char * str)
{
   char * name;
   if (XFetchName (Xdisplay, TermWin.parent, &name)) name = NULL;
   if (name == NULL || strcmp (name, str))
     XStoreName (Xdisplay, TermWin.parent, str);
   if (name) XFree (name);
}
#else
# define set_title(str) XStoreName (Xdisplay, TermWin.parent, str)
#endif

#ifdef SMART_WINDOW_TITLE
static void
set_iconName (const char * str)
{
   char * name;
   if (XGetIconName (Xdisplay, TermWin.parent, &name)) name = NULL;
   if (name == NULL || strcmp (name, str))
     XSetIconName (Xdisplay, TermWin.parent, str);
   if (name) XFree (name);
}
#else
# define set_iconName(str) XSetIconName (Xdisplay, TermWin.parent, str)
#endif

#ifdef XTERM_COLOR_CHANGE
static void
set_window_color (int idx, const char * color)
{
   const char * const msg = "can't load color \"%s\"";
   XColor xcol;
   int i;

   if (color == NULL || *color == '\0') return;

   /* handle color aliases */
   if (isdigit (*color))
     {
        i = atoi (color);
        if (i >= 8 && i <= 15)        /* bright colors */
          {
             i -= 8;
# ifndef NO_BRIGHTCOLOR
             PixColors [idx] = PixColors [minBright + i];
             goto Done;
# endif
          }
        if (i >= 0 && i <= 7)        /* normal colors */
          {
             PixColors [idx] = PixColors [minColor + i];
             goto Done;
          }
     }

   if (!XParseColor (Xdisplay, Xcmap, color, &xcol) ||
       !XAllocColor (Xdisplay, Xcmap, &xcol))
     {
        print_error (msg, color);
        return;
     }

   /* XStoreColor (Xdisplay, Xcmap, XColor*); */

   /*
    * FIXME: should free colors here, but no idea how to do it so instead,
    * so just keep gobbling up the colormap
    */
# if 0
   for (i = blackColor; i <= whiteColor; i++)
     if (PixColors [idx] == PixColors [i])
       break;
   if (i > whiteColor)
     {
        /* fprintf (stderr, "XFreeColors: PixColors [%d] = %lu\n", idx, PixColors [idx]); */
        XFreeColors (Xdisplay, Xcmap, (PixColors + idx), 1,
                     DisplayPlanes(Xdisplay, Xscreen));
     }
# endif

   PixColors [idx] = xcol.pixel;

   /* XSetWindowAttributes attr; */
   /* Cursor cursor; */
   Done:
   if (idx == bgColor)
   {
     XSetWindowBackground (Xdisplay, TermWin.vt, PixColors [bgColor]);

   /* handle colorBD, scrollbar background, etc. */

   set_colorfgbg ();
     {
        XColor fg, bg;
        fg.pixel = PixColors [fgColor]; XQueryColor (Xdisplay, Xcmap, &fg);
        bg.pixel = PixColors [bgColor]; XQueryColor (Xdisplay, Xcmap, &bg);
        XRecolorCursor (Xdisplay, TermWin_cursor, &fg, &bg);
     }
   /* the only reasonable way to enforce a clean update */
   scr_poweron ();
}
#else
# define set_window_color(idx,color) ((void)0)
#endif        /* XTERM_COLOR_CHANGE */

#ifdef XPM_BACKGROUND
/*
 * search for FILE in the current working directory, and within the
 * colon-delimited PATHLIST, adding the file extension EXT if required.
 *
 * FILE is either semi-colon or zero terminated
 */
static const char *
search_path (const char * pathlist, const char * file, const char * ext)
{
   static char name [256];
   char * p, * path;
   int maxpath, len;

   if (!access (file, R_OK)) return file;

   /* semi-colon delimited */
   if ((p = strchr (file, ';')) == NULL) p = strchr (file, '\0');
   len = (p - file);

   /* check about adding a trailing extension */
   if (ext != NULL)
     {
        char * dot;
        dot  = strrchr (p, '.');
        path = strrchr (p, '/');
        if (dot != NULL || (path != NULL && dot <= path)) ext = NULL;
     }

   /* leave room for an extra '/' and trailing '\0' */
   maxpath = sizeof (name) - (len + (ext ? strlen (ext) : 0) + 2);
   if (maxpath <= 0) return NULL;

   for (path = pathlist; path != NULL && *path != '\0'; path = p)
     {
        int n;
        /* colon delimited */
        if ((p = strchr (path, ':')) == NULL) p = strchr (path, '\0');

        n = (p - path);
        if (*p != '\0') p++;

        if (n > 0 && n <= maxpath)
          {
             strncpy (name, path, n);
             if (name [n-1] != '/') name [n++] = '/';
             name [n] = '\0';
             strncat (name, file, len);

             if (!access (name, R_OK)) return name;
             if (ext)
               {
                  strcat (name, ext);
                  if (!access (name, R_OK)) return name;
               }
          }
     }
   return NULL;
}
#endif        /* XPM_BACKGROUND */

#ifdef XPM_BACKGROUND
#define XPM_EXT        ".xpm"
Pixmap
set_bgPixmap (const char * file)
{
   char * f;

   assert (file != NULL);

   if (bgPixmap.pixmap != None)
     {
        XFreePixmap (Xdisplay, bgPixmap.pixmap);
        bgPixmap.pixmap = None;
     }
   XSetWindowBackground (Xdisplay, TermWin.vt, PixColors [bgColor]);

   if (*file != '\0')
     {
        XWindowAttributes attr;
        XGetWindowAttributes (Xdisplay, Xroot, &attr);

        xpmAttr.closeness = 30000;
        xpmAttr.colormap  = attr.colormap;
        xpmAttr.valuemask = (XpmCloseness|XpmColormap|XpmSize|XpmReturnPixels);

        /* search environment variables here too */
        if ((f = search_path (rs_path, file, XPM_EXT)) == NULL)
# ifdef PATH_ENV
          if ((f = search_path (getenv (PATH_ENV), file, XPM_EXT)) == NULL)
# endif
            f = search_path (getenv ("PATH"), file, XPM_EXT);

        if (f == NULL || XpmReadFileToPixmap (Xdisplay, Xroot, f,
                                              &bgPixmap.pixmap,
                                              NULL, &xpmAttr))
          {
             char * p;
             /* semi-colon delimited */
             if ((p = strchr (file, ';')) == NULL) p = strchr (file, '\0');

             print_error ("couldn't load XPM file \"%.*s\"", (p - file), file);
             resize_pixmap ();
          }
        else if (bgPixmap.pixmap != None) resize_pixmap ();
     }

   XClearWindow (Xdisplay, TermWin.vt);
   scr_touch ();
   XFlush (Xdisplay);
   return bgPixmap.pixmap;
}
#undef XPM_EXT
#endif        /* XPM_BACKGROUND */

/*
 * XTerm escape sequences: ESC ] Ps;Pt BEL
 *         0 = change iconName/title
 *         1 = change iconName
 *         2 = change title
 *        46 = change logfile (not implemented)
 *        50 = change font
 *
 * rxvt extensions:
 *        10 = menu
 *        20 = bg pixmap
 *        39 = change default fg color
 *        49 = change default bg color
 */
void
xterm_seq (int op, char * str)
{
   int changed = 0;

   assert (str != NULL);
   switch (op) {
    case XTerm_name:                set_title (str);        /* drop */
    case XTerm_iconName:        set_iconName (str);        break;
    case XTerm_title:                set_title (str);        break;
    case XTerm_Menu:                menubar_dispatch (str);        break;
    case XTerm_Pixmap:
#ifdef XPM_BACKGROUND
      if (*str != ';')
        set_bgPixmap (str);

      while ((str = strchr (str, ';')) != NULL)
        {
           str++;
           changed += scale_pixmap (str);
        }
      if (changed)
        {
           resize_pixmap ();
           scr_touch ();
        }
#endif        /* XPM_BACKGROUND */
      break;

    case XTerm_restoreFG:        set_window_color (fgColor, str);        break;
    case XTerm_restoreBG:        set_window_color (bgColor, str);        break;
    case XTerm_logfile:                break;
    case XTerm_font:                change_font (0, str);        break;
   }
}
/*}}}*/

/*{{{ change_font() - Switch to a new font */
/*
 * init = 1        - initialize
 *
 * fontname == FONT_UP        - switch to bigger font
 * fontname == FONT_DN        - switch to smaller font
 */
void
change_font (int init, const char * fontname)
{
   const char * const msg = "can't load font \"%s\"";
   XFontStruct * xfont;
   static char * newfont [NFONTS];
#ifndef NO_BOLDFONT
   static XFontStruct * boldFont = NULL;
#endif
   static int fnum = FONT0_IDX;                /* logical font number */
   int idx = 4;                                /* index into rs_font[] */

#if (FONT0_IDX == 0)
# define IDX2FNUM(i) (i)
# define FNUM2IDX(f) (f)
#else                     
# define IDX2FNUM(i) (i == 0? FONT0_IDX : (i <= FONT0_IDX? (i-1) : i))
# define FNUM2IDX(f) (f == FONT0_IDX ? 0 : (f < FONT0_IDX ? (f+1) : f))
#endif
#define FNUM_RANGE(i)        (i <= 0 ? 0 : (i >= NFONTS ? (NFONTS-1) : i))

   if (!init)
     {
        switch (fontname [0]) {
         case '\0':
           fnum = FONT0_IDX;
           fontname = NULL;
           break;

           /* special (internal) prefix for font commands */
         case FONT_CMD:
           idx = atoi (fontname+1);
           switch (fontname [1]) {
            case '+':                /* corresponds to FONT_UP */
              fnum += (idx ? idx : 1);
              fnum = FNUM_RANGE (fnum);
              break;

            case '-':                /* corresponds to FONT_DN */
              fnum += (idx ? idx : -1);
              fnum = FNUM_RANGE (fnum);
              break;

            default:
              if (fontname [1] != '\0' && !isdigit (fontname [1]))
                return;
              if (idx < 0 || idx >= (NFONTS))
                return;
              fnum = IDX2FNUM (idx);
              break;
           }
           fontname = NULL;
           break;

         default:
           if (fontname != NULL)
             {
                /* search for existing fontname */
                for (idx = 0; idx < NFONTS; idx++)
                  {
                     if (!strcmp (rs_font [idx], fontname))
                       {
                          fnum = IDX2FNUM (idx);
                          fontname = NULL;
                          break;
                       }
                  }
             }
           else
             return;
           break;
        }
        /* re-position around the normal font */
        idx = FNUM2IDX (fnum);

        if (fontname != NULL)
          {
             char * name;
             xfont = XLoadQueryFont (Xdisplay, fontname);
             if (!xfont)
               return;

             name = MALLOC (strlen(fontname+1)*sizeof(char), "font");

             if (name == NULL)
               {
                  XFreeFont (Xdisplay, xfont);
                  return;
               }

             strcpy (name, fontname);
             if (newfont [idx] != NULL)
               FREE (newfont [idx], "id", "fn");
             newfont [idx] = name;
             rs_font [idx] = newfont [idx];
          }
     }

   if (TermWin.font)
     XFreeFont (Xdisplay, TermWin.font);

   /* load font or substitute */
   xfont = XLoadQueryFont (Xdisplay, rs_font [idx]);
   if (!xfont)
     {
        print_error (msg, rs_font [idx]);
        rs_font [idx] = "fixed";
        xfont = XLoadQueryFont (Xdisplay, rs_font [idx]);
        if (!xfont)
          {
             print_error (msg, rs_font [idx]);
             goto Abort;
          }
     }
   TermWin.font = xfont;

#ifndef NO_BOLDFONT
   /* fail silently */
   if (init && rs_boldFont != NULL)
     boldFont = XLoadQueryFont (Xdisplay, rs_boldFont);
#endif

#ifdef KANJI
   if (TermWin.kanji)
     XFreeFont (Xdisplay, TermWin.kanji);

   /* load font or substitute */
   xfont = XLoadQueryFont (Xdisplay, rs_kfont [idx]);
   if (!xfont)            
     {
        print_error (msg, rs_kfont [idx]);
        rs_kfont [idx] = "k14";
        xfont = XLoadQueryFont (Xdisplay, rs_kfont [idx]);
        if (!xfont)
          {
             print_error (msg, rs_kfont [idx]);
             goto Abort;
          }
     }
   TermWin.kanji = xfont;
#endif        /* KANJI */

   /* alter existing GC */
   if (!init)
     {
        XSetFont (Xdisplay, TermWin.gc, TermWin.font->fid);
#ifndef NO_MENUBAR
        menubar_expose ();
#endif        /* NO_MENUBAR */
     }

   /* set the sizes */
     {
        int fw = XTextWidth (TermWin.font, "MMMMMMMMMM", 10) / 10;
        int fh = TermWin.font->ascent + TermWin.font->descent;

        /* not the first time thru and sizes haven't changed */
        if (fw == TermWin.fwidth && fh == TermWin.fheight)
          return;

        TermWin.fwidth  = fw;
        TermWin.fheight = fh;
 TermWin.ncol=(Mainl-SB_WIDTH)/fw;
 TermWin.nrow=Mainh/fh;
     }

   /* check that size of boldFont is okay */
#ifndef NO_BOLDFONT
   if (boldFont != NULL &&
       TermWin.fwidth == (XTextWidth (boldFont, "MMMMMMMMMM", 10) / 10) &&
       TermWin.fheight == (boldFont->ascent + boldFont->descent))
     TermWin.boldFont = boldFont;
   else
     TermWin.boldFont = NULL;
#endif        /* NO_BOLDFONT */

   set_colorfgbg ();
   TermWin.ncol=(Mainl-SB_WIDTH)/TermWin.fwidth;
   TermWin.nrow=Mainh/TermWin.fheight;

   TermWin.width  = TermWin.ncol * TermWin.fwidth;
   TermWin.height = TermWin.nrow * TermWin.fheight;

   szHint.width_inc  = TermWin.fwidth;
   szHint.height_inc = TermWin.fheight;

   szHint.min_width  = szHint.base_width  + szHint.width_inc;
   szHint.min_height = szHint.base_height + szHint.height_inc;

   szHint.width  = szHint.base_width  + TermWin.width;
   szHint.height = szHint.base_height + TermWin.height;

   szHint.flags = PMinSize|PResizeInc|PBaseSize|PWinGravity;

   if (!init)
     resize ();

   return;
   Abort:
   print_error ("aborting");        /* fatal problem */
   exit (EXIT_FAILURE);
#undef IDX2FNUM
#undef FNUM2IDX
#undef FNUM_RANGE
}
/*}}}*/

extern void (*xnc_init)(int,char**);
extern char xncvt[];
extern void (*xnc_go)();
/*{{{ main() */
int
main (int argc, char * argv [])
{
   int i;
   char * val, ** cmd_argv = NULL;
   /* "WINDOWID=\0" = 10 chars, UINT_MAX = 10 chars */
   static char windowid_string [20], * display_string;

/* Disable commands starting
   for (i = 0; i < argc; i++)
     {
        if (!strcmp (argv [i], "-e"))
          {
             argc = i;
             argv [argc] = NULL;
             if (argv [argc + 1] != NULL)
               {
                  cmd_argv = (argv + argc + 1);
                  if (cmd_argv [0] != NULL)
                    rs_iconName = rs_title = my_basename (cmd_argv [0]);
               }
             break;
          }
     }
*/
   rs_name = my_basename ("rxvt");

   /*
    * Open display, get options/resources and create the window
    */
   if ((display_name = getenv ("DISPLAY")) == NULL)
     display_name = ":0";

   get_options (argc, argv);

   Xdisplay = XOpenDisplay (display_name);
   if (!Xdisplay)
     {
        print_error ("can't open display %s", display_name);
        exit (EXIT_FAILURE);
     }
   xnc_init(argc,argv);
   extract_resources (Xdisplay, rs_name);

   /*
    * set any defaults not already set
    */
   if (!rs_title) rs_title = rs_name;
   if (!rs_iconName) rs_iconName = rs_name;
   if (!rs_saveLines || (TermWin.saveLines = atoi (rs_saveLines)) < 0)
     TermWin.saveLines = SAVELINES;

   /* no point having a scrollbar without having any scrollback! */
   if (!TermWin.saveLines) Options &= ~Opt_scrollBar;

#ifdef PRINTPIPE
   if (!rs_print_pipe) rs_print_pipe = PRINTPIPE;
#endif
   if (!rs_cutchars) rs_cutchars = CUTCHARS;

#ifndef NO_BOLDFONT
   if (rs_font [0] == NULL && rs_boldFont != NULL)
     {
        rs_font [0] = rs_boldFont;
        rs_boldFont = NULL;
     }
#endif
   for (i = 0; i < NFONTS; i++)
     {
        if (!rs_font [i])  rs_font [i]  = def_fontName [i];
#ifdef KANJI
        if (!rs_kfont [i]) rs_kfont [i] = def_kfontName [i];
#endif
     }

#ifdef XTERM_REVERSE_VIDEO
   /* this is how xterm implements reverseVideo */
   if (Options & Opt_reverseVideo)
     {
        if (!rs_color [fgColor]) rs_color [fgColor] = def_colorName [bgColor];
        if (!rs_color [bgColor]) rs_color [bgColor] = def_colorName [fgColor];
     }
#endif

   for (i = 0; i < NRS_COLORS; i++)
     if (!rs_color [i]) rs_color [i] = def_colorName [i];

#ifndef XTERM_REVERSE_VIDEO
   /* this is how we implement reverseVideo */
   if (Options & Opt_reverseVideo)
     {
        const char * name;
        /* swap foreground/background colors */

        name = rs_color [fgColor];
        rs_color [fgColor] = rs_color [bgColor];
        rs_color [bgColor] = name;

        name = def_colorName [fgColor];
        def_colorName [fgColor] = def_colorName [bgColor];
        def_colorName [bgColor] = name;
     }
#endif

   /* convenient aliases for setting fg/bg to colors */
   color_aliases (fgColor);
   color_aliases (bgColor);
#ifndef NO_CURSORCOLOR
   color_aliases (cursorColor);
   color_aliases (cursorColor2);
#endif        /* NO_CURSORCOLOR */
#ifndef NO_BOLDUNDERLINE
   color_aliases (colorBD);
   color_aliases (colorUL);
#endif        /* NO_BOLDUNDERLINE */

   Create_Windows (argc, argv);
   scr_reset ();                /* initialize screen */
   Gr_reset ();                        /* reset graphics */

   /* add scrollBar, do it directly to avoid resize() */
   /*
#ifdef DEBUG_X
   XSynchronize (Xdisplay, True);
   XSetErrorHandler ((XErrorHandler)abort);
#else
   XSetErrorHandler ((XErrorHandler)xerror_handler);
#endif
   */

#ifdef DISPLAY_IS_IP
   /* Fixup display_name for export over pty to any interested terminal
    * clients via "ESC[7n" (e.g. shells).  Note we use the pure IP number
    * (for the first non-loopback interface) that we get from
    * network_display().  This is more "name-resolution-portable", if you
    * will, and probably allows for faster x-client startup if your name
    * server is beyond a slow link or overloaded at client startup.  Of
    * course that only helps the shell's child processes, not us.
    *
    * Giving out the display_name also affords a potential security hole
    */
   val = display_name = network_display (display_name);
   if (val == NULL)
#endif        /* DISPLAY_IS_IP */
     val = XDisplayString (Xdisplay);
   if (display_name == NULL)
     display_name = val;        /* use broken `:0' value */

   i = strlen (val);
   display_string = MALLOC ((i+9)*sizeof(char), "display_string");

   sprintf (display_string, "DISPLAY=%s", val);
   sprintf (windowid_string, "WINDOWID=%u", (unsigned int)TermWin.parent);

   /* add entries to the environment:
    * @ DISPLAY:   in case we started with -display
    * @ WINDOWID:  X window id number of the window
    * @ COLORTERM: terminal sub-name and also indicates its color
    * @ TERM:      terminal name
    */
   putenv (display_string);
   putenv (windowid_string);
   if (Xdepth <= 2)
     {
        putenv ("COLORTERM=" COLORTERMENV "-mono");
        putenv ("TERM=" TERMENV);
     }
   else
     {
#ifdef XPM_BACKGROUND
        putenv ("COLORTERM=" COLORTERMENV "-xpm");
#else
        putenv ("COLORTERM=" COLORTERMENV);
#endif
#ifdef DEFINE_XTERM_COLOR
        putenv ("TERM=" TERMENV "-color");
#else
        putenv ("TERM=" TERMENV);
#endif
     }
   if(is_term_on())
           bool_term_on=1;
   else
           if(Options & Opt_systemUse)
                   bool_term_on=0;
   xnc_go();
   init_command (cmd_argv);
   main_loop ();                /* main processing loop */
   return EXIT_SUCCESS;
}
/*}}}*/
/*----------------------- end-of-file (C source) -----------------------*/
