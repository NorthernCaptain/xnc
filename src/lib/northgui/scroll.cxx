/****************************************************************************
*  Copyright (C) 1996-97 by Leo Khramov
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
#include "scroll.h"
#include "up.xbm"
#include "down.xbm"
#include "left.xbm"
#include "right.xbm"
#include "baseguiplugin.h"

#define SCROLLDELAY 25
#define SCROLLBAR_WIDTH 13

void   prect(Window w, GC & gcw, int x, int y, int l, int h);
void   urect(Window w, GC & gcw, int x, int y, int l, int h);
Pixmap pup, pdown, pleft, pright, pcenter;

void   init_scroll()
{
  pup = XCreatePixmapFromBitmapData(disp, Main, (char *)up_bits, up_width, up_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
  pdown = XCreatePixmapFromBitmapData(disp, Main, (char *)down_bits, down_width, down_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
  pleft = XCreatePixmapFromBitmapData(disp, Main, (char *)left_bits, left_width, left_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
  pright = XCreatePixmapFromBitmapData(disp, Main, (char *)right_bits, right_width, right_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
/* pcenter=XCreatePixmapFromBitmapData(disp,Main,(char*)center_bits,center_width,center_height,
   cols[0],keyscol[1],DefaultDepth(disp,DefaultScreen(disp))); */
}

void   deinit_scroll()
{
  XFreePixmap(disp, pup);
  XFreePixmap(disp, pdown);
  XFreePixmap(disp, pleft);
  XFreePixmap(disp, pright);
// XFreePixmap(disp,pcenter);
}
// *INDENT-OFF*        
BKey::BKey(int ix, int iy, int il, int ih, Gui * io, int f):Gui()
// *INDENT-ON*        

{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  o = io;
  prflg = 0;
  ff = f;
  hflg = 0;
  guitype = GUI_BKEY;
}

void   BKey::setpixmap(Pixmap p, int il, int ih)
{
  pix = p;
  pl = il;
  ph = ih;
}

void   BKey::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
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
  gcv.foreground = keyscol[0];
  gcw = XCreateGC(disp, w, GCBackground | GCForeground, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | LeaveWindowMask |
               EnterWindowMask);        // | OwnerGrabButtonMask);

}

void   BKey::expose()
{
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  XCopyArea(disp, pix, w, gcw, 0, 0, pl, ph, l / 2 - pl / 2, h / 2 - ph / 2);
  hflg = 1;
}

void   BKey::press()
{
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  XCopyArea(disp, pix, w, gcw, 0, 0, pl, ph, l / 2 - pl / 2, h / 2 - ph / 2);
}

void   BKey::click()
{
  Window rw, cw;
  int    rwx;
  uint   mask;
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
              if (o)
                if (ff)
                  o->scrollup(this);
                else
                  o->scrolldown(this);
              if (guiobj)
                guiobj->guifunc(this, 0);
              XFlush(disp);
              prflg = 1;
              delay(SCROLLDELAY * 5);
              while (XQueryPointer(disp, w, &rw, &cw, &rwx, &rwx, &rwx, &rwx, &mask))
                {
                  if (!(mask & Button1Mask))
                    break;        /* button released */
                  if (o)
                    if (ff)
                      o->scrollup(this);
                    else
                      o->scrolldown(this);
                  if (guiobj)
                    guiobj->guifunc(this, 0);
                  XFlush(disp);
                  delay(SCROLLDELAY);
                }
            }
          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1 && prflg == 1)
            {
              prflg = 0;
              expose();
//   if(o) if(ff) o->scrollup(this); else o->scrolldown(this);
            }
          break;
        case LeaveNotify:
          if (prflg)
            {
              prflg = 0;
              expose();
            }
          break;
        case EnterNotify:
          if (prflg == 0 && ev.xcrossing.state == Button1Mask)
            {
              prflg = 1;
              press();
            };
          break;
        };
    }
}
// *INDENT-OFF*        
ScrollBar::ScrollBar(int ix, int iy, int ih, Gui * io):Gui()
// *INDENT-ON*        

{
  x = ix;
  y = iy;
  l = SCROLLBAR_WIDTH;
  h = ih;
  o = io;
  prflg = 0;
  hflg = 0;
  scrup = scrdown = scr = scring = NULL;
  pages = 1;
  pagesize = 1;
  guitype=GUI_SCROLL;
  shown=0;
}

void   ScrollBar::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
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
  h -= SCROLLBAR_WIDTH*2;
  y += SCROLLBAR_WIDTH;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, darker_bg_color);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = keyscol[1];
  gcv.foreground = keyscol[0];
  gcw = XCreateGC(disp, w, GCBackground | GCForeground, &gcv);
  XSelectInput(disp, w, PointerMotionHintMask |
    ButtonPressMask | ExposureMask | ButtonReleaseMask | Button1MotionMask);
  upk = baseguiplugin->new_BKey(x, y - l, l, l, this, 1);
  downk = baseguiplugin->new_BKey(x, y + h + 2, l, l, this, 0);
  upk->init(parent);
  downk->init(parent);
  upk->setpixmap(pup, up_width, up_height);
  downk->setpixmap(pdown, down_width, down_height);
  val = minval;
}

void   ScrollBar::setpages(int i)
{
  pages = i;
  if (pages == 0)
    pages = 1;
  pagesize = range / pages;
}

void   ScrollBar::setrange(int imin, int imax)
{
  minval = imin;
  maxval = imax;
  range = maxval - minval;
  pagesize = range / pages;
  if (range == 0)
    range = 1;
}

void   ScrollBar::reconfigure(int ix, int iy, int ih)
{
  y = iy + l;
  x = ix - l;
  h = ih - 34;
  XMoveWindow(disp, w, x, y);
  XMoveWindow(disp, upk->w, x, y - l);
  XMoveWindow(disp, downk->w, x, y + h + 2);
  XResizeWindow(disp, w, l, h);
  expose();
}

void   ScrollBar::show()
{
// *INDENT-OFF*        
  Gui::show();
// *INDENT-ON*        

  upk->show();
  downk->show();
  shown=1;
}

void   ScrollBar::hide()
{
  upk->hide();
  downk->hide();
// *INDENT-OFF*        
  Gui::hide();
// *INDENT-ON*        
  shown=0;

}

void   ScrollBar::expose()
{
  int    vy, vl;
  if(!shown)
      return;
  vl = (h - 9) / pages;
  if (vl == 0)
    vl = 1;
  if (range == 0)
    range = 1;
  if (val > maxval)
    val = maxval;
  else if (val < minval)
    val = minval;
  if (range != 0)
    vy = (val - minval) * (h - 8 - vl) / range + 4;
  else
    vy = 4;
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, normal_bg_color);
  XFillRectangle(disp, w, gcw, 0, vy, l-1, vl);
  if (prflg == 0)
    {
	urect(w, gcw, 0, vy, l-1, vl);
	if(gui_options & THICK_BORDER)
	    urect(w, gcw, 1, vy + 1, l-3, vl - 2);
    }
  else
    {
      prect(w, gcw, 0, vy, l-1, vl);
      if(gui_options & THICK_BORDER)
          prect(w, gcw, 1, vy + 1, l-3, vl - 2);
    }
  urect(w, gcw, 0, 0, l-1, 3);
  if(gui_options & THICK_BORDER)
      urect(w, gcw, 0, h - 4, l-1, 3);
  hflg = 1;
}

void   ScrollBar::scrollup(Gui *)
{
  if (val > minval)
    {
      val--;
      expose();
      if (scrup)
        scrup(this);
      if (o)
        o->scrollup(this);
    }
}

void   ScrollBar::scrolldown(Gui *)
{
  if (val < maxval)
    {
      val++;
      expose();
      if (scrdown)
        scrdown(this);
      if (o)
        o->scrolldown(this);
    }
}

void   ScrollBar::click()
{
  int    my, ival;
  uint   mask;
  Window rw, cw;
  int    rwx, rwy, step, dh;
  int    vl = (h - 9) / pages;
  if (vl == 0)
    vl = 1;
  int    vy = (val - minval) * (h - 8 - vl) / range + 4;
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
              my = ev.xbutton.y;
              if (my < vy)
                {
                  val -= pagesize;
                  if (val < 0)
                    val = 0;
                  expose();
                  if (scr)
                    scr(this);
                  if (o)
                    o->scroll(this);
                  step = -pagesize;
                }
              else if (my > vy + vl)
                {
                  val += pagesize;
                  if (val > maxval)
                    val = maxval;
                  expose();
                  if (scr)
                    scr(this);
                  if (o)
                    o->scroll(this);
                  step = pagesize;
                }
              else
                step = 0;
              if (step)
                {
                  XFlush(disp);
                  delay(SCROLLDELAY * 6);
                  while (XQueryPointer(disp, w, &rw, &cw, &rwx, &rwx, &rwx, &rwx, &mask))
                    {
                      if (!(mask & Button1Mask))
                        break;        /* button released */
                      if (val == 0 && step < 0)
                        continue;
                      if (val == maxval && step > 0)
                        continue;
                      val += step;
                      if (val < 0)
                        val = 0;
                      else if (val > maxval)
                        val = maxval;
                      expose();
                      if (scr)
                        scr(this);
                      if (o)
                        o->scroll(this);
                      XFlush(disp);
                      delay(SCROLLDELAY * 3);
                    }
                }
              else
                {
                  prflg = 1;
                  bmy = my - vy;
                  expose();
                };

            };
          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1 && prflg == 1)
            {
              prflg = 0;
              expose();
              if (scr)
                scr(this);
              if (o)
                o->scroll(this);
            }
          break;
        case MotionNotify:
          if (prflg == 0)
            break;
          my = ev.xmotion.y;
          while (XPending(disp))
            {
              XPeekEvent(disp, &ev);
              if (ev.type == MotionNotify)
                {
                  XNextEvent(disp, &ev);
                }
              else
                break;
            }
          XQueryPointer(disp, w, &rw, &cw, &rwx, &rwy, &rwx, &my, &mask);
          prflg = 1;
          my -= 4;
          my -= bmy;
          if (my < 0)
            ival = minval;
          else
            {
              dh = h - 9 - vl;
              if (dh > 0)
                ival = my * range / dh;
              else
                break;
              if (ival > maxval)
                ival = maxval;
            }
          if (ival != val)
            {
              val = ival;
              expose();
              if (scring != NULL)
                scring(this);
              if (o)
                o->scrolling(this);
            };
          break;
        };
    }
}
