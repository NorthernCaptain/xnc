#ifndef GUI_KEY_H
#define GUI_KEY_H
#include "gui_base.h"

// Class that implements KEY widget

class KEY: public Gui
{
 protected:
    int highlighted;
    int prflg,tx,ty,hflg,dflg;
    uint tl;
    ulong col;
    char *name;
    void (*escfunc)();
    Sprite *skin;
    void (*func)();
    virtual void set_focus();
    virtual void unset_focus();
 public:
    KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)());
    virtual ~KEY() {if(disp) XFreeGC(disp,gcw);hflg=0;dflg=0;};
    void set_escapefunc(void (*v)()) {escfunc=v;};
    virtual void init(Window ipar);
    virtual void expose();
    virtual void click();
    virtual void press();
    virtual void move_window(int newx, int newy);
};         

class IconKey: public KEY
{
 protected:
    int icon_idx;
    int iconset_idx;
 public:
    IconKey(int ix,int iy,int il,int ih,int iicon_idx,void (*ifunc)());
    ~IconKey() {};
    virtual void init(Window ipar);
    virtual void expose();
    virtual void press();
};

#endif
