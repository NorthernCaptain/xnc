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
#include "gui_text.h"

////////////////////////////////Text class///////////////////////////////
void   Text::show()
{
  XClearArea(disp, w, x, y-fixfontstr->max_bounds.ascent, l, h+1, 0);
  tl = strlen(_(name));
  l = XTextWidth(fixfontstr, _(name), tl);
  if(shadow)
  {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, x + 1, y + 1, _(name), tl);
  }
  XSetForeground(disp, gcw, cols[col]);
  XDrawString(disp, w, gcw, x, y, _(name), tl);
}

void   Text::clear_text_area(int il)
{
  XClearArea(disp, w, x, y-fixfontstr->max_bounds.ascent, il, h+1, 0);
}

void   Text::hide()
{
  foc = 0;
}

void   Text::init(Window par)
{
  parent = par;
  w = par;
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fixfontstr->fid);
  recalc();
}

void   Text::recalc()
{
  tl = strlen(_(name));
  h = fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent;
  l = XTextWidth(fixfontstr, _(name), tl);
}

void   Text::expose()
{
  XSetForeground(disp, gcw, keyscol[1]);
  XClearArea(disp, w, x, y-fixfontstr->max_bounds.ascent, l, h, 0);
  tl = strlen(_(name));
  l = XTextWidth(fixfontstr, _(name), tl);
  if(shadow)
  {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, x + 1, y + 1, _(name), tl);
  }
  XSetForeground(disp, gcw, cols[col]);
  XDrawString(disp, w, gcw, x, y, _(name), tl);
}

void   Text::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}

int   Text::set_selffocus()
{
    if(next)
	return next->set_selffocus();
    return 0;
}

////////////////////////////////wText class///////////////////////////////
void   WText::show()
{
// *INDENT-OFF*        
  Gui::show();
// *INDENT-ON*        
  XMapWindow(disp, w);
}

void   WText::hide()
{
  foc = 0;
// *INDENT-OFF*        
  Gui::hide();
// *INDENT-ON*        
}

void   WText::init(Window par)
{
  parent = par;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, 0, keyscol[1]);
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fontstr->fid);
  XSelectInput(disp, w,
               ExposureMask);
  recalc();
}

void   WText::recalc()
{
  tl = strlen(name);
  h = fontstr->max_bounds.ascent - fontstr->max_bounds.descent;
  ty = fontstr->max_bounds.ascent;
  tx = XTextWidth(fontstr, name, tl);
}

void   WText::expose()
{
  XClearWindow(disp, w);
  tl = strlen(name);
  tx = XTextWidth(fontstr, name, tl);
  if(shadow)
          XSetForeground(disp, gcw, cols[0]);
  if (centered)
    {
          if(shadow)
              XDrawString(disp, w, gcw, l / 2 - tx / 2 + 1, ty + 1, name, tl);
      XSetForeground(disp, gcw, cols[col]);
      XDrawString(disp, w, gcw, l / 2 - tx / 2, ty, name, tl);
    }
  else
    {
          if(shadow)
              XDrawString(disp, w, gcw, 3, ty + 1, name, tl);
      XSetForeground(disp, gcw, cols[col]);
      XDrawString(disp, w, gcw, 2, ty, name, tl);
    }
}

void   WText::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}

////////////////////////////////End of file/////////////////////////////////////////

