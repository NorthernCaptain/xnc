#ifndef INFOWIN_H
#define INFOWIN_H
#include "xh.h"
#include "gui.h"

class InfoWin:public Win
{
 public:
  char message[256];
  long fullcoun,currentcoun;
  int need_key,bgbit,shown;
  KEY *bgkey;
  int curtime, lasttime, lastsize;
  InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey=0):
    Win(ix,iy,il,ih,iname,icol,0)
   {
       x=ix;
       y=iy;
       l=il;
       h=ih;
       col=icol;
       name=iname;
       hflg=0;
       guitype=GUI_INFOWIN;
       guiexpose=NULL;
       bgkey=NULL;
       escape_func=NULL;
       shown=0;
       message[0]=0;
       fullcoun=currentcoun=0;
       need_key=ikey;
       bgbit=0;
       lasttime=curtime=lastsize=0;
   };
  ~InfoWin() {//if(disp){XFreeGC(disp,gcw);XDestroyWindow(disp,w);};
        if(bgkey) delete bgkey;};
  virtual void init(Window);
  virtual void show();
  virtual void hide();
  virtual void click();
  virtual void expose();
  virtual void guifunc(Gui*, int, ...);
  virtual void expose_clip_area();
  virtual void expose_counter();
  void add_toexpose(Gui* ex) {guiexpose=ex;};
  void set_escapefunc(void (*v)()) {escape_func=v;hflg=1;};
  void set_message(char*);
  void set_coun(int);
  void update_coun(int);
  void draw_coun(int, int, int=0);
};

extern InfoWin *infowin;

extern void create_infowin(char *head);
extern void create_ftp_infowin(char *head);
extern void del_infowin();
extern void to_infowin(char *str);
extern void to_infowin_lastused();

#endif
/* ------------ End of file -------------- */

