#ifndef FBROWSER
#define FBROWSER
#include "xh.h"
#include "gui.h"

class FileBrowser:public Gui
{
 public:
  int ty,ffl;
  uint tl;
  int col,prflg,hflg;
  char *name;
  List *oldel;
  Input *inp,*flt;
  Text *t1,*t2;
  KEY *kok,*kcan;
  Panel *pan;
  char *list[4096];
  char filter[256];
  char fname[1024];
  char curdir[1024];
  int lmax,tm;
  void (*func)();
  FileBrowser(int ix,int iy,int il,int ih,char *iname,int icol,int hfl=0):Gui()
   {x=ix;y=iy;l=il;h=ih;col=icol;name=iname;hflg=hfl;guitype=GUI_FBROWSE;strcpy(filter,"*");fname[0]=0;
    getcwd(curdir,800);func=NULL;
    };
  ~FileBrowser() {if(disp){XFreeGC(disp,gcw);XDestroyWindow(disp,w);};};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  virtual void guifunc(Gui*,int,...);
  void nonresizable();
  void gen_fullname();
  void add_tolist(char*,int);
  void show_info();
  void filter_with_mask(char*,int);
  void set_filter(char* f) {strcpy(filter,f);flt->setbuf(filter,255);};
  int is_filtered(char*,char*);
  void set_func(void (*f)()) {func=f;};
  int read_dir();
};

#endif/* ------------ End of file -------------- */

