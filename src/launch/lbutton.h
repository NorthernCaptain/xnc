#ifndef LBUTTON_H
#define LBUTTON_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "xh.h"
#include "image.h"
#include "gui.h"

class LaunchButton: public Gui
{
 public:
  char *launch;
  char *name;
  Pixmap im;
  int x,y,l,h,px,py,prflg,tx,ty,tw,tl,hflg,th;
  uint pl,ph;
  LaunchButton(int ix,int iy,int il,int ih,char *iname,char *ilaunch,char *imname);
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  void press();
};  
  
#endif/* ------------ End of file -------------- */

