/****************************************************************************
*  Copyright (C) 1996-98 by Leo Khramov
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
#include "infowin.h"
#include "panel.h"
#include "pluginman.h"
//This file contains InfoWin class functions.
//InfoWin class used for showing file information 
//while copying/moving/deleting files.

///////////////////////////////////InfoWin class///////////////////////////////

void   InfoWin::init(Window ipar)
{
  if(need_key)
    h+=20;
  Win::init(ipar);
  bgbit=0;
  if(need_key)
  {
    bgkey=guiplugin->new_KEY(-10,-10,40,20,"to BG",1,NULL);
    bgkey->init(w);
    bgkey->link(bgkey);
    bgkey->guiobj=this;
  }
}

void   InfoWin::expose()
{
  Win::expose();
  expose_clip_area(); 
}

void InfoWin::expose_counter()
{
  int ll;
  int deltay=0;
  char buf[80];
  float val;
  time_t t;
  if(bgkey)
    deltay=20;
  if(fullcoun<=0)
    fullcoun=1;
  prect(w, gcw, 10, h-25-deltay, l-20, 10);
  ll=int(float((l-24))*float(currentcoun)/fullcoun);
  XSetForeground(disp,gcw,keyscol[1]);
  XFillRectangle(disp, w, gcw, 12+ll, h-23-deltay, l-24-ll,6);
  XSetForeground(disp,gcw,cols[4]);
  XFillRectangle(disp, w, gcw, 12, h-23-deltay, ll,6);

  if(bgkey)
  {
    if(lastsize>currentcoun)
    {
      time(&t);
      lasttime=t;
    }
    if(curtime-lasttime>0)
    {
      XSetForeground(disp,gcw,cols[1]);
      val=(float)(currentcoun)/1024.0/(float)(curtime-lasttime);
      sprintf(buf,"%2.2fk/sec   ", val);
      XDrawImageString(disp, w, gcw, 10, h-16, buf, strlen(buf));
    }
    lastsize=currentcoun;
  }                          
}

void InfoWin::expose_clip_area()
{
  int textl;
  int ll;
  int deltay=0;
  if(bgkey)
    deltay=20;
  if(shown)
  {
    XSetForeground(disp, gcw, keyscol[1]);
    if(fullcoun)
    {
      XFillRectangle(disp, w, gcw, 8, 30, l-16, 30);
      XSetForeground(disp, gcw, cols[1]);
      textl=strlen(message);
      XDrawString(disp, w, gcw, l / 2 - XTextWidth(fontstr, message, textl) / 2, 45, message, textl);
      expose_counter();
    } else
    {
      XFillRectangle(disp, w, gcw, 8, 40, l-16, 30);
      XSetForeground(disp, gcw, cols[1]);
      textl=strlen(message);
      XDrawString(disp, w, gcw, l / 2 - XTextWidth(fontstr, message, textl) / 2, need_key ? 45 : 55, message, textl);
    }
  }
}

void   InfoWin::update_coun(int delta)
{
  if(need_key)
    return;
  currentcoun+=delta;
  if(currentcoun<0)
    currentcoun=0;
  else if(currentcoun>fullcoun)
    currentcoun=fullcoun;
  expose_clip_area();
  XFlush(disp);
}

void   InfoWin::draw_coun(int total, int delta, int itime)
{
  fullcoun=total;
  currentcoun=delta;
  curtime=itime;
  if(currentcoun<0)
    currentcoun=0;
  else if(currentcoun>fullcoun)
    currentcoun=fullcoun;
  if(shown)
  {
    expose_counter();
    XFlush(disp);
  }
}

void InfoWin::set_coun(int fullcounter)
{
  if(need_key)
    return;
  fullcoun=fullcounter;
  currentcoun=0;
  expose_clip_area();
  XFlush(disp);
}

void   InfoWin::click()
{
  if (ev.xany.window == w)
  {
    switch (ev.type)
    {
    case Expose:
      expose();
      break;
    case ButtonPress:
      break;
    case ButtonRelease:
      break;
    };
  }
}

void   InfoWin::show()
{
  time_t t;
  if(shown==0)
  {
    oldel = el.next;
    el.next = NULL;
    if(x!=centerx-l/2 || y!=centery-h/2)
    {
      x=centerx-l/2;
      y=centery-h/2;
      XMoveWindow(disp,w,x,y);
    }
    XMapRaised(disp, w);
    addto_el(this, w);
    ffl = 1;
    shown=1;
    if(need_key)
      bgkey->show();
    time(&t);
    lasttime=t;
    lastsize=0;
  }
}

void   InfoWin::set_message(char* msg)
{
  strcpy(message, msg);
  expose_clip_area();
}

void   InfoWin::hide()
{
  if(shown)
  {
    if(need_key)
      bgkey->hide();
    delall_el();
    delfrom_exp(this);
    el.next = oldel;
    XUnmapWindow(disp, w);
    shown=fullcoun=0;
  }
}

void   InfoWin::guifunc(Gui * o, int nu,...)
{
  bgbit=1;
}

//Global InfoWin object. Listers fcopy/fmove/fdelete make calls to it.
InfoWin *infowin=NULL;

void   create_infowin(char *head)
{
  if (infowin == NULL)
  {
    infowin = guiplugin->new_InfoWin(10, 50, 290, 85, head, 5);
    infowin->init(Main);
    infowin->show();
    infowin->expose();
    XFlush(disp);
  }
}

void   create_ftp_infowin(char *head)
{
  if (infowin == NULL)
  {
    infowin = guiplugin->new_InfoWin(10, 50, 290, 85, head, 5, 1);
    infowin->set_iname("INFOWINK");
    infowin->init(Main);
    infowin->show();
    infowin->expose();
    XFlush(disp);
  }
}

void   del_infowin()
{
  if (infowin == NULL)
    return;
  infowin->hide();
  delete infowin;
  XFlush(disp);
  infowin = NULL;
  if (panel)
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   to_infowin(char *str)
{
  if (infowin == NULL)
    return;
  infowin->set_message(str);
  XFlush(disp);
}

void   to_infowin_lastused()
{
  if (infowin == NULL)
    return;
  infowin->expose_clip_area();
  XFlush(disp);
}

