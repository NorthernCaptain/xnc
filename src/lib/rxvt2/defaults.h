/*--------------------------------*-C-*---------------------------------*
 * File:	defaults.h
 *
 * Some wired-in defaults so we can run without external resources.
 * Split from feature.h so fiddling won't require a full rebuild.
 *
 * These values are only used in main.c
 * ----------------------------------------------------------------------
 * Copyright 1995, 1996
 * mj olesen <olesen@me.queensu.ca> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
#ifndef _DEFAULTS_H
#define _DEFAULTS_H

#define BORDERWIDTH	1	/* borderWidth [pixels] */
#define SAVELINES	512	/* saveLines [lines] */

/* character class of separating chars for multiple-click selection */
#define CUTCHARS	"\t \"&'()*,;<=>?@[\\]^`{|}~"

/* fonts used */
#ifdef KANJI
# define KFONT0	"k14"
# define KFONT1	"jiskan16"
# define KFONT2	"jiskan18"
# define KFONT3	"jiskan24"
# define KFONT4	"jiskan26"
/* sizes matched to kanji fonts */
# define FONT0	"7x14"
# define FONT1	"8x16"
# define FONT2	"9x18"
# define FONT3	"12x24"
# define FONT4	"13x26"
#else	/* KANJI */
# define FONT0	"7x14"
# define FONT1	"6x10"
# define FONT2	"6x13"
# define FONT3	"8x13"
# define FONT4	"9x15"
#endif	/* KANJI */

/* the logical position of font0 in the list */
#define FONT0_IDX 4

#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

