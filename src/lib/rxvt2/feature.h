/*--------------------------------*-C-*---------------------------------*
 * File:        feature.h
 *
 * Compile-time configuration.
 * Additional compile-time configuration in "defaults.h"
 * ----------------------------------------------------------------------
 * Copyright 1995,1996
 * mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
#ifndef _FEATURE_H
#define _FEATURE_H

/*{{{ system hacks: */
/*----------------------------------------------------------------------*/
/* Consistent defines - please report on the necessity
 * @ Unixware: defines (__svr4__)
 */
#if defined (SVR4) && !defined (__svr4__)
# define __svr4__
#endif
#if defined (sun) && !defined (__sun__)
# define __sun__
#endif

/*
 * sun <sys/ioctl.h> isn't properly protected?
 * anyway, it causes problems when <termios.h> is also included
 */
#if defined (__sun__)
# undef HAVE_SYS_IOCTL_H
#endif
/*}}}*/

/*{{{ debugging: */
/*----------------------------------------------------------------------*
 * #define NDEBUG
 *        to disable whichever assert() macros are used in the code
 *
 * #define DEBUG_SCREEN
 *        to do self-check for internal screen corruption
 *
 * #define DEBUG_MALLOC
 *        to do self-check on out-of-bound memory usage
 *
 * #define DEBUG_CMD
 *        to output some data structures of command.c
 *        (key-buffer contents, command line, tty permissions)
 *
 * #define DEBUG_TTY
 *        to output tty settings
 *
 * #define DEBUG_COLORS
 *        to print out current color/renditions as they change
 *
 * #define DEBUG_SELECTION
 *        to use XK_Print to dump information about the current selection
 *
 * #define DEBUG_DEPTH        1
 *  to set the X color depth, for debugging lower depth modes
 *----------------------------------------------------------------------*/
/* #define NDEBUG */
#ifndef NDEBUG
/* # define DEBUG_MENU */
/* # define DEBUG_MENUARROWS */
/* # define DEBUG_SCREEN */
/* # define DEBUG_MALLOC */
/* # define DEBUG_CMD */
/* # define DEBUG_TTY */
/* # define DEBUG_COLORS */
/* # define DEBUG_SELECTION */
/* # define DEBUG_DEPTH 1 */
#endif
/*}}}*/

/*{{{ screen/colors: */
/*----------------------------------------------------------------------*
 * #define XPM_BACKGROUND
 *        to add sexy-looking background pixmaps.  Needs libxpm
 *
 * #define XPM_SCALING
 *        to allow pixmaps to be dynamically scaled
 *
 * #define PATH_ENV "RXVTPATH"
 *        to define the name of the environment variable to be used in
 *        addition to the "PATH" environment and the `path' resource
 *
 * #define XPM_BUFFERING
 *        to use xpm buffers for the screen update
 *        (bigger & faster? ... but does it work correctly?)
 *
 * #define NO_CURSORCOLOR
 *        to avoid enabling a color cursor (-cr, cursorColor, cursorColor2)
 *
 * #define NO_BRIGHTCOLOR
 *        to suppress use of BOLD and BLINK attributes for setting
 *        bright foreground and background, respectively.
 *        Simulate BOLD using colorBD, boldFont or overstrike characters.
 *
 * #define NO_BOLDUNDERLINE
 *        to disable separate colors for bold/underline
 *
 * #define NO_BOLDOVERSTRIKE
 *        to disable using simulated bold using overstrike
 *
 * #define NO_BOLDFONT
 *        to compile without support for real bold fonts
 *
 * #define NO_SECONDARY_SCREEN
 *        to disable the secondary screen ("\E[?47h" / "\E[?47l")
 *
 * #define REFRESH_PERIOD <num>
 *        to limit the number of screenfulls between screen refreshes
 *        during hard & fast scrolling [default: 1]
 *
 * #define USE_XCOPYAREA
 *        to use XCopyArea (in place of re-draws) to speed up rxvt.
 *        - I've been told this helps with some graphics adapters like the
 *          PC's et4000. OK, it's good on monochrome Sun-3's that I've tried
 *          too. /RN
 *        - sometimes looks worse and slower /mjo
 *
 * #define RXVT_GRAPHICS
 *        to use Rob Nation's own graphics mode
 *
 * #define PRINTPIPE  "lpr"
 *        to define a printer pipe which will be used for emulation of an
 *        attached vt100 printer
 *
 * #define OLD_COLOR_MODEL
 *        to use the old color model whereby erasing is done with the
 *        default rendition rather than the current rendition
 *        NB: this make break some applications and should used with caution
 *----------------------------------------------------------------------*/
/*#define XPM_BACKGROUND*/
/*#define XPM_SCALING*/
#define PATH_ENV        "RXVTPATH"
/* #define XPM_BUFFERING */
/* #define NO_CURSORCOLOR */
/* #define NO_BRIGHTCOLOR */
/* #define NO_BOLDUNDERLINE */
#define NO_BOLDOVERSTRIKE
#define NO_BOLDFONT
/* #define NO_SECONDARY_SCREEN */
/* #define REFRESH_PERIOD        1 */
/* #define USE_XCOPYAREA */
/* #define RXVT_GRAPHICS */
/* #define PRINTPIPE        "lpr" */
/* #define OLD_COLOR_MODEL */
/*}}}*/

/*{{{ resources: */
/*----------------------------------------------------------------------*
 * #define NO_RESOURCES
 *        to blow off the Xdefaults altogether
 *
 * #define USE_XGETDEFAULT
 *        to use XGetDefault() instead of the default, which is to use a
 *        substitute for using XGetDefaults() that saves 60-150kB memory
 *
 *        The default is best if all you want to do is put rxvt defaults
 *        in ~/.Xdefaults file,
 *
 * #define XAPPLOADDIR        "/usr/lib/X11/app-defaults"
 *        to define where to find installed application defaults for rxvt
 *        Only if USE_XGETDEFAULT is not defined.
 *
 * #define OFFIX_DND
 *        to add support for the Offix DND (Drag 'n' Drop) protocol
 *----------------------------------------------------------------------*/
/* #define NO_RESOURCES */
/* #define USE_XGETDEFAULT */
#ifndef XAPPLOADDIR
/* #define XAPPLOADDIR        "/usr/lib/X11/app-defaults" */
#endif
/* #define OFFIX_DND */
/*}}}*/

/*{{{ keys: */
/*----------------------------------------------------------------------*
 * #define NO_DELETE_KEY
 *        to use the unadulterated X server value for the Delete key
 *
 * #define DONT_GUESS_BACKSPACE
 *        to use ^H for the Backspace key and avoid using the current stty
 *        setting of erase to guess a Backspace value of either ^H or ^?
 *
 * #define HOTKEY_CTRL
 * #define HOTKEY_META
 *        choose one of these values to be the `hotkey' for changing font.
 *        -- obsolete
 *
 * #define LINUX_KEYS
 *        to use
 *                Home = "\E[1~", End = "\E[4~"
 *        instead of
 *                Home = "\E[7~", End = "\E[8~"        [default]
 *
 * #define KEYSYM_RESOURCE
 *        to enable the keysym resource which allows you to define
 *        strings associated with various KeySyms (0xFF00 - 0xFFFF).
 *        Only works with the default hand-rolled resources.
 *
 * #define NO_XLOCALE
 *        to disable X11R6 support for European languages
 *        - possibly still beta
 *----------------------------------------------------------------------*/
#if defined (__sun__) || defined (__svr4__)
# define NO_DELETE_KEY                /* favoured settings for these systems */
#endif
/* #define NO_DELETE_KEY */
/* #define DONT_GUESS_BACKSPACE */

/* #define HOTKEY_CTRL */
/* #define HOTKEY_META */

/* #define LINUX_KEYS */
#define KEYSYM_RESOURCE
/* #define NO_XLOCALE */
/*}}}*/

/*{{{ mouse/selection: */
/*----------------------------------------------------------------------*
 * #define NO_SCROLLBAR_REPORT
 *        to disable sending escape sequences (up, down, page up/down)
 *        from the scrollbar when XTerm mouse reporting is enabled
 *
 * #define CUTCHAR_RESOURCE
 *        to add run-time support for changing the default cutchars
 *        for double click selection
 *
 * #define MOUSE_REPORT_DOUBLECLICK
 *        to have mouse reporting include double-click info for button1
 *
 * #define MULTICLICK_TIME <num>
 *        to set delay between multiple click events [default: 500]
 *----------------------------------------------------------------------*/
/* #define NO_SCROLLBAR_REPORT */
/* #define CUTCHAR_RESOURCE */
/* #define MOUSE_REPORT_DOUBLECLICK */
/* #define MULTICLICK_TIME 500 */
/*}}}*/

/*{{{ bell: */
/*----------------------------------------------------------------------*
 * #define NO_MAPALERT
 *        to disable automatic de-iconify when a bell is received
 *
 * #define MAPALERT_OPTION
 *        to have mapAlert behaviour selectable with mapAlert resource
 *----------------------------------------------------------------------*/
/* #define NO_MAPALERT */
/* #define MAPALERT_OPTION */
/*}}}*/

/*{{{ scrollbar: */
/*----------------------------------------------------------------------*
 * #define XTERM_SCROLLBAR
 *        to only use the XTerm-style scrollbar - no arrows, bitmapped
 *        instead of the regular scrollbar (with arrows)
 *
 * ---------------------------------------------------------------------*
 * #define SCROLLBAR_RIGHT
 *        to have the scrollbar on the right-hand side
 *
 * #define SB_WIDTH        <width>
 *        to choose the scrollbar width - should be an even number [default: 10]
 *        for XTERM_SCROLLBAR it is *always* 14.
 *----------------------------------------------------------------------*/
/* #define XTERM_SCROLLBAR */
#define SCROLLBAR_RIGHT
/* #define SB_WIDTH 10 */
/*}}}*/

/*{{{ menuBar: */
/*----------------------------------------------------------------------*
 * #define NO_MENUBAR
 *        to supress use of the (experimental) menuBar
 *
 * #define MENUBAR_SHADOW_IN
 *        to change the default shadow style
 *
 * #define MENU_SHADOW_IN
 *        to change the default shadow style
 *----------------------------------------------------------------------*/
#define NO_MENUBAR
/* #define MENUBAR_SHADOW_IN */
#define MENU_SHADOW_IN
/*}}}*/

/*{{{ multi-lingual: */
/*----------------------------------------------------------------------*
 * #define META8_OPTION
 *        to allow run-time selection of Meta (Alt) to set the 8th bit on
 *
 * #define GREEK_SUPPORT
 *        to include support for the Greek Elot-928 & IBM-437 keyboard
 *        see doc/README.greek
 *
 * #define KANJI
 *        to compile with Kanji support
 *        after compilation, rename executable as `kxvt'
 *----------------------------------------------------------------------*/
/* #define META8_OPTION */
/* #define GREEK_SUPPORT */
/* #define KANJI */
/*}}}*/

/*{{{ misc: */
/*----------------------------------------------------------------------*
 * #define DISPLAY_IS_IP
 *        to have DISPLAY environment variable and "\E[7n" transmit
 *        display with an IP number
 *
 * #define ENABLE_DISPLAY_ANSWER
 *        to have "\E[7n" transmit the display name.
 *        This has been cited as a potential security hole.
 *
 * #define ESCZ_ANSWER        "\033[?1;2C"
 *        to change what ESC Z transmits instead of the default "\E[?1;2c"
 *
 * #define SMART_WINDOW_TITLE
 *        to check the current value of the window-time/icon-name and
 *        avoid re-setting it to the same value -- avoids unnecessary window
 *        refreshes
 *
 * #define XTERM_COLOR_CHANGE
 *        to allow foreground/background color to be changed with an
 *        xterm escape sequence "\E]39;color^G" -- still experimental
 *
 * #define DEFINE_XTERM_COLOR
 *        to define TERM="xterm-color" instead of just TERM="xterm", which
 *        is a useful addition to COLORTERM for distinguishing color
 *        characteristics since it will be exported across rlogin/rsh
 *----------------------------------------------------------------------*/
/* #define DISPLAY_IS_IP */
/* #define ENABLE_DISPLAY_ANSWER */
/* #define ESCZ_ANSWER        "\033[?1;2C" */
/* #define SMART_WINDOW_TITLE */
/* #define XTERM_COLOR_CHANGE */
#define DEFINE_XTERM_COLOR
/*}}}*/

/*{{{ utmp: */
/*----------------------------------------------------------------------*
 * #define UTMP_SUPPORT
 *        for utmp support to update `/etc/utmp' to show rxvt logins
 *
 * For this to work,
 *        - make Rxvt setuid root, a potential security hole but is
 *          reportedly okay - do at your own risk
 *                su
 *                chown root.root rxvt
 *                chmod a+s rxvt
 *        - make Rxvt setuid/setgid to match user/group that owns `/etc/utmp'
 *
 * #define UTMP_FILENAME "/var/adm/utmp"        (Irix, dec alpha)
 * #define UTMP_FILENAME "/var/run/utmp"        (FreeBSD, NetBSD 0.9)
 *        to define where the utmp file is located if it isn't /etc/utmp
 *        and isn't defined by one of the myriad names in utmp.c
 *
 * #define TTYTAB_FILENAME "/etc/ttys"                (FreeBSD, NetBSD 0.9)
 *        for BSD-type systems, to define where the tty table is located
 *        if it isn't /etc/ttytab
 *----------------------------------------------------------------------*/
/* #define UTMP_SUPPORT */
#ifdef UTMP_SUPPORT
# ifndef UTMP_FILENAME
/* #  define UTMP_FILENAME        "/var/adm/utmp" */
/* #  define UTMP_FILENAME        "/var/adm/utmp" */
# endif
# ifndef TTYTAB_FILENAME
/* #  define TTYTAB_FILENAME "/etc/ttys" */
# endif
#endif
/*}}}*/

/*{{{ sort out conflicts */
/*----------------------------------------------------------------------*
 * end of user configuration section
 *----------------------------------------------------------------------*/
#ifdef KANJI
# undef GREEK_SUPPORT                /* Kanji/Greek together is too weird */
# undef XTERM_FONT_CHANGE        /* can't ensure font sizes will match */
#endif

#ifdef XPM_BUFFERING
# ifndef XPM_SCALING
#  define XPM_SCALING
# endif
#endif
/* Define if you have the Xpm library (-lXpm).  */
#ifndef HAVE_LIBXPM
# undef XPM_BACKGROUND
#endif
/* disable what can't be used */
#ifndef XPM_BACKGROUND
# undef XPM_BUFFERING
# undef XPM_SCALING
# undef XTERM_PIXMAP_CHANGE
#endif

/* no point having a fancy menubar with a simple XTerm scrollBar */
#ifdef XTERM_SCROLLBAR
# ifndef NO_MENUBAR
#  define NO_MENUBAR
# endif
#endif

#define APL_CLASS        "XTerm"        /* class name */
#define APL_SUBCLASS        "Rxvt"        /* also check resources under this name */
#define APL_NAME        "rxvt"        /* normal name */

/* COLORTERM, TERM environment variables */
#define COLORTERMENV        "rxvt"
#ifdef KANJI
# define TERMENV        "kterm"
#else
# define TERMENV        "xterm"
#endif

#ifdef NO_MOUSE_REPORT
# ifndef NO_MOUSE_REPORT_SCROLLBAR
#  define NO_MOUSE_REPORT_SCROLLBAR
# endif
#endif

#if defined (NO_RESOURCES) || defined (USE_XGETDEFAULT)
# undef KEYSYM_RESOURCE
#endif
#ifdef NO_RESOURCES
# undef USE_XGETDEFAULT
#endif
/*}}}*/

#endif        /* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

