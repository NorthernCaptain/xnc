/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:     old@sunct2.jinr.dubna.su
*
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
#include "imload.h"


void IMLoader::load(char *name)
{
  if((im[max] =LoadXImage(name,CMP_COM,F_AUTO))==NULL)
     width[max]=0;
  else
  {
   width[max]=im[max]->width;
   hei[max]=im[max]->height;
   max++;
   }
}
 
void IMLoader::hide()
{
  Gui::hide();
  shown=0;
}

void IMLoader::init(Window ip)
{
 parent=ip;
 w=XCreateSimpleWindow(disp,parent,x,y,l,h,1,0,keyscol[1]);
 gcw=XCreateGC(disp,w,0,NULL);
 XSetForeground(disp,gcw,cols[2]);
 XSelectInput(disp,w,ExposureMask);
}       

void IMLoader::click()
{
 if(ev.xany.window==w && ev.type==Expose)
  expose();
}

void IMLoader::expose()
{
 shown=1;
// XClearWindow(disp,w);
  if(width[cur]!=0)
   XPutImage(disp,w,gcw,im[cur],0,0,0,0,width[cur],hei[cur]);
  else XDrawString(disp,w,gcw,l/2-40,h/2+10,"Picture Error",13);
}


