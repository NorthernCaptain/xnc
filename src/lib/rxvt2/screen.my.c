/*--------------------------------*-C-*---------------------------------*
 * File:        screen.c
 */
/*{{{ notes: */
/*----------------------------------------------------------------------*
 * This module is all new by Robert Nation
 * <nation@rocket.sanders.lockheed.com>
 *
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions are applied.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *
 * Design of this module was heavily influenced by the original xvt
 * design of this module. See info relating to the original xvt elsewhere
 * in this package.
 *----------------------------------------------------------------------*/
/*}}}*/
/*{{{ includes */
#include "main.h"

#include <sys/types.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#include <X11/Xatom.h>
#include <X11/Xmd.h>                /* CARD32 */

#include "command.h"
#include "debug.h"
#include "graphics.h"
#include "screen.h"
/*}}}*/

/*            
#ifndef XPM_BACKGROUND
#define XPM_BACKGROUND
#endif
*/
/*{{{ defines */
#define PROP_SIZE        4096
#define        TABSIZE                8        /* default tab size */

/* Memory copy methods -- rgg 20/11/95 */
#define MEMCOPY         memcpy          /* no overlap -- rgg 04/11/95 */
/* #define MEMCOPY      memmove */      /* allows overlap -- rgg 04/11/95 */
/*}}}*/

/*{{{ extern functions referenced */
/*}}}*/
/*{{{ extern variables referenced */
/*}}}*/
/*{{{ extern variables declared here */
#ifndef NO_BRIGHTCOLOR
unsigned int colorfgbg = DEFAULT_RSTYLE;
#endif
/*}}}*/

/* #define DEBUG_SCREEN */
/* to do self-check for internal screen corruption */
/* #define DEBUG_COLORS */
/* to print out current color/renditions as they change */

/*{{{ local variables */
/*----------------------------------------------------------------------*
 */
typedef unsigned char text_t;
typedef        unsigned int rend_t;

static rend_t rstyle = DEFAULT_RSTYLE;

#define SPACE_CHAR ' '
#ifdef OLD_COLOR_MODEL
# define FILL_STYLE        DEFAULT_RSTYLE
#else
# define FILL_STYLE        (rstyle)
#endif

/* macro prototypes */
void ZERO_LINES (text_t *, rend_t *, int /* nlines */);
void BLANK_FILL (text_t *, rend_t *, int /* nchars */);

/* zero both text and rendition */
#define ZERO_LINES(ptext,prend,nlines)        do {\
int n = (nlines) * (TermWin.ncol+1);\
if (n > 0) {\
memset ((ptext), 0, (n * sizeof(text_t)));\
memset ((prend), 0, (n * sizeof(rend_t)));\
}} while (0)

/* fill text with spaces and fill rendition */
#define        BLANK_FILL(ptext,prend,nchars)        do {\
int n = (nchars);\
if (n > 0) {\
rend_t * p = prend;\
memset ((ptext), SPACE_CHAR, (n * sizeof(text_t)));\
while (n-- > 0) *p++ = FILL_STYLE;\
}} while (0)
/*}}}*/
/*{{{ screen_t */
/*
 * how the screen accounting works
 *
 * `text' contains text including the scrollback buffer. Each line is a
 * fixed length [TermWin.ncol+1] with the final character of each:
 *        '\n':        for wrapped lines
 *        `\0':        for non-wrapped lines
 *
 * `rend' contains rendition information (font, bold, color, etc)
 *
 * the layout:
 * Rows [0 .. (TermWin.saveLines-1)] == scrollback region
 * Rows [TermWin.saveLines .. TermWin.saveLines + (TermWin.nrow-1)] ==
 *        screen region [0 .. (TermWin.nrow-1)]
 *
 * `row', `tscroll', `bscroll' are bounded by (0, TermWin.nrow)
 *
 * `col' is bounded by (0, TermWin.ncol)
 *
 * `TermWin.saveLines'
 *        is the maximum number of lines to save in the scrollback buffer.
 *        This is a fixed number for any particular rxvt instance and is set
 *        by the option `-sl' or resource `saveLines'
 *
 * `TermWin.nscrolled'
 *        how many lines have been scrolled (saved)
 *                0 <= TermWin.nscrolled <= TermWin.saveLines
 *
 * `TermWin.view_start'
 *        the offset back into the scrollback buffer for our current view
 *                -(TermWin.nscrolled) <= TermWin.view_start <= 0
 *
 * The selection region is defined for [0 .. (TermWin.nrow-1)], which
 * corresponds to the regular screen and for [-1 .. -(TermWin.nscrolled)]
 * which corresponds to the scrolled region.
 */

typedef struct {
   text_t * text;                /* all the text, including scrollback        */
   rend_t * rend;                /* rendition, using the `RS_' flags        */
   short row, col;                /* cursor position                        */
   short tscroll, bscroll;        /* top/bottom of scroll region                */
   short charset;                /* character set number [0..3]                */
   unsigned int flags;
} screen_t;
#define Screen_Relative                (1<<0)        /* relative origin mode flag        */
#define Screen_VisibleCursor        (1<<1)        /* cursor visible?                */
#define Screen_Autowrap                (1<<2)        /* auto-wrap flag                */
#define Screen_Insert                (1<<3)        /* insert mode (vs. overstrike)        */
#define Screen_WrapNext                (1<<4)        /* need to wrap for next char?        */

#define Screen_DefaultFlags        (Screen_VisibleCursor|Screen_Autowrap)

static screen_t screen =
{ NULL, NULL, 0, 0, 0, 0, 0, Screen_DefaultFlags };

#ifdef NO_SECONDARY_SCREEN
# define NSCREENS        0
#else
# define NSCREENS        1
static screen_t swap_screen = { NULL, NULL, 0, 0, 0, 0, 0, Screen_DefaultFlags };
#endif
/*}}}*/
/*{{{ local variables */
static short current_screen = PRIMARY;
static short rvideo = 0;        /* reverse video */

static char * tabs = NULL;        /* a 1 for a location with a tab-stop */
static text_t * linebuf = NULL;

#ifdef KANJI
static short multiByte = 0;
#endif

/* Data for save-screen */
static struct {
   short row, col;
   short charset;
   char charset_char;
   rend_t rstyle;
} save = { 0, 0, 0, 'B', DEFAULT_RSTYLE };

/* This tells what's actually on the screen */
static text_t * drawn_text = NULL;
static rend_t * drawn_rend = NULL;

static char charsets [4] = {'B','B','B','B'};        /* all ascii */
/*}}}*/
/*{{{ selection */
/* save selection text with '\n' line endings, but translate
 * '\n' to '\r' for pasting */
static struct {
   unsigned char * text;        /* selected text */
   int len;                        /* length of selected text */
   short op;                        /* current operation */
#define SELECTION_CLEAR        0
#define SELECTION_BEGIN        1
#define SELECTION_INIT        2
#define SELECTION_CONT        3
#define SELECTION_DONE        4
   short screen;                /* which screen is being used */
   struct {
      short row, col;
   } beg, end, mark;
} selection = {NULL, 0, SELECTION_CLEAR, PRIMARY, {0, 0}, {0, 0}, {0, 0} };
/* also could add in these:
 * int firstr, lastr;                -- firstr <= row < lastr
 * if trying to implement X11 mouse highlighting
 */

/*}}}*/

/*{{{ local functions referenced */
static int scroll_text (int row1, int row2, int count);
static inline void selection_check (void);
/*----------------------------------------------------------------------*/
/*}}}*/

/*----------------------------------------------------------------------*/
/*{{{ check_text() - check integrity of screen data structures */
#ifdef DEBUG_SCREEN
static void
check_text (const char * str)
{
   int r, x;
   static const char * prev = "?";

   fprintf (stderr, "%s\n", str);
   for (r = 0, x = TermWin.ncol;
        r < (TermWin.nrow + TermWin.saveLines);
        r++, x += (TermWin.ncol+1))
     {
        text_t final = screen.text [x];
             if (final != '\0' && final != '\n')
          {
             fprintf (stderr,"%s: %s Violation on row %d\n", str, prev, x);
             exit (EXIT_FAILURE);
          }
     }
   MEM_CHECK ("check_text", str);
   prev = str;
}
#else
# define check_text(a)        ((void)0)
#endif        /* DEBUG_SCREEN */
/*}}}*/

/*{{{ Kanji translation units */
#ifdef KANJI
static void
eucj2jis (unsigned char * str, int len)
{
   register int i;
   for (i = 0; i < len; i++)
     str [i] &= 0x7F;
}

static void
sjis2jis (unsigned char * str, int len)
{
   register int i;
   for (i = 0; i < len; i += 2, str += 2)
     {
        unsigned char * high = str;
        unsigned char * low  = str + 1;

        (*high) -= (*high > 0x9F ? 0xB1: 0x71);
        *high = (*high) * 2 + 1;
        if (*low > 0x9E)
          {
             *low -= 0x7E;
             (*high)++;
          }
        else
          {
             if (*low > 0x7E)
               (*low)--;
             *low -= 0x1F;
          }
     }
}
static void (*kanji_decode) (unsigned char * str, int len) = eucj2jis;

void
set_kanji_encoding (const char * str)
{
   if (str && *str)
     {
        if (!strcmp (str, "sjis"))                kanji_decode = sjis2jis;
        else if (!strcmp (str, "eucj"))                kanji_decode = eucj2jis;
     }
}
#endif        /* KANJI */
/*}}}*/

/*{{{ blank_lines() - add COUNT blank lines in the fill rendition style */
static inline void
blank_lines (text_t * text, rend_t * rend, int count)
{
   int r;

   if (count <= 0) return;

   /* fill with blank lines */
   BLANK_FILL (text, rend, (count * (TermWin.ncol+1)));

   /* terminate each line */
   text += (TermWin.ncol);
   for (r = 0; r < count; r++, text += (TermWin.ncol+1))
     *text = '\0';
}
/*}}}*/
/*{{{ Reset the screen */
/*
 * called whenever the screen needs to be repaired due
 * to re-sizing or initialization
 */
void
scr_reset (void)
{
   static short prev_nrow = -1, prev_ncol = -1;
   int i, ncol = 0;

   TermWin.view_start = 0;

   if (prev_ncol == TermWin.ncol && prev_nrow == TermWin.nrow) return;
#ifdef DEBUG_SCREEN
   fprintf (stderr, "reset\n");
#endif
   /* In case rows/columns are invalid */
   if (TermWin.ncol <= 0) TermWin.ncol = 80;
   if (TermWin.nrow <= 0) TermWin.nrow = 24;

#ifndef min
# define min(a,b) (((a)<(b)) ? (a) : (b))
# define max(a,b) (((a)>(b)) ? (a) : (b))
#endif
   ncol = min (prev_ncol, TermWin.ncol);

   if (prev_nrow < 0)
     TermWin.nscrolled = 0;

   for (i = 0; i <= NSCREENS; i++)
     {
        text_t * old_text;
        rend_t * old_rend;
        int r, oldr, oldr_max;
        int histsize = TermWin.saveLines;
        screen_t * scr = &screen;

        /* allocate space for scrollback (primary screen) + screen */
#if NSCREENS
        if (i)
          {
             histsize = 0;
             scr = &swap_screen;
          }
#endif

         /* copy from old buffer to new buffer, as appropriate. */
        oldr = oldr_max = prev_nrow;
        if (prev_nrow > 0)
          {
             int n = (TermWin.nrow - prev_nrow);
             if (n > 0)                /* window made taller */
               {
                  oldr = (TermWin.nrow);
                  if (histsize)
                    {
                       if (n <= TermWin.nscrolled)   /* enough */
                         {
                            scr->row += (n);
                            oldr = oldr_max;
                         }
                       else
                         {
                            scr->row += (TermWin.nscrolled);
                            oldr     -= (TermWin.nscrolled);
                         }
                       TermWin.nscrolled -= (n);
                    }
               }
             else if (n < 0)        /* window made shorter */
               {
                  if (scr->row < TermWin.nrow)
                    {
                       oldr_max = TermWin.nrow;
                    }
                  else                /* put extra into scrolled */
                    {
                       oldr_max = (scr->row+1);
                       if (histsize)
                         TermWin.nscrolled += (oldr_max - TermWin.nrow);
                    }
                  oldr = oldr_max;
               }
             oldr_max += histsize;
             oldr += histsize;
             oldr--;
          }

        if (scr->row < 0) scr->row = 0;
        else if (scr->row >= TermWin.nrow) scr->row = (TermWin.nrow-1);

        if (scr->col < 0) scr->col = 0;
        else if (scr->col >= TermWin.ncol) scr->col = (TermWin.ncol-1);

        /* reset scroll regions */
        scr->tscroll = 0; scr->bscroll = (TermWin.nrow-1);

        old_text = scr->text;
        old_rend = scr->rend;

        scr->text = MALLOC (((histsize+TermWin.nrow)*(TermWin.ncol+1)) * sizeof(text_t),
                            "text");
        scr->rend = MALLOC (((histsize+TermWin.nrow)*(TermWin.ncol+1)) * sizeof(rend_t),
                            "rend");

        blank_lines (scr->text, scr->rend, (histsize+TermWin.nrow));
        if (ncol > 0)
          {
             for (r = (TermWin.nrow+histsize-1);
                  r >= 0 && oldr >= 0;
                  r--, oldr--)
               {
                  if (oldr < oldr_max)
                    {
                       int roffset = r * (TermWin.ncol+1);
                       int oroffset = oldr * (prev_ncol+1);
                       memcpy (&scr->text [roffset], &old_text [oroffset],
                               ncol * sizeof(text_t));
                       memcpy (&scr->rend [roffset], &old_rend [oroffset],
                               ncol * sizeof(rend_t));
                    }
               }
          }

        FREE (old_text, "scr_reset", "scr_reset");
        FREE (old_rend, "scr_reset", "scr_reset");
     }

   /* Make sure the cursor is on the screen */
   if (TermWin.nscrolled < 0)
     TermWin.nscrolled = 0;
   else if (TermWin.nscrolled > TermWin.saveLines)
     TermWin.nscrolled = TermWin.saveLines;

   prev_ncol = TermWin.ncol;
   prev_nrow = TermWin.nrow;

   drawn_text = REALLOC (drawn_text,
                         (TermWin.nrow * (TermWin.ncol+1)) * sizeof(text_t),
                         "drawn_text");
   drawn_rend = REALLOC (drawn_rend,
                         (TermWin.nrow * (TermWin.ncol+1)) * sizeof(rend_t),
                         "drawn_rend");

   ZERO_LINES (drawn_text, drawn_rend, TermWin.nrow);

   /* ensure the cursor is on the screen */
   if (save.row >= TermWin.nrow) save.row = (TermWin.nrow-1);
   if (save.col >= TermWin.ncol) save.col = (TermWin.ncol-1);

   tabs = REALLOC (tabs, TermWin.ncol * sizeof(char), "tabs");
   linebuf = REALLOC (linebuf, (TermWin.ncol+1) * sizeof(text_t), "linebuf");

   memset (tabs, 0, TermWin.ncol * sizeof(char));
#if TABSIZE
   for (i = 0; i < TermWin.ncol; i += TABSIZE)
     tabs [i] = 1;
#endif
   tt_resize ();
}
/*}}}*/
/*{{{ Restore power-on configuration */
/*
 * Clears screen, restores default fonts, etc
 *
 * also reset the scrollback buffer
 */
void
scr_poweron (void)
{
   screen_t * scr = &screen;
   int i;

   TermWin.view_start = 0;
   TermWin.nscrolled = 0;        /* xterm doesn't do this */

   memset (charsets, 'B', sizeof(charsets));
   rvideo = 0;
   scr_rendition (0, ~RS_None);

#if NSCREENS
   scr_change_screen (SECONDARY);
   scr_erase_screen (2);
#endif
   scr_change_screen (PRIMARY);
   scr_erase_screen (2);

   for (i = 0; i <= NSCREENS; i++)
     {
        scr->tscroll = 0;
        scr->bscroll = (TermWin.nrow-1);
        scr->row = scr->col = 0;
        scr->charset = 0;
        scr->flags = Screen_DefaultFlags;
#if NSCREENS
        scr = &swap_screen;
#endif
     }
   scr_cursor (SAVE);
   ZERO_LINES (drawn_text, drawn_rend, TermWin.nrow);

   scr_reset ();
   XClearWindow (Xdisplay, TermWin.vt);
   scr_refresh (SLOW_REFRESH);
   Gr_reset ();
}
/*}}}*/

/*{{{ Set the rstyle parameter to reflect the selected font */
static inline void
set_font_style (void)
{
   rstyle &= ~RS_fontMask;
   switch (charsets [screen.charset]) {
      /* DEC Special Character and Line Drawing Set */
    case '0': rstyle |= RS_acsFont;        break;
      /* United Kingdom (UK) */
    case 'A': rstyle |= RS_ukFont;        break;
      /* United States (USASCII) */
      /* case 'B': break; */                /* ascii */

      /* <: Multinational character set */
      /* 5: Finnish character set */
      /* C: Finnish character set */
      /* K: German character set */
   }
}
/*}}}*/
/*{{{ Save/restore the cursor position and rendition style */
void
scr_cursor (int mode)
{
#ifdef DEBUG_COLORS
   debug_colors ();
#endif
   switch (mode) {
    case SAVE:
      save.row = screen.row;
      save.col = screen.col;
      save.rstyle = rstyle;
      save.charset = screen.charset;
      save.charset_char = charsets [save.charset];
      break;
    case RESTORE:
      screen.row = save.row;
      screen.col = save.col;
      rstyle = save.rstyle;
      screen.charset = save.charset;
      charsets [screen.charset] = save.charset_char;
      set_font_style ();
      break;
   }
#ifdef DEBUG_COLORS
   debug_colors ();
#endif
}
/*}}}*/

/*{{{ Change between the main and alternate screen */
int
scr_change_screen (int scrn)
{
   register int x;
   TermWin.view_start = 0;

   if (current_screen == scrn)
     return current_screen;
   else
     {
        int tmp;
        tmp = current_screen;
        current_screen = scrn;
        scrn = tmp;
     }

#if NSCREENS
   check_text ("change screen");

   /* swap screens, but leave scrollback untouched */
     {
        const int count = (TermWin.nrow) * (TermWin.ncol+1);
        const int roffset = (TermWin.saveLines) * (TermWin.ncol+1);

        text_t * text = &screen.text [roffset];
        rend_t * rend = &screen.rend [roffset];

        for (x = 0; x < count; x++)
          {
             register text_t t;
             register rend_t r;

             t = text [x];
             text [x] = swap_screen.text [x];
             swap_screen.text [x] = t;

             r = rend [x];
             rend [x] = swap_screen.rend [x];
             swap_screen.rend [x] = r;
          }
     }

   x = screen.row;
   screen.row = swap_screen.row;
   swap_screen.row = x;

   x = screen.col;
   screen.col = swap_screen.col;
   swap_screen.col = x;

   x = screen.charset;
   screen.charset = swap_screen.charset;
   swap_screen.charset = x;

   x = screen.flags;
   screen.flags = swap_screen.flags;
   swap_screen.flags = x;

   screen.flags |= Screen_VisibleCursor;
   swap_screen.flags |= Screen_VisibleCursor;

   if (Gr_Displayed ()) { Gr_scroll (0); Gr_ChangeScreen (); }
#else
   /* put contents of secondary screen into the scrollback */
   if (Gr_Displayed ()) Gr_ClearScreen ();
   else if (current_screen == PRIMARY)
     scroll_text (0, (TermWin.nrow-1), TermWin.nrow);
#endif
   return scrn;
}
/*}}}*/

/*{{{ debug_colors() */
#ifdef DEBUG_COLORS
static void
debug_colors (void)
{
   int color;
   char * name [] =
     { "fg", "bg",
        "black", "red", "green", "yellow",
        "blue", "magenta", "cyan", "white" };

   fprintf (stderr, "Color ( ");
   if (rstyle & RS_RVid)        fprintf (stderr, "rvid ");
   if (rstyle & RS_Bold)        fprintf (stderr, "bold ");
   if (rstyle & RS_Blink)        fprintf (stderr, "blink ");
   if (rstyle & RS_Uline)        fprintf (stderr, "uline ");
   fprintf (stderr, "): ");

   color = GET_FGCOLOR (rstyle);
#ifndef NO_BRIGHTCOLOR
   if (color >= minBright && color <= maxBright)
     {
        color -= (minBright - minColor);
        fprintf (stderr, "bright ");
     }
#endif
   fprintf (stderr, "%s on ", name [color]);

   color = GET_BGCOLOR (rstyle);
#ifndef NO_BRIGHTCOLOR
   if (color >= minBright && color <= maxBright)
     {
        color -= (minBright - minColor);
        fprintf (stderr, "bright ");
     }
#endif
   fprintf (stderr, "%s\n", name [color]);
}
#else
# define debug_colors()        ((void)0)
#endif        /* DEBUG_COLORS */
/*}}}*/
/*{{{ Set foreground/background text color */
/*
 * Intensity:
 *        RS_Bold  = foreground
 *        RS_Blink = background
 */
void
scr_color (unsigned int color, unsigned int Intensity)
{
   switch (color) {
    case restoreFG: color = fgColor; assert (Intensity == RS_Bold);        break;
    case restoreBG: color = bgColor; assert (Intensity == RS_Blink);        break;

    default:
      if (Xdepth <= 2)                /* Monochrome - ignore color changes */
        {
           switch (Intensity) {
            case RS_Bold:  color = fgColor; break;
            case RS_Blink: color = bgColor; break;
           }
        }
#ifndef NO_BRIGHTCOLOR
      else
        {
           if ((rstyle & Intensity) && color >= minColor && color <= maxColor)
             color += (minBright - minColor);
           else if (color >= minBright && color <= maxBright)
             {
                if (rstyle & Intensity)
                  return;                /* already bold enough! */
                color -= (minBright - minColor);
             }
        }
#endif        /* NO_BRIGHTCOLOR */
      break;
   }

   switch (Intensity) {
    case RS_Bold:  rstyle = SET_FGCOLOR (rstyle, color); break;
    case RS_Blink: rstyle = SET_BGCOLOR (rstyle, color); break;
   }
   debug_colors ();
}

#ifdef RXVT_GRAPHICS
/* return the current foreground/background colors */
inline int scr_get_fgcolor (void) { return GET_FGCOLOR (rstyle); }
inline int scr_get_bgcolor (void) { return GET_BGCOLOR (rstyle); }
#endif
/*}}}*/
/*{{{ Change the rendition style */
void
scr_rendition (int set, int style)
{
   unsigned int color;

   debug_colors ();
   if (set)
     {
        rstyle |= style;        /* set rendition */
        switch (style) {
#ifndef NO_BRIGHTCOLOR
         case RS_Bold:
           color = GET_FGCOLOR (rstyle);
           scr_color ((color == fgColor ?
                       GET_FGCOLOR (colorfgbg) : color), RS_Bold);
           break;

         case RS_Blink:
           color = GET_BGCOLOR (rstyle);
           scr_color ((color == bgColor ?
                       GET_BGCOLOR (colorfgbg) : color), RS_Blink);
           break;
#endif        /* NO_BRIGHTCOLOR */

         case RS_RVid:
           if (rvideo) rstyle &= ~(RS_RVid);        /* reverse video mode */
           break;
        }
     }
   else
     {
        rstyle &= ~(style);        /* unset rendition */

        switch (style) {
         case ~RS_None:                /* default fg/bg colors */
           rstyle = DEFAULT_RSTYLE;
           if (rvideo) rstyle |= RS_RVid;        /* reverse video mode */
           break;

#ifndef NO_BRIGHTCOLOR
         case RS_Bold:
           color = GET_FGCOLOR (rstyle);
           if (color >= minBright && color <= maxBright)
             {
                scr_color (color, RS_Bold);
                /* scr_color (color - (minBright - minColor), RS_Bold); */
                if ((rstyle & RS_fgMask) == (colorfgbg & RS_fgMask))
                  scr_color (restoreFG, RS_Bold);
             }
           break;

         case RS_Blink:
           color = GET_BGCOLOR (rstyle);
           if (color >= minBright && color <= maxBright)
             {
                scr_color (color, RS_Blink);
                /* scr_color (color - (minBright - minColor), RS_Blink); */
                if ((rstyle & RS_bgMask) == (colorfgbg & RS_bgMask))
                  scr_color (restoreBG, RS_Blink);
             }
           break;
#endif        /* NO_BRIGHTCOLOR */

         case RS_RVid:
           if (rvideo) rstyle |= RS_RVid;        /* reverse video mode */
           break;
        }
     }
   debug_colors ();
}
/*}}}*/

/*{{{ Add lines */
void
scr_add_lines (const unsigned char * str, int nlines, int cnt)
{
   int i, roffset;                /* row offset */
#ifdef KANJI
   enum {SBYTE, WBYTE1, WBYTE2} chstat = SBYTE;
#endif

   TermWin.view_start = 0;

   check_text ("add lines");
   if (selection.op) selection_check ();

   /* do scrolling up front, most of the time */
   if (nlines > 0)
     {
        nlines += (screen.row - screen.bscroll);

        if ((nlines > 0) &&
            (screen.tscroll == 0) &&
            (screen.bscroll == (TermWin.nrow-1)))
          {
             scroll_text (screen.tscroll, screen.bscroll, nlines);

             /* add text into the scrollback buffer */
             screen.row -= nlines;
             if (screen.row < -TermWin.saveLines)
               screen.row = -TermWin.saveLines;
          }
     }

   if (screen.col >= TermWin.ncol) screen.col = (TermWin.ncol-1);

   roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
   for (i = 0; i < cnt; i++)
     {
        /*
         * Adds a single character at the current cursor location
         * new lines in the string
         */
        if (Gr_Displayed ()) Gr_scroll (0);

#ifdef KANJI
        switch (chstat)
#endif
          {
#ifdef KANJI
           case WBYTE1:        break;                /* never happens? */

           case WBYTE2:
             rstyle |= RS_kanjiMask;
             chstat = SBYTE;
             break;

           case SBYTE:
             if (multiByte || (str [i] & 0x80))
               {
                  rstyle &= ~RS_kanjiMask;
                  rstyle |= RS_kanji1;
                  chstat = WBYTE2;
               }
             else
#endif        /* KANJI */
               switch (str [i]) {
                case 127:                        continue; break;
                case '\t':        scr_tab (1);        continue; break;
                case '\n':
                  screen.flags &= ~Screen_WrapNext;
                  screen.text [roffset + (TermWin.ncol)] = '\0';
                  if (screen.row == screen.bscroll)
                    scroll_text (screen.tscroll, screen.bscroll, 1);
                  else if (screen.row < (TermWin.nrow-1))
                    {
                       screen.row++;
                       roffset = ((screen.row + TermWin.saveLines) *
                                  (TermWin.ncol+1));
                    }
                  continue; break;

                case '\r':
                  screen.col = 0;
                  screen.flags &= ~Screen_WrapNext;
                  screen.text [roffset + (TermWin.ncol)] = '\0';
                  continue; break;

                default:
#ifdef KANJI
                  rstyle &= ~(RS_kanjiMask);
#endif
                  break;
               }
          }

        if (screen.flags & Screen_WrapNext)
          {
             screen.text [roffset + (TermWin.ncol)] = '\n'; /* wrap line */
             if (screen.row == screen.bscroll)
               scroll_text (screen.tscroll, screen.bscroll, 1);
             else if (screen.row < (TermWin.nrow-1))
               {
                  screen.row++;
                  roffset = ((screen.row + TermWin.saveLines) *
                             (TermWin.ncol+1));
               }
             screen.col = 0;
             screen.flags &= ~Screen_WrapNext;
          }
        if (screen.flags & Screen_Insert)
          scr_insdel_chars (1, INSERT);
        screen.text [roffset + (screen.col)] = str [i];
        screen.rend [roffset + (screen.col)] = rstyle;

        screen.col++;
        if (screen.col == TermWin.ncol)
          {
             screen.col--;
             if (screen.flags & Screen_Autowrap)
               screen.flags |= Screen_WrapNext;
             else
               screen.flags &= ~Screen_WrapNext;
          }
     }
}
/*}}}*/

/*{{{ Scroll text on the screen */
/*
 * Scroll COUNT lines from ROW1 to ROW2 inclusive (ROW1 <= ROW2)
 * scrolling is up for a +ve COUNT and down for a -ve COUNT
 */
static int
scroll_text (int row1, int row2, int count)
{
   int r;
   text_t *t_dst, *t_src;
   rend_t *r_dst, *r_src;

   if (selection.op)                /* move selected region too */
     {
        selection.beg.row -= count;
        selection.end.row -= count;
        selection.mark.row -= count;
        /*
         * could check ranges here and make sure selection is okay
         * don't scroll into scrollback depending on the region etc,
         * but leave for now
         */
     }

   if (count > 0)                /* scroll up */
     {
        int n, x;
        /* if the line scrolls off the top of the screen,
         * shift the entire scrollback buffer too */
        if ((row1 == 0) && (current_screen == PRIMARY))
          {
             row1 = -TermWin.saveLines;
             TermWin.nscrolled += count;
             if (TermWin.nscrolled > TermWin.saveLines)
               TermWin.nscrolled = TermWin.saveLines;
          }

        x = ((row1 + TermWin.saveLines) * (TermWin.ncol+1));
        t_dst = &screen.text [x];
        r_dst = &screen.rend [x];

        n = (row2 - row1 + 1);
        if (count > n)
          {
             count = n;
             n = 0;
          }
        else
          {
             n -= count;
          }

        x += count * (TermWin.ncol+1);
        t_src = &screen.text [x];
        r_src = &screen.rend [x];

        /* Forward overlapping memcpy's -- probably OK */
        if (n > 0)
          {
             n *= (TermWin.ncol+1);
             MEMCOPY (t_dst, t_src, n * sizeof(text_t));
             MEMCOPY (r_dst, r_src, n * sizeof(rend_t));
             t_dst += n;
             r_dst += n;
          }
#if 0        /* this destroys the '\n' mark for the autowrapped lines */
        *(t_dst-1) = '\0';        /* terminate previous line */
#endif
        /* copy blank lines in at the bottom */
        blank_lines (t_dst, r_dst, count);
     }
   else if (count < 0)                /* scroll down */
     {
        int x;

        /* do one line at a time to avoid backward overlapping memcpy's */
        x = (row2 + TermWin.saveLines) * (TermWin.ncol+1);
        t_dst = &screen.text [x];
        r_dst = &screen.rend [x];

        x += (count) * (TermWin.ncol+1);
        t_src = &screen.text [x];
        r_src = &screen.rend [x];
        for (r = row2; r >= (row1 - count); r--)
          {
             MEMCOPY (t_dst, t_src, (TermWin.ncol+1) * sizeof(text_t));
             t_dst -= (TermWin.ncol+1);
             t_src -= (TermWin.ncol+1);

             MEMCOPY (r_dst, r_src, (TermWin.ncol+1) * sizeof(rend_t));
             r_dst -= (TermWin.ncol+1);
             r_src -= (TermWin.ncol+1);
          }

        /* copy blank lines in at the top */
        for (; r >= row1; r--)
          {
             BLANK_FILL (t_dst, r_dst, TermWin.ncol);
             t_dst [TermWin.ncol] = '\0';
             t_dst -= (TermWin.ncol+1);
             r_dst -= (TermWin.ncol+1);
          }
     }
   if (Gr_Displayed ()) Gr_scroll (count);
   return count;
}
/*}}}*/

/*{{{ Handle a backspace */
void
scr_backspace (void)
{
   check_text ("backspace");

   if (selection.op) selection_check ();
   if (screen.col == 0 && screen.row > 0)
     {
        screen.row--;
        screen.col = (TermWin.ncol-1);
     }
   else if (screen.flags & Screen_WrapNext)
     {
        int roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
        screen.text [roffset + (TermWin.ncol)] = '\0';
        screen.flags &= ~Screen_WrapNext;
     }
   else
     scr_gotorc (0, -1, RELATIVE);
}
/*}}}*/

/*{{{ Move the cursor to a new tab position */
/*
 * COUNT is +ve, move forward.  COUNT is -ve, move backward
 */
void
scr_tab (int count)
{
   int x = screen.col;

   if (count > 0)                 /* tab forward */
     {
        int i;
        for (i = x+1; i < TermWin.ncol; i++)
          {
             if (tabs [i])
               {
                  x = i;
                  count--;
                  if (!count) break;
               }
          }
     }
   else if (count < 0)                /* tab backward */
     {
        int i;
        count = -count;
        for (i = x-1; i >= 0; i--)
          {
             if (tabs [i])
               {
                  x = i;
                  count--;
                  if (!count) break;
               }
          }
     }
   else
     return;

   if (x != screen.col)
     scr_gotorc (0, x, R_RELATIVE);
}
/*}}}*/

/*{{{ Move the cursor to a new position */
/*
 * The relative argument is a pair of flags that specify relative
 * rather than absolute motion.
 */
void
scr_gotorc (int row, int col, int relative)
{
   TermWin.view_start = 0;
   if (Gr_Displayed ()) Gr_scroll (0);

   screen.col = (relative & C_RELATIVE ? (screen.col + col) : col);

   if (screen.col < 0) screen.col = 0;
   else if (screen.col >= TermWin.ncol) screen.col = (TermWin.ncol-1);

   if (screen.flags & Screen_WrapNext)
     {
        int roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
        screen.text [roffset + (TermWin.ncol)] = '\0';
        screen.flags &= ~Screen_WrapNext;
     }

   if (relative & R_RELATIVE)
     {
        if (row > 0)
          {
             if ((screen.row <= screen.bscroll) &&
                 (screen.row + row > screen.bscroll))
               screen.row = screen.bscroll;
             else
               screen.row += row;
          }
        else if (row < 0)
          {
             if ((screen.row >= screen.tscroll) &&
                 (screen.row + row < screen.tscroll))
               screen.row = screen.tscroll;
             else
               screen.row += row;
          }
     }
   else
     {
        if (screen.flags & Screen_Relative)        /* relative origin mode */
          {
             screen.row = row + screen.tscroll;
             if (screen.row > screen.bscroll)
               screen.row = screen.bscroll;
          }
        else
          screen.row = row;
     }
   if (screen.row < 0) screen.row = 0;
   else if (screen.row >= TermWin.nrow) screen.row = (TermWin.nrow-1);
}
/*}}}*/

/*{{{ Move the cursor down one line and scroll if necessary */
void
scr_index (int dirn)
{
   TermWin.view_start = 0;
   if (Gr_Displayed ()) Gr_scroll (0);

   check_text ("index");

   if (screen.flags & Screen_WrapNext)
     {
        int roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
        screen.text [roffset + (TermWin.ncol)] = '\0';
        screen.flags &= ~Screen_WrapNext;
     }

   if ((screen.row == screen.bscroll && dirn == UP) ||
       (screen.row == screen.tscroll && dirn == DN))
     scroll_text (screen.tscroll, screen.bscroll, dirn);
   else
     screen.row += dirn;
   if (selection.op) selection_check ();
}
/*}}}*/

/*{{{ Erase part or the whole of a line */
void
scr_erase_line (int mode)
{
   int count, roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
   TermWin.view_start = 0;
   if (Gr_Displayed ()) Gr_scroll (0);

   if (screen.flags & Screen_WrapNext)
     {
        screen.text [roffset + (TermWin.ncol)] = '\0';
        screen.flags &= ~Screen_WrapNext;
     }
   switch (mode) {
    case 0:                /* erase to end */
      check_text ("erase line End");
      roffset += (screen.col);
      count = (TermWin.ncol - screen.col);
      break;

    case 1:                /* erase to beginning */
      check_text ("erase line Start");
      count = (screen.col + 1);
      break;

    case 2:                /* erase entire */
      check_text ("erase line Entire");
      count = (TermWin.ncol);
      break;

    default:
      check_text ("erase line None");
      return;
   }

   BLANK_FILL (&screen.text [roffset], &screen.rend [roffset], count);

   check_text ("erase line Done");
   if (selection.op) selection_check ();
}
/*}}}*/

/*{{{ Erase part or the whole of the screen */
void
scr_erase_screen (int mode)
{
   int count, roffset = (TermWin.saveLines) * (TermWin.ncol+1);
   TermWin.view_start = 0;

   switch (mode) {
    case 0:                        /* erase to end */
      check_text ("erase screen End");
      scr_erase_line (0);
      count = (TermWin.nrow - (screen.row + 1));
      roffset += ((screen.row + 1) * (TermWin.ncol+1));
      break;

    case 1:                        /* erase to beginning */
      check_text ("erase screen Start");
      scr_erase_line (1);
      count = screen.row;
      break;

    case 2:                        /* erase entire */
      check_text ("erase screen Entire");
      Gr_ClearScreen ();
      count = TermWin.nrow;
      break;

    default:
      check_text ("erase screen None");
      return;
      break;
   }

   blank_lines (&screen.text [roffset], &screen.rend [roffset], count);
   check_text ("erase screen Done");
}
/*}}}*/

/*{{{ Fill screen with E's */
void
scr_E (void)
{
   int r, roffset = (TermWin.saveLines) * (TermWin.ncol+1);

   check_text ("E");
   TermWin.view_start = 0;
   memset (&screen.text [roffset], 'E',
           ((TermWin.nrow) * (TermWin.ncol+1)) * sizeof(text_t));
   for (r = 0; r < (TermWin.nrow); r++, roffset += (TermWin.ncol+1))
     screen.text [roffset + TermWin.ncol] = '\0';
}
/*}}}*/

/*{{{ Insert or Delete COUNT lines and scroll */
/*
 * insdel == +1
 *        delete lines, scroll up the bottom of the screen to fill the gap
 * insdel == -1
 *        insert lines, scroll down the lower lines
 * other values of insdel are undefined
 */
void
scr_insdel_lines (int count, int insdel)
{
   check_text ("insdel lines");

   if (screen.row > screen.bscroll)
     return;

   if (count > (screen.bscroll - screen.row + 1))
     {
        if (insdel == DELETE)
          return;
        else if (insdel == INSERT)
          count = (screen.bscroll - screen.row + 1);
     }

   TermWin.view_start = 0;
   if (Gr_Displayed ()) Gr_scroll (0);

   if (screen.flags & Screen_WrapNext)
     {
        int roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
        screen.text [roffset + (TermWin.ncol)] = '\0';
        screen.flags &= ~Screen_WrapNext;
     }

   scroll_text (screen.row, screen.bscroll, insdel * count);
}
/*}}}*/

/*{{{ Insert or Delete COUNT characters from the current position */
/*
 * insdel == +2, erase  chars
 * insdel == +1, delete chars
 * insdel == -1, insert chars
 */
void
scr_insdel_chars (int count, int insdel)
{
   int roffset = (screen.row + TermWin.saveLines) * (TermWin.ncol+1);
   text_t *text, *textend;
   rend_t *rend, *rendend;

   check_text ("insdel chars");

   if (insdel == ERASE)
     {
        if (count > screen.col)
          count = screen.col;
        if (count <= 0)
          return;
        screen.col -= count;        /* move backwards */
        insdel = DELETE;        /* delete chars */
     }
   else if (count > (TermWin.ncol - screen.col))
     {
        count = (TermWin.ncol - screen.col);
     }
   if (count <= 0)
     return;

   TermWin.view_start = 0;
   if (Gr_Displayed ()) Gr_scroll (0);
   if (selection.op) selection_check ();

   screen.text [roffset + (TermWin.ncol)] = '\0';
   screen.flags &= ~Screen_WrapNext;

   text = &screen.text [roffset + (screen.col)];
   rend = &screen.rend [roffset + (screen.col)];
   if (insdel == DELETE)
     {
        /* overlapping copy */
        for (/*nil*/; (*text && text [count]); text++, rend++)
          {
             *text = text [count];
             *rend = rend [count];
          }

        /* fill in the end of the line */
        for (/*nil*/; *text; text++, rend++)
          {
             *text = SPACE_CHAR;
             *rend = FILL_STYLE;
          }
     }
   else
     {
        /* INSERT count characters */
        textend = &screen.text [roffset + (TermWin.ncol-1)];
        rendend = &screen.rend [roffset + (TermWin.ncol-1)];

        for (/*nil*/; (textend - count >= text); textend--, rendend--)
          {
             *textend = *(textend-count);
             *rendend = *(rendend-count);
          }

        /* fill in the gap */
        for (/*nil*/; (textend >= text); textend--, rendend--)
          {
             *textend = SPACE_CHAR;
             *rendend = FILL_STYLE;
          }
     }
}
/*}}}*/

/*{{{ Set the scroll region */
void
scr_scroll_region (int top, int bot)
{
   if (top < 0) top = 0;
   if (bot >= TermWin.nrow) bot = (TermWin.nrow-1);
   if (top > bot) return;

   screen.tscroll = top;
   screen.bscroll = bot;
   scr_gotorc (0, 0, 0);
}
/*}}}*/

/*{{{ set visible/invisible cursor */
void
scr_cursor_visible (int mode)
{
   if (mode) screen.flags |= Screen_VisibleCursor;
   else screen.flags &= ~Screen_VisibleCursor;
}
/*}}}*/
/*{{{ Set/Unset automatic wrapping */
void
scr_autowrap (int mode)
{
   if (mode) screen.flags |= Screen_Autowrap;
   else screen.flags &= ~Screen_Autowrap;
}
/*}}}*/

/*{{{ Set/Unset margin origin mode */
/*
 * In absolute origin mode, line numbers are counted relative to top margin
 * of screen, the cursor can be moved outside the scrolling region. In
 * relative mode line numbers are relative to top margin of scrolling
 * region and the cursor cannot be moved outside
 */
void
scr_relative_origin (int mode)
{
   if (mode) screen.flags |= Screen_Relative;
   else screen.flags &= ~Screen_Relative;
   scr_gotorc (0, 0, 0);
}
/*}}}*/

/*{{{ Set/Unset automatic insert mode */
void
scr_insert_mode (int mode)
{
   if (mode) screen.flags |= Screen_Insert;
   else screen.flags &= ~Screen_Insert;
}
/*}}}*/

/*{{{ Move the display to line represented by scrollbar */
/*
 * Move the display so that line represented by scrollbar value Y is at
 * the top of the screen
 */
int
scr_move_to (int y, int len)
{
   int start = TermWin.view_start;

   TermWin.view_start = ((len - y) * ((TermWin.nrow-1) + TermWin.nscrolled)
                         / (len)) - (TermWin.nrow-1);

   if (TermWin.view_start < 0)
     TermWin.view_start = 0;
   else if (TermWin.view_start > TermWin.nscrolled)
     TermWin.view_start = TermWin.nscrolled;

   if (Gr_Displayed ()) Gr_scroll (0);

   return (TermWin.view_start - start);
}
/*}}}*/

/*{{{ page the screen up/down NLINES */
int
scr_page (int dirn, int nlines)
{
   int start = TermWin.view_start;

   if (!dirn || !nlines)
     return 0;

   if (nlines <= 0)
     nlines = 1;
   else if (nlines > TermWin.nrow)
     nlines = TermWin.nrow;
   TermWin.view_start += nlines * dirn;

   if (TermWin.view_start < 0)
     TermWin.view_start = 0;
   else if (TermWin.view_start > TermWin.nscrolled)
     TermWin.view_start = TermWin.nscrolled;

   if (Gr_Displayed ()) Gr_scroll (0);

   return (TermWin.view_start - start);
}
/*}}}*/

/*{{{ selection service functions */
/*
 * If (row,col) is within a selected region of text, remove the selection
 */
static inline void
selection_check (void)
{
   int c1, c2, r1, r2;
   check_text ("check selection");

   if (current_screen != selection.screen)
     return;

   if ((selection.mark.row < -TermWin.nscrolled) ||
       (selection.mark.row >= TermWin.nrow) ||
       (selection.beg.row < -TermWin.nscrolled) ||
       (selection.beg.row >= TermWin.nrow) ||
       (selection.end.row < -TermWin.nscrolled) ||
       (selection.end.row >= TermWin.nrow))
     {
        selection_reset ();
        return;
     }

   r1 = (screen.row - TermWin.view_start);
   c1 = ((r1 - selection.mark.row) * (r1 - selection.end.row));

   /* selection.mark.row > screen.row - TermWin.view_start
    * or
    * selection.end.row > screen.row - TermWin.view_start
    */
   if (c1 < 0)
     selection_reset ();
   /* selection.mark.row == screen.row || selection.end.row == screen.row */
   else if (c1 == 0)
     {
        /* We're on the same row as the start or end of selection */
        if ((selection.mark.row < selection.end.row) ||
            ((selection.mark.row == selection.end.row) &&
             (selection.mark.col < selection.end.col)))
          {
             r1 = selection.mark.row;
             c1 = selection.mark.col;
             r2 = selection.end.row;
             c2 = selection.end.col;
          }
        else
          {
             r1 = selection.end.row;
             c1 = selection.end.col;
             r2 = selection.mark.row;
             c2 = selection.mark.col;
          }
        if ((screen.row == r1) && (screen.row == r2))
          {
             if ((screen.col >= c1) && (screen.col <= c2))
               selection_reset ();
          }
        else if (((screen.row == r1) && (screen.col >= c1)) ||
                 ((screen.row == r2) && (screen.col <= c2)))
          selection_reset ();
     }
}

#if 0
static inline void
selection_range (int firstr, int lastr)
{
   if (firstr >= lastr ||
       firstr < 0 || firstr >= TermWin.nrow ||
       lastr <= 0 || lastr > TermWin.nrow)
    return;
   selection.firstr = firstr;
   selection.lastr = lastr;
}
#endif
/*}}}*/

/*{{{ make selection */
/*
 * make the selection currently delimited by the selection end markers
 */
void
selection_make (Time tm)
{
   unsigned char * str;
   int r, startr, startc, endr, endc;
   int roffset;                        /* row offset */

   switch (selection.op) {
    case SELECTION_CONT:
      break;

    case SELECTION_INIT:
      selection_reset ();
      selection.end.row = selection.mark.row = selection.beg.row;
      selection.end.col = selection.mark.col = selection.beg.col;
      /*drop*/
    case SELECTION_BEGIN:
      selection.op = SELECTION_DONE;
      /*drop*/
    default:
      return;
      break;
   }
   selection.op = SELECTION_DONE;

   FREE (selection.text, "sel_make", "sel_make");
   selection.text = NULL;
   selection.len = 0;

   selection.screen = current_screen;
   /* Set start/end row/col to point to the selection endpoints */
   if (selection.end.row < selection.mark.row ||
       (selection.end.row == selection.mark.row &&
        selection.end.col <= selection.mark.col))
     {
        startr = selection.end.row; endr = selection.mark.row;
        startc = selection.end.col; endc = selection.mark.col;
     }
   else
     {
        startr = selection.mark.row; endr = selection.end.row;
        startc = selection.mark.col; endc = selection.end.col;
     }

   if ((startr < -TermWin.nscrolled || endr >= TermWin.nrow))
     {
        selection_reset ();
        return;
     }

   str = MALLOC (((endr - startr + 1)*(TermWin.ncol+1) + 1) * sizeof(char),
                 "sel_text");
   selection.text = str;
   *str = '\0';

   /* save all points between start and end with selection flag */
   roffset = ((startr + TermWin.saveLines) * (TermWin.ncol+1));
   for (r = startr; r <= endr; r++, roffset += (TermWin.ncol+1))
     {
        int c;
        int c1 = (r == startr ? startc : 0);
        int c2 = (r == endr ? endc : (TermWin.ncol-1));

#ifdef KANJI
        if ((screen.rend [roffset+c1] & RS_kanjiMask) == RS_kanjiMask) c1--;
        if ((screen.rend [roffset+c2] & RS_kanjiMask) == RS_kanji1) c2++;
#endif        /* KANJI */
        for (c = c1; c <= c2; c++)
          *str++ = screen.text [roffset + c];

        /*
         * end-of-line and not autowrap
         * remove trailing space, but don't remove an entire line!
         */
        if (c2 == (TermWin.ncol-1) && !screen.text [roffset + (TermWin.ncol)])
          {
             str--;
             for (c = c2; c >= c1 && isspace (*str); c--) str--;
             str++;
             *str++ = '\n';
          }
     }
   *str = '\0';

   selection.len = strlen (selection.text);
   if (selection.len <= 0)
     return;
   XSetSelectionOwner (Xdisplay, XA_PRIMARY, TermWin.vt, tm);
   if (XGetSelectionOwner (Xdisplay, XA_PRIMARY) != TermWin.vt)
     print_error ("can't get primary selection");

   /* Place in CUT_BUFFER0 for backup */
   XChangeProperty (Xdisplay, Xroot, XA_CUT_BUFFER0,
                    XA_STRING, 8, PropModeReplace,
                    selection.text, selection.len);
}
/*}}}*/

/*{{{ respond to a request for our current selection */
void
selection_send (XSelectionRequestEvent * rq)
{
   XEvent ev;
   static Atom xa_targets = None;
   if (xa_targets == None)
     xa_targets = XInternAtom (Xdisplay, "TARGETS", False);

   ev.xselection.type      = SelectionNotify;
   ev.xselection.property  = None;
   ev.xselection.display   = rq->display;
   ev.xselection.requestor = rq->requestor;
   ev.xselection.selection = rq->selection;
   ev.xselection.target           = rq->target;
   ev.xselection.time      = rq->time;

    if (rq->target == xa_targets)
     {
        /*
         * On some systems, the Atom typedef is 64 bits wide.
         * We need to have a typedef that is exactly 32 bits wide,
         * because a format of 64 is not allowed by the X11 protocol.
         */
        typedef CARD32 Atom32;

        Atom32 target_list[2];

         target_list[0] = (Atom32) xa_targets;
         target_list[1] = (Atom32) XA_STRING;

         XChangeProperty (Xdisplay, rq->requestor, rq->property,
                          xa_targets, 8*sizeof(target_list[0]), PropModeReplace,
                          (char *)target_list,
                         sizeof(target_list)/sizeof(target_list[0]));
         ev.xselection.property = rq->property;
     }
   else if (rq->target == XA_STRING)
     {
        XChangeProperty (Xdisplay, rq->requestor, rq->property,
                         XA_STRING, 8, PropModeReplace,
                         selection.text, selection.len);
        ev.xselection.property = rq->property;
     }
   XSendEvent (Xdisplay, rq->requestor, False, 0, &ev);
}
/*}}}*/

/*{{{ paste selection */
static void
PasteIt (unsigned char * data, unsigned int nitems)
{
   unsigned char * p = data, * pmax = data + nitems;

   for (nitems = 0; p < pmax; p++)
     {
        /* do newline -> carriage-return mapping */
        if (*p == '\n')
          {
             unsigned char cr = '\r';
             tt_write (data, nitems);
             tt_write (&cr, 1);
             data += (nitems+1);
             nitems = 0;
          }
        else
          nitems++;
     }
   if (nitems)
     tt_write (data, nitems);
}

/*
 * Respond to a notification that a primary selection has been sent
 */
void
selection_paste (Window win, unsigned prop, int Delete)
{
   long nread, bytes_after;

   if (prop == None)
     return;

   nread = 0;
   do
     {
        unsigned char * data;
        Atom actual_type;
        int actual_fmt;
        long nitems;

        if ((XGetWindowProperty (Xdisplay, win, prop,
                                 nread/4, PROP_SIZE, Delete,
                                 AnyPropertyType, &actual_type, &actual_fmt,
                                 &nitems, &bytes_after,
                                 &data) != Success) ||
            (actual_type != XA_STRING))
          {
             XFree (data);
             return;
          }

        nread += nitems;
        PasteIt (data, nitems);

        XFree (data);
     }
   while (bytes_after > 0);
}
/*}}}*/

/*{{{ Request the current primary selection */
void
selection_request (Time tm, int x, int y)
{
   /* is release within the window? */
   if (x < 0 || y < 0 || x >= TermWin.width || y >= TermWin.height)
     return;

   if (selection.text != NULL)
     {
        /* internal selection */
        PasteIt (selection.text, selection.len);
     }
   else if (XGetSelectionOwner (Xdisplay, XA_PRIMARY) == None)
     {
        /* no primary selection - use CUT_BUFFER0 */
        selection_paste (Xroot, XA_CUT_BUFFER0, False);
     }
   else
     {
        Atom prop = XInternAtom (Xdisplay, "VT_SELECTION", False);
        XConvertSelection (Xdisplay, XA_PRIMARY, XA_STRING,
                           prop, TermWin.vt, tm);
     }
}
/*}}}*/

/*{{{ Clear the current selection */
void
selection_reset (void)
{
   int x, nrow = TermWin.nrow;

   selection.op = SELECTION_CLEAR;
   selection.end.row = selection.mark.row = 0;
   selection.end.col = selection.mark.col = 0;

   if (current_screen == PRIMARY)
     nrow += TermWin.saveLines;

   for (x = 0; x < nrow * (TermWin.ncol+1); x++)
     screen.rend [x] &= ~(RS_Select);
}

void
selection_clear (void)
{
   FREE (selection.text, "sel_clear", "sel_clear");
   selection.text = NULL;
   selection.len = 0;

   selection.op = SELECTION_CLEAR;
   selection_reset ();
}
/*}}}*/

/*{{{ mark selected points (used by selection_extend) */
static void
selection_setclr (int set, int startr, int startc, int endr, int endc)
{
   int r, roffset = ((startr + TermWin.saveLines) * (TermWin.ncol+1));

   /* startr <= endr */
   if ((startr < -TermWin.nscrolled) || (endr >= TermWin.nrow))
     {
        selection_reset ();
        return;
     }

   for (r = startr; r <= endr; r++)
     {
        int c1 = (r == startr ? startc : 0);
        int c2 = (r == endr ? endc : (TermWin.ncol-1));

#ifdef KANJI
        if ((screen.rend [roffset+c1] & RS_kanjiMask) == RS_kanjiMask) c1--;
        if ((screen.rend [roffset+c2] & RS_kanjiMask) == RS_kanji1) c2++;
#endif        /* KANJI */
        for (/*nil*/; c1 <= c2; c1++)
          {
             if (set)
               screen.rend [roffset + c1] |= RS_Select;
             else
               screen.rend [roffset + c1] &= ~(RS_Select);
          }
        roffset += (TermWin.ncol+1);
     }
}
/*}}}*/

/*{{{ start a selection at the specified col/row */
static void
selection_start_colrow (int col, int row)
{
   if (selection.op)
     {
        /* startr <= endr */
        if ((selection.end.row < -TermWin.nscrolled) ||
            (selection.mark.row < - TermWin.nscrolled))
          {
             selection_reset ();
          }
        else                        /* direction of new selection */
          {
             if (selection.end.row < selection.mark.row ||
                 (selection.end.row == selection.mark.row &&
                  selection.end.col <= selection.mark.col))
               selection_setclr (0,    /* up */
                                 selection.end.row, selection.end.col,
                                 selection.mark.row, selection.mark.col);
             else
               selection_setclr (0,        /* down */
                                 selection.mark.row, selection.mark.col,
                                 selection.end.row, selection.end.col);
          }
     }
   selection.op = SELECTION_INIT;

   selection.beg.col = col;
   selection.beg.row = row;
   selection.beg.row -= TermWin.view_start;
}

/*
 * start a selection at the specified x/y pixel location
 */
void
selection_start (int x, int y)
{
   selection_start_colrow (Pixel2Col (x), Pixel2Row (y));
}
/*}}}*/

/*{{{ extend the selection to the specified col/row */
static void
selection_extend_colrow (int col, int row)
{
   int old_row, old_col, old_dirn, dirn;

   switch (selection.op) {
    case SELECTION_INIT:
      selection_reset ();
      selection.end.col = selection.mark.col = selection.beg.col;
      selection.end.row = selection.mark.row = selection.beg.row;
      /*drop*/
    case SELECTION_BEGIN:
      selection.op = SELECTION_BEGIN;
      break;

    case SELECTION_DONE:
    case SELECTION_CONT:
      selection.op = SELECTION_CONT;
      break;

    case SELECTION_CLEAR:
      selection_start_colrow (col, row);
      /*drop*/
    default:
      return;
      break;
   }

   /* Remember old selection for virtual removal */
   old_row = selection.end.row;
   old_col = selection.end.col;

   if ((old_row < -TermWin.nscrolled) ||
       (selection.mark.row < -TermWin.nscrolled))
     {
        selection_reset ();
        return;
     }

   /* Figure out where new selection is */
   selection.end.col = col;
   selection.end.row = row;

   if (selection.end.col < 0) selection.end.col = 0;
   else if (selection.end.col >= TermWin.ncol)
     selection.end.col = (TermWin.ncol-1);

   selection.end.row -= TermWin.view_start;
   if (selection.end.row < -TermWin.nscrolled)
     {
        selection_reset ();
        return;
     }
   else if (selection.end.row >= TermWin.nrow)
     selection.end.row = (TermWin.nrow-1);

   if ((selection.op == SELECTION_BEGIN) &&
       ((selection.end.col != selection.mark.col) ||
        (selection.end.row != selection.mark.row)))
     selection.op = SELECTION_CONT;

   /* If new selection is same as old selection just return
    * or if no highlighting was requested
    */
   if (selection.end.row == old_row && selection.end.col == old_col)
     return;

   /* virtual removal -- delete old highlighting and replace with new */

   /* determine direction of old selection */
   old_dirn = ((old_row < selection.mark.row ||
                (old_row == selection.mark.row &&
                 old_col <= selection.mark.col)) ? UP : DN);

   /* determine direction of new selection */
   dirn = ((selection.end.row < selection.mark.row ||
            (selection.end.row == selection.mark.row &&
             selection.end.col <= selection.mark.col)) ? UP : DN);

   /* If old and new direction are different, clear old, set new */
   if (dirn != old_dirn)
     {
        if (old_dirn == UP)
          {
             selection_setclr (0,
                               old_row, old_col,
                               selection.mark.row, selection.mark.col);
             selection_setclr (1,
                               selection.mark.row, selection.mark.col,
                               selection.end.row, selection.end.col);
          }
        else
          {
             selection_setclr (0,
                               selection.mark.row, selection.mark.col,
                               old_row, old_col);
             selection_setclr (1,
                               selection.end.row, selection.end.col,
                               selection.mark.row, selection.mark.col);
          }
     }
   else
     {
        if (old_dirn == UP)
          {
             if (old_row < selection.end.row ||
                 (old_row == selection.end.row &&
                  old_col < selection.end.col))
               {
                  selection_setclr (0,
                                    old_row, old_col,
                                    selection.end.row, selection.end.col);
                  selection_setclr (1,
                                    selection.end.row, selection.end.col,
                                    selection.end.row, selection.end.col);
               }
             else
               {
                  selection_setclr (1,
                                    selection.end.row, selection.end.col,
                                    old_row, old_col);
               }
          }
        else
          {
             if (selection.end.row < old_row ||
                 (selection.end.row == old_row &&
                  selection.end.col < old_col))
               {
                  selection_setclr (0,
                                    selection.end.row, selection.end.col,
                                    old_row, old_col);
                  selection_setclr (1,
                                    selection.end.row, selection.end.col,
                                    selection.end.row, selection.end.col);
               }
             else
               {
                  selection_setclr (1,
                                    old_row, old_col,
                                    selection.end.row, selection.end.col);
               }
          }
     }
}

/*
 * extend the selection to the specified x/y pixel location
 */
void
selection_extend (int x, int y)
{
   selection_extend_colrow (Pixel2Col (x), Pixel2Row (y));
}
/*}}}*/

/*{{{ double/triple click selection */
/*
 * by Edward. Der-Hua Liu, Taiwan
 * cut char support added by A. Haritsis <ah@doc.ic.ac.uk>
 */
void
selection_click (int clicks, int x, int y)
{
   if (clicks <= 1)
     {
        selection_start (x, y);                /* single click */
     }
   else
     {
        int beg_c, end_c, beg_r, end_r;
        text_t * text;

        /* ensure rows/columns are on the screen */
        x = Pixel2Col (x);
        x = (x <= 0 ? 0 : (x >= TermWin.ncol ? (TermWin.ncol-1) : x));
        beg_c = end_c = x;

        y = Pixel2Row (y);
        y = (y <= 0 ? 0 : (y >= TermWin.nrow ? (TermWin.nrow-1) : y));
        beg_r = end_r = y;

        switch (clicks) {
         case 3:
           /*
            * triple click
            */
           beg_c = 0;
           end_c = (TermWin.ncol-1);
           break;

         case 2:
           /*
            * double click: handle autowrapped lines
            */
           for (text = (screen.text +
                        (beg_r + TermWin.saveLines - TermWin.view_start) *
                        (TermWin.ncol+1));
                /*forever*/;
                beg_r--, text -= (TermWin.ncol+1))
             {
                while (beg_c > 0 &&
                       !strchr (rs_cutchars, text [beg_c-1]))
                  beg_c--;

                 if (beg_c == 0 &&
                    beg_r > (TermWin.view_start - TermWin.nscrolled) &&
                    *(text-1) == '\n' &&
                    !strchr (rs_cutchars, *(text-2)))
                  beg_c = (TermWin.ncol-1);
                else
                  break;
             }

           for (text = (screen.text +
                        (end_r + TermWin.saveLines - TermWin.view_start) *
                        (TermWin.ncol+1));
                /*forever*/;
                end_r++, text += (TermWin.ncol+1))
             {
                while (end_c < (TermWin.ncol-1) &&
                       !strchr (rs_cutchars, text [end_c+1]))
                  end_c++;

                if (end_c == (TermWin.ncol-1) &&
                    end_r < (TermWin.view_start + TermWin.nrow-1) &&
                    text [TermWin.ncol] == '\n' &&
                    !strchr (rs_cutchars, text [TermWin.ncol+1]))
                  end_c = 0;
                else
                  break;
             }
           break;

         default:
           return;
           break;
        }
        selection_start_colrow (beg_c, beg_r);
        selection_extend_colrow (end_c, end_r);
     }
}
/*}}}*/

/*{{{ Report the current cursor position */
void
scr_report_position (void)
{
   tt_printf ("\033[%d;%dR", screen.row + 1, screen.col + 1);
}
/*}}}*/

/*{{{ Charset/Font functions */
/*
 * choose a font
 */
void
scr_charset_choose (int set)
{
   screen.charset = set;
   set_font_style ();
}

/*
 * Set a font
 */
void
scr_charset_set (int set, unsigned int ch)
{
#ifdef KANJI
   multiByte = (set < 0);
   set = abs (set);
#endif
   charsets [set] = (unsigned char) ch;
   set_font_style ();
}
/*}}}*/

/*{{{ scr_expose / scr_touch */
/*
 * for the box starting at x, y with size width, height
 * touch the displayed values
 */
void
scr_expose (int x, int y, int width, int height)
{
   int row, col, end_row, end_col;

   if (drawn_text == NULL) return;

   check_text ("touch");

   col = Pixel2Col (x);
   row = Pixel2Row (y);
   if (col < 0) col = 0; else if (col >= TermWin.ncol) col = (TermWin.ncol-1);
   if (row < 0) row = 0; else if (row >= TermWin.nrow) row = (TermWin.nrow-1);
   end_col = col + 1 + Pixel2Width (width);
   end_row = row + 1 + Pixel2Height (height);

   if (end_row >= TermWin.nrow) end_row = (TermWin.nrow-1);
   if (end_col >= TermWin.ncol) end_col = (TermWin.ncol-1);

   width = (end_col - col + 1);
   for (/*nil*/; row <= end_row; row++)
     {
        int roffset = (col + row * (TermWin.ncol+1));
        memset (&drawn_text [roffset], 0, width * sizeof(text_t));
        memset (&drawn_rend [roffset], 0, width * sizeof(rend_t));
     }
}

/* touch the entire screen */
void
scr_touch (void) { scr_expose (0, 0, TermWin.width, TermWin.height); }
/*}}}*/

/*{{{ Refresh screen */
/*
 * refresh the region defined by rows STARTR and ENDR, inclusively.
 *
 * Actually draws to the X window
 * For X related speed-ups, this is a good place to fiddle.
 * The arrays drawn_text and drawn_rend contain what I
 * believe is currently shown on the screen. The arrays in screen contain
 * what should be displayed. This routine can decide how to refresh the
 * screen. Calls in command.c decide when to refresh.
 */

void
scr_refresh (int type)
{
#ifdef XPM_BACKGROUND
# undef USE_XCOPYAREA
# ifndef NO_BOLDOVERSTRIKE
#  define NO_BOLDOVERSTRIKE
# endif
# ifdef XPM_BUFFERING
   int update = 0;
#  define drawBuffer        (TermWin.buf_pixmap)
# endif        /* XPM_BUFFERING */
#endif        /* XPM_BACKGROUND */

#ifndef drawBuffer
# define drawBuffer        (TermWin.vt)
#endif

   static int last_xcursor = 0;
   int r, roffset, doffset, xcursor;
#ifdef KANJI
   static int wbyte = 0;
#endif

   if (type == NO_REFRESH)        /* Window not visible, don't update */
     return;

   check_text ("refresh region");

   if (last_xcursor < (TermWin.nrow * (TermWin.ncol+1)))
     {
        /* make sure to update it */
           drawn_rend [last_xcursor] = RS_attrMask;
     }

   xcursor = ((screen.row + TermWin.saveLines)*(TermWin.ncol+1)
              + screen.col);
   last_xcursor = (screen.row + TermWin.view_start);
   if (last_xcursor >= TermWin.nrow)
     {
        last_xcursor = 0;
     }
   else
     {
        last_xcursor *= (TermWin.ncol+1);
        if (screen.flags & Screen_VisibleCursor)
          {
             screen.rend [xcursor] |= RS_Cursor;
#ifdef KANJI
             if ((screen.rend [xcursor] & RS_kanjiMask) == RS_kanji1)
               screen.rend [xcursor+1] |= RS_Cursor;
#endif
          }
     }
   last_xcursor += screen.col;

#ifdef USE_XCOPYAREA
   /*
    * scroll using bitblt wherever possible
    * a dirty approximation will ignore the rendition field here
    * and fix it up later
    */
   if (type == FAST_REFRESH && !Gr_Displayed ())
     for (r = 0; r < TermWin.nrow; r++)
     {
        int k;
        int doffset = r * (TermWin.ncol+1);
        int roffset = doffset + ((TermWin.saveLines - TermWin.view_start) *
                             (TermWin.ncol+1));

        if (!memcmp (&drawn_text [doffset],
                     &screen.text [roffset],
                     (TermWin.ncol) * sizeof(text_t)))
          continue;

        /*
         * look for a similar line
         */
        for (doffset = 0, k = 0;
             k < TermWin.nrow;
             k++, doffset += (TermWin.ncol + 1))
          {
             if (!memcmp (&drawn_text [doffset],
                          &screen.text [roffset],
                          TermWin.ncol * sizeof(text_t)))
               break;
          }
        /* found it */
        if (k < TermWin.nrow)
          {
             int count;
             int j = r;

             roffset += (TermWin.ncol+1);
             doffset += (TermWin.ncol+1);
             r++;
             for (count = 1;
                  ((r < TermWin.nrow) &&
                   !memcmp (&drawn_text [doffset],
                            &screen.text [roffset],
                            TermWin.ncol * sizeof(text_t))
                   );
                  count++, r++)
               {
                  roffset += (TermWin.ncol+1);
                  doffset += (TermWin.ncol+1);
               }
             r--;
             XCopyArea (Xdisplay,
                        TermWin.vt,
                        TermWin.vt,
                        TermWin.gc,
                        TermWin_internalBorder, Row2Pixel (k),
                        Width2Pixel (1), Height2Pixel (count),
                        TermWin_internalBorder, Row2Pixel (j));

             /*
              * Forward overlapping memcpy's are probably OK,
              * but backwards doesn't work on SunOS 4.1.3
              */
             k *= (TermWin.ncol+1);
             j *= (TermWin.ncol+1);
             if (k > j)
               {
                  while (count-- > 0)
                    {
                       MEMCOPY (&drawn_text [j],
                                &drawn_text [k],
                                count * (TermWin.ncol+1) * sizeof(text_t));
                       MEMCOPY (&drawn_rend [j],
                                &drawn_rend [k],
                                count * (TermWin.ncol+1) * sizeof(rend_t));
                       k += (TermWin.ncol+1);
                       j += (TermWin.ncol+1);
                    }
               }
             else
               {
                  k += (count-1) * (TermWin.ncol+1);
                  j += (count-1) * (TermWin.ncol+1);
                  while (count-- > 0)
                    {
                       MEMCOPY (&drawn_text [j],
                                &drawn_text [k],
                                (TermWin.ncol+1) * sizeof(text_t));
                       MEMCOPY (&drawn_rend [j],
                                &drawn_rend [k],
                                (TermWin.ncol+1) * sizeof(rend_t));
                       k -= (TermWin.ncol+1);
                       j -= (TermWin.ncol+1);
                    }
               }
          }
     }
#endif        /* USE_XCOPYAREA */

   doffset = 0 * (TermWin.ncol+1);
   roffset = doffset + ((TermWin.saveLines - TermWin.view_start) * (TermWin.ncol+1));

   /* For a first cut, do it one character at a time */
   for (r = 0;
        r < TermWin.nrow;
        roffset += (TermWin.ncol+1),
        doffset += (TermWin.ncol+1),
        r++)
     {
        int c;
        int ypixel = TermWin.font->max_bounds.ascent + Row2Pixel (r);

#ifndef KANJI
        /* fast way to avoid the next loop (most of the time) ? */
        if (!memcmp (&drawn_text [doffset],
                     &screen.text [roffset],
                     (TermWin.ncol) * sizeof(text_t)) &&
            !memcmp (&drawn_rend [doffset],
                     &screen.rend [roffset],
                     (TermWin.ncol) * sizeof(rend_t)))
          continue;
#endif

        for (c = 0; c < TermWin.ncol; c++)
          {
             int count;
             int x = roffset + c;
             int x1 = doffset + c;

             if ((drawn_text [x1] != screen.text [x]) ||
                 (drawn_rend [x1] != screen.rend [x])
#ifdef KANJI
                 || (((screen.rend [x] & RS_kanjiMask) == RS_kanji1) &&
                     (drawn_text [x1+1] != screen.text [x+1]))
#endif        /* KANJI */
                 )
               {
                  int fore, back, rend;
                  XGCValues gcvalue;                /* GC values */
                  unsigned long gcmask = 0;        /* GC mask */
                  int outlineCursor = False;        /* block cursor */
                  int xpixel = Col2Pixel (c);

                  drawn_text [x1] = screen.text [x];
                  drawn_rend [x1] = screen.rend [x];
                  linebuf [0] = screen.text [x];
                  rend = screen.rend [x];

                  x++; c++;
                  for (count = 1;
                       (c < TermWin.ncol &&
#ifdef KANJI
                        ((rend & ~RS_kanji0) ==
                         (screen.rend [x] & ~RS_kanji0)) &&
#else        /* KANJI */
                        (rend == screen.rend [x]) &&
#endif        /* KANJI */
                        (drawn_text [doffset+c] != screen.text [x] ||
                         drawn_rend [doffset+c] != screen.rend [x] ||
#ifdef KANJI
                         (x > 0 && (screen.rend [x-1] & ~RS_kanji1)) ||
#endif        /* KANJI */
                         (c+1 < TermWin.ncol &&
                          drawn_text [doffset+c+1] != screen.text [x+1])));
                       count++, x++, c++)
                    {
                       drawn_text [doffset+c] = screen.text [x];
                       drawn_rend [doffset+c] = screen.rend [x];
                       linebuf [count] = screen.text [x];
                    }
                  c--;
                  linebuf [count] ='\0';        /* zero-terminate */
#ifdef KANJI
                  /* ensure the correct font is used */
                  if (rend & RS_kanji1)
                    {
                       if (!wbyte)
                         {
                            wbyte = 1;
                            XSetFont (Xdisplay, TermWin.gc, TermWin.kanji->fid);
                         }
                       if (linebuf [0] & 0x80)
                         kanji_decode (linebuf, count);
                    }
                  else
                    {
                       if (wbyte)
                         {
                            wbyte = 0;
                            XSetFont (Xdisplay, TermWin.gc, TermWin.font->fid);
                         }
                    }
#endif        /* KANJI */

                  fore = GET_FGCOLOR (rend);
                  back = GET_BGCOLOR (rend);
                  rend = GET_ATTR (rend);
                  if (rend)
                    {
                       int rvid = 0;

                       if ((rend & (RS_RVid|RS_Select)) == (RS_RVid|RS_Select))
                         rend &= ~(RS_RVid|RS_Select);
                       else if (rend & (RS_RVid|RS_Select))
                         rvid = 1;

                       if (rend & RS_Cursor)
                         {
                            if (!TermWin.focus)
                              {
                                 outlineCursor = True;
                                 rend &= ~(RS_Cursor);
                              }
                            else
                              rvid = (!rvid
#ifndef NO_CURSORCOLOR
                                      || (PixColors [cursorColor] != PixColors [bgColor])
#endif
                                      );
                         }

                       /* swap foreground/background colors */
                       if (rvid)
                         {
                            int tmp = back; back = fore; fore = tmp;
                         }

                       /*
                        * do some font character switching
                        */
                       switch (rend & RS_fontMask) {
                        case RS_acsFont:
                          for (x = 0; x < count; x++)
                            if (linebuf [x] >= 0x5F && linebuf [x] < 0x7F)
                              linebuf [x] = (linebuf [x] == 0x5F ?
                                             0x7F : linebuf [x] - 0x5F);
                          break;
                        case RS_ukFont:
                          for (x = 0; x < count; x++)
                            if (linebuf [x] == '#') linebuf [x] = '\036';
                          break;
                       }
                    }

                  /* bold characters - order of preference:
                   * 1 - change the foreground color to colorBD
                   * 2 - change the foreground color to bright
                   * 3 - use boldFont
                   * 4 - simulate with overstrike
                   */
#ifdef NO_BRIGHTCOLOR
# define MonoBold(x)        ((x) & (RS_Bold|RS_Blink))
#else        /* NO_BRIGHTCOLOR */
# define MonoBold(x)        (((x) & RS_Bold) && fore == fgColor)
#endif        /* NO_BRIGHTCOLOR */

/* # define MonoBold(x)        ((((x) & RS_Bold) && fore == fgColor) || (((x) & RS_Blink) && back == bgColor)) */
/*
 * blink simulated by simulated bold (overstrike) seems a bit farfetched
 * comment this out (v2.17 - 04APR96) and see how many people complain
 */

                  if (fore != fgColor)
                    {
                       gcvalue.foreground = PixColors [fore];
                       gcmask |= GCForeground;
                    }
#ifndef NO_BOLDUNDERLINE
                  else
                    {
                       if (rend & RS_Bold)  /* do bold first */
                         {
                            gcvalue.foreground = PixColors [colorBD];
                            if (gcvalue.foreground != PixColors [fgColor])
                              {
                                 gcmask |= GCForeground;
                                 rend &= ~RS_Bold;
                              }
                         }
                       else if (rend & RS_Uline)
                         {
                            gcvalue.foreground = PixColors [colorUL];
                            if (gcvalue.foreground != PixColors [fgColor])
                              {
                                 gcmask |= GCForeground;
                                 rend &= ~RS_Uline;
                              }
                         }
                    }
#endif        /* NO_BOLDUNDERLINE */

                  if (back != bgColor)
                    {
                       gcvalue.background = PixColors [back];
                       gcmask |= GCBackground;
                    }

#ifndef NO_CURSORCOLOR
                  if ((rend & RS_Cursor) &&
                      (PixColors [cursorColor] != PixColors [bgColor]))
                    {
                       gcvalue.background = PixColors [cursorColor];
                       gcmask |= GCBackground;
                       if (PixColors [cursorColor2] != PixColors [fgColor])
                         {
                            gcvalue.foreground = PixColors [cursorColor2];
                            gcmask |= GCForeground;
                         }
                    }
#endif        /* NO_CURSORCOLOR */

                  if (gcmask)
                    XChangeGC (Xdisplay, TermWin.gc, gcmask, &gcvalue);

#ifdef XPM_BACKGROUND
# ifdef XPM_BUFFERING
                  update = 1;
# endif        /* XPM_BUFFERING */
#endif        /* XPM_BACKGROUND */

/*----------------------------------------------------------------------*/
/*
 * how to write the strings
 */
#undef drawStringPrep
#undef drawString

#ifdef XPM_BACKGROUND
# ifdef XPM_BUFFERING
#  define        drawStringPrep()\
XCopyArea (Xdisplay, TermWin.pixmap, drawBuffer, TermWin.gc,\
        xpixel, (ypixel - TermWin.font->ascent),\
        Width2Pixel (count), Height2Pixel (1), \
        xpixel, (ypixel - TermWin.font->ascent))
# else        /* XPM_BUFFERING */
#  define        drawStringPrep()\
XClearArea (Xdisplay, drawBuffer, xpixel, \
        (ypixel - TermWin.font->max_bounds.ascent),\
        Width2Pixel (count), Height2Pixel (1), 0)
# endif        /* XPM_BUFFERING */
# endif /* XPM_BACKGROUND */
#define drawString(strFunc)\
strFunc (Xdisplay, drawBuffer, TermWin.gc, xpixel, ypixel, linebuf, count)
/*----------------------------------------------------------------------*/

# ifdef KANJI
                  if (wbyte)
                    {
                       count /= 2;
# ifdef XPM_BACKGROUND
                       if (back == bgColor)
                         {
                            drawStringPrep ();
                            drawString (XDrawString16);
                         }
                       else
# endif        /* XPM_BACKGROUND */
                         drawString (XDrawImageString16);
#ifndef NO_BOLDOVERSTRIKE
                       if (MonoBold (rend))
                         {
                            xpixel++;
                            drawString (XDrawString16);
                            xpixel--;
                         }
#endif        /* NO_BOLDOVERSTRIKE */
                    }
                  else
#endif        /* KANJI */

#ifndef NO_BOLDFONT
                    if (MonoBold (rend) && TermWin.boldFont != NULL)
                    {
                       XSetFont (Xdisplay, TermWin.gc, TermWin.boldFont->fid);

#ifdef XPM_BACKGROUND
                       if (back == bgColor)
                         {
                            drawStringPrep ();
                            drawString (XDrawString);
                         }
                       else
#endif        /* XPM_BACKGROUND */
                         drawString (XDrawImageString);
                       XSetFont (Xdisplay, TermWin.gc, TermWin.font->fid);
                    }
                  else
#endif        /* NO_BOLDFONT */
                    {
#ifdef XPM_BACKGROUND
                       if (back == bgColor)
                         {
                            drawStringPrep ();
                            drawString (XDrawString);
                         }
                       else
#endif        /* XPM_BACKGROUND */
                         drawString (XDrawImageString);

#ifndef NO_BOLDOVERSTRIKE
                       if (MonoBold (rend))
                         {
                            xpixel++;
                            drawString (XDrawString);
                            xpixel--;
                         }
#endif        /* NO_BOLDOVERSTRIKE */
                    }
#undef MonoBold

                  /*
                   * On the smallest font, underline overwrites next row
                   */
                  if ((rend & RS_Uline) && (TermWin.font->descent > 1))
                    {
                       XDrawLine (Xdisplay,
                                  drawBuffer,
                                  TermWin.gc,
                                  xpixel,
                                  ypixel + 1,
                                  xpixel + Width2Pixel (count) - 1,
                                  ypixel + 1);
                    }

                  if (outlineCursor)
                    {
#ifndef NO_CURSORCOLOR
                       if (PixColors [cursorColor] != PixColors [bgColor])
                         {
                            gcvalue.foreground = PixColors [cursorColor];
                            gcmask |= GCForeground;
                            XChangeGC (Xdisplay, TermWin.gc, gcmask, &gcvalue);
                         }
#endif        /* NO_CURSORCOLOR */
                       XDrawRectangle (Xdisplay,
                                       drawBuffer,
                                       TermWin.gc,
                                       xpixel,
                                       (ypixel - TermWin.font->ascent),
                                       Width2Pixel (1) - 1,
                                       Height2Pixel (1) - 1);
                    }
                  if (gcmask)
                    {
                       /* restore normal colors */
                       gcvalue.foreground = PixColors [fgColor];
                       gcvalue.background = PixColors [bgColor];
                       XChangeGC (Xdisplay, TermWin.gc, gcmask, &gcvalue);
                    }
               }
          }
     }
   if (screen.flags & Screen_VisibleCursor)
     {
        screen.rend [xcursor] &= ~(RS_Cursor);
#ifdef KANJI
        screen.rend [xcursor+1] &= ~(RS_Cursor);
#endif        /* KANJI */
     }

#ifdef XPM_BUFFERING
   if (update)
     {
        XClearArea (Xdisplay,
                    TermWin.vt,
                    0, Row2Pixel (0),
                    TermWin_TotalWidth (),
                    TermWin_TotalHeight (),
                    0);
        XFlush (Xdisplay);
     }
#endif        /* XPM_BUFFERING */

#undef drawStringPrep
#undef drawString
#undef drawBuffer
}
/*}}}*/

/*{{{ Tabs: set/clear */
/*
 *  -1 = clear all tabs
 *  +0 = clear tab stop at current column
 *  +1 = set   tab stop at current column
 */
void
scr_set_tab (int mode)
{
   if (mode < 0)
     memset (tabs, 0, TermWin.ncol * sizeof(char));
   else if (screen.col < TermWin.ncol)
     tabs [screen.col] = (mode != 0);
}
/*}}}*/

/*{{{ toggle reverse video settings */
void
scr_rvideo_mode (int mode)
{
   if (rvideo != mode)
     {
        register int x, count;

        rvideo = mode;
        rstyle ^= RS_RVid;

        count = (TermWin.nrow + TermWin.saveLines) * (TermWin.ncol+1);
        for (x = 0; x < count; x++) screen.rend [x] ^= RS_RVid;
        scr_refresh (SLOW_REFRESH);

#ifdef DEBUG_COLORS
        debug_colors ();
#endif
     }
}
/*}}}*/

/*{{{ Handle receipt of a bell character */
void
scr_bell (void)
{
#ifndef NO_MAPALERT
#ifdef MAPALERT_OPTION
   if (Options & Opt_mapAlert)
#endif
   if(is_term_on())
     XMapWindow (Xdisplay, TermWin.parent);
#endif

   if (Options & Opt_visualBell)
     {
        scr_rvideo_mode (!rvideo);
        scr_rvideo_mode (!rvideo);
     }
   else
     XBell (Xdisplay, 0);
}
/*}}}*/

/*{{{ Print-Pipe */
#ifdef PRINTPIPE
void
scr_printscreen (int fullhist)
{
   text_t * text;
   int r, nrows;
   FILE * fd;

   if ((fd = popen_printer ()) == NULL)
     return;

   nrows = TermWin.nrow;
   if (fullhist)
     nrows += TermWin.view_start;

   text = &screen.text [((TermWin.saveLines - TermWin.view_start) *
                         (TermWin.ncol+1))];
   for (r = 0; r < nrows; r++, text += (TermWin.ncol+1))
     {
        int i;
        for (i = (TermWin.ncol-1); i >= 0 && isspace (text [i]); i--) /*nil*/;
        i++;
        fprintf (fd, "%.*s\n", i, text);
     }

   pclose_printer (fd);
}
#endif
/*}}}*/

/*{{{ Debugging for selection */
/*
 * hidden debugging dump
 */
#ifdef DEBUG_SELECTION
static void
debug_PasteIt (unsigned char * data, int nitems)
{
   unsigned char * p, * pmax = data + nitems;
   int i;
   printf ("<text>\n\"");

   for (nitems = 0, p = data; p < pmax; p++)
     {
        if (*p == '\n')
          {
             printf ("[%d chars]\n%.*s\\n\n", nitems, nitems, data);
             data += (nitems+1);
             nitems = 0;
          }
        else if (*p == '\r')
          {
             printf ("[%d chars]\n%.*s\\r\n", nitems, nitems, data);
             data += (nitems+1);
             nitems = 0;
          }
        else
          nitems++;
     }
   if (nitems)
     printf ("%*s", (nitems), data);
   printf ("\"\n</text>");
}

int
debug_selection (void)
{
   printf ("\n%dx%d [chars] %dx%d [pixels] (font: %ux%u), scroll = %d/%d\n",
           TermWin.ncol,   TermWin.nrow,
           TermWin.width,  TermWin.height,
           Width2Pixel (1), Height2Pixel (1),
           screen.tscroll, screen.bscroll);

   printf ("%d lines scrollback, %d lines scrolled, start = %d\n",
           TermWin.saveLines, TermWin.nscrolled, TermWin.view_start);

   printf ("selection = screen %d, op = ", selection.screen);

   switch (selection.op) {
    case SELECTION_CLEAR:        printf ("CLEAR");        break;
    case SELECTION_BEGIN:        printf ("BEGIN");        break;
    case SELECTION_INIT:        printf ("INIT");        break;
    case SELECTION_CONT:        printf ("CONT");        break;
    case SELECTION_DONE:        printf ("DONE");        break;
    default:                        printf ("Unknown");        break;
   }
   printf ("\n\trow/col\n"
           "beg\t%d %d\nend\t%d %d\nanchor\t%d %d\ncursor\t%d %d\n"
           "selection [%d chars]\n",
           selection.beg.row, selection.beg.col,
           selection.end.row, selection.end.col,
           selection.mark.row, selection.mark.col,
           screen.row, screen.col,
           selection.len);

   if (selection.text != NULL)
     debug_PasteIt (selection.text, selection.len);

   return 0;
}
#endif
/*}}}*/

/*{{{ Mouse Reporting */
/* add the bits:
 * @ 1 - shift
 * @ 2 - meta
 * @ 4 - ctrl
 */
#define ButtonNumber(x) ((x) == AnyButton ? 3 : ((x) - Button1))
#define KeyState(x) ((((x)&(ShiftMask|ControlMask))+(((x)&Mod1Mask)?2:0))<<2)
void
mouse_report (XButtonEvent * ev)
{
   tt_printf ("\033[M%c%c%c",
              (040 + ButtonNumber (ev->button) + KeyState (ev->state)),
              (041 + Pixel2Col (ev->x)),
              (041 + Pixel2Row (ev->y)));
}

#if 0        /* X11 mouse tracking: not yet - maybe never! */
void
mouse_tracking (int report, int x, int y, int firstrow, int lastrow)
{
   static int top, bot;

   if (report)
     {
        /* If either coordinate is past the end of the line:
         * "ESC [ T CxCyCxCyCxCy"
         * The parameters are begx, begy, endx, endy,
         * mousex, and mousey */

        if ((selection.beg.row < selection.end.row) ||
            ((selection.beg.row == selection.end.row) &&
             (selection.beg.col < selection.end.col)))
          {
             if (selection.beg.row >= top && selection.end.row <= bot)
               tt_printf ("\033[t");        /* start/end are valid locations */
             else
               tt_printf ("\033[T%c%c%c%c",
                          selection.beg.col + 1, selection.beg.row + 1,
                          selection.end.col + 1, selection.end.row + 1);
          }
        else
          {
             if (selection.end.row >= top && selection.beg.row <= bot)
               tt_printf ("\033[t");        /* start/end are valid locations */
             else
               tt_printf ("\033[T%c%c%c%c",
                          selection.end.col + 1, selection.end.row + 1,
                          selection.beg.col + 1, selection.beg.row + 1)
          }
        tt_printf ("%c%c", Pixel2Col (x) + 1, Pixel2Row (y) + 1);
     }
   else
     {
        selection_start_colrow (x - 1, y - 1);
        top = firstrow;
        bot = lastrow;
     }
}
#endif
/*}}}*/
/*----------------------- end-of-file (C source) -----------------------*/
