#ifndef QHELP_H
#define QHELP_H
#include "gui.h"

class QuickHelp: public Gui
{
 public:
  int prflg,tx,ty,nl,ty2;
  uint tl;
  ulong col;
  int mapped,shown;
  char name[256];
  QuickHelp();
  virtual ~QuickHelp() 
   {if(disp) {
     XFreeGC(disp,gcw);
     };};
  void show_this_info(char *head,char *text,int ix,int iy);
  virtual void init(Window ipar);
  virtual void expose();
  virtual void click();
  virtual void show();
  virtual void hide();
};         

#endif/* ------------ End of file -------------- */

