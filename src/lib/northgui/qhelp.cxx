
/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
*  email:   leo@pop.convey.ru
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
#include "qhelp.h"
#include "xh.h"

extern int disable_reread;        //If 1 reread disables in Panel::expose;
// *INDENT-OFF*        
QuickHelp::QuickHelp():Gui()
// *INDENT-ON*        

{
  x = -1;
  y = 20;
  l = 50;
  h = 18;
  foc = 0;
  guitype = GUI_QHELP;
  guiobj = NULL;
  shown = mapped = 0;
}

void   QuickHelp::init(Window ipar)
{
  Window wtemp;
  XGCValues gcv;
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, 0, 
			  WhitePixel(disp, DefaultScreen(disp)));
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = light_bg_color;
  gcv.foreground = window_text_color;
  gcv.font = mfixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont | GCForeground, &gcv);
  XSelectInput(disp, w,
               ExposureMask);
  ty = mfixfontstr->max_bounds.ascent + mfixfontstr->max_bounds.descent;
  ty2 = mfixfontstr->max_bounds.ascent + 1;
  if (ty < 0)
    ty = -ty;
  h = ty + 2;
}

void   QuickHelp::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}

void   QuickHelp::show_this_info(char *head, char *text, int ix, int iy)
{
  if (head)
    sprintf(name, "%s:%s", head, text);
  else
    strcpy(name, text);
  nl = strlen(name);
  l = XTextWidth(mfixfontstr, name, nl) + 6;
  if (ix < 0)
    ix = -ix - l;
  x = ix;
  if (iy < 0)
    iy = -iy - h;
  y = iy;
  XMoveResizeWindow(disp, w, x, y, l, h);
  if (shown)
    expose();
  disable_reread = 2;
}

void   QuickHelp::expose()
{
  XClearWindow(disp, w);
  XDrawString(disp, w, gcw, 3, ty2, name, nl);
}

void   QuickHelp::show()
{
  if (shown)
    XRaiseWindow(disp, w);
  else
    {
      XMapRaised(disp, w);
      addto_el(this, w);
      shown = 1;
    }
}

void   QuickHelp::hide()
{
  if (shown)
    {
// *INDENT-OFF*        
      Gui::hide();
// *INDENT-ON*        

      shown = 0;
      disable_reread = 2;
    }
}
