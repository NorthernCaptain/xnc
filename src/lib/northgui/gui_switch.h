#ifndef GUI_SWITCH_H
#define GUI_SWITCH_H
#include "gui_base.h"

// Class that implements SWITCH (choice) widget

class Sw_panel;

class Switch: public Gui
{
    static int  button_width;
    int         highlighted;
 protected:
    void        (*func)();
    int         prflg,tx,ty,hflg;
    uint        tl;
    ulong       col;
    char        *name;
    Pixmap      gl;
    Sw_panel    *sp;
    int         disable;
    void        (*escfunc)();    
 public:
    int sw,excl;

    Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)()=NULL);
    ~Switch() {if(disp){ XFreeGC(disp,gcw);XFreePixmap(disp,gl);hflg=0;};};
    virtual void init(Window ipar);
    virtual void expose();
    virtual void click();
    virtual void press();
    virtual void select();
    void setexcl() {excl=1;};
    void setpanel(Sw_panel* isp) {sp=isp;};
    void set_disable(int i) {disable=i;};
    void set_escapefunc(void (*esc)()) {escfunc=esc;};
};

struct MenuItem;

//Panel of switches

class Sw_panel: public Gui
{
 protected:
  int max;
  MenuItem *smn;
 public:
  Switch **ar;
  int ty,tx,shown,initfl,columns,columnl;
  char *head;
  
  Sw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn=1);
  ~Sw_panel();
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  virtual void show();
  virtual void hide();
  virtual void link(Gui*);
  virtual int  set_selffocus();
  virtual int  is_selfdraw() { return 0;}; //Redraw object by itself (own window)
  void set(int n,int s);
  void excl();
  int get(int n) {return ar[n]->sw;};
  void reaction(Switch*);
  void set_escapefunc(void (*esc)());
};

#endif
