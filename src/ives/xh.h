#ifndef XH_H
#define XH_H
#include "globals.h"
#include "xheaders.h"
#include "commonfuncs.h"
#include "xncdebug.h"
#include "colors.h"

#define BDWIDTH   1
#define XNC_DIR "/usr/local/lib"

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
extern int win_inp;       //if win_inp==1 then create_win with Input ....
extern int win_resize;  //if win_resize==1 then create_win with resize ....
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
  
extern XFontStruct *fontstr;
extern XFontStruct *lfontstr;
extern XFontStruct *fixfontstr;
extern XFontStruct *vfontstr;
extern XFontStruct *mfixfontstr;
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
extern Window create_win(char*,int,int,int,int,int,int);
extern void win_nodecor(Window W);
extern void DeinitXClient();
extern void msg(char*,char*,int,FList *o=NULL);
extern void find_panel(int=540);

extern void XEVENT();

extern char *finduser(int);
extern char *findgroup(int);

//////////////////////////////////End of file/////////////////////////////
#endif
        
/* ------------ End of file -------------- */

