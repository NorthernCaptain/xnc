#ifndef GUI_WIN_H
#define GUI_WIN_H
#include "gui_base.h"
#include "xshrloader.h"
struct List;

// Class that implements WINDOW

class Win:public Gui
{
 protected:
  virtual void press_close();
  void         post_expose();
 public:
  int ty,ffl;
  uint tl;
  int col,prflg,hflg;
  char *name;
  List *oldel;
  Gui *guiexpose;
  void (*escape_func)();
  Sprite *spr_con1;
  Sprite *spr_con2;
  Sprite *spr_con3;
  Sprite *spr_con4;
  RecArea *ra_tbl;
  Win(int ix,int iy,int il,int ih,char *iname,int icol,int hfl=0):Gui()
   {x=ix;y=iy;l=il;h=ih;col=icol;name=iname;hflg=hfl;guitype=GUI_WIN;
    guiexpose=NULL;escape_func=NULL;ra_tbl=NULL;};
  ~Win() {if(disp){XFreeGC(disp,gcw);XDestroyWindow(disp,w);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  void nonresizable();
  void add_toexpose(Gui* ex) {guiexpose=ex;};
  void set_escapefunc(void (*v)()) {escape_func=v;hflg=1;};
};


// Window with special configuration (window form)

class SpecialWin:public Gui
{
 public:
  int ty,ffl;
  uint tl;
  int col,prflg,hflg,nn,al;
  char *name;
  Pixmap pm1;
  SpecialWin(int ix,int iy):Gui()
   {x=ix;y=iy;guitype=GUI_SPWIN;};
  ~SpecialWin() {if(disp) {XFreePixmap(disp,pm1);XFreeGC(disp,gcw);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  void nextpiece();
  void initroute(FList *,int);
};


// Window with special configuration and background image (xSprite)
class SpriteWin:public Gui
{
 protected:
    List *oldel;
    int ty,ffl;
    uint tl;
    int col,prflg,hflg,nn,al;
    xSprite  sprite;
 public:
    SpriteWin(int ix,int iy, ResId);
    ~SpriteWin() 
	{
	    if(disp) {XFreeGC(disp,gcw);};
	};
    virtual void init(Window);
    virtual void show();
    virtual void hide();
    virtual void click();
    virtual void expose();

    void    set_center_to(int ix, int iy) { x=ix - l/2; y=iy - h/2;};
};


#endif
