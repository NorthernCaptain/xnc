/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:     leo@xnc.dubna.su
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
#include "pager.h"
#include "skin.h"
#include "geomfuncs.h"

void drawh(Window w,GC gcw,int x,int y,int l,int h,int fl=0)
{
   XSetForeground(disp,gcw,light_bg_color);
   XDrawLine(disp,w,gcw,x,y+h,x,y+5);
   XDrawLine(disp,w,gcw,x,y+5,x+5,y);
   XDrawLine(disp,w,gcw,x+5,y,x+l-5,y);
   if(fl)
   {
       XDrawLine(disp,w,gcw,x+1,y+h+1,x+1,y+5);
       XDrawLine(disp,w,gcw,x+1,y+5,x+5,y+1);
       XDrawLine(disp,w,gcw,x+5,y+1,x+l-5,y+1);
   }
   XSetForeground(disp,gcw,dark_bg_color);
   XDrawLine(disp,w,gcw,x+l-5,y,x+l,y+5);
   XDrawLine(disp,w,gcw,x+l,y+5,x+l,y+h);
   if(fl)
   {
       XDrawLine(disp,w,gcw,x+l-5,y+1,x+l-1,y+5);
       XDrawLine(disp,w,gcw,x+l-1,y+5,x+l-1,y+h+1);
       XSetForeground(disp,gcw,cols[0]);
       XDrawLine(disp,w,gcw,x+l-3,y+1,x+l+1,y+5);
       XDrawLine(disp,w,gcw,x+l+1,y+6,x+l+1,y+h);
   }
}

void draw_round_head(Window w,GC gcw,int x,int y,int l,int h,int fl=0)
{
   XSetForeground(disp,gcw,light_bg_color);
   XDrawLine(disp,w,gcw,x,y+h,x,y+2);
   XDrawLine(disp,w,gcw,x,y+2,x+2,y);
   XDrawLine(disp,w,gcw,x+2,y,x+l-2,y);
   XSetForeground(disp,gcw,dark_bg_color);
   XDrawLine(disp,w,gcw,x+l-2,y,x+l,y+2);
   XDrawLine(disp,w,gcw,x+l,y+2,x+l,y+h);
}

void draw_rec_head(Window w,GC gcw,int x,int y,int l,int h,int fl=0)
{
    XSetForeground(disp,gcw,light_bg_color);
    XDrawLine(disp,w,gcw,x,y+h,x,y);
    XDrawLine(disp,w,gcw,x,y,x+l,y);
    XSetForeground(disp,gcw,dark_bg_color);
    XDrawLine(disp,w,gcw,x+l,y,x+l,y);
    XDrawLine(disp,w,gcw,x+l,y,x+l,y+h);
}

///////////////////////////////////Pager class///////////////////////////////
void Pager::init(Window ipar)
{
 int tw;
 parent=ipar;
 w=XCreateSimpleWindow(disp,parent,x,y,l,h,0,0,normal_bg_color);
 gcv.background=normal_bg_color;
 gcv.font=fontstr->fid;
 gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
 XSelectInput(disp,w,ExposureMask | ButtonPressMask | ButtonReleaseMask);
 ty=fontstr->max_bounds.ascent-fontstr->max_bounds.descent;
 prflg=0;
}

void Pager::expose()
{
 int i,itl,itx;
 XClearWindow(disp,w);
 XSetForeground(disp,gcw,light_bg_color);
 XDrawLine(disp,w,gcw,0,25,0,h);
 // XDrawLine(disp,w,gcw,1,25,1,h-1);
 XDrawLine(disp,w,gcw,0,25,cur*phl,25);
 XDrawLine(disp,w,gcw,(cur+1)*phl-2,25,l,25);
 XSetForeground(disp,gcw,dark_bg_color);
 XDrawLine(disp,w,gcw,0,h-1,l,h-1);
 XDrawLine(disp,w,gcw,l-1,25,l-1,h);
 XDrawLine(disp,w,gcw,1,h-2,l-1,h-2);
 // XDrawLine(disp,w,gcw,l-2,25,l-2,h-1);

 for(i=0;i<max;i++)
   if(i==cur)
   {
     draw_round_head(w,gcw,cur*phl,0,phl-2,24,1);
     //     drawh(w,gcw,cur*phl,0,phl-2,24,1);
     if(pname[i])
     {
       XSetForeground(disp,gcw,cols[1]);
       itl=strlen(_(pname[i]));
       itx=XTextWidth(fontstr,_(pname[i]),itl);
       XDrawString(disp,w,gcw,i*phl+phl/2-itx/2,20,_(pname[i]),itl);
     }
   }
   else
   {
       //     drawh(w,gcw,i*phl,3,phl-1,21);
     XSetForeground(disp,gcw, darker_bg_color);
     XFillRectangle(disp,w,gcw, i*phl, 3, phl-1, 21);
     draw_rec_head(w,gcw, i*phl, 3, phl-1, 21);
     if(pname[i])
     {
       XSetForeground(disp,gcw,cols[0]);
       itl=strlen(_(pname[i]));
       itx=XTextWidth(fontstr,_(pname[i]),itl);
       XDrawString(disp,w,gcw,i*phl+phl/2-itx/2,21,_(pname[i]),itl);
     }
   }
 if(o[cur]!=NULL)
 {
     for(i=0;i<omax[cur];i++)
	 if(!o[cur][i]->is_selfdraw()) 
	     o[cur][i]->click();
 }
 XFlush(disp);
 while(XCheckWindowEvent(disp,w,ExposureMask,&ev));
   
}

void Pager::hidepage()
{
    if(o[cur])
    {
	for(int i=0;i<omax[cur];i++)
	    if(o[cur][i]) o[cur][i]->hide();
    }
}

void Pager::showpage()
{
    int focused=0;
    if(o[cur])
    {
	for(int i=0;i<omax[cur];i++)
	    if(o[cur][i]) 
	    {
		o[cur][i]->show();
		if(!focused)
		    focused=o[cur][i]->set_selffocus();
	    }
    }
}

void Pager::show()
{
    Gui::show();
    showpage();
}

void Pager::click()
{
    int cx;
    if(ev.xany.window==w)
    {
	switch(ev.type)
	{
	case Expose: 
	    expose();
	    break;
	case ButtonPress: 
	    if(ev.xbutton.y<25)
	    {
		cx=ev.xbutton.x/phl;
		if(cx!=cur && cx<max)
		{
		    hidepage();
		    cur=cx;
		    showpage();
		    //     ev.type=Expose;
		    //     expose();
		}
	    };
	    break;
	case ButtonRelease:
	    break;
	};
    }
}

void Pager::setpage(int p)
{
//////////////////////////Add switching when Pager is already shown/////////

 cur=p;
}
 
////////////////////////////////End of file/////////////////////////////////////////
