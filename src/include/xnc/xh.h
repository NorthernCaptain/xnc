#ifndef XH_H
#define XH_H

#include "globals.h"
#include "xheaders.h"
#include "systemfuncs.h"
#include "colors.h"

#define BDWIDTH   1
#define XNC_DIR "/usr/local/lib"
extern char host[];
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

extern int layout;
extern int viewx,viewy;
extern int ScreenL,ScreenH;
extern uint viewh,viewl;
extern char rfile[];
extern char tmppath[];
extern char ver[];
extern char **menunames,**menucom;
extern int menumax;
extern int extmax;
extern int centerx,centery,vcenterx,vcentery, wm_y_correction;

//global vars for comand line options.
extern int margc;
extern char** margv;
extern int allow_t_raising;
extern int win_resize;

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
extern Window           base_win;
extern Window           main_win;
extern GC               gc;
extern XGCValues        gcv;
extern XEvent           ev;
extern XColor           color;
extern Colormap         defcmp;
extern int              scr;
extern int allow_animation;
extern int allow_bookmark;
extern int allow_dir_icon;
extern int allow_file_icon;
extern int              done;
extern char syspath[];
extern int              Mainl,Mainh,Mainx,Mainy;
extern char             winame[];

#include "gui.h"

extern int        term_sticky;

extern char **extcom;
extern int   ext_find(char*);
extern int   ext_find2(char*,int);
extern void load_ext();
extern void load_menu();
extern int findmenukey(KeySym);
extern int getps();
extern int getdf(Panel*);
extern int getfree(Panel*);
extern void delps(int);
extern void man_page();
 
/////////////////////////////////////////////////////////////////////////

extern unsigned long gencol(XColor& col,int per);
extern void InitXClient(int argc,char** argv,int wx,int wy,int wl,int wh,char* winname=NULL);
extern Window create_win(char*,int,int,int,int,int,int=1);
extern void DeinitXClient();
extern void msg(char*,char*,int,FList *o=NULL);
extern void find_panel(int=540);
extern "C" void tt_printf(char *fmt,...);
extern "C" void tt_write(unsigned char *buf, int count);
extern "C" int psystem(char *);
extern "C" void process_x_event(XEvent *);
extern "C" void set_cmdchild_signal();
extern "C" int is_term_on();
extern int str_casecmp(char*,char*);

extern void XEVENT();

extern int is_new_version();
extern void   make_home_file_dir();

//////////////////////////////////End of file/////////////////////////////
#endif
        
/* ------------ End of file -------------- */

