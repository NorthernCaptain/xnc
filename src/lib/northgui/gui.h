#ifndef GUI_H
#define GUI_H
#include "gui_base.h"
#include "flist.h"
#include "history.h"
#include "guilocale.h"
#include "metacaller.h"
#include "iconsets.h"

#include "gui_menu.h"
#include "gui_input.h"
#include "gui_key.h"
#include "gui_text.h"
#include "gui_win.h"
#include "gui_switch.h"
#include "gui_pan.h"

enum RuState {KOI8,CP866,WIN1251};

class Viewer:public Gui
{
 public:
  GC gct;
  int ty,ffl,tx,cx,cy,row,highlight,unmap,totallines;
  uint tl;
  int col,prflg,hflg,base,vish,visl,cbase,sl,nn,n,hex,tab,stx,koi;
  char *name;
  char *buf;
  char findbuf[255];
  List *oldel;
  FList *node;
  ScrollBar *scr;
  void (*endfunc)(Viewer*);
  void (*swfunc)(Viewer*);
  void (*external_reconfigure)(Viewer*,int,int,int,int);
  RuState  rusconv_state;
  Viewer(int ix,int iy,int il,int ih,
         char *iname,int icol,void(*ff)(Viewer*)=NULL):Gui()
    {
      x=ix;y=iy;l=il;h=ih;col=icol;
      name=iname;unmap=0;hflg=0;
      buf=NULL;
      guitype=GUI_VIEWER;
      endfunc=ff;
      findbuf[0]=0;
      swfunc=NULL;
      external_reconfigure=NULL;
      rusconv_state=KOI8;
	};
  ~Viewer() {if(disp) {XFreeGC(disp,gcw);XFreeGC(disp,gct);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  virtual void scroll(Gui*);
  virtual void scrolling(Gui*);
  virtual void scrollup(Gui*);
  virtual void scrolldown(Gui*);
  void view(FList*);
  char* scanlines();
  void viewmem(FList*,char*);
  void showhexfile();
  void showasc();
  void back();
  void hexoff();
  void rusconvert();
  void reconfigure(int,int);
  void set_external_reconfigure(void (*reconf)(Viewer*,int,int,int,int)) {external_reconfigure=reconf;};
  void wrusconvert();
  void linenum();
  void showcurs(int=1);
  void setswfunc(void (*ff)(Viewer*)) {swfunc=ff;};
  int pagedown(int);
  void info();
  void insert(KeySym*);
  void line(int);
  void save();
  void func(int);
  void hex_asc();
  void find();
};

class Editor:public Gui
{
 public:
  GC gct;
  ScrollBar *scr;
  int ty,ffl,tx,cx,cy,row,dx,bw,ent,dos,koi,rmode;
  int *xl;
  int *chl;
  uint tl;
  int col,prflg,hflg,base,vish,visl,stx,maxr,mod,highe,highb,unmap;
  char *name;
  char **buf;
  char fname[255];
  char findbuf[255];
  List *oldel;
  FList *node;
  void (*endfunc)(Editor*);
  void (*swfunc)(Editor*);
  void (*external_reconfigure)(Editor*,int,int,int,int);
  void (*cfind_func)(char *);
  int enable_bracket;
  GuiLocale gl;

  Editor(int ix,int iy,int il,int ih,
         char *iname,int icol,void(*ff)(Editor*)=NULL):Gui()
   {
     x=ix;y=iy;l=il;h=ih;col=icol;
     name=iname;
     mod=unmap=0;
     hflg=0;guitype=GUI_EDITOR;
     endfunc=ff;
     findbuf[0]=0;
     swfunc=NULL;
     external_reconfigure=NULL;
     cfind_func=NULL;
     enable_bracket=0;
   };
  ~Editor() {if(disp){XFreeGC(disp,gcw);XFreeGC(disp,gct);XDestroyWindow(disp,w);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  virtual void scroll(Gui*);
  virtual void scrolling(Gui*);
  virtual void scrollup(Gui*);
  virtual void scrolldown(Gui*);
  char trans(int mode,char sym);
  void set_external_reconfigure(void (*reconf)(Editor*,int,int,int,int)) {external_reconfigure=reconf;};
  void setswfunc(void (*ff)(Editor*)) {swfunc=ff;};
  void set_cfind_func(void (*ff)(char*)) {cfind_func=ff;};
  void load(FList*);
  void newfile(FList*);
  void view();
  void showcurs(int=1);
  void showline(int);
  int  calclen(int,int);
  void do_convertion();
  void do_wconvertion();
  void pageup();
  void pagedown();
  void rusconvert();
  void reconfigure(int,int);
  void wrusconvert();
  void info();
  void xyinfo();
  void back();
  void backln();
  void delln(int);
  void insertln();
  void insert(char);
  void insspace();
  void insempty(int);
  void func(int);
  int  save();
  void find();
  void find_brackets();
  void reverse_find_bracket();
  void ssave();
  void sfind();
  void cfindfunc(int);
  void cfindwin();
  void cfindpromptwin();
  void set_straight_and_find(char *str);
  void copy();
  void paste();
  void cut();
  void del();
  void sgoto();
  void gotoln();
  void cut_cur_word(char *wordto);
  void exit_and_save();
  void enter_sym(XEvent*);
};

extern char   *getext(char* str);

extern int    str_casecmp(char*,char*);
extern int    str_ncasecmp(char*,char*,int);

extern Pixmap main_pixmap;
extern void   guiSetInputFocus(Display* disp, Window w, int revert_to, Time t);
extern void   prect(Window w, GC & gcw, int x, int y, int l, int h);
extern void   urect(Window w, GC & gcw, int x, int y, int l, int h);
extern int    gui_dummy_x_error_handler(Display*, XErrorEvent*);

extern void   delay(int usec);
extern void   init_l10n();
extern int    shadow;

#endif
///////////////////////////////////////End of file///////////////////////////////////////////

/* ------------ End of file -------------- */

