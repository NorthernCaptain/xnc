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
#include "gui_win.h"

///////////////////////////////////Win class///////////////////////////////
void   Win::init(Window ipar)
{
  int    tw;
  GEOM_TBL *tbl;
  Sprite *skin;
  parent = ipar;
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  if(tbl)
  {
   skin=(Sprite*)tbl->data1;
   ra_tbl=(RecArea*)tbl->data2;
  }
  else
   skin=NULL;
  if(skin)
  {
    spr_con1=&skin[3];  //Upper-left
    spr_con2=&skin[2];  //Upper-right
    spr_con3=&skin[1];  //Down-left
    spr_con4=&skin[0];  //Down-right
  }
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, keyscol[1]);
  gcv.background = keyscol[1];
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ExposureMask | ButtonPressMask | ButtonReleaseMask);
  tl = strlen(_(name));
  tw = XTextWidth(fontstr, name, tl);
  if (l < tw + 40)
    l = tw + 40;
  ty = (21 + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  prflg = 0;
}

void   Win::expose()
{
  int twl,rax,ray,i;
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  //  XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
  //  XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
  XDrawLine(disp, w, gcw, 1, 21, l - 1, 21);
  XDrawLine(disp, w, gcw, 21, 2, 21, 20);
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  //  XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
  //  XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
  XDrawLine(disp, w, gcw, 2, 20, l - 2, 20);
  XDrawLine(disp, w, gcw, 20, 2, 20, 20);
  if(shadow)
  {
      XSetForeground(disp, gcw, shadow_color);
      XDrawString(disp, w, gcw, 25, ty + 1, _(name), tl);
      XDrawString(disp, w, gcw, 6, ty + 2, "X", 1);
  }
  XSetForeground(disp, gcw, cols[col]);
  XDrawString(disp, w, gcw, 24, ty, _(name), tl);
  XSetForeground(disp, gcw, window_warn_color);
  XDrawString(disp, w, gcw, 4, ty, "X", 1);
  XDrawString(disp, w, gcw, 5, ty + 1, "X", 1);

  post_expose();
}

void   Win::post_expose()
{
    Gui* o;
    
    if (ffl)
    {
	if (next)
	    next->set_selffocus();
	ffl = 0;
    }

    o=next;
    do
    {
	if(o)
	{
	    if(!o->is_selfdraw())
		o->expose();
	    o->set_mark();
	    o=o->next;
	}
    } while(o && !o->is_marked());

    o=next;
    do
    {
	if(o)
	{
	    o->unset_mark();
	    o=o->next;
	}
    } while(o && o->is_marked());

    o=guiexpose;
    do
    {
	if(o)
	{
	    o->expose();
	    o->set_mark();
	    o=o->next;
	}
    } while(o && !o->is_marked());

    o=guiexpose;
    do
    {
	if(o)
	{
	    o->unset_mark();
	    o=o->next;
	}
    } while(o && o->is_marked());
}

void   Win::press_close()
{
  if (ev.xbutton.x < 21 && ev.xbutton.y < 21)
  {
    XSetForeground(disp, gcw, light_bg_color);
    XDrawLine(disp, w, gcw, 1, 20, 20, 20);
    XDrawLine(disp, w, gcw, 20, 2, 20, 20);
    XSetForeground(disp, gcw, dark_bg_color);
    XDrawLine(disp, w, gcw, 2, 2, 20, 2);
    XDrawLine(disp, w, gcw, 2, 2, 2, 20);
    prflg = 1;
  }
}

void   Win::click()
{
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case ButtonPress:
          if (hflg)
	    press_close();
          break;
        case ButtonRelease:
          if (prflg)
	  {
	    prflg = 0;
	    if (escape_func)
	      escape_func();
	  };
          break;
        };
    }
}

void   Win::show()
{
  oldel = el.next;
  el.next = NULL;
  if(main_pixmap)
     XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  XMapRaised(disp, w);
  addto_el(this, w);
  ffl = 1;
}

void   Win::hide()
{
  delall_el();
  delfrom_exp(this);
  el.next = oldel;
  XUnmapWindow(disp, w);
}

void   Win::nonresizable()
{
  XSizeHints *xhint;
  long   lg;
  xhint = XAllocSizeHints();
  XGetWMNormalHints(disp, w, xhint, &lg);
  xhint->flags = (PMinSize | PMaxSize);
  xhint->min_width = l;
  xhint->min_height = h;
  xhint->max_width = l;
  xhint->max_height = h;
  XSetWMNormalHints(disp, w, xhint);
  XFree(xhint);
}

///////////////////////////////////SpecialWin class///////////////////////////////
void   SpecialWin::init(Window ipar)
{
  int    tw;
  GC     pgc;
  int    es1, es2;
  parent = ipar;
  l = 250;
  h = 160;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, 0, cols[0]);
  gcv.background = keyscol[1];
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ExposureMask);
  pm1 = XCreatePixmap(disp, Main, l, h, 1);
  pgc = XCreateGC(disp, pm1, 0, NULL);
  tw = h - 20;
  XSetForeground(disp, pgc, 0);
  XFillRectangle(disp, pm1, pgc, 0, 0, l, h);
  XSetForeground(disp, pgc, 1);
  XFillArc(disp, pm1, pgc, l / 2 - tw / 2, 0, tw, tw, 0, 360 * 64);
  XFillRectangle(disp, pm1, pgc, 0, tw - 5, l, 25);
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
  if (XShapeQueryExtension(disp, &es1, &es2))
    XShapeCombineMask(disp, w, ShapeBounding, 0, 0, pm1, ShapeSet);
  else
#endif
    fprintf(stderr, "XShape extensions not supported.\n");
  XFreeGC(disp, pgc);
  prflg = 0;
  tl = tw;
  al = 0;
  name = NULL;
}

void   SpecialWin::expose()
{
  int    ll = fixfontstr->max_bounds.rbearing - fixfontstr->min_bounds.lbearing;
  int    ln = (l - 10) / ll, tw;
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[2]);
  XFillArc(disp, w, gcw, l / 2 - tl / 2 + 10, 10, tl - 20, tl - 20, 0, 360 * 64);
  XFillRectangle(disp, w, gcw, 4, tl - 3, l - 8, 20);
  if (name)
    {
      XSetForeground(disp, gcw, cols[0]);
      tw = strlen(name);
      if (tw > ln)
        {
          name += (tw - ln);
          tw -= ln;
        };
      tw = XTextWidth(fixfontstr, name, tw) / 2;
      XDrawString(disp, w, gcw, l / 2 - tw, tl + 12, name, strlen(name));
    }
  XSetForeground(disp, gcw, cols[2]);
  XDrawLine(disp, w, gcw, l / 2, tl / 2 - 1, l / 2, 24);
  if (al)
    {
      al--;
      nextpiece();
    };
}

void   SpecialWin::initroute(FList * o, int bs)
{
  nn = (o->size + bs - 1) / bs * 2;
  name = o->name;
  al = 0;
  expose();
  XFlush(disp);
}

void   SpecialWin::nextpiece()
{
  int    con1;
  al++;
  con1 = (23040 * al / nn);
  XSetForeground(disp, gcw, cols[2]);
  XFillArc(disp, w, gcw, l / 2 - tl / 2 + 20, 20, tl - 40, tl - 40, 5760 - con1, con1);
  XFlush(disp);
}

void   SpecialWin::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}

void   SpecialWin::show()
{
  XMapWindow(disp, w);
  addto_exp(this, w);
}

void   SpecialWin::hide()
{
  XUnmapWindow(disp, w);
  XDestroyWindow(disp, w);
  delfrom_exp(this);
}

//-------------------------------------------------------------------------
// Constructor for Sprited Window
//-------------------------------------------------------------------------
SpriteWin::SpriteWin(int ix,int iy, ResId res):Gui()
{
    x=ix;
    y=iy;
    guitype=GUI_SPWIN;
    sprite=default_loader->load_xsprite(res);
    l=(int)sprite.l;
    h=(int)sprite.h;
};

//-------------------------------------------------------------------------
// Initialize window
//-------------------------------------------------------------------------
void   SpriteWin::init(Window ipar)
{
    int    tw, es1, es2;
    parent = ipar;
    w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, 0, normal_bg_color);
    XSelectInput(disp, w, 
		 ExposureMask |
		 ButtonPressMask |
		 KeyPressMask);
#if 0    
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
    if (XShapeQueryExtension(disp, &es1, &es2))
	XShapeCombineMask(disp, w, ShapeBounding, 0, 0, sprite.image_mask, ShapeSet);
    else
#endif
#endif
    XSetWindowBackgroundPixmap(disp, w, sprite.image);
    fprintf(stderr, "XShape extensions not supported.\n");
    
}

//-------------------------------------------------------------------------
// Show window
//-------------------------------------------------------------------------
void   SpriteWin::show()
{
  oldel = el.next;
  el.next = NULL;
  if(main_pixmap)
     XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  XMapRaised(disp, w);
  XMoveWindow(disp, w, x, y);
  addto_el(this, w);
  ffl = 1;
  guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
}

//-------------------------------------------------------------------------
// Hide window
//-------------------------------------------------------------------------
void   SpriteWin::hide()
{
  delall_el();
  delfrom_exp(this);
  el.next = oldel;
  XUnmapWindow(disp, w);
}

//-------------------------------------------------------------------------
// Processing event for window
//-------------------------------------------------------------------------
void   SpriteWin::click()
{
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
	    expose();
	    break;
        case ButtonPress:
	    hide();
	    break;
	case KeyPress:
	    hide();
	    break;
        };
    }
}


//-------------------------------------------------------------------------
// Expose event processing
//-------------------------------------------------------------------------
void   SpriteWin::expose()
{
    GC gc=XCreateGC(disp, w, 0, 0);
    XCopyArea(disp, sprite.image, w, gc, 0, 0, sprite.l, sprite.h, 0, 0);
    XFlush(disp);
    XFreeGC(disp, gc);
}

////////////////////////////////End of file/////////////////////////////////////////
