#ifndef SCROLL_H
#define SCROLL_H
#include "xh.h"
#include "gui.h"

void init_scroll();

void deinit_scroll();

class BKey: public Gui
{
  Gui *o;
 public:
  int prflg,hflg,ff;
  uint pl,ph;
  Pixmap pix;
  BKey(int ix,int iy,int il,int ih,Gui*,int);
  ~BKey() {if(disp) XFreeGC(disp,gcw);hflg=0;};
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  void press();
  void setpixmap(Pixmap,int,int);
};         

class ScrollBar: public Gui
{
  Gui *o;
 public:
  int prflg,hflg,val,minval,maxval,range;
  BKey *upk,*downk;
  void (*scrup)(Gui*);
  void (*scrdown)(Gui*);
  void (*scr)(Gui*);
  void (*scring)(Gui*);
  int pages,pagesize,bmy,shown;
  ScrollBar(int ix,int iy,int ih,Gui*);
  ~ScrollBar() {if(disp) XFreeGC(disp,gcw);hflg=0;delete upk;delete downk;};
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  virtual void show();
  virtual void hide();
  virtual void scrollup(Gui*);
  virtual void scrolldown(Gui*);
  void reconfigure(int ix,int iy,int ih);
  void setrange(int,int);
  void setpages(int);
  void setfuncs(void (*isu)(Gui*),void (*isd)(Gui*),void (*is)(Gui*),void(*ising)(Gui*))
    {scrup=isu;scrdown=isd;scr=is;scring=ising;};
  int  get_h() { return h;};
  int  get_l() { return l;};
};         

#endif

/* ------------ End of file -------------- */

