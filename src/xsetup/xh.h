#ifndef XH_H
#define XH_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <globdef.h>
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "types.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
#include <X11/extensions/shape.h>
#endif
#include <X11/keysym.h>

#define BDWIDTH   1
#define XNC_DIR "/usr/local/lib/xnc/data"

#include "colors.h"

typedef struct X_PAR
{
 char *name;
 char *cname;
 char *pstr;
} X_PAR;

extern char 
     bgchr[],
     fgchr[],
     fontchr[],
     kbgchr[],
     *dispchr,
     selectchr[],
     editname[],
     viewname[];

extern int viewx,viewy;
extern uint viewh,viewl;
extern char rfile[80];
extern char* mnt[];
extern int mntmax;
extern char **menunames,**menucom;
extern int menumax;
extern int extmax;
extern int win_resize;
extern int centerx,centery,vcenterx,vcentery;

//global vars for comand line options.
extern int margc;
extern char** margv;
  
extern XFontStruct *fontstr;
extern XFontStruct *lfontstr;
extern XFontStruct *fixfontstr;
extern XFontStruct *vfontstr;
extern unsigned long bgpix,fgpix,selectpix;

extern XWMHints         *pxwmh;
extern XSizeHints       *pxsh;
extern XClassHint       *pch;
extern XTextProperty    wname;
extern XTextProperty    iname;
extern Display          *disp;
extern Window           Main;
extern GC               gc;
extern XGCValues        gcv;
extern XrmDatabase      rdb;
extern XrmValue         rmv;
extern XEvent           ev;
extern XColor           color;
extern Colormap         defcmp;
extern int              scr;
extern int              done;
extern int              Mainl,Mainh;

#include "gui.h"

extern char **extcom;
extern char* finduser(int);
extern char* findgroup(int);
extern char* findmntent(char*);
extern int   ext_find(char*);
extern int   ext_find2(char*,int);
extern void load_ext();
extern void load_menu();
extern int findmenukey(KeySym);
extern int getps();
extern int getdf(Panel*);
extern void delps(int);
extern void man_page();
 
/////////////////////////////////////////////////////////////////////////

extern unsigned long gencol(XColor& col,int per);
extern void InitXClient(int argc,char** argv,int wx,int wy,int wl,int wh,char* winname=NULL);
extern Window create_win(char*,int,int,int,int,int);
extern void DeinitXClient();
extern void msg(char*,char*,int,FList *o=NULL);
extern void find_panel(int=540);
extern char *finduser(int);
extern char *findgroup(int);

extern void XEVENT();

//////////////////////////////////End of file/////////////////////////////
#endif
        
/* ------------ End of file -------------- */

