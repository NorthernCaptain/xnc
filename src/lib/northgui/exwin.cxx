/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
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
#include "exwin.h"
///////////////////////////////////EXWin class///////////////////////////////
Atom wmDelWin;

void   EXWin::init(Window ipar)
{
  parent = DefaultRootWindow(disp);
  w = create_win(name, x, y, l, h, ExposureMask | StructureNotifyMask, 0);
  gcv.background = keyscol[1];
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  ty = (21 + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  prflg = 0;
  wmDelWin = XInternAtom(disp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(disp, w, &wmDelWin, 1);
  ffl=0;
}

void   EXWin::expose()
{
  if (func)
    func(w, gcw);
}

void   EXWin::click()
{
  if (ev.xany.window == w)
    {
      switch (ev.type)
      {
        case Expose:
              if (func != NULL)
                func(w, gcw);
              break;
        case ClientMessage:
              if(ev.xclient.data.l[0]==wmDelWin
                && ev.xclient.format==32 && killfunc!=NULL)
                        killfunc();
              break;
        case ConfigureNotify:
              if(ev.xconfigure.x!=0 && ev.xconfigure.y!=21) //Workaround for wmaker bug
              {
                  x=ev.xconfigure.x;
                  y=ev.xconfigure.y;
              } else
                  fprintf(stderr, "Debug: WindowMaker still have ConfigureNotify bug...\n");
              break;
      }
    }
}

void   EXWin::show()
{
  if(main_pixmap)
      XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  XMapRaised(disp, w);
  addto_el(this, w);
  addto_exp(this, w);
  ffl = 1;
}

void   EXWin::hide()
{
  delfrom_exp(this);
  delfrom_el(this);
  XUnmapWindow(disp, w);
}

///////////////////////////////////////////End of file/////////////////////////////////////////////
