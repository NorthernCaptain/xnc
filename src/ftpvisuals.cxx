/****************************************************************************
*  Copyright (C) 1998 by Leo Khramov
*  email:     leo@unix1.jinr.dubna.su
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

//This file contains classes/functions and objects for FTP visualization

#include "ftpvisuals.h"
#include "vfs.h"
#include "ftpfs.h"
#include "query_windows.h"
#include "skin.h"
#include "geomfuncs.h"
#include "panel.h"

#define TUMBSIZE  110
#define ANIMSTEP   10


extern FTP *ftparr[];

///////////////////////////Switcher class/////////////////////////////////////
// *INDENT-OFF*        
Switcher::Switcher(int ix, int iy, int il, int ih):Gui()
// *INDENT-ON*        
{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  prflg = 0;
  foc = 0;
  hflg = 0;
  dflg = 1;
  escfunc = NULL;
  guitype = GUI_SWITCHER;
  guiobj = NULL;
  skin=skinpr=NULL;
  maxarr=curarr=0;
}

void Switcher::add_gui(Gui *o)
{
  arr[maxarr]=o;
  maxarr++;
}

void Switcher::reconfigure(int ix, int iy, int il, int ih)
{
  x=ix;
  y=iy;
  XMoveWindow(disp,w,x,y);
}

void   Switcher::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  GEOM_TBL *tbl;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  skin=(Sprite*)tbl->data1;
  skinpr=(Sprite*)tbl->data2;
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  if (x < 0)
    {
      x += pl - l;
      if (y < 0)
        {
          y += ph - h;
          xswa.win_gravity = SouthEastGravity;
        }
      else
        xswa.win_gravity = SouthWestGravity;
    }
  else if (y < 0)
    {
      y += ph - h;
      xswa.win_gravity = NorthEastGravity;
    }
  else
    xswa.win_gravity = NorthWestGravity;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, keyscol[1]);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = keyscol[1];
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | OwnerGrabButtonMask);
  ty = (h + fixfontstr->max_bounds.ascent - fixfontstr->max_bounds.descent) / 2;
}

void   Switcher::expose()
{
  int xl;
  if(skin)
  {
    XCopyArea(disp, skin->im->skin, w, gcw, skin->x, skin->y,
            l, h, 0, 0);
  }
  else
  {
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
/*  XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
  XDrawLine(disp, w, gcw, 1, 1, 1, h - 1); */
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
/*  XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
  XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1); */
  xl=XTextWidth(fixfontstr, ">", 1);
  if(shadow)
  {
  XSetForeground(disp, gcw, cols[0]);
  XDrawString(disp,w,gcw,(l-xl)/2,ty+1,">",1);
  }
  XSetForeground(disp, gcw, cols[5]);
  XDrawString(disp,w,gcw,(l-xl)/2-1,ty,">",1);
  hflg = 1;
  }
}

void   Switcher::press()
{
  if(skinpr)
  {
    XCopyArea(disp, skinpr->im->skin, w, gcw, skinpr->x, skinpr->y,
            (unsigned)skinpr->l, (unsigned)skinpr->h, 0, 0);
  }
  else
  {
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
  XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
  XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
  if(shadow)
  {
  XSetForeground(disp, gcw, cols[0]);
  XDrawString(disp,w,gcw,6,ty+2,">",1);
  }
  XSetForeground(disp, gcw, cols[5]);
  XDrawString(disp,w,gcw,4,ty,">",1);
  }
}

void   Switcher::click()
{
  if (dflg == 0)
    return;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case ButtonPress:
          if (ev.xbutton.button == Button1)
            {
              press();
              prflg = 1;
            }
          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1 && prflg == 1)
            {
              prflg = 0;
              expose();
              arr[curarr]->hide();
              curarr++;
              if(curarr>=maxarr)
                curarr=0;
              arr[curarr]->show();
              if (guiobj)
                guiobj->guifunc(this, 0);
            }
          break;
        };
    }
}

void Switcher::switch_to(int i)
{
        if(curarr!=i && i<maxarr)
        {
                arr[curarr]->hide();
                curarr=i;
                arr[curarr]->show();
        }
}

//////////////////////////////////////FtpVisual class////////////////////////////////////////////
// *INDENT-OFF*        
FtpVisual::FtpVisual(int ix, int iy, int il, int ih):Gui()
// *INDENT-ON*        
{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  guitype = GUI_FVISUAL;
  guiobj = NULL;
  hidden=1;
}

void   FtpVisual::show()
{
  Gui::show();
  hidden=0;
}

void   FtpVisual::hide()
{
  Gui::hide();
  hidden=1;
}

void   FtpVisual::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
  geometry_by_iname();
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  if (x < 0)
    {
      x += pl - l;
      if (y < 0)
        {
          y += ph - h;
          xswa.win_gravity = SouthEastGravity;
        }
      else
        xswa.win_gravity = SouthWestGravity;
    }
  else if (y < 0)
    {
      y += ph - h;
      xswa.win_gravity = NorthEastGravity;
    }
  else
    xswa.win_gravity = NorthWestGravity;
  rgc = XCreateGC(disp, Main, 0, NULL);
  XSetSubwindowMode(disp, rgc, IncludeInferiors);
  XSetFunction(disp, rgc, GXxor);
  XSetForeground(disp, rgc, cols[1]);
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, keyscol[1]);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = keyscol[1];
  gcv.font = mfixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | OwnerGrabButtonMask);
  ty = (h + mfixfontstr->max_bounds.ascent - mfixfontstr->max_bounds.descent) / 2;
  tx=XTextWidth(mfixfontstr, "M",1);
  qh.init(Main);
}

void   FtpVisual::expose()
{
  rescan();
  hflg = 1;
}

void   FtpVisual::click()
{
  int i,tl=tx*3+10,dx;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case ButtonPress:
          if(ev.xbutton.button==Button1 && ev.xbutton.x>tl)
          {
            i=(ev.xbutton.x-tl)/TUMBSIZE;
            if(i>=MaxFtps) break;
            if(ftparr[i]==NULL)
                    {
                          panel->create_ftp_link();
                          break;
                    }
            dx=(ev.xbutton.x-tl)%TUMBSIZE;
            if(dx>TUMBSIZE-12)
            {
              if(ev.xbutton.y<11)
              {
                ftparr[i]->autoraise^=1;
                ftparr[i]->autoraise&=1;
                show_tumb(i,i*TUMBSIZE+tl);
              } else
              {
                  qhnum=i;
                  prect(w,gcw,i*TUMBSIZE+tl+TUMBSIZE-12,12,9,4);
                  qh.show_this_info(ftparr[i]->host,ftparr[i]->com.reason, i*TUMBSIZE+tl,22);
                  qh.show();
              }
              break;
            } else if(dx<12 && ev.xbutton.y<10)
                       {
                         if(ftparr[i]->work)
                           {
                                   simple_mes("FTP",_("Can't close during operation!"));
                                   break;
                           } else
                           {
                               panel->check_and_pop(ftparr[i]);
                               panel->panel2->check_and_pop(ftparr[i]);
                               ftparr[i]->close_fs();
                           }
                          break;
                       }
             if(dx>12 && dx<TUMBSIZE-12)
             {
                    animate_moving(i);
                    ftparr[i]->bg_switch();
                    break;
             }

          } else
                  if(ev.xbutton.button==Button2 && ev.xbutton.x>tl)
                  {
                    i=(ev.xbutton.x-tl)/TUMBSIZE;
                    if(i>=MaxFtps) break;
                    if(ftparr[i]==NULL)
                    {
                          panel->create_ftp_link();
                          break;
                    }
                    animate_moving(i);
                    ftparr[i]->bg_switch();
                  }
          break;   
        case ButtonRelease:
            if(qhnum>=MaxFtps || qhnum<0) break;
            if(ftparr[qhnum])
             urect(w,gcw,qhnum*TUMBSIZE+tl+TUMBSIZE-12,12,9,4);
            qh.hide();
            break;
        };
    }
}

void FtpVisual::reconfigure(int ix, int iy, int il, int ih)
{
  x=ix;
  y=iy;
  l=il;
  h=ih;
  XMoveResizeWindow(disp,w,x,y,l,h);
}

void FtpVisual::show_tumb(int i, int ix)
{
  FTP* fo=ftparr[i];
  int tl=strlen(fo->host);
  if(tl>10) tl=10;
  urect(w,gcw,ix+1,0,TUMBSIZE,h);
  XSetForeground(disp,gcw,keyscol[2]);
  XDrawLine(disp,w,gcw,ix+TUMBSIZE+1,0,ix+TUMBSIZE+1,h-2);
  XSetForeground(disp,gcw,cols[0]);
  if(shadow)
    XDrawString(disp,w,gcw,ix+21,ty+1,fo->host,tl);
  XDrawLine(disp,w,gcw,ix+6,4,ix+10,8);
  XDrawLine(disp,w,gcw,ix+10,4,ix+6,8);
  XSetForeground(disp,gcw,cols[2]);
  XDrawLine(disp,w,gcw,ix+5,3,ix+9,7);
  XDrawLine(disp,w,gcw,ix+9,3,ix+5,7);
  XSetForeground(disp,gcw,cols[4]);
  XDrawString(disp,w,gcw,ix+20,ty,fo->host,tl);
  if(fo->work)
     XSetForeground(disp,gcw,cols[2]);
  else
     XSetForeground(disp,gcw,cols[3]);
  XFillRectangle(disp,w,gcw,ix+6,12,4,4);
  prect(w,gcw,ix+4,10,7,7);
  if(fo->autoraise)
    prect(w,gcw,ix+TUMBSIZE-12,4,9,4);
  else
    urect(w,gcw,ix+TUMBSIZE-12,4,9,4);
  urect(w,gcw,ix+TUMBSIZE-12,12,9,4);
}

void FtpVisual::show_empty_tumb(int i, int ix)
{
  int ttx=ix+(TUMBSIZE-XTextWidth(mfixfontstr,_("Vacant FTP"),
				  strlen(_("Vacant FTP"))))/2;
  urect(w,gcw,ix+1,0,TUMBSIZE,h);
  XSetForeground(disp,gcw,keyscol[2]);
  XDrawLine(disp,w,gcw,ix+TUMBSIZE+1,0,ix+TUMBSIZE+1,h-2);
  if(shadow)
  {
          XSetForeground(disp,gcw,cols[0]);
          XDrawString(disp,w,gcw,ttx+1,ty+1,_("Vacant FTP"),
		      strlen(_("Vacant FTP")));
  }
  XSetForeground(disp,gcw,cols[1]);
  XDrawString(disp,w,gcw,ttx,ty,_("Vacant FTP"),
	      strlen(_("Vacant FTP")));
}

void FtpVisual::refresh(FTP* fo)
{
  int i,tl=tx*3+10;
  if(hidden) return;
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i]==fo)
    {
         show_tumb(i,i*TUMBSIZE+tl);
         break;
    }
}      

void FtpVisual::rescan()
{
  int i,tl=tx*3+10;
  if(hidden) return;
  XClearWindow(disp, w);
  urect(w,gcw,0,0,tl,h);
  urect(w,gcw,tl+1,0,l-tl-2,h);
  if(shadow)
  {
      XSetForeground(disp,gcw,cols[0]);
      XDrawString(disp,w,gcw,6,ty+1,"FTP",3);
  }
  XSetForeground(disp,gcw,cols[1]);
  XDrawString(disp,w,gcw,5,ty,"FTP",3);
   for(i=0;i<MaxFtps;i++)
       if(ftparr[i])
         show_tumb(i,i*TUMBSIZE+tl);
       else
         show_empty_tumb(i,i*TUMBSIZE+tl);
}

void   FtpVisual::animate_moving(int i)
{
  int    ix = x + tx*3 +9 +(i+1)*TUMBSIZE, iy = y+h-1;
  int    il = TUMBSIZE-1, ih = h - 1;
  int    bh = ih, bl = il;
  int    bx = ix, by = iy;
  int    istep = 0;
  int    dx=0, dy = panel->y + 25 - by;
  int    dl=0, dh = 25 - ih;
  int    oldl, oldh, oldx, oldy;
  switch (panel->lay)
    {
    case 0:
      dx = panel->x + panel->l - 1 - bx;
      dl = panel->l - il - 1;
      break;
    case 1:
      dx = panel->x + panel->l / 2 + 19 - bx;
      dl = panel->l / 2 + 19 - il;
      break;
    case 2:
      dx = panel->x + panel->l - 1 - bx;
      dl = panel->l / 2 + 19 - il;
      break;
    };
  XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
  XSync(disp, 0);
  delay(20);
  while (istep < ANIMSTEP)
    {
      istep++;
      oldx = ix - il;
      oldy = iy - ih;
      oldl = il;
      oldh = ih;
      il = bl + (dl * istep / ANIMSTEP);
      ih = bh + (dh * istep / ANIMSTEP);
      ix = bx + (dx * istep / ANIMSTEP);
      iy = by + (dy * istep / ANIMSTEP);
      XDrawRectangle(disp, Main, rgc, oldx, oldy, oldl, oldh);
      XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
      XSync(disp, 0);
      delay(20);
    }
  XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
  XSync(disp, 0);
}

