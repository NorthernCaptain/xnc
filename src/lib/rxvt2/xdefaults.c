/*--------------------------------*-C-*---------------------------------*
 * File:        xdefaults.c
 *
 * get resources from ~/.Xdefaults or ~/.Xresources with the memory-saving
 * default or with XGetDefault() (#define USE_XGETDEFAULT)
 *
 * This module is all new by Rob Nation
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 * ---------------------------------------------------------------------*
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions.
 *
 * Coding style:
 *        resource strings are indicated by an `rs_' prefix followed by
 *        the resource name.
 *        eg, `rs_saveLines' is the resource string corresponding to
 *            the `saveLines' resource
 *----------------------------------------------------------------------*/
/*{{{ includes */
#include "main.h"

#include "debug.h"
#include "defaults.h"
#include "grkelot.h"
#include "xdefaults.h"
#include "xncversion.h"
/*}}}*/

/* #define DEBUG_RESOURCES */
/* extern functions referenced */
/* extern variables referenced */
/*{{{ extern variables declared here */
const char * rs_system_use = NULL;
const char * rs_term_off = NULL;
const char * rs_look_five = NULL;
const char * rs_back_load = NULL;
const char * rs_shared_load = NULL;
const char * rs_no_intro = NULL;
const char * rs_fast_load = NULL;
const char * rs_plugin_name = NULL;
const char * rs_title = NULL;                /* title name for window */
const char * rs_iconName = NULL;        /* icon name for window */
const char * rs_geometry = NULL;        /* window geometry */
const char * rs_saveLines = NULL;        /* scrollback buffer [lines] */
#ifdef KEYSYM_RESOURCE
const unsigned char * KeySym_map [256];        /* probably mostly empty */
#endif
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
/* recognized when combined with HOTKEY */
KeySym ks_bigfont = XK_greater;
KeySym ks_smallfont = XK_less;
#endif
#ifdef XPM_BACKGROUND
const char * rs_backgroundPixmap = NULL;
const char * rs_path = NULL;
#endif
/*}}}*/

/* local functions referenced */
/*{{{ local variables */
static const char * rs_loginShell = NULL;
static const char * rs_utmpInhibit = NULL;
static const char * rs_scrollBar = NULL;
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
static const char * rs_bigfont_key = NULL;
static const char * rs_smallfont_key = NULL;
#endif

#ifndef NO_MAPALERT
# ifdef MAPALERT_OPTION
static const char * rs_mapAlert = NULL;
# endif
#endif
static const char * rs_visualBell = NULL;
static const char * rs_reverseVideo = NULL;
#ifdef META8_OPTION
static const char * rs_meta8 = NULL;
#endif
#ifdef KANJI
static const char * rs_kanji_encoding = NULL;
#endif
#ifdef GREEK_SUPPORT
static const char * rs_greek_keyboard = NULL;
#endif
/*}}}*/

/*{{{ monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 *
 * if there's no desc(ription), it won't appear in usage()
 */
static const struct {
   unsigned long flag;
   const char * * dp;                /* data pointer */
   const char * const kw;        /* keyword */
   const char * const opt;        /* option */
   const char * const arg;        /* argument */
   const char * const desc;        /* description */
} optList[] = {
   /* INFO() - descriptive information only */
#define INFO(opt,arg,desc)       {0, NULL, NULL, opt, arg, desc}
   /* STRG() - command-line option, with/without resource */
#define STRG(p,kw,opt,arg,desc)  {0, &p, kw, opt, arg, desc}
   /* RSTRG() - resource/long-option */
#define RSTRG(p,kw,arg)          {0, &p, kw, NULL, arg, NULL}
   /* BOOL() - regular boolean `-/+' flag */
#define BOOL(p,kw,opt,flag,desc) {(Opt_Boolean|flag), &p, kw, opt, NULL, desc}
   /* SWCH() - `-' flag */
#define SWCH(opt,flag,desc)      {(flag), NULL, NULL, opt, NULL, desc}
   /* convenient macros */
#define optList_strlen(i) (optList[i].flag ? 0 : (optList[i].arg ? strlen (optList[i].arg) : 1))
#define optList_isBool(i) (optList[i].flag & Opt_Boolean)
#define optList_size()    (sizeof(optList)/sizeof(optList[0]))
   STRG(
        display_name,
        NULL,
        "display", "displayname", "X server to contact" ),
   STRG(
        rs_geometry,
        "geometry",
        "geometry", "geom", "size (in characters) and position" ),
   STRG( display_name, NULL, "d", NULL, NULL ),        /* short form */
   STRG( rs_geometry, NULL, "g", NULL, NULL ),         /* short form */
   BOOL(
        rs_reverseVideo,
        "reverseVideo",
        "rv", Opt_reverseVideo, "reverse video" ),
/*   STRG(
        rs_color [bgColor],
        "background",
        "bg", "color", "background color" ),
*/
   STRG(
        rs_color [fgColor],
        "foreground",
        "fg", "color", "terminal foreground color" ),
   /* colors: command-line long-option = resource name */
   RSTRG( rs_color [minColor+0],  "color0", "color" ),
   RSTRG( rs_color [minColor+1],  "color1", "color" ),
   RSTRG( rs_color [minColor+2],  "color2", "color" ),
   RSTRG( rs_color [minColor+3],  "color3", "color" ),
   RSTRG( rs_color [minColor+4],  "color4", "color" ),
   RSTRG( rs_color [minColor+5],  "color5", "color" ),
   RSTRG( rs_color [minColor+6],  "color6", "color" ),
   RSTRG( rs_color [minColor+7],  "color7", "color" ),
#ifndef NO_BRIGHTCOLOR
   RSTRG( rs_color [minBright+0], "color8",  "color" ),
   RSTRG( rs_color [minBright+1], "color9",  "color" ),
   RSTRG( rs_color [minBright+2], "color10", "color" ),
   RSTRG( rs_color [minBright+3], "color11", "color" ),
   RSTRG( rs_color [minBright+4], "color12", "color" ),
   RSTRG( rs_color [minBright+5], "color13", "color" ),
   RSTRG( rs_color [minBright+6], "color14", "color" ),
   RSTRG( rs_color [minBright+7], "color15", "color" ),
#endif        /* NO_BRIGHTCOLOR */
#ifndef NO_BOLDUNDERLINE
   RSTRG( rs_color [colorBD], "colorBD", "color" ),
   RSTRG( rs_color [colorUL], "colorUL", "color" ),
#endif        /* NO_BOLDUNDERLINE */
#ifndef XTERM_SCROLLBAR
   RSTRG( rs_color [scrollColor],       "scrollColor",    "color"),
#endif        /* XTERM_SCROLLBAR */
#ifdef XPM_BACKGROUND
   STRG( rs_backgroundPixmap,
        "backgroundPixmap",
        "pixmap", "file[;geom]", "background pixmap [scaling optional]" ),
   RSTRG( rs_path, "path", "search path" ),
#endif        /* XPM_BACKGROUND */
#ifndef NO_BOLDFONT
   STRG(
        rs_boldFont,
        "boldFont",
        "fb", "fontname", "bold text font" ),
#endif
   STRG(
        rs_font [4],
        "font",
        "fn", "fontname", "normal terminal text font" ),
   /* fonts: command-line option = resource name */
   RSTRG( rs_font [1], "font1", "fontname" ),
   RSTRG( rs_font [2], "font2", "fontname" ),
   RSTRG( rs_font [3], "font3", "fontname" ),
   RSTRG( rs_font [4], "font4", "fontname" ),
#ifdef KANJI
   STRG(
        rs_kfont [0],
        "kfont",
        "fk", "fontname", "kanji font" ),

   /* fonts: command-line option = resource name */
   RSTRG( rs_kfont [1], "kfont1", "fontname" ),
   RSTRG( rs_kfont [2], "kfont2", "fontname" ),
   RSTRG( rs_kfont [3], "kfont3", "fontname" ),
   RSTRG( rs_kfont [4], "kfont4", "fontname" ),

   STRG(
        rs_kanji_encoding,
        "kanji_encoding",
        "km", "mode", "kanji encoding; mode = eucj | sjis" ),
#endif        /* KANJI */
#ifdef GREEK_SUPPORT
   STRG(
        rs_greek_keyboard,
        "greek_keyboard",
        "grk", "mode", "greek keyboard mapping; mode = iso | ibm" ),
#endif
   SWCH( "iconic", Opt_iconic, "start iconic" ),
   SWCH( "ic", Opt_iconic, NULL ),        /* short form */
   STRG(
        rs_name,
        NULL,
        "name", "string", "client instance, icon, and title strings" ),
   STRG(
        rs_title,
        "title",
        "title", "string", "title name for window" ),
   STRG( rs_title, NULL, "T", NULL, NULL ),  /* short form */
   STRG(
        rs_iconName,
        "iconName",
        "n", "string", "icon name for window" ),
#ifndef NO_CURSORCOLOR
   STRG(
        rs_color [cursorColor],
        "cursorColor",
        "cr", "color", "cursor color" ),
   /* command-line option = resource name */
   RSTRG( rs_color [cursorColor2], "cursorColor2", "color" ),
#endif        /* NO_CURSORCOLOR */
   BOOL(
        rs_loginShell,
        "loginShell",
        "ls", Opt_loginShell, "login shell" ),
   BOOL(
        rs_scrollBar,
        "scrollBar",
        "sb", Opt_scrollBar, "scrollbar" ),
   STRG(
        rs_saveLines,
        "saveLines",
        "sl", "number", "number of scrolled lines to save" ),
   BOOL(
        rs_utmpInhibit,
        "utmpInhibit",
        "ut", Opt_utmpInhibit, "utmp inhibit" ),
   BOOL(
        rs_visualBell,
        "visualBell",
        "vb", Opt_visualBell, "visual bell" ),
#ifndef NO_MAPALERT
# ifdef MAPALERT_OPTION
   BOOL(
        rs_mapAlert,
        "mapAlert",
        NULL, Opt_mapAlert, NULL ),
# endif
#endif
#ifdef META8_OPTION
   BOOL(
        rs_meta8,
        "meta8",
        NULL, Opt_meta8, NULL ),
#endif

#ifdef PRINTPIPE
   RSTRG( rs_print_pipe, "print-pipe", "string" ),
#endif

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
   RSTRG( rs_bigfont_key,   "bigfont_key", "keysym" ),
   RSTRG( rs_smallfont_key, "smallfont_key", "keysym" ),
#endif
#ifdef CUTCHAR_RESOURCE
   RSTRG( rs_cutchars, "cutchars", "string" ),
#endif        /* CUTCHAR_RESOURCE */

   SWCH( "C", Opt_console, "intercept console messages" ),
   BOOL(
        rs_no_intro,
        "showIntro",
        "si", Opt_showIntro, "intro banner" ),
   BOOL(
        rs_fast_load,
        "fastLoad",
        "fl", Opt_fastLoad, "fast loading" ),
   STRG(
        rs_plugin_name,
        "plugin",
        "plugin", "plugin-name", "plugin name for load different look" ),
   BOOL(
	rs_back_load,
	"backLoad",
	"bl", Opt_backLoad, "background resource loading"),
   BOOL(
	rs_shared_load,
	"sharedLoad",
	"shared", Opt_sharedLoad, "shared resource using with xncloader"),
   INFO( "e", "command arg ...", "command to execute" )
};
#undef INFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL
/*}}}*/

/*{{{ usage: */
/*----------------------------------------------------------------------*/
static void
usage (int type)
{                     
   int i, col;
#define INDENT 30

   fprintf (stderr, "\nUsage of XNC "XNCVERSION":\n\n   xnc ");
   switch (type) {
    case 0:                        /* brief listing */
      fprintf (stderr, " [-help]\n   ");
      col = 3;
      for (i = 0; i < optList_size (); i++)
        {
           if (optList[i].desc != NULL)
             {
                int len = 2;
                if (!optList_isBool (i))
                  {
                     len = optList_strlen (i);
                     if (len > 0)
                       len++;                /* account for space */
                  }
                len += 4 + strlen (optList[i].opt);

                col += len;
                if (col > 79)                /* assume regular width */
                  {
                     fprintf (stderr, "\n   ");
                     col = 3 + len;
                  }
                fprintf (stderr, " [-");
                if (optList_isBool (i))
                  fprintf (stderr, "/+");
                fprintf (stderr, "%s", optList[i].opt);
                if (optList_strlen (i))
                  fprintf (stderr, " %s]", optList[i].arg);
                else
                  fprintf (stderr, "]");
             }
        }
      fprintf (stderr, "\n\n");
      break;

    case 1:                        /* full command-line listing */
      fprintf (stderr,
               " [options]\n\n"
               "where options include:\n");

      for (i = 0; i < optList_size () -1; i++)
        if (optList[i].desc != NULL)
          fprintf (stderr, "    %s%s %-*s%s%s\n",
                   (optList_isBool (i) ? "-/+" : "-"),
                   optList[i].opt,
                   (INDENT - strlen (optList[i].opt)
                    + (optList_isBool (i) ? 0 : 2)),
                   (optList[i].arg ? optList[i].arg : ""),
                   (optList_isBool (i) ? "turn on/off " : ""),
                   optList[i].desc);
      fprintf (stderr, "\n    --help to list long-options\n\n");
      break;

    case 2:                        /* full resource listing */
      fprintf (stderr,
               " [options]\n\n"
               "where resources (long-options) include:\n");

      for (i = 0; i < optList_size () -1; i++)
        if (optList[i].kw != NULL)
          fprintf (stderr, "    %s: %*s\n",
                   optList[i].kw,
                   (INDENT - strlen (optList[i].kw)),
                   (optList_isBool (i) ? "boolean" : optList[i].arg));

#ifdef KEYSYM_RESOURCE
      fprintf (stderr, "    " "keysym.sym" ": %*s\n",
               (INDENT - strlen ("keysym.sym")), "keysym");
#endif
      fprintf (stderr, "\n    -help to list options\n\n");
      break;
   }
   exit (EXIT_FAILURE);
}
/*}}}*/

/*{{{ get command-line options before getting resources */
void
get_options (int argc, char * argv[])
{
   int i, bad_option = 0;
   static const char * const On = "ON";
   static const char * const Off = "OFF";

   for (i = 1; i < argc; i++)
     {
        int entry, longopt = 0;
        const char * flag;
        char * opt = argv [i];

#ifdef DEBUG_RESOURCES
        fprintf (stderr, "argv[%d] = %s: ", i, argv[i]);
#endif
        if (*opt == '-')
          {
             flag = On;
             if (*++opt == '-') longopt = *opt++;   /* long option */
          }
        else if (*opt == '+')
          {
             flag = Off;
             if (*++opt == '+') longopt = *opt++;   /* long option */
          }
        else
          {
             bad_option = 1;
             print_error ("bad option \"%s\"", opt);
             continue;
          }

        if (!strcmp (opt, "help")) usage (longopt ? 2 : 1);

        /* feature: always try to match long-options */
        for (entry = 0; entry < optList_size (); entry++)
          if ((optList[entry].kw && !strcmp (opt, optList[entry].kw)) ||
              (!longopt &&
               optList[entry].opt && !strcmp (opt, optList[entry].opt)))
            break;

        if (entry < optList_size ())
          {
             if (optList_strlen (entry))        /* string value */
               {
                  char * str = argv [++i];
#ifdef DEBUG_RESOURCES
                  fprintf (stderr, "string (%s,%s) = ",
                           optList[entry].opt ? optList[entry].opt : "nil",
                           optList[entry].kw ? optList[entry].kw : "nil");
#endif
                  if (flag == On && str && optList [entry].dp) {
#ifdef DEBUG_RESOURCES
                     fprintf (stderr, "\"%s\"\n", str);
#endif
                     *(optList[entry].dp) = str;

                     /* special cases */
                     if (!strcmp (opt, "n")) {
                        if (!rs_title)    rs_title = str;
                     } else if (!strcmp (opt, "name")) {
                        if (!rs_title)    rs_title = str;
                        if (!rs_iconName) rs_iconName = str;
                     }
                  }
#ifdef DEBUG_RESOURCES
                  else fprintf (stderr, "???\n");
#endif
               }
             else                        /* boolean value */
               {
#ifdef DEBUG_RESOURCES
                  fprintf (stderr, "boolean (%s,%s) = %s\n",
                           optList[entry].opt, optList[entry].kw, flag);
#endif
                  if (flag == On)
                    Options |= (optList[entry].flag);
                  else
                    Options &= ~(optList[entry].flag);

                  if (optList [entry].dp)
                    *(optList [entry].dp) = flag;
               }
          }
        else
          {
             /* various old-style options, just ignore
              * Obsolete since about Jan 96,
              * so they can probably eventually be removed
              */
             const char * msg = "bad";
             if (longopt)
               {
                  opt--;
                  bad_option = 1;
               }
             else if (!strcmp (opt, "7") || !strcmp (opt, "8") ||
                      !strcmp (opt, "fat") || !strcmp (opt, "thin")
#ifdef GREEK_SUPPORT
                      /* these obsolete 12 May 1996 (v2.17) */
                      || !strcmp (opt, "grk4") || !strcmp (opt, "grk9")
#endif
                      )
               msg = "obsolete";
             else
               bad_option = 1;

             print_error ("%s option \"%s\"", msg, --opt);
          }
     }

   if (bad_option)
     usage (0);
   if(Options & Opt_fastLoad)
           Options&= ~Opt_showIntro;
}
/*}}}*/

#ifndef NO_RESOURCES
/*----------------------------------------------------------------------*/
/*{{{ string functions */
/*
 * a replacement for strcasecmp() to avoid linking an entire library
 */
static int
my_strcasecmp (const char * s1, const char * s2)
{
   for (/*nil*/; (*s1 && *s2); s1++, s2++)
     {
        register int c1 = toupper (*s1);
        register int c2 = toupper (*s2);
        if (c1 != c2)
          return (c1 - c2);
     }
   return (int) (*s1 - *s2);
}
/*}}}*/

#ifndef USE_XGETDEFAULT
/*{{{ more string functions */
/*
 * remove leading/trailing space and strip-off leading/trailing quotes
 */
static char *
trim_string (char * str)
{
   if (str && *str)
     {
        int n;
        while (*str && isspace (*str)) str++;

        n = strlen (str) - 1;
        while (n > 0 && isspace (str [n])) n--;
        str [n+1] = '\0';

        /* strip leading/trailing quotes */
        if (str [0] == '"')
          {
             str++; n--;
             if (str [n] == '"')
               str [n--] = '\0';
          }
        if (n < 0) str = NULL;
     }
   return str;
}
/*}}}*/
/*{{{ get_xdefaults() */
/*
 * the matching algorithm used for memory-save fake resources
 */
static void
get_xdefaults (FILE * stream, const char * name)
{
   unsigned int len;
   char * str, buffer [256];

   if (stream == NULL) return;
   len = strlen (name);
   while ((str = fgets (buffer, sizeof(buffer), stream)) != NULL)
     {
        unsigned int entry, n;

        while (*str && isspace (*str)) str++;    /* leading whitespace */

        if ((str [len] != '*' && str [len] != '.') ||
            (len && strncmp (str, name, len)))
          continue;
        str += (len + 1);        /* skip `name*' or `name.' */

        /*
         * look for something like this (XK_Delete)
         * rxvt*keysym.0xFFFF: "\177"
         */
# ifdef KEYSYM_RESOURCE
        n = strlen ("keysym");
        if (str [n] == '.' && !strncmp (str, "keysym", n))
          {
             int sym;
             str += (n + 1);        /* skip `keysym.' */

             /* some scanf() have trouble with a 0x prefix */
             if (str [0] == '0' && toupper (str [1]) == 'X') str += 2;
             if (sscanf (str, "%x:", &sym) == 1)
               {
                  /* cue to ':', it's there since sscanf() worked */
                  str = strchr (str, ':');
                  str = trim_string (str + 1);
                  n = escaped_string (str);

                  /* only do extended keys */
                  if (sym >= 0xFF00) sym -= 0xFF00;
                  if (sym < 0 || sym > 0xFF) continue;

                  if (n >= 256) n = 255;
                   if (n && KeySym_map [sym] == NULL)        /* not already set */
                    {
                       char * p = MALLOC ((n+1) * sizeof(char), "keysym");
                       p [0] = n;
                       memcpy (p+1, str, n);
                       KeySym_map [sym] = p;
                    }
               }
          }
        else
# undef KEYSYM_kw
# endif        /* KEYSYM_RESOURCE */
          for (entry = 0; entry < optList_size (); entry++)
          {
             const char * const kw = optList[entry].kw;
             if (kw == NULL) continue;
             n = strlen (kw);
             if (str [n] == ':' && !strncmp (str, kw, n))
               {
                  /* skip `keyword:' */
                  str = trim_string (str + n + 1);
                  n = (str ? strlen (str) : 0);
                  if (n && *(optList[entry].dp) == NULL)
                    {
                       /* not already set */
                       char * p = MALLOC ((n+1) * sizeof(char), kw);
                       strcpy (p, str);
                       *(optList[entry].dp) = p;
                       if (optList_isBool (entry))
                         {
                            if (!my_strcasecmp (str, "TRUE"))
                              Options |= (optList[entry].flag);
                            else
                              Options &= ~(optList[entry].flag);
                         }
                    }
                  break;
               }
          }
     }
   rewind (stream);
}
/*}}}*/
#endif        /* ! USE_XGETDEFAULT */
#endif        /* NO_RESOURCES */

/*{{{ read the resources files */
/*
 * using XGetDefault() or the hand-rolled replacement
 */
void
extract_resources (Display * display, const char * name)
{
#ifndef NO_RESOURCES
#ifdef USE_XGETDEFAULT
   /*
    * get resources using the X library function
    */
   int entry;
   for (entry = 0; entry < optList_size (); entry++)
     {
        char * p;
        const char * kw =  optList[entry].kw;
        if (kw == NULL || *(optList[entry].dp) != NULL)
          continue;        /* previously set */
        if ((p = XGetDefault (display, name, kw)) != NULL ||
            (p = XGetDefault (display, APL_SUBCLASS, kw)) != NULL ||
            (p = XGetDefault (display, APL_CLASS, kw)) != NULL)
          {
             *optList[entry].dp = p;

             if (optList_isBool (entry))
               {
                  if (!my_strcasecmp (p, "TRUE"))
                    Options |= (optList[entry].flag);
                  else
                    Options &= ~(optList[entry].flag);
               }
          }
     }
#else        /* USE_XGETDEFAULT */
   /* get resources the hard way, but save lots of memory */
   const char * fname[] = { ".Xdefaults", ".Xresources" };
   FILE * fd = NULL;
   char * home;

   if ((home = getenv ("HOME")) != NULL)
     {
        int i, len = strlen (home) + 2;
        char * f = NULL;
        for (i = 0; i < (sizeof(fname)/sizeof(fname[0])); i++)
          {
             f = REALLOC (f, (len + strlen (fname[i])) * sizeof(char),
                          fname [i]);
             sprintf (f, "%s/%s", home, fname [i]);

             if ((fd = fopen (f, "r")) != NULL)
               break;
          }
        FREE (f, "get_Xdefaults", "fname");
     }

   /*
    * The normal order to match resources is the following:
    * @ global resources (partial match, ~/.Xdefaults)
    * @ application file resources (XAPPLOADDIR/RXvt)
    * @ class resources (~/.Xdefaults)
    * @ private resources (~/.Xdefaults)
    *
    * However, for the hand-rolled resources, the matching algorithm
    * checks if a resource string value has already been allocated
    * and won't overwrite it with (in this case) a less specific
    * resource value.
    *
    * This avoids multiple allocation.  Also, when we've called this
    * routine command-line string options have already been applied so we
    * needn't to allocate for those resources.
    *
    * So, search in resources from most to least specific.
    *
    * Also, use a special sub-class so that we can use either or both of
    * "XTerm" and "RXvt" as class names.
    */

   get_xdefaults (fd, name);
   get_xdefaults (fd, APL_SUBCLASS);

#ifdef XAPPLOADDIR
     {
        FILE * ad = fopen (XAPPLOADDIR "/" APL_SUBCLASS, "r");
        if (ad != NULL)
          {
             get_xdefaults (ad, "");
             fclose (ad);
          }
     }
#endif        /* XAPPLOADDIR */

   get_xdefaults (fd, APL_CLASS);
   get_xdefaults (fd, "");                /* partial match */
   if (fd != NULL)
     fclose (fd);
#endif        /* USE_XGETDEFAULT */
#endif        /* NO_RESOURCES */

   /*
    * even without resources, at least do this setup for command-line
    * options and command-line long options
    */
#ifdef KANJI
   set_kanji_encoding (rs_kanji_encoding);
#endif
#ifdef GREEK_SUPPORT
   /* this could be a function in grkelot.c */
   /* void set_greek_keyboard (const char * str); */
   if (rs_greek_keyboard)
     {
        if (!strcmp (rs_greek_keyboard, "iso"))
          greek_setmode (GREEK_ELOT928);        /* former -grk9 */
        else if (!strcmp (rs_greek_keyboard, "ibm"))
          greek_setmode (GREEK_IBM437);        /* former -grk4 */
     }
#endif        /* GREEK_SUPPORT */

#define to_keysym(pks,str) do { KeySym sym;\
if (str && ((sym = XStringToKeysym(str)) != 0)) *pks = sym; } while (0)

#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
   to_keysym (&ks_bigfont,        rs_bigfont_key);
   to_keysym (&ks_smallfont,        rs_smallfont_key);
#endif
#undef to_keysym
}
/*}}}*/
/*----------------------- end-of-file (C source) -----------------------*/
