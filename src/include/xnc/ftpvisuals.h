#ifndef FTPVISUALS_H
#define FTPVISUALS_H

//Some classes for FTP visualization.        04APR98 [Leo].

#include "xh.h"
#include "gui.h"
#include "vfs.h"
#include "qhelp.h"
#include "skin.h"

class Switcher: public Gui
{
 public:
  int prflg,tx,ty,hflg,dflg;
  uint tl;
  ulong col;
  char *name;
  Gui *arr[10];
  int maxarr,curarr;
  void (*escfunc)();
  Sprite *skin, *skinpr;
  Switcher(int ix,int iy,int il,int ih);
  ~Switcher() {if(disp) XFreeGC(disp,gcw);hflg=0;dflg=0;};
  void set_escapefunc(void (*v)()) {escfunc=v;};
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  void reconfigure(int ix,int iy,int pl,int ph);
  void press();
  void add_gui(Gui *o);
  void switch_to(int i);
};         


class FtpVisual: public Gui
{
 public:
  int x,y,prflg,tx,ty,hflg,dflg;
  uint l,h,tl,hidden;
  QuickHelp qh;
  int qhnum;
  GC rgc;
  
  FtpVisual(int ix,int iy,int il,int ih);
  ~FtpVisual() {if(disp) XFreeGC(disp,gcw);hflg=0;dflg=0;qhnum=0;};
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  virtual void show();
  virtual void hide();
  virtual void reconfigure(int ix,int iy,int pl,int ph);
  virtual void show_tumb(int i, int ix);
  virtual void show_empty_tumb(int i, int ix);
  void refresh(FTP* fo);
  virtual void rescan();
  void animate_moving(int);
};


#endif
/* ------------ End of file -------------- */

