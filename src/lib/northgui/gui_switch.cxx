/****************************************************************************
*  Copyright (C) 1996-2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
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
#include "xh.h"
#include "gui_switch.h"

#include "gl.xbm"
#include "baseguiplugin.h"

///////////////////////////Switch class/////////////////////////////////////

int Switch::button_width=12;

// *INDENT-OFF*        
Switch::Switch(int ix, int iy, int il, char *iname, ulong icol, void
              (*ifunc) ()):Gui()
// *INDENT-ON*        

{
  x = ix;
  y = iy;
  l = il;
  col = icol;
  func = ifunc;
  prflg = 0;
  name = iname;
  h = 20;
  foc = 0;
  hflg = 0;
  sw = 0;
  excl = 0;
  sp = NULL;
  guitype = GUI_SWITCH;
  disable=0;
  escfunc=NULL;
  highlighted=-1;
}

void   Switch::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    tw, itemp;
  parent = ipar;
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  tl = strlen(_(name));
  tw = XTextWidth(fontstr, _(name), tl);
  h = fontstr->max_bounds.ascent + fontstr->max_bounds.descent + 4;
  h += h % 2;
  if (l < tw + 20)
    l = tw + 20;
  geometry_by_iname();
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, normal_bg_color);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = normal_bg_color;
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | LeaveWindowMask |
    EnterWindowMask | OwnerGrabButtonMask | KeyPressMask | FocusChangeMask);
  tx = h + 12;
  ty = (h + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  gl = XCreateBitmapFromData(disp, w, (char *)gl_bits, gl_width, gl_height);
  XSetStipple(disp, gcw, gl);

}

void   Switch::expose()
{
  int    j = h / 2 - 1;
  int    delta_x1= (h - button_width)/2;
  int    delta_x2= delta_x1+button_width - 1;

  switch(highlighted)
  {
  case 1:
      XSetWindowBackground(disp, w, lighter_bg_color);
      break;
  case 0:
      XSetWindowBackground(disp, w, normal_bg_color);
      break;
  }
  highlighted=-1;
  
  XClearWindow(disp, w);
  
#if 0
  XSetForeground(disp, gcw, light_bg_color);
  
  XDrawLine(disp, w, gcw, 0, j, j, 0);
  XDrawLine(disp, w, gcw, 0, j, j, j + j);
  XSetForeground(disp, gcw, dark_bg_color);

  XDrawLine(disp, w, gcw, j, 0, j + j, j);
  XDrawLine(disp, w, gcw, j + j, j, j, j + j);

#else

  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, delta_x2, delta_x2, delta_x2, delta_x1);
  XDrawLine(disp, w, gcw, delta_x1, delta_x2, delta_x2, delta_x2);
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, delta_x1, delta_x1, delta_x1, delta_x2);
  XDrawLine(disp, w, gcw, delta_x1, delta_x1, delta_x2, delta_x1);

#endif
  
  if(shadow)
  {
      XSetForeground(disp, gcw, shadow_color);
      XDrawString(disp, w, gcw, tx + 1, ty + 1, _(name), tl);
  }
  XSetForeground(disp, gcw, disable ? normal_bg_color : cols[col]);
  XDrawString(disp, w, gcw, tx, ty, _(name), tl);
  sw &= 1;
  if (sw)
  {
      XSetForeground(disp, gcw, window_text_color);
      XSetFillStyle(disp, gcw, FillStippled);
      XFillRectangle(disp, w, gcw, 0, 0, 32, 32);
      XSetFillStyle(disp, gcw, FillSolid);
  }
  select();
  hflg = 1;
}

void   Switch::select()
{
  if (foc)
    {
      XSetForeground(disp, gcw, dark_bg_color);
      XDrawRectangle(disp, w, gcw, h + 4, 1, l - h - 8, h - 3);
    }
  else
    {
      XSetForeground(disp, gcw, normal_bg_color);
      XDrawRectangle(disp, w, gcw, h + 4, 1, l - h - 8, h - 3);
    }
}

void   Switch::press()
{
  int    j = h / 2 - 1;
  int    delta_x1= (h - button_width)/2;
  int    delta_x2= delta_x1+button_width - 1;

  sw &= 1;
  XClearWindow(disp, w);

#if 0
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, 0, j, j, 0);
  XDrawLine(disp, w, gcw, 0, j, j, j + j);
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, j, 0, j + j, j);
  XDrawLine(disp, w, gcw, j + j, j, j, j + j);
#else


  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, delta_x2, delta_x2, delta_x2, delta_x1);
  XDrawLine(disp, w, gcw, delta_x1, delta_x2, delta_x2, delta_x2);
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, delta_x1, delta_x1, delta_x1, delta_x2);
  XDrawLine(disp, w, gcw, delta_x1, delta_x1, delta_x2, delta_x1);

#endif

  if(shadow)
  {
      XSetForeground(disp, gcw, shadow_color);
      XDrawString(disp, w, gcw, tx + 1, ty + 1, _(name), tl);
  }
  XSetForeground(disp, gcw, disable ? darker_bg_color : cols[col]);
  XDrawString(disp, w, gcw, tx, ty, _(name), tl);
  if (foc == 0)
    guiSetInputFocus(disp, w, RevertToParent, CurrentTime);
  if (sw)
    {
      XSetForeground(disp, gcw, shadow_color);
      XSetFillStyle(disp, gcw, FillStippled);
      XFillRectangle(disp, w, gcw, 0, 0, 32, 32);
      XSetFillStyle(disp, gcw, FillSolid);
    }
}

void   Switch::click()
{
  KeySym ks;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case FocusIn:
          if (foc == 0)
            {
              if(disable)
                {
                  if (next)
                    guiSetInputFocus(disp, next->w, RevertToParent, CurrentTime);
                  break;
                }
              foc = 1;
              select();
              focobj = this;
            };
          break;
        case FocusOut:
          if (foc)
            {
              foc = 0;
              select();
              focobj = NULL;
            };
          break;
        case KeyPress:
          if (foc)
            {
              ks = XLookupKeysym(&ev.xkey, 0);
              switch (ks)
                {
                case XK_Return:
                case XK_space:
                  press();
                  prflg = 1;
                  sw &= 1;
                  sw ^= 1;
                  if (guiobj)
                    guiobj->guifunc(this, sw);
                  if (excl && sp != NULL && sw == 1)
                    sp->reaction(this);
                  if (func && sw == 1)
                    func();
                  if (excl && sw == 0)
                    sw = 1;
                  if (hflg)
                    expose();
                  break;
                case XK_Right:
                case XK_Down:
                case XK_Tab:
                  if (next)
                    guiSetInputFocus(disp, next->w, RevertToParent, CurrentTime);
                  break;
                case XK_Left:
                case XK_Up:
                  if (prev)
                    guiSetInputFocus(disp, prev->w, RevertToParent, CurrentTime);
                  break;
                case XK_Escape:
                        if(escfunc)
                                escfunc();
                        break;
                };
            };
          break;
        case ButtonPress:
          if(disable)
                  break;
          guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
          if (ev.xbutton.button == Button1) // && ev.xbutton.x < h)
            {
              press();
              prflg = 1;
            }
          break;
        case ButtonRelease:
          if(disable)
                  break;
          if (ev.xbutton.button == Button1 && prflg == 1)
            {
              prflg = 0;
              sw &= 1;
              sw ^= 1;
              if (guiobj)
                guiobj->guifunc(this, sw);
              if (excl && sw == 0)
                {
                  sw = 1;
                  expose();
                  break;
                };
              expose();
              if (excl && sp != NULL && sw == 1)
                sp->reaction(this);
              if (func && sw == 1)
                func();
            }
          break;
        case LeaveNotify:
          if(disable)
	      break;
	  highlighted=0;
          if (prflg)
              prflg = 0;
	  expose();
          break;
        case EnterNotify:
          if(disable)
	      break;
	  highlighted=1;
	  expose();
          break;
        };
    }
}
//////////////////////////////Sw_panel Class/////////////////////////////
// *INDENT-OFF*        
Sw_panel::Sw_panel(int ix, int iy, int il, char *ihead, MenuItem * mn, int imax, int icolumn):Gui()
// *INDENT-ON*        
{
  columnl = il;
  columns = icolumn;
  l = columnl * columns;
  x = ix;
  y = iy;
  max = imax;
  head = ihead;
  ar = new Switch *[max];
  smn = mn;
  shown = initfl = 0;
  guitype = GUI_SWPANEL;
}
// *INDENT-OFF*        
Sw_panel::~Sw_panel()
// *INDENT-ON*        

{
  if (initfl)
    {
      for (int i = 0; i < max; i++)
        delete ar[i];
      delete ar;
      if(disp)
          XFreeGC(disp, gcw);
    }
}

void   Sw_panel::set_escapefunc(void (*esc)())
{
        int i;
        for(i=0;i<max;i++)
                ar[i]->set_escapefunc(esc);
}

void   Sw_panel::init(Window ipar)
{
  int    i, cnumy, cnumx;
  int    cn = (max + columns - 1) / columns;
  parent = w = ipar;
  geometry_by_iname();
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fontstr->fid);
  ty = fontstr->max_bounds.ascent + fontstr->max_bounds.descent;
  head=_(head);
  for (i = 0; i < max; i++)
    {
      cnumy = i / cn;
      cnumx = i % cn;
      ar[i] = baseguiplugin->new_Switch(x + 5 + cnumy * columnl, y + ty + 5 + cnumx * 25, 
					columnl - 10, smn[i].name, smn[i].col, smn[i].func);
      ar[i]->init(w);
      ar[i]->setpanel(this);
    }
  tx = strlen(head);
  h = 5 + ty + cn * 25;
  initfl = 1;
}

void   Sw_panel::set(int n, int s)
{
  if (n < max)
    {
      if (ar[n]->excl)
        for (int i = 0; i < max; i++)
          ar[i]->sw = 0;
      ar[n]->sw = s;
    }
}

void   Sw_panel::excl()
{
  for (int i = 0; i < max; i++)
    ar[i]->setexcl();
}

void   Sw_panel::reaction(Switch * s)
{
  for (int i = 0; i < max; i++)
    if (ar[i] != s && ar[i]->sw == 1)
      {
        ar[i]->sw = 0;
        ar[i]->expose();
      }
}

void   Sw_panel::show()
{
  int    i;
  expose();
  if (shown == 0)
    for (i = 0; i < max; i++)
      ar[i]->show();
  shown = 1;
}

void   Sw_panel::link(Gui * o)
{
  int    i;
  for (i = 0; i < max - 1; i++)
    ar[i]->link(ar[i + 1]);
  ar[i]->link(o);
// next=o;
}

void   Sw_panel::hide()
{
  for (int i = 0; i < max; i++)
    {
      ar[i]->hide();
//  delete ar[i];
    }
  shown = 0;
}

void   Sw_panel::expose()
{
    int text_center_y=y+(fontstr->max_bounds.ascent +
			 fontstr->max_bounds.descent)/2;
    XClearArea(disp, w, x, y, l, h, 0);
    XSetForeground(disp, gcw, dark_bg_color);
    XDrawLine(disp, w, gcw, x+1, text_center_y, x+3, text_center_y);
    XDrawLine(disp, w, gcw, x, text_center_y+1, x, y + h - 2);
    XDrawLine(disp, w, gcw, x+l-1, text_center_y+1, 
	      x+l-1, y + h - 2);
    XDrawLine(disp, w, gcw, x+l-2, text_center_y, 
	      x+7+XTextWidth(fontstr, head, tx), text_center_y);

    XDrawLine(disp, w, gcw, x+1, y+h-1, x+l-2, y+h-1);

    if(shadow)
    {
	XSetForeground(disp, gcw, shadow_color);
	XDrawString(disp, w, gcw, x + 6, y + 1 + fontstr->max_bounds.ascent, head, tx);
    }
    XSetForeground(disp, gcw, cols[0]);
    XDrawString(disp, w, gcw, x + 5, y + fontstr->max_bounds.ascent, head, tx);
    if (next && next->w == w)
	next->expose();
}

void   Sw_panel::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}


int   Sw_panel::set_selffocus()
{
    if(ar[0])
	return ar[0]->set_selffocus();
    return 0;
}
//---------------------------------- End of file ------------------------------------
