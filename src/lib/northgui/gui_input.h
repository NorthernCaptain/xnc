#ifndef GUI_INPUT_H
#define GUI_INPUT_H
#include "gui_base.h"

// Class that implements text input widget

class Input:public Gui
{
 protected:
  void (*func)();
  GuiLocale gl;

  virtual void    show_cursor();
  virtual void    hide_cursor();
  virtual void    set_focus();
  virtual void    unset_focus();

  void            goto_end();
  void            go_home();

 public:
  char hist_id[32];
  int ty;
  uint dl,bl;
  int col,hflg,cp,ll,tl,el,firstfl;
  char *buf;
  void (*escfunc)();
  History *hist;
  int first_hist;
  int passwd;        //If == 1 then blind input.
  int bool_expand;
  Input(int ix,int iy,int il,int icol,void (*ifunc)()=NULL):Gui()
   {x=ix;y=iy;l=il;col=icol;tl=0;cp=0;bl=0;el=255;func=ifunc;escfunc=NULL;guitype=GUI_INPUT;strcpy(hist_id,"UNREG");
        hist=NULL;passwd=0;bool_expand=0;};
  ~Input() {if(disp) XFreeGC(disp,gcw);};
  void         set_histid(char *id) {strcpy(hist_id,id);};
  virtual void press();
  virtual void showbuf();
  void         flush();
  void         insert(char);
  void         set_escapefunc(void (*esc)()) {escfunc=esc;};
  void         setbuf(char *ibuf,int nn) {buf=ibuf;el=nn;cp=0;bl=strlen(buf);dl=0;};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  void         extract_path_part(char *buf, int pos, char *topath, char *part);
  int          do_tab_expansion(char *buf, int pos, int maxl);
  void         partial_completion(char*, char*, int*, int*);
  virtual int  dnd_drop(DndData*, int x, int y, DndAction action);
  void         set_passwd_mode() { passwd=1; };
};


#endif
