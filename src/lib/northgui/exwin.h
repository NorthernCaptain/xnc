#ifndef EXWIN_H
#define EXWIN_H
#include "xh.h"
#include "gui.h"
class EXWin:public Gui
{
 public:
  int ty,ffl;
  uint tl;
  int col,prflg,hflg;
  char *name;
  void (*func)(Window,GC&);
  void (*killfunc)();
  EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl=0):Gui()
   {x=ix;y=iy;l=il;h=ih;name=iname;hflg=hfl;func=ifunc;killfunc=NULL;guitype=GUI_EXWIN;};
  ~EXWin() {if(disp){XFreeGC(disp,gcw);XDestroyWindow(disp,w);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  void on_destroy(void (*f)()) {killfunc=f;};
};

#endif
/* ------------ End of file -------------- */

