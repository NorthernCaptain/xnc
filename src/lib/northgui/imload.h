#ifndef IMLOAD_H
#define IMLOAD_H
#include "image.h"
#include "xh.h"

class IMLoader:public Gui
{
  public:
   int shown,max,cur;
   XImage **im;
   int *width,*hei;
   IMLoader(int ix,int iy,int il,int ih,int imax):Gui()
    {x=ix;y=iy;l=il;h=ih;max=imax;im=new XImage* [max];shown=0;guitype=GUI_IMLOAD;
      width=new int[max];hei=new int [max];cur=0;max=0;};
   ~IMLoader() {/*if(disp) {XFreeGC(disp,gcw);for(int i=0;i<max;i++) XDestroyImage(im[i]);};*/
      delete im;delete hei;delete width;};
   virtual void init(Window);
   virtual void click();
   virtual void expose();
   virtual void hide();
   void setfont(char *nfont);
   void load(char *);
   void setpage(int n) {cur=n;if(shown) expose();};
};

#endif
////////////////////////////////////End of file//////////////////////////////////////


/* ------------ End of file -------------- */

