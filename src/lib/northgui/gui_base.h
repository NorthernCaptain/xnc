#ifndef GUI_BASE_H
#define GUI_BASE_H

#include "globals.h"
#include "xheaders.h"
#include "geomfuncs.h"
#include "xdndclass.h"

//Notification types
#define MENU_HIDE        1
#define MENU_SHOW        2

//Gui types
enum GuiType
 {
     GUI_UNREG=                0,
     GUI_IGNORE=               254,
     GUI_KEY=                  11,
     GUI_SWITCH=               12,
     GUI_SWPANEL=              13,
     GUI_LISTER=               44,
     GUI_PANEL=                17,
     GUI_WIN=                  1,
     GUI_SEP=                  2,
     GUI_VIEWER=               3,
     GUI_EDITOR=               4,
     GUI_SPWIN=                5,
     GUI_CMDLINE=              6,
     GUI_INPUT=                7,
     GUI_TEXT=                 8,
     GUI_WTEXT=                18,
     GUI_MENUBAR=              9,
     GUI_MENU=                 10,
     GUI_BKEY=                 14,
     GUI_SCROLL=               45,
     GUI_EXWIN=                46,
     GUI_PAGER=                47,
     GUI_IMLOAD=               48,
     GUI_FBROWSE=              16,
     GUI_SWITCHER=             49,
     GUI_FVISUAL=              50,
     GUI_BOOKMARK=             51,
     GUI_QHELP=                52,
     GUI_INFOWIN=              53,
     GUI_ICONKEY=              54
 };

extern     int gui_options;
//Defines for 'gui_options' variable
#define        THICK_BORDER            1

//Common class of all GUI objects used in xnc and friends

class Gui:public DndObject
{
  int  mark;
 public:
  int foc;
  Gui *next,*prev,*guiobj;
  Window w,parent;
  GC gcw;
  GuiType guitype;
  char in_name[32];
  char in_family[32];
  int x,y;
  uint l,h;
  Gui():DndObject() 
      {
	  next=prev=NULL;
	  foc=0;
	  guiobj=NULL;
	  in_name[0]=0;
	  in_family[0]=0;
	  guitype=GUI_UNREG;
	  mark=0;
      };
  virtual void expose() {};
  virtual void click()=0;
  virtual void show();
  virtual void hide();
  virtual void init(Window) {};
  virtual void link(Gui *o) {next=o;o->prev=this;};
  virtual void scrollup(Gui*) {};
  virtual void scrolldown(Gui*) {};
  virtual void scroll(Gui*) {};
  virtual void scrolling(Gui*) {};
  virtual void guifunc(Gui*,int,...) {};
  virtual void set_iname(char *iname) {strcpy(in_name, iname);};
  virtual void set_ifamily(char *iname) {strcpy(in_family, iname);};
  virtual int  geometry_by_iname();
  virtual int  set_selffocus();
  virtual int  is_selfdraw() { return 1;}; //Redraw object by itself (own window)
  virtual int  is_marked() { return mark;};
  virtual void set_mark() { mark=1;};
  virtual void unset_mark() { mark=0;};
};

struct List
{
    Gui *obj;
    Window w;
    List *next,*prev;
    List() {next=prev=NULL;obj=NULL;};
};

extern List el, l_exp;
extern Gui* find(Window ww);
extern Gui* findexp(Window ww);
extern void addto_el(Gui* o,Window w);
extern void addto_exp(Gui* o,Window w);
extern void delall_el();
extern void delall_exp();
extern void delfrom_el(Gui*);
extern void delfrom_exp(Gui*);


extern Gui *focobj;

#endif
