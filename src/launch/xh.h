#ifndef XH_H
#define XH_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <globdef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
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

#include "colors.h"

typedef struct X_PAR
{
 char *name;
 char *cname;
 char **pstr;
} X_PAR;

extern char 
     *bgchr,
     *fgchr,
     *fontchr,
     *kbgchr,
     *dispchr,
     *selectchr,
     *editname,
     *viewname;

extern int viewx,viewy;
extern int win_inp;       //if win_inp==1 then create_win with Input ....
extern uint viewh,viewl;
extern char rfile[80];
extern char* mnt[];
extern int mntmax;
extern char **menunames,**menucom;
extern int menumax;
extern int extmax;
extern int ScreenL,ScreenH;
extern int centerx,centery,vcenterx,vcentery;

//global vars for comand line options.
extern int margc;
extern char** margv;
extern char *searchpath;
  
extern XFontStruct *fontstr;
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
extern XEvent           ev;
extern XColor           color;
extern Colormap         defcmp;
extern int              scr;
extern int              done;
extern int              Mainl,Mainh;
extern unsigned int padl,padh,butl,buth;

#include "gui.h"

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
extern void InitXClient(int argc,char** argv,int wx,int wy,uint wl,uint wh,char* winname=NULL);
extern Window create_win(char*,int,int,int,int,int);
extern void win_nodecor(Window W);
extern void DeinitXClient();
extern void msg(char*,char*,int,FList *o=NULL);
extern void find_panel(int=540);

extern void XEVENT();

//////////////////////////////////End of file/////////////////////////////
#endif
        
/* ------------ End of file -------------- */

