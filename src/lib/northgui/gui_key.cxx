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
#include "gui_key.h"
#include "iconman.h"
#include "iconsets.h"

///////////////////////////KEY class/////////////////////////////////////
// *INDENT-OFF*        
KEY::KEY(int ix, int iy, int il, int ih, char *iname, ulong icol, void
           (*ifunc) ()):Gui()
// *INDENT-ON*        
{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  col = icol;
  func = ifunc;
  prflg = 0;
  name = iname;
  strncpy(in_name, name, 20);
  in_name[20]=0;
  if (h < 20)
    h = 20;
  foc = 0;
  hflg = 0;
  dflg = 1;
  escfunc = NULL;
  guitype = GUI_KEY;
  guiobj = NULL;
  skin = NULL;
  highlighted=0;
}

void   KEY::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    tw, itemp;
  GEOM_TBL *tbl;
  parent = ipar;
  name=_(name);
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  tl = strlen(name);
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  if(tbl)
      skin=(Sprite*)tbl->data1;
  else
      skin=NULL;
  tw = XTextWidth(fontstr, _(name), tl);

  if (l < tw + 10)
    l = tw + 10;

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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, skin ? 0 : 0, 
			  0, normal_bg_color);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = normal_bg_color;
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | LeaveWindowMask |
    EnterWindowMask | OwnerGrabButtonMask | KeyPressMask | FocusChangeMask);
  tx = (l - tw) / 2;
  ty = (h + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
}

void   KEY::expose()
{
  if(skin)
  {
    XCopyArea(disp, skin[0].im->skin, w, gcw, skin[0].x, skin[0].y,
         (unsigned)skin[0].l, (unsigned)skin[0].h, 0, 0);
    XSetForeground(disp, gcw, cols[1]);
    XDrawString(disp, w, gcw, tx, ty, name, tl);
  } else
  {
      if(highlighted)
	  XSetForeground(disp, gcw, lighter_bg_color);
      else
	  XSetForeground(disp, gcw, normal_bg_color);
      XFillRectangle(disp, w, gcw, 0, 0, l, h);
      XSetForeground(disp, gcw, light_bg_color);
      XDrawLine(disp, w, gcw, 0, 0, l-1, 0);
      XDrawLine(disp, w, gcw, 0, 0, 0, h-1);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
          XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
      }
      XSetForeground(disp, gcw, dark_bg_color);
      XDrawLine(disp, w, gcw, 1, h - 1, l, h - 1);
      XDrawLine(disp, w, gcw, l - 1, 1, l - 1, h);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
          XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
      }
      if(shadow)
      {
	  XSetForeground(disp, gcw, shadow_color);
	  XDrawString(disp, w, gcw, tx + 1, ty + 1, name, tl);
      }
      XSetForeground(disp, gcw, cols[col]);
      XDrawString(disp, w, gcw, tx, ty, name, tl);
      if (foc)
	  XSetWindowBorderWidth(disp, w, 1);
      else
        XSetWindowBorderWidth(disp, w, 0);
  }
  hflg = 1;
}

void   KEY::press()
{
  if(skin)
  {
    XCopyArea(disp, skin[1].im->skin, w, gcw, skin[1].x, skin[1].y,
         (unsigned)skin[1].l, (unsigned)skin[1].h, 0, 0);
    XSetForeground(disp, gcw, cols[5]);
    XDrawString(disp, w, gcw, tx, ty, name, tl);
  } else
  {
      if(highlighted)
	  XSetForeground(disp, gcw, darker_bg_color);
      else
	  XSetForeground(disp, gcw, normal_bg_color);
      XFillRectangle(disp, w, gcw, 0, 0, l, h);
      XSetForeground(disp, gcw, dark_bg_color);
      XDrawLine(disp, w, gcw, 0, 0, l-1, 0);
      XDrawLine(disp, w, gcw, 0, 0, 0, h-1);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
          XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
      }
      XSetForeground(disp, gcw, light_bg_color);
      XDrawLine(disp, w, gcw, 1, h - 1, l, h - 1);
      XDrawLine(disp, w, gcw, l - 1, 1, l - 1, h);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
          XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
      }
      if(shadow)
      {
	  XSetForeground(disp, gcw, shadow_color);
	  XDrawString(disp, w, gcw, tx + 3, ty + 3, name, tl);
      }
      XSetForeground(disp, gcw, cols[col]);
      XDrawString(disp, w, gcw, tx + 1, ty + 1, name, tl);
  }
  if (foc == 0)
    guiSetInputFocus(disp, w, RevertToParent, CurrentTime);
}

void   KEY::set_focus()
{
  if (foc == 0)
  {
    foc = 1;
    if(!skin)
      XSetWindowBorderWidth(disp, w, 1);
    focobj = this;
  }
}

void   KEY::unset_focus()
{
  if (foc)
  {
    foc = 0;
    if(!skin)
      XSetWindowBorderWidth(disp, w, 0);
    focobj = NULL;
  }
}

void   KEY::click()
{
  KeySym ks;
  if (dflg == 0)
    return;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case FocusIn:
	  set_focus();
          break;
        case FocusOut:
	  unset_focus();
          break;
        case KeyPress:
          if (foc)
            {
              ks = XLookupKeysym(&ev.xkey, 0);
              switch (ks)
                {
                case XK_Execute:
                case XK_Escape:
                  if (escfunc)
                    escfunc();
                  break;
                case XK_Return:
                  press();
                  prflg = 1;
                  if (hflg)
                    expose();
                  if (func)
                    {
                      func();
                      return;
                    };
                  if (guiobj)
                    guiobj->guifunc(this, 0);
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
                };
            };
          break;
        case ButtonPress:
          if (ev.xbutton.button == Button1)
            {
              guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
              press();
              prflg = 1;
            }
          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1 && prflg == 1)
            {
              prflg = 0;
              expose();
              if (func)
                {
                  func();
                  return;
                };
              if (guiobj)
                guiobj->guifunc(this, 0);
            }
          break;
        case LeaveNotify:
          if (prflg)
            {
              prflg = 0;
            }
	  highlighted=0;
	  expose();
          break;
        case EnterNotify:
	    highlighted=1;
	    expose();
	    /*          if (prflg == 0 && ev.xcrossing.state == Button1Mask)
			{
			prflg = 1;
			press();
			};*/
	    break;
        };
    }
}

void KEY::move_window(int newx, int newy)
{
    x=newx;
    y=newy;
    XMoveWindow(disp, w, x, y);
}

///////////////////////////IconKey class/////////////////////////////////////
// *INDENT-OFF*        
IconKey::IconKey(int ix, int iy, int il, int ih, int iicon_idx,
		 void (*ifunc) ()):KEY(ix,iy,il,ih,"ICONKEY",0,ifunc)
// *INDENT-ON*        
{
  l = il;
  h = ih;
  icon_idx=iicon_idx;
  iconset_idx=menu_iconset;
}

void   IconKey::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    tw, itemp;
  GEOM_TBL *tbl;
  parent = ipar;
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  if(tbl)
      skin=(Sprite*)tbl->data1;
  else
      skin=NULL;
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, skin ? 0 : 0, 
			  0, normal_bg_color);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = normal_bg_color;
  gcw = XCreateGC(disp, w, GCBackground, &gcv);
  XSelectInput(disp, w,
      ButtonPressMask | ExposureMask | ButtonReleaseMask | LeaveWindowMask |
    EnterWindowMask | OwnerGrabButtonMask | KeyPressMask 
	       //| FocusChangeMask
	       );
}

void   IconKey::expose()
{
  if(skin)
  {
    XCopyArea(disp, skin[0].im->skin, w, gcw, skin[0].x, skin[0].y,
         (unsigned)skin[0].l, (unsigned)skin[0].h, 0, 0);
  } else
  {
      if(highlighted)
	  XSetForeground(disp, gcw, lighter_bg_color);
      else
	  XSetForeground(disp, gcw, normal_bg_color);
      XFillRectangle(disp, w, gcw, 0, 0, l, h);
      XSetForeground(disp, gcw, light_bg_color);
      XDrawLine(disp, w, gcw, 0, 0, l-1, 0);
      XDrawLine(disp, w, gcw, 0, 0, 0, h-1);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
          XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
      }
      XSetForeground(disp, gcw, dark_bg_color);
      XDrawLine(disp, w, gcw, 1, h - 1, l, h - 1);
      XDrawLine(disp, w, gcw, l - 1, 1, l - 1, h);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
          XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
      }
      if (foc)
	  XSetWindowBorderWidth(disp, w, 1);
      else
        XSetWindowBorderWidth(disp, w, 0);
  }
  if(highlighted)
      default_iconman->display_icon_from_set(w,2,2+h/2,iconset_idx, icon_idx);
  else
      default_iconman->display_icon_from_set_with_shadow(w,2,2+h/2,iconset_idx, icon_idx);
  hflg = 1;
}

void   IconKey::press()
{
  if(skin)
  {
    XCopyArea(disp, skin[1].im->skin, w, gcw, skin[1].x, skin[1].y,
         (unsigned)skin[1].l, (unsigned)skin[1].h, 0, 0);
  } else
  {
      if(highlighted)
	  XSetForeground(disp, gcw, darker_bg_color);
      else
	  XSetForeground(disp, gcw, normal_bg_color);
      XFillRectangle(disp, w, gcw, 0, 0, l, h);
      XSetForeground(disp, gcw, dark_bg_color);
      XDrawLine(disp, w, gcw, 0, 0, l-1, 0);
      XDrawLine(disp, w, gcw, 0, 0, 0, h-1);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
          XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
      }
      XSetForeground(disp, gcw, light_bg_color);
      XDrawLine(disp, w, gcw, 1, h - 1, l, h - 1);
      XDrawLine(disp, w, gcw, l - 1, 1, l - 1, h);
      if(gui_options & THICK_BORDER)
      {
          XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
          XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
      }
  }
  default_iconman->display_icon_from_set(w,2,2+h/2,iconset_idx, icon_idx);
  /*  if (foc == 0)
    guiSetInputFocus(disp, w, RevertToParent, CurrentTime);
  */
}

////////////////////////////////End of file/////////////////////////////////////////


