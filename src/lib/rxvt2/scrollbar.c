/*--------------------------------*-C-*---------------------------------*
 * File:        scrollbar.c
 *
 * scrollbar routines
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
#include "main.h"
#include "scrollbar.h"
/*}}}*/

/* extern functions referenced */
/* extern variables referenced */
/*{{{ extern variables declared here */
scrollBar_t scrollBar;
/*}}}*/

/*----------------------------------------------------------------------*
 */
static GC scrollbarGC;

#ifdef XTERM_SCROLLBAR                /* bitmap scrollbar */
# define sb_width 14
# define sb_height 2
static char sb_bits[] = {
   0xaa, 0x0a, 0x54, 0x15};

# if (SB_WIDTH != sb_width)
Error, check scrollbar width
# endif

#else        /* XTERM_SCROLLBAR */
static GC topShadowGC, botShadowGC;

/* draw triangular up button with a shadow of 2 pixels */
static void
Draw_up_button (int x, int y, int state)
{
   const unsigned int sz = (SB_WIDTH), sz2 = (SB_WIDTH / 2);
   XPoint pt [3];
   GC top, bot;

   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break;
    case -1: top = botShadowGC; bot = topShadowGC; break;
    case 0: top = bot = scrollbarGC; break;
   }

   /* fill triangle */
   pt[0].x = x;                        pt[0].y = y + sz - 1;
   pt[1].x = x + sz - 1;        pt[1].y = y + sz - 1;
   pt[2].x = x + sz2;                pt[2].y = y;
   XFillPolygon (Xdisplay, scrollBar.win, scrollbarGC,
                 pt, 3, Convex, CoordModeOrigin);

   /* draw base */
   XDrawLine (Xdisplay, scrollBar.win, bot,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);

   /* draw shadow */
   pt[1].x = x + sz2 - 1;        pt[1].y = y;
   XDrawLine (Xdisplay, scrollBar.win, top,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# if (SHADOW > 1)
   /* doubled */
   pt[0].x++; pt[0].y--; pt[1].y++;
   XDrawLine (Xdisplay, scrollBar.win, top,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# endif
   /* draw shadow */
   pt[0].x = x + sz2;                pt[0].y = y;
   pt[1].x = x + sz - 1;        pt[1].y = y + sz - 1;
   XDrawLine (Xdisplay, scrollBar.win, bot,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# if (SHADOW > 1)
   /* doubled */
   pt[0].y++; pt[1].x--; pt[1].y--;
   XDrawLine (Xdisplay, scrollBar.win, bot,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# endif
}

/* draw triangular down button with a shadow of 2 pixels */
static void
Draw_dn_button (int x, int y, int state)
{
   const unsigned int sz = (SB_WIDTH), sz2 = (SB_WIDTH / 2);
   XPoint pt [3];
   GC top, bot;

   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break;
    case -1: top = botShadowGC; bot = topShadowGC; break;
    case 0: top = bot = scrollbarGC; break;
   }

   /* fill triangle */
   pt[0].x = x;                        pt[0].y = y;
   pt[1].x = x + sz - 1;        pt[1].y = y;
   pt[2].x = x + sz2;                pt[2].y = y + sz;
   XFillPolygon (Xdisplay, scrollBar.win, scrollbarGC,
                 pt, 3, Convex, CoordModeOrigin);

   /* draw base */
   XDrawLine (Xdisplay, scrollBar.win, top,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);

   /* draw shadow */
   pt[1].x = x + sz2 - 1;                pt[1].y = y + sz - 1;
   XDrawLine (Xdisplay, scrollBar.win, top,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# if (SHADOW > 1)
   /* doubled */
   pt[0].x++; pt[0].y++; pt[1].y--;
   XDrawLine (Xdisplay, scrollBar.win, top,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# endif
   /* draw shadow */
   pt[0].x = x + sz2;                        pt[0].y = y + sz - 1;
   pt[1].x = x + sz - 1;                pt[1].y = y;
   XDrawLine (Xdisplay, scrollBar.win, bot,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# if (SHADOW > 1)
   /* doubled */
   pt[0].y--; pt[1].x--; pt[1].y++;
   XDrawLine (Xdisplay, scrollBar.win, bot,
              pt[0].x, pt[0].y, pt[1].x, pt[1].y);
# endif
}
#endif        /* XTERM_SCROLLBAR */

int
scrollbar_mapping (int map)
{
   int change = 0;
   if (map && !scrollbar_visible ())
     {
        scrollBar.state = 1;
        if(is_term_on())
                XMapWindow (Xdisplay, scrollBar.win);
        change = 1;
     }
   else if (!map && scrollbar_visible ())
     {
        scrollBar.state = 0;
        XUnmapWindow (Xdisplay, scrollBar.win);
        change = 1;
     }

   return change;
}

int
scrollbar_show (int update)
{
   static short last_top, last_bot;        /* old (drawn) values */

   if (!scrollbar_visible ()) return 0;

   if (scrollbarGC == None)
     {
        XGCValues gcvalue;
#ifdef XTERM_SCROLLBAR
        gcvalue.stipple = XCreateBitmapFromData (Xdisplay, scrollBar.win,
                                                 sb_bits,
                                                 sb_width,
                                                 sb_height);
        if (!gcvalue.stipple)
          {
             print_error ("can't create bitmap");
             exit (EXIT_FAILURE);
          }

        gcvalue.fill_style = FillOpaqueStippled;
        gcvalue.foreground = PixColors [fgColor];
        gcvalue.background = PixColors [bgColor];

        scrollbarGC = XCreateGC (Xdisplay, scrollBar.win,
                                 GCForeground|GCBackground |
                                 GCFillStyle|GCStipple,
                                 &gcvalue);
#else        /* XTERM_SCROLLBAR */
        gcvalue.foreground = (Xdepth <= 2 ?
                              PixColors [fgColor] :
                              PixColors [scrollColor]);
        scrollbarGC = XCreateGC (Xdisplay, scrollBar.win,
                                 GCForeground,
                                 &gcvalue);

        gcvalue.foreground = PixColors [topShadowColor];
        topShadowGC = XCreateGC (Xdisplay, scrollBar.win,
                                 GCForeground,
                                 &gcvalue);

        gcvalue.foreground = PixColors [bottomShadowColor];
        botShadowGC = XCreateGC (Xdisplay, scrollBar.win,
                                 GCForeground,
                                 &gcvalue);
#endif        /* XTERM_SCROLLBAR */
     }

   if (update)
     {
        int top = (TermWin.nscrolled - TermWin.view_start);
        int bot = top + (TermWin.nrow-1);
        int len = (TermWin.nscrolled + (TermWin.nrow-1));

        scrollBar.top = (scrollBar.beg +
                         (top * scrollbar_size ()) / len);
        scrollBar.bot = (scrollBar.beg +
                         (bot * scrollbar_size ()) / len);
        /* no change */
        if ((scrollBar.top == last_top) && (scrollBar.bot == last_bot))
          return 0;
     }

   /* instead of XClearWindow (Xdisplay, scrollBar.win); */
   if (last_top < scrollBar.top)
     XClearArea (Xdisplay, scrollBar.win,
                 0, last_top, SB_WIDTH, (scrollBar.top - last_top),
                 False);

   if (scrollBar.bot < last_bot)
     XClearArea (Xdisplay, scrollBar.win,
                 0, scrollBar.bot, SB_WIDTH, (last_bot - scrollBar.bot),
                 False);

   last_top = scrollBar.top;
   last_bot = scrollBar.bot;

   /* scrollbar slider */
   XFillRectangle (Xdisplay, scrollBar.win, scrollbarGC,
                   0, scrollBar.top, SB_WIDTH,
                   (scrollBar.bot - scrollBar.top));

#ifndef XTERM_SCROLLBAR
   /* shadow for scrollbar slider */
   Draw_Shadow (scrollBar.win,
                topShadowGC, botShadowGC,
                0, scrollBar.top, SB_WIDTH,
                (scrollBar.bot - scrollBar.top));

   /*
    * Redraw scrollbar arrows
    */
   Draw_up_button (0, 0, (scrollbar_isUp() ? -1 : +1));
   Draw_dn_button (0, (scrollBar.end + 1), (scrollbar_isDn() ? -1 : +1));
#endif        /* XTERM_SCROLLBAR */

   return 1;
}
/*----------------------- end-of-file (C source) -----------------------*/
