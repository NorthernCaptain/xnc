#ifndef GUI_TEXT_H
#define GUI_TEXT_H
#include "gui_base.h"

// Class that implements TEXT (label) widget

class Text:public Gui
{
 public:
  uint tl;
  int col;
  char *name;
  Text(int ix,int iy,char *iname,int icol):Gui()
   {x=ix;y=iy;col=icol;name=iname;guitype=GUI_TEXT;};
  ~Text() {if(disp) XFreeGC(disp,gcw);};
  void recalc();
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  virtual int  set_selffocus();
  virtual int  is_selfdraw() { return 0;}; //Redraw object by itself (own window)
  void clear_text_area(int);  
};

//The same as above but creates its own window for label

class WText:public Gui
{
 public:
  int ty;
  uint tl,tx;
  int col,centered;
  char *name;
  WText(int ix,int iy,int il,char *iname,int icol):Gui()
   {x=ix;y=iy;col=icol;l=il;h=18;name=iname;guitype=GUI_WTEXT;centered=1;};
  ~WText() {if(disp) XFreeGC(disp,gcw);};
  void recalc();
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
};


#endif
