/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:     leo@pop.convey.ru
*  Fido:      2:5030/627.15
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/
#include "imview.h"
extern Atom wmDeleteWindow;

void IMViewer::show()
{
 XMapRaised(disp,w);
 addto_el(this,w);
 ffl=1;
}

void IMViewer::hide()
{            
 XUnmapWindow(disp,w);
 delfrom_el(this);
 delfrom_exp(this);
  shown=0;
 im_freecmp(cmp,pt,ptnum);
}

void IMViewer::init(Window ip)
{
 parent=ip;
 im=LoadPixmap(fname,type,l,h);
 info=NULL;
  if(im)
  {
   info=getpicinfo();
   ptnum=im_getpixels(pt);
   cmp=im_get_colormap();
   }
 else {l=220;h=120;im=0;};
 w=create_win(name,20,20,l,h,ExposureMask | ButtonPressMask | KeyPressMask |
   StructureNotifyMask);
 gcw=XCreateGC(disp,w,0,NULL);
 XSetForeground(disp,gcw,cols[1]);
 XSetBackground(disp,gcw,cols[0]);
 XSetFont(disp,gcw,fontstr->fid);
 if(im) XSetWindowBackgroundPixmap(disp,w,im);
 if(cmp) XSetWindowColormap(disp,w,cmp);
 XSetWMProtocols (disp, w, &wmDeleteWindow, 1);
}       

void IMViewer::click()
{
 KeySym ks;
 if(ev.xany.window==w)
 switch(ev.type){
 case UnmapNotify: unmap=1;break;
 case MapNotify: unmap=0;break;
  case ClientMessage:
    if (ev.xclient.format == 32 && ev.xclient.data.l[0] == wmDeleteWindow && endfunc)
     endfunc(this);
  break;
 case Expose:
  expose();break;
  case ButtonPress:
   if(ev.xbutton.button==Button3 && endfunc) endfunc(this);
   else 
   {
    if(im==0 && err_func!=NULL) err_func(); else
    if(unmap==0) XSetInputFocus(disp,w,RevertToNone,CurrentTime);
   }
   break;
  case KeyPress:
   ks=XLookupKeysym(&ev.xkey,0);
   if((ks==XK_Escape || ks==XK_Execute) && endfunc!=NULL) endfunc(this);
   else if(ks==XK_Tab || ks==XK_F6){ if(swfunc) swfunc(this);}
   else if(ks==XK_i || ks==XK_I)
    if(info) XDrawImageString(disp,w,gcw,2,18,info,strlen(info));
   break;
  }
}

void IMViewer::expose()
{
 int ty,tx;
 shown=1;
 if(im==0)
 XDrawString(disp,w,gcw,l/2-40,h/2+10,"Picture Error",13);
 if(ffl)
 {
  XSetInputFocus(disp,w,RevertToNone,CurrentTime);
  ffl=0;
 }
}


