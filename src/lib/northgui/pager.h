#ifndef PAGER_H
#define PAGER_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "xh.h"
class Pager:public Gui
{
 public:
  int ty,ffl,phl;
  uint tl;
  int prflg,max,cur;
  Gui*** o;
  int *omax;
  char **pname;
  Pager(int ix,int iy,int il,int ih,int imax):Gui()
   {
       x=ix;y=iy;
       l=il;h=ih;
       phl=90;
       max=imax;
       cur=0;
       pname=new char* [max];
       guitype=GUI_PAGER;
       o=new Gui** [max];
       for(int i=0;i<max;i++) 
	   o[i]=NULL;
       omax=new int [max];
   };
  ~Pager() 
      {
	  if(disp) 
	  {
	      XFreeGC(disp,gcw);
	      XDestroyWindow(disp,w);
	  }
          for(int i=0;i<max;i++) 
	      if(o[i]) delete o[i];
	  delete o;
	  delete omax;
	  delete pname;
      };
  virtual void init(Window);
  virtual void click();
  virtual void show();
  virtual void expose();
  virtual void showpage();
  void hidepage();
  void pagemaxobj(int n) {o[cur]=new Gui* [n];omax[cur]=0;};
  void setpage(int p);
  void setpagename(char* ipname) {pname[cur]=ipname;};
  void setpageheadlen(int il) {phl=il;};
  void addobj(Gui *obj) {o[cur][omax[cur]]=obj;omax[cur]++;};
  void delobj(Gui* obj) 
      {
	  int i=0;
	  while(o[cur][i]!=obj) i++; 
	  o[cur][i]=NULL;
	  omax[cur]=0;
      };
};
#endif
/////////////////////////////////////////////End of file//////////////////////////////////////

/* ------------ End of file -------------- */

