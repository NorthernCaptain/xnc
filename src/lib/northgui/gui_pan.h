#ifndef GUI_PAN_H
#define GUI_PAN_H
#include "gui_base.h"

// Class that implements Panel widget


class ScrollBar;

class Panel: public Gui
{
 protected:
  int              prflg,tx,ty,hflg,max,base,cur,vh;
  uint             tl;
  ulong            col;
  char             **names;
  void             (*canfunc)();
  void             (*keyfunc)(KeySym,Panel*);
  ScrollBar        *scr;
  int              dbtime;
  int              lmx,lmy,bclk, every_move;
  int              (*func)(int,char*);
  int              (*every_move_func)(int,char*);
  void             move_line_up();
  void             move_line_down();
 public:
  Panel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*));
  ~Panel() {if(disp) XFreeGC(disp,gcw);hflg=0;};
  void             setkeyfunc(void (*ik)(KeySym,Panel*)) {keyfunc=ik;}
  virtual void     init(Window ipar);
  virtual void     expose();
  virtual void     click();
  virtual void     show();
  virtual void     hide();
  virtual void     scroll(Gui*);
  virtual void     scrollup(Gui* o) {scroll(o);};
  virtual void     scrolling(Gui* o) {scroll(o);};
  virtual void     scrolldown(Gui* o) {scroll(o);};
  virtual void     shownames();
  void             reset_list();
  virtual void     showcurs(int=1);
  void             escfunc(void (*ecanfunc)()) {canfunc=ecanfunc;};
  void             add_element(char*);
  void             find_and_set(char*);
  void             activate_function();
  int              get_current_index() { return base+cur;};
  void             set_every_move_to(int (*ifunc)(int,char*)) {every_move_func=ifunc;};
  void             set_max_to(int i) { max=i;};
  void             cursor_to(int i) { cur=i % vh; base=i-cur;};
  void             set_names_to(char **inames) {names=inames;};
};



#endif
