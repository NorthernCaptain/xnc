#ifndef IMVIEWER_H
#define IMVIEWER_H
#include "xh.h"
#include "../image2/image.h"

class IMViewer:public Gui
{
  public:
   int x,y,l,h,shown,max,cur,type,unmap,ffl;
   Colormap cmp;
   Pixmap im;
   unsigned long pt[256];
   int ptnum;
   char *name;
   char *fname;
   char *info;
   void (*endfunc)(IMViewer*);
   void (*swfunc)(IMViewer*);
   void (*err_func)();
   IMViewer(char *iname,char *ifname,int itype,void (*ff)(IMViewer*)=NULL):Gui()
    {shown=0;cmp=0;name=iname;fname=ifname;type=itype;endfunc=ff;swfunc=NULL;err_func=NULL;};
   ~IMViewer() {XFreeGC(disp,gcw);if(im) XFreePixmap(disp,im);
                   XDestroyWindow(disp,w);if(info) delete info;};
   virtual void init(Window);
   virtual void click();
   virtual void expose();
   virtual void hide();
   virtual void show();
   void setswfunc(void (*ff)(IMViewer*)) {swfunc=ff;};
   void error_func(void (*ee)()) {err_func=ee;};
};

#endif
////////////////////////////////////End of file///////////////////////////////////////* ------------ End of file -------------- */

