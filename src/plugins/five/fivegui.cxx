/****************************************************************************
*  Copyright (C) 2000 by Leo Khramov
*  email:     leo@xnc.dubna.su
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
 ****************************************************************************/
/* $Id: fivegui.cxx,v 1.17 2003/12/25 12:43:19 leo Exp $ */
#include "guiplugin.h"
#include "fivegui.h"
#include "scroll.h"
#include "internals.h"
#include "fivefuncs.h"

#include "history.xbm"

const Time DBLCLICK = 250;

extern GuiPlugin* guiplugin;
extern int layout;

extern Lister *dnd_creator;
extern int dnd_startx, dnd_starty;

extern char *vfs_iname(char *vfsname, char *path);

////////////////////////////////Text class///////////////////////////////
void   FiveText::show()
{
  XSetForeground(disp, gcw, SCOL21);
  XFillRectangle(disp, w, gcw, x, y-fixfontstr->max_bounds.ascent, l, h);
  tl = strlen(name);
  l = XTextWidth(fixfontstr, name, tl);
  XSetForeground(disp, gcw, SCOL2);
  XDrawString(disp, w, gcw, x, y, name, tl);
}


void   FiveText::expose()
{
  XSetForeground(disp, gcw, SCOL21);
  XFillRectangle(disp, w, gcw, x, y-fixfontstr->max_bounds.ascent, l, h);
  tl = strlen(name);
  l = XTextWidth(fixfontstr, name, tl);
  XSetForeground(disp, gcw, SCOL2);
  XDrawString(disp, w, gcw, x, y, name, tl);
}

//////////////////////////////EXWin class////////////////////////////////
void FiveEXWin::init(Window par)
{
  EXWin::init(par);
  XSetWindowBackground(disp,w,SCOL8);
}

//////////////////////////////KEY class//////////////////////////////////
void   FiveKEY::press()
{
    highlighted=0;
    if(skin)
    {
	XCopyArea(disp, skin[1].im->skin, w, gcw, skin[1].x, skin[1].y,
		  (unsigned)skin[1].l, (unsigned)skin[1].h, 0, 0);
	XSetForeground(disp, gcw, SCOL13);
	XDrawString(disp, w, gcw, tx, ty, name, tl);
    }
    if (foc == 0)
	guiSetInputFocus(disp, w, RevertToParent, CurrentTime);
}

void   FiveKEY::expose()
{
    highlighted=0;
    if(skin)
    {
	XCopyArea(disp, skin[0].im->skin, w, gcw, skin[0].x, skin[0].y,
		  (unsigned)skin[0].l, (unsigned)skin[0].h, 0, 0);
	XSetForeground(disp, gcw, SCOL22);
	XDrawString(disp, w, gcw, tx, ty, name, tl);
    }
    hflg = 1;
}

void   FiveKEY::set_focus()
{
  if (foc == 0)
  {
    foc = 1;
    XSetForeground(disp, gcw, prflg ? SCOL13 : cols[1]);
    XDrawString(disp, w, gcw, tx, ty, name, tl);
    focobj = this;
  }
}

void   FiveKEY::unset_focus()
{
  if (foc)
  {
    foc = 0;
    XSetForeground(disp, gcw, SCOL22);
    XDrawString(disp, w, gcw, tx, ty, name, tl);
    focobj = NULL;
  }
}

//////////////////////////////Win class//////////////////////////////////
#define WINBLEN        6
#define WINBLEN2       10
#define WINHEADH       26
void   FiveWin::init(Window ipar)
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, cols[0], SCOL21);
  gcv.background = keyscol[1];
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ExposureMask | ButtonPressMask | ButtonReleaseMask);
  tl = strlen(name);
  tw = XTextWidth(fontstr, name, tl);
  if (l < tw + 40)
    l = tw + 40;
  ty = (21 + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  prflg = 0;
}

void   FiveWin::expose()
{
  Gui* o;
  int twl,rax,ray,i;
  XSetForeground(disp, gcw, SCOL8);
  XFillRectangle(disp, w, gcw, 0, 0, l, WINBLEN);
  XFillRectangle(disp, w, gcw, 0, h-WINBLEN2, l, WINBLEN2);
  XFillRectangle(disp, w, gcw, 0, 0, WINBLEN, h);
  XFillRectangle(disp, w, gcw, l-WINBLEN, 0, WINBLEN, h);
  twl=XTextWidth(fontstr, name, tl);
  XSetForeground(disp, gcw, cols[0]);
  XDrawRectangle(disp, w, gcw, WINBLEN, WINBLEN, l-2*WINBLEN, h-WINBLEN-WINBLEN2);
  XDrawRectangle(disp, w, gcw, WINBLEN+1, WINBLEN+1, l-2*WINBLEN-2, h-WINBLEN-WINBLEN2-2);
  XSetForeground(disp, gcw, SCOL8);
  XFillRectangle(disp, w, gcw, WINBLEN, WINBLEN, twl+WINBLEN+spr_con4->l, WINHEADH-WINBLEN);
  XCopyArea(disp, spr_con4->im->skin, w, gcw, spr_con4->x, spr_con4->y,
	    (unsigned)spr_con4->l, (unsigned)spr_con4->h, twl+2*WINBLEN, WINHEADH-spr_con4->h);
  XSetForeground(disp, gcw, cols[0]);
  XDrawLine(disp, w, gcw, WINBLEN, WINHEADH-1, twl+2*WINBLEN, WINHEADH-1);
  XDrawLine(disp, w, gcw, WINBLEN, WINHEADH-2, twl+2*WINBLEN, WINHEADH-2);
  XDrawLine(disp, w, gcw, twl+2*WINBLEN+spr_con4->l-2, WINBLEN,
	    twl+2*WINBLEN+spr_con4->l-2, WINHEADH-spr_con4->h);
  XDrawLine(disp, w, gcw, twl+2*WINBLEN+spr_con4->l-1, WINBLEN,
	    twl+2*WINBLEN+spr_con4->l-1, WINHEADH-spr_con4->h);
  XSetForeground(disp, gcw, SCOL27);
  XDrawString(disp, w, gcw, 8, ty+2, name, tl);
  if(ra_tbl)
  {
    i=0;
    do
    {
      rax=ra_tbl[i].x;
      if(rax<0)
	rax+=l;
      ray=ra_tbl[i].y;
      if(ray<0)
	ray+=h;
      XSetForeground(disp, gcw, SCOL8);
      XFillRectangle(disp, w, gcw, rax, ray, ra_tbl[i].l, ra_tbl[i].h);
      XSetForeground(disp, gcw, cols[0]);
      if(ra_tbl[i].sides & TOP_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray, rax+ra_tbl[i].l-1, ray);
	XDrawLine(disp, w, gcw, rax, ray+1, rax+ra_tbl[i].l-1, ray+1);
      }
      if(ra_tbl[i].sides & BOTTOM_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray-1+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray-1+ra_tbl[i].h);
	XDrawLine(disp, w, gcw, rax, ray-2+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray-2+ra_tbl[i].h);
      }
      if(ra_tbl[i].sides & LEFT_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray, rax, ray+ra_tbl[i].h-1);
	XDrawLine(disp, w, gcw, rax+1, ray, rax+1, ray+ra_tbl[i].h-1);
      }
      if(ra_tbl[i].sides & RIGHT_SIDE)
      {
	XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-1, ray, rax+ra_tbl[i].l-1, ray+ra_tbl[i].h-1);
	XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-2, ray, rax+ra_tbl[i].l-2, ray+ra_tbl[i].h-1);
      }
      if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	XCopyArea(disp, spr_con1->im->skin, w, gcw, spr_con1->x, spr_con1->y,
                  (unsigned)spr_con1->l, (unsigned)spr_con1->h, rax, ray);
      if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	XCopyArea(disp, spr_con2->im->skin, w, gcw, spr_con2->x, spr_con2->y,
                  (unsigned)spr_con2->l, (unsigned)spr_con2->h, 
                  rax+ra_tbl[i].l-spr_con2->l, ray);
      if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	XCopyArea(disp, spr_con3->im->skin, w, gcw, spr_con3->x, spr_con3->y,
                  (unsigned)spr_con3->l, (unsigned)spr_con3->h, 
                  rax, ray+ra_tbl[i].h-spr_con3->h);
      if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	XCopyArea(disp, spr_con4->im->skin, w, gcw, spr_con4->x, spr_con4->y,
                  (unsigned)spr_con4->l, (unsigned)spr_con4->h, 
                  rax+ra_tbl[i].l-spr_con4->l, ray+ra_tbl[i].h-spr_con4->h);
      i++;
    } while((ra_tbl[i-1].sides & LAST_RECAREA)==0);
  }
  
  post_expose();
}

//////////////////////////////Input class//////////////////////////////////////
const unsigned tw = 5;

#define INP_PULDOWN   0

void   FiveInput::init(Window ip)
{
  parent = ip;
  w = XCreateSimpleWindow(disp, parent, x, y, l, 21, 1, 0, SCOL8);
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fixfontstr->fid);
  XSetForeground(disp, gcw, cols[col]);
  gl.init(w, ExposureMask | KeyPressMask | FocusChangeMask |
	  ButtonPressMask);
  ty = (21 + fixfontstr->max_bounds.ascent - fixfontstr->max_bounds.descent) / 2;
  ll = XTextWidth(fixfontstr, "MMMMMMMMMM",10)/10;
  hflg = foc = 0;
  dl = 0;
  tl = (l - tw - 1- INP_PULDOWN) / ll;
  firstfl = 1;
}

void   FiveInput::expose()
{
  if (foc)
    XSetWindowBorderWidth(disp, w, 2);
  else
    XSetWindowBorderWidth(disp, w, 1);
  showbuf();
#if INP_PULDOWN
  urect(w,gcw,l-INP_PULDOWN,1,INP_PULDOWN-2,18);
#endif
  hflg = 1;
}

void   FiveInput::hide_cursor()
{
  XSetForeground(disp, gcw, SCOL8);
  XDrawRectangle(disp, w, gcw, tw + cp * ll, 1, ll, 18);
}

void   FiveInput::unset_focus()
{
  if (foc)
  {
    foc = 0;
    XSetWindowBorderWidth(disp, w, 1);
    focobj = NULL;
    XSetForeground(disp, gcw, SCOL8);
    XDrawRectangle(disp, w, gcw, tw + cp * ll, 1, ll, 18);
    gl.unsetFocus();
  }
}

void   FiveInput::showbuf()
{
  char tmp[128];
  int i;
  XSetForeground(disp, gcw, SCOL8);
  XFillRectangle(disp, w, gcw, tw, 1, l - tw - 1 - INP_PULDOWN, 19);
  XSetForeground(disp, gcw, cols[col]);
  if(passwd)
  {
    for(i=0;i< (cp==0 ? bl : cp);i++)
      tmp[i]='%';
    tmp[i]=0;
    XDrawString(disp, w, gcw, tw, ty, tmp, i);
  } else         
    XDrawString(disp, w, gcw, tw, ty, buf + dl, (bl - dl)<tl ? bl-dl : tl);
}

///////////////////////Switch class////////////////////////////////////////////
#include "gl.xbm"

void   FiveSwitch::init(Window ipar)
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL21);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = keyscol[1];
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

void   FiveSwitch::expose()
{
  int    j = h / 2 - 1;
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[2]);

  XDrawLine(disp, w, gcw, 0, j, j, 0);
  XDrawLine(disp, w, gcw, 0, j, j, j + j);
  XSetForeground(disp, gcw, keyscol[0]);

  XDrawLine(disp, w, gcw, j, 0, j + j, j);
  XDrawLine(disp, w, gcw, j + j, j, j, j + j);
  
  XSetForeground(disp, gcw, disable ? SCOL16 : SCOL2);
  XDrawString(disp, w, gcw, tx, ty, _(name), tl);
  sw &= 1;
  if (sw)
    {
      XSetForeground(disp, gcw, cols[0]);
      XSetFillStyle(disp, gcw, FillStippled);
      XFillRectangle(disp, w, gcw, 0, 0, 32, 32);
      XSetFillStyle(disp, gcw, FillSolid);
    }
  select();
  hflg = 1;
}

void   FiveSwitch::select()
{
  if (foc)
    {
      XSetForeground(disp, gcw, cols[0]);
      XSetLineAttributes(disp, gcw, 0, LineOnOffDash, CapNotLast, JoinMiter);
      XDrawRectangle(disp, w, gcw, h + 4, 2, l - h - 8, h - 5);
      XSetLineAttributes(disp, gcw, 0, LineSolid, CapNotLast, JoinMiter);
    }
  else
    {
      XSetForeground(disp, gcw, SCOL21);
      XDrawRectangle(disp, w, gcw, h + 4, 2, l - h - 8, h - 5);
    }
}

void   FiveSwitch::press()
{
  int    j = h / 2 - 1;
  sw &= 1;
  XClearWindow(disp, w);
  XSetForeground(disp, gcw, keyscol[0]);

  XDrawLine(disp, w, gcw, 0, j, j, 0);
  XDrawLine(disp, w, gcw, 0, j, j, j + j);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, j, 0, j + j, j);
  XDrawLine(disp, w, gcw, j + j, j, j, j + j);

  XSetForeground(disp, gcw, SCOL16);

  XDrawString(disp, w, gcw, tx, ty, _(name), tl);
  if (foc == 0)
    guiSetInputFocus(disp, w, RevertToParent, CurrentTime);
  if (sw)
    {
      XSetForeground(disp, gcw, cols[0]);
      XSetFillStyle(disp, gcw, FillStippled);
      XFillRectangle(disp, w, gcw, 0, 0, 32, 32);
      XSetFillStyle(disp, gcw, FillSolid);
    }
}


///////////////////////////////Sw_panel class///////////////////////////////////////


void   FiveSw_panel::expose()
{
  XSetForeground(disp, gcw, SCOL21);
  XFillRectangle(disp, w, gcw, x, y, l, h);
  XSetForeground(disp, gcw, SCOL2);
  XDrawArc(disp, w, gcw, x, y, 10, 10, 90*64, 90*64);
  XDrawArc(disp, w, gcw, x, y+h-10, 10, 10, 180*64, 90*64);
  XDrawArc(disp, w, gcw, x+l-10, y+h-10, 10, 10, 270*64, 90*64);
  XDrawArc(disp, w, gcw, x+l-10, y, 10, 10, 360*64, 90*64);
  XDrawLine(disp, w, gcw, x+10, y, x+l-10, y);
  XDrawLine(disp, w, gcw, x+10, y+h, x+l-10, y+h);
  XDrawLine(disp, w, gcw, x, y+10, x, y+h-10);
  XDrawLine(disp, w, gcw, x+l, y+10, x+l, y+h-10);
  XDrawString(disp, w, gcw, x + 5, y + 2 + fontstr->max_bounds.ascent, head, tx);
  if (next && next->w == w)
    next->expose();
}

///////////////////////////////Menubar class//////////////////////////////////////
void   FiveMenuBar::expose()
{
  Window wtemp;
  unsigned u, pl, ph;
  int    itemp, i;
  XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
  for (i = 0; i < max; i++)
    showitem(i);
}

//////////////////////////////Menu class//////////////////////////////////////////
#include "mgl5.xbm"
extern Cursor menucr;

void   FiveMenu::show()
{
  if (shflg == 0)
    {
      calculate_xy_by_parent();
      w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL21);
      gcv.background = normal_bg_color;
      gcv.font = fontstr->fid;
      gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
      XSelectInput(disp, w,
                   ExposureMask | 
		   ButtonReleaseMask | 
		   LeaveWindowMask | 
		   OwnerGrabButtonMask |
                   EnterWindowMask | 
		   PointerMotionMask | 
		   ButtonPressMask);
      addto_el(this, w);
      if(main_pixmap)
          XSetWindowBackgroundPixmap(disp, w, main_pixmap);
      XMapRaised(disp, w);
      shflg = 1;
      actfl = 0;
      cur = -1;
      xpm = XCreatePixmapFromBitmapData(disp, w, (char *)mgl5_bits, mgl5_width, mgl5_height, SCOL2, SCOL21,
                                   DefaultDepth(disp, DefaultScreen(disp)));
      if (menucr == 0)
        menucr = XCreateFontCursor(disp, XC_arrow);
      XDefineCursor(disp, w, menucr);
      xncdprintf(("Menu::menu_to_notify set to %x\n", menu_to_notify));
      grab_now();
    }
}

void   FiveMenu::showitem(int i)
{
  char  *name = item_name(i);
  int    iy = mitemh * i + Menu::window_border;
  XSetForeground(disp, gcw, SCOL21);
  XFillRectangle(disp, w, gcw, Menu::window_border, iy, 
		 l - Menu::window_border*2, mitemh - 1);
  if (items[i].menu_state == menu_function)
    {
      XSetForeground(disp, gcw, SCOL2);
      XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		  iy + ty, name, tl[i]);
      if(action_len[i])
	  XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		      iy + ty, action_name(i), action_len[i]);
    }
  else
    {
      if (items[i].menu_state == menu_sw_set || 
	  items[i].menu_state == menu_ch_set)
        XCopyArea(disp, xpm, w, gcw, 0, 0, mgl5_width, mgl5_height, 5, iy + 5);
      XSetForeground(disp, gcw, SCOL2);
      XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		  iy + ty, name, tl[i]);
      if(action_len[i])
	  XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		      iy + ty, action_name(i), action_len[i]);
    }
  if (name[tl[i] - 1] == ' ')
    {
      XSetForeground(disp, gcw, SCOL2);
      XDrawLine(disp, w, gcw, Menu::window_border, iy + mitemh -1, 
		l - Menu::window_border*2, iy + mitemh -1);
    }
  default_iconman->display_icon_from_set(w, 
					 Menu::icon_delta_x, iy + mitemh/2,
					 menu_iconset, items[i].icon_type);
}

void   FiveMenu::expose()
{
  int    i;
  XSetForeground(disp, gcw, SCOL2);
  if(menu_to_notify)
  {
      if(right_justified)
          XDrawLine(disp, w, gcw, 0, 0, l-4, 0);
      else
          XDrawLine(disp, w, gcw, 3, 0, l, 0);
  } else
      XDrawLine(disp, w, gcw, 0, 0, l, 0);      
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  for (i = 0; i < max; i++)
    showitem(i);
  XRaiseWindow(disp,w);
}

void   FiveMenu::select(int i)
{
  char  *name = item_name(i);
  int    iy = mitemh * i + Menu::window_border;
  XSetForeground(disp, gcw, cursor_color);
  XFillRectangle(disp, w, gcw, Menu::window_border, iy, 
		 l - Menu::window_border*2, mitemh -1);
  XSetForeground(disp, gcw, dark_cursor_color);
  XDrawRectangle(disp, w, gcw, Menu::window_border, iy, 
		 l - Menu::window_border*2-1, mitemh -2);
  
  XSetForeground(disp, gcw, SCOL21);
  XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
	      iy + ty, name, tl[i]);
  if(action_len[i])
      XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		  iy + ty, action_name(i), action_len[i]);
  default_iconman->display_icon_from_set_with_shadow(w, 
						     Menu::icon_delta_x, iy + mitemh/2,
						     menu_iconset, items[i].icon_type);
}

///////////////////////////////Panel class/////////////////////////////////////
void   FivePanel::init(Window ipar)
{
  parent = ipar;
  ty = fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent;
  vh = (h - 6) / ty;
  h = vh * ty + 6;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 2, 0, SCOL8);
  gcv.background = keyscol[1];
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ButtonReleaseMask |
           ButtonPressMask | ExposureMask | KeyPressMask | FocusChangeMask);
  base = cur = 0;
  scr = new ScrollBar(-1, 0, h - 2, this);
  scr->setrange(0, 1);
  scr->init(w);
}

void   FivePanel::expose()
{
  shownames();
  if (foc)
    XSetWindowBorderWidth(disp, w, 3);
  else
    XSetWindowBorderWidth(disp, w, 2);
  showcurs();
  hflg = 1;
}

void   FivePanel::shownames()
{
  int    as = fixfontstr->max_bounds.ascent + 5, i, k;
  for (i = 0; i < vh; i++)
    {
      XSetForeground(disp, gcw, SCOL8);
      XFillRectangle(disp, w, gcw, 1, i * ty + 5, l - 2, ty);
      if (i + base < max)
      {
	k = strlen(names[i + base]);
	XSetForeground(disp, gcw, cols[col]);
	XDrawString(disp, w, gcw, 4, i * ty + as - 1, names[i + base], k);
      }
    }
}

void   FivePanel::showcurs(int f)
{
  int    as = fixfontstr->max_bounds.ascent + 5, k;
  if (max > 0)
    {
      k = strlen(names[cur + base]);
      if (f)
      {
        XSetForeground(disp, gcw, SCOL21);
        XFillRectangle(disp, w, gcw, 3, cur * ty + 5, l - 6, ty);
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 4, cur * ty + as - 1, names[cur + base], k);
      }
      else
       {
        XSetForeground(disp, gcw, SCOL8);
        XFillRectangle(disp, w, gcw, 3, cur * ty + 5, l - 6, ty);
	XSetForeground(disp, gcw, cols[col]);
	XDrawString(disp, w, gcw, 4, cur * ty + as - 1, names[cur + base], k);
       }
        
      if (f)
        {
          scr->maxval = max - 1;
          scr->range = max;
          scr->val = base + cur;
          scr->setpages(max / vh);
          scr->expose();
        }
    }
}

void   FivePanel::click()
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
              foc = 1;
	      XSetWindowBorderWidth(disp, w, 3);
              focobj = this;
            };
          break;
        case FocusOut:
          if (foc)
            {
              foc = 0;
	      XSetWindowBorderWidth(disp, w, 2);
              focobj = NULL;
            };
          break;
        case KeyPress:
          if (foc)
            {
              ks = XLookupKeysym(&ev.xkey, 0);
              switch (ks)
                {
                case XK_Execute:
                case XK_Escape:
                  if (canfunc)
                    canfunc();
                  break;
                case XK_Return:
                  if (func)
                    {
                      func(cur + base, names[cur + base]);
                      return;
                    };
                  if (guiobj)
                    guiobj->guifunc(this, cur + base);
                  break;
                case XK_Prior:
                  if (base > 0)
                    {
                      base -= vh;
                      if (base < 0)
                        {
                          base = cur = 0;
                        };
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }

                    };
                  break;
                case XK_Next:
                  if (base + vh + cur < max)
                    {
                      base += vh;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Right:
                  if (cur + base + 1 != max)
                    {
                      cur = (max - 1) % vh;
                      base = max - cur - 1;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Down:
                  if (cur + base + 1 < max)
                    {
                      showcurs(0);
                      cur++;
                      if (cur >= vh)
                        {
                          cur--;
                          base++;
                          expose();
                        }
                      else
                        showcurs();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Tab:
                  if (next)
                    guiSetInputFocus(disp, next->w, RevertToParent, CurrentTime);
                  break;
                case XK_Left:
                  if (cur + base != 0)
                    {
                      base = cur = 0;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Up:
                  if (cur + base > 0)
                    {
                      showcurs(0);
                      cur--;
                      if (cur < 0)
                        {
                          cur = 0;
                          base--;
                          expose();
                        }
                      else
                        showcurs();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                  break;
                default:
                  if (keyfunc)
                    keyfunc(ks, this);
                };
            };
          break;
        case ButtonPress:
          guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
          showcurs(0);
          cur = (ev.xbutton.y - 3) / ty;
          if (cur + base >= max)
            cur = max - base - 1;
          showcurs();
	  if (every_move_func)
	  {
	      every_move_func(cur + base, names[cur + base]);
	  }
          if (ev.xbutton.button == Button3)
            {
              if (func)
                {
                  func(cur + base, names[cur + base]);
                  return;
                };
              if (guiobj)
                guiobj->guifunc(this, cur + base);
            }
          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1)
            {
              if (bclk)
                {
                  bclk = 0;
                  prflg = 1;
                  if ((ev.xbutton.time - dbtime) <= DBLCLICK &&
                      abs(ev.xbutton.x - lmx) <= 10 && abs(ev.xbutton.y - lmy) <= 10)
                    {
                      if (func)
                        {
                          func(cur + base, names[cur + base]);
                          return;
                        };
                      if (guiobj)
                        guiobj->guifunc(this, cur + base);
                    };
                }
              if (prflg)
                {
                  bclk = 1;
                  prflg = 0;
                  dbtime = ev.xbutton.time;
                  lmx = ev.xbutton.x;
                  lmy = ev.xbutton.y;
                }
            };
          break;
        };
    }
}

/////////////////////////////Pager class///////////////////////////////////////
void drawh_lookfive(Window w,GC gcw,int x,int y,int l,int h,int fl=0)
{
   XSetForeground(disp,gcw,SCOL8);
   XDrawLine(disp,w,gcw,x,y+h,x,y+5);
   XDrawLine(disp,w,gcw,x,y+5,x+5,y);
   XDrawLine(disp,w,gcw,x+5,y,x+l-5,y);
   if(fl)
   {
   XDrawLine(disp,w,gcw,x+1,y+h+1,x+1,y+5);
   XDrawLine(disp,w,gcw,x+1,y+5,x+5,y+1);
   XDrawLine(disp,w,gcw,x+5,y+1,x+l-5,y+1);
   }
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

void FivePager::init(Window ipar)
{
 int tw;
 Sprite* skin;
 GEOM_TBL *tbl;
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
 parent=ipar;
 w=XCreateSimpleWindow(disp,parent,x,y,l,h,0,0,SCOL21);
 gcv.background=SCOL21;
 gcv.font=fontstr->fid;
 gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
 XSelectInput(disp,w,ExposureMask | ButtonPressMask | ButtonReleaseMask);
 ty=fontstr->max_bounds.ascent-fontstr->max_bounds.descent;
 prflg=0;
}

void FivePager::expose()
{
 int i,itl,itx;
 XClearWindow(disp,w);
 XSetForeground(disp,gcw,SCOL8);
 XDrawLine(disp,w,gcw,0,25,0,h-45);
 XDrawLine(disp,w,gcw,0,h-40,0,h-27);
 XDrawLine(disp,w,gcw,0,h-18,0,h-10);
 XDrawLine(disp,w,gcw,0,h-5,0,h-2);
 XDrawLine(disp,w,gcw,1,25,1,h-47);
 XDrawLine(disp,w,gcw,0,25,cur*phl,25);
 XDrawLine(disp,w,gcw,(cur+1)*phl-2,25,l-60,25);
 XDrawLine(disp,w,gcw,l-55,25,l-30,25);
 XDrawLine(disp,w,gcw,l-25,25,l-15,25);
 XDrawLine(disp,w,gcw,l-10,25,l-5,25);
 for(i=0;i<max;i++)
   if(i==cur)
   {
     drawh_lookfive(w,gcw,cur*phl,0,phl-2,24,1);
     if(pname[i])
     {
       XSetForeground(disp,gcw,cols[0]);
       itl=strlen(pname[i]);
       itx=XTextWidth(fontstr,pname[i],itl);
       XDrawString(disp,w,gcw,i*phl+phl/2-itx/2,20,pname[i],itl);
     }
   }
   else
   {
     drawh_lookfive(w,gcw,i*phl,1,phl-1,23);
     if(pname[i])
     {
       XSetForeground(disp,gcw, SCOL8);
       itl=strlen(pname[i]);
       itx=XTextWidth(fontstr,pname[i],itl);
       XDrawString(disp,w,gcw,i*phl+phl/2-itx/2,20,pname[i],itl);
     }
   }
 if(o[cur]!=NULL)
 {
   for(i=0;i<omax[cur];i++)
     if(o[cur][i]->w==w) o[cur][i]->click();
 }
 draw_areas();
 XFlush(disp);
 while(XCheckWindowEvent(disp,w,ExposureMask,&ev));
 
}

void  FivePager::draw_areas()
{
 int rax,ray,i,j;
 int* ptr;
 if(ra_tbl)
 {
   ptr=(int*)ra_tbl->data;
   if(!ptr)
     return;
   for(j=0;ptr[j]!=-1;j++)
     if(cur==ptr[j])
     {
       i=0;
       do
       {
	 rax=ra_tbl[i].x;
	 if(rax<0)
	   rax+=l;
	 ray=ra_tbl[i].y;
	 if(ray<0)
	   ray+=h;
	 XSetForeground(disp, gcw, SCOL8);
	 XFillRectangle(disp, w, gcw, rax, ray, ra_tbl[i].l, ra_tbl[i].h);
	 XSetForeground(disp, gcw, cols[0]);
	 if(ra_tbl[i].sides & TOP_SIDE)
	 {
	   XDrawLine(disp, w, gcw, rax, ray, rax+ra_tbl[i].l-1, ray);
	   XDrawLine(disp, w, gcw, rax, ray+1, rax+ra_tbl[i].l-1, ray+1);
	 }
	 if(ra_tbl[i].sides & BOTTOM_SIDE)
	 {
	   XDrawLine(disp, w, gcw, rax, ray-1+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray-1+ra_tbl[i].h);
	   XDrawLine(disp, w, gcw, rax, ray-2+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray-2+ra_tbl[i].h);
	 }
	 if(ra_tbl[i].sides & LEFT_SIDE)
	 {
	   XDrawLine(disp, w, gcw, rax, ray, rax, ray+ra_tbl[i].h-1);
	   XDrawLine(disp, w, gcw, rax+1, ray, rax+1, ray+ra_tbl[i].h-1);
	 }
	 if(ra_tbl[i].sides & RIGHT_SIDE)
	 {
	   XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-1, ray, rax+ra_tbl[i].l-1, ray+ra_tbl[i].h-1);
	   XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-2, ray, rax+ra_tbl[i].l-2, ray+ra_tbl[i].h-1);
	 }
	 if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	   XCopyArea(disp, spr_con1->im->skin, w, gcw, spr_con1->x, spr_con1->y,
		     (unsigned)spr_con1->l, (unsigned)spr_con1->h, rax, ray);
	 if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	   XCopyArea(disp, spr_con2->im->skin, w, gcw, spr_con2->x, spr_con2->y,
		     (unsigned)spr_con2->l, (unsigned)spr_con2->h, 
		     rax+ra_tbl[i].l-spr_con2->l, ray);
	 if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	   XCopyArea(disp, spr_con3->im->skin, w, gcw, spr_con3->x, spr_con3->y,
		     (unsigned)spr_con3->l, (unsigned)spr_con3->h, 
		     rax, ray+ra_tbl[i].h-spr_con3->h);
	 if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	   XCopyArea(disp, spr_con4->im->skin, w, gcw, spr_con4->x, spr_con4->y,
		     (unsigned)spr_con4->l, (unsigned)spr_con4->h, 
		     rax+ra_tbl[i].l-spr_con4->l, ray+ra_tbl[i].h-spr_con4->h);
	 i++;
       } while((ra_tbl[i-1].sides & LAST_RECAREA)==0);
       break;
     }
 }
}

//////////////////////////////InfoWin class////////////////////////////////////
void FiveInfoWin::expose_counter()
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
  XSetForeground(disp, gcw, SCOL8);
  XDrawRectangle(disp, w, gcw, 10, h-25-deltay, l-20, 10);
  ll=int(float((l-24))*float(currentcoun)/fullcoun);
  XSetForeground(disp, gcw, SCOL21);
  XFillRectangle(disp, w, gcw, 12+ll, h-23-deltay, l-24-ll,6);
  XSetForeground(disp,gcw, SCOL8);
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
      XSetForeground(disp,gcw, SCOL8);
      XSetBackground(disp,gcw, SCOL21);
      val=(float)(currentcoun)/1024.0/(float)(curtime-lasttime);
      sprintf(buf,"%2.2fk/sec   ", val);
      XDrawImageString(disp, w, gcw, 10, h-16, buf, strlen(buf));
    }
    lastsize=currentcoun;
  }
}

void FiveInfoWin::expose_clip_area()
{
  int textl;
  int ll;
  int deltay=0;
  if(bgkey)
    deltay=20;
  if(shown)
  {
    XSetForeground(disp, gcw, SCOL21);
    if(fullcoun)
    {
      XFillRectangle(disp, w, gcw, 8, 30, l-16, 30);
      XSetForeground(disp, gcw, SCOL8);
      textl=strlen(message);
      XDrawString(disp, w, gcw, l / 2 - XTextWidth(fontstr, message, textl) / 2, 45, message, textl);
      expose_counter();
    } else
    {
      XFillRectangle(disp, w, gcw, 8, 40, l-16, 30);
      XSetForeground(disp, gcw, SCOL8);
      textl=strlen(message);
      XDrawString(disp, w, gcw, l / 2 - XTextWidth(fontstr, message, textl) / 2, need_key ? 45 : 55, message, textl);
    }
  }
}

void   FiveInfoWin::init(Window ipar)
{
  int    tw;
  GEOM_TBL *tbl;
  Sprite *skin;
  if(need_key)
    h+=20;
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 1, cols[0], SCOL21);
  gcv.background = keyscol[1];
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ExposureMask | ButtonPressMask | ButtonReleaseMask);
  tl = strlen(name);
  tw = XTextWidth(fontstr, name, tl);
  if (l < tw + 40)
    l = tw + 40;
  ty = (21 + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  prflg = 0;
  bgbit=0;
  if(need_key)
  {
    bgkey=new KEY(-10,-10,40,20,"to BG",1,NULL);
    bgkey->init(w);
    bgkey->link(bgkey);
    bgkey->guiobj=this;
  }
}

void   FiveInfoWin::expose()
{
  Gui* o;
  int twl,rax,ray,i;
  XSetForeground(disp, gcw, SCOL8);
  XFillRectangle(disp, w, gcw, 0, 0, l, WINBLEN);
  XFillRectangle(disp, w, gcw, 0, h-WINBLEN2, l, WINBLEN2);
  XFillRectangle(disp, w, gcw, 0, 0, WINBLEN, h);
  XFillRectangle(disp, w, gcw, l-WINBLEN, 0, WINBLEN, h);
  twl=XTextWidth(fontstr, name, tl);
  XSetForeground(disp, gcw, cols[0]);
  XDrawRectangle(disp, w, gcw, WINBLEN, WINBLEN, l-2*WINBLEN, h-WINBLEN-WINBLEN2);
  XDrawRectangle(disp, w, gcw, WINBLEN+1, WINBLEN+1, l-2*WINBLEN-2, h-WINBLEN-WINBLEN2-2);
  XSetForeground(disp, gcw, SCOL8);
  XFillRectangle(disp, w, gcw, WINBLEN, WINBLEN, twl+WINBLEN+spr_con4->l, WINHEADH-WINBLEN);
  XCopyArea(disp, spr_con4->im->skin, w, gcw, spr_con4->x, spr_con4->y,
	    (unsigned)spr_con4->l, (unsigned)spr_con4->h, twl+2*WINBLEN, WINHEADH-spr_con4->h);
  XSetForeground(disp, gcw, cols[0]);
  XDrawLine(disp, w, gcw, WINBLEN, WINHEADH-1, twl+2*WINBLEN, WINHEADH-1);
  XDrawLine(disp, w, gcw, WINBLEN, WINHEADH-2, twl+2*WINBLEN, WINHEADH-2);
  XDrawLine(disp, w, gcw, twl+2*WINBLEN+spr_con4->l-2, WINBLEN,
	    twl+2*WINBLEN+spr_con4->l-2, WINHEADH-spr_con4->h);
  XDrawLine(disp, w, gcw, twl+2*WINBLEN+spr_con4->l-1, WINBLEN,
	    twl+2*WINBLEN+spr_con4->l-1, WINHEADH-spr_con4->h);
  XSetForeground(disp, gcw, SCOL27);
  XDrawString(disp, w, gcw, 8, ty+2, name, tl);
  if(ra_tbl)
  {
    i=0;
    do
    {
      rax=ra_tbl[i].x;
      if(rax<0)
	rax+=l;
      ray=ra_tbl[i].y;
      if(ray<0)
	ray+=h;
      XSetForeground(disp, gcw, SCOL8);
      XFillRectangle(disp, w, gcw, rax, ray, ra_tbl[i].l, ra_tbl[i].h);
      XSetForeground(disp, gcw, cols[0]);
      if(ra_tbl[i].sides & TOP_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray, rax+ra_tbl[i].l-1, ray);
	XDrawLine(disp, w, gcw, rax, ray+1, rax+ra_tbl[i].l-1, ray+1);
      }
      if(ra_tbl[i].sides & BOTTOM_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray+ra_tbl[i].h);
	XDrawLine(disp, w, gcw, rax, ray-1+ra_tbl[i].h, rax+ra_tbl[i].l-1, ray-1+ra_tbl[i].h);
      }
      if(ra_tbl[i].sides & LEFT_SIDE)
      {
	XDrawLine(disp, w, gcw, rax, ray, rax, ray+ra_tbl[i].h-1);
	XDrawLine(disp, w, gcw, rax+1, ray, rax+1, ray+ra_tbl[i].h-1);
      }
      if(ra_tbl[i].sides & RIGHT_SIDE)
      {
	XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-1, ray, rax+ra_tbl[i].l-1, ray+ra_tbl[i].h-1);
	XDrawLine(disp, w, gcw, rax+ra_tbl[i].l-2, ray, rax+ra_tbl[i].l-2, ray+ra_tbl[i].h-1);
      }
      if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	XCopyArea(disp, spr_con1->im->skin, w, gcw, spr_con1->x, spr_con1->y,
                  (unsigned)spr_con1->l, (unsigned)spr_con1->h, rax, ray);
      if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & TOP_SIDE))
	XCopyArea(disp, spr_con2->im->skin, w, gcw, spr_con2->x, spr_con2->y,
                  (unsigned)spr_con2->l, (unsigned)spr_con2->h, 
                  rax+ra_tbl[i].l-spr_con2->l, ray);
      if((ra_tbl[i].sides & LEFT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	XCopyArea(disp, spr_con3->im->skin, w, gcw, spr_con3->x, spr_con3->y,
                  (unsigned)spr_con3->l, (unsigned)spr_con3->h, 
                  rax, ray+ra_tbl[i].h-spr_con3->h);
      if((ra_tbl[i].sides & RIGHT_SIDE) && (ra_tbl[i].sides & BOTTOM_SIDE))
	XCopyArea(disp, spr_con4->im->skin, w, gcw, spr_con4->x, spr_con4->y,
                  (unsigned)spr_con4->l, (unsigned)spr_con4->h, 
                  rax+ra_tbl[i].l-spr_con4->l, ray+ra_tbl[i].h-spr_con4->h);
      i++;
    } while((ra_tbl[i-1].sides & LAST_RECAREA)==0);
  }
  
  if (ffl)
  {
    if (next)
      if (next->foc == 0 && next->w != w)
	guiSetInputFocus(disp, next->w, RevertToNone, CurrentTime);
      else if (next->next)
	guiSetInputFocus(disp, next->next->w, RevertToNone, CurrentTime);
    ffl = 0;
  }
  if (next)
    if (next->w == w)
      next->expose();
  o=guiexpose;
  while(o && o->w==w)
  {
    o->expose();
    o=o->next;
  }
  expose_clip_area(); 
}


/////////////////////////////Separator class//////////////////////////////
void   FiveSeparator::expose()
{
  XSetForeground(disp,gcw,SCOL8);
  if(!vertical)
  {
    XDrawLine(disp,w,gcw,x,y,x+l,y);
    XDrawLine(disp,w,gcw,x,y+1,x+l,y+1);
  }
  else
  {
    XDrawLine(disp,w,gcw,x,y,x,y+l);
    XDrawLine(disp,w,gcw,x+1,y,x+1,y+l);
  }
}

////////////////////////////FtpVisual class//////////////////////////////
#define TUMBSIZE  110
#define ANIMSTEP   10


extern FTP *ftparr[];
void   FiveFtpVisual::init(Window ipar)
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL8);
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

void  FiveFtpVisual::reconfigure(int ix,int iy,int pl,int ph)
{
  x=ix;
  y=iy;
  l=pl;
  XMoveResizeWindow(disp,w,x,y,l,h);
}

void FiveFtpVisual::show_tumb(int i, int ix)
{
  FTP* fo=ftparr[i];
  int tl=strlen(fo->host);
  if(tl>10) tl=10;
  urect(w,gcw,ix+1,0,TUMBSIZE-1,h-1);
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
  XSetForeground(disp,gcw,cols[1]);
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

////////////////////////////Cmdline class///////////////////////////////
void   FiveCmdline::init(Window ip)
{
  parent = ip;
  bbp = bp = 0;
  h=20;
  geometry_by_iname();
  for (int i = 0; i < max_cmd_history; i++)
  {
      bak[i][0] = 0;
  }
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL8);
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fixfontstr->fid);
  XSetForeground(disp, gcw, cols[col]);

  gl.init(w, ExposureMask | ButtonPressMask);
  tl = strlen(name);
  ty = h/2 - (fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent)/2 +
      fixfontstr->max_bounds.ascent;
  ll = XTextWidth(fixfontstr, "MMMMMMMMMM",10)/10;
  hflg = 0;

  history_pixmap = XCreatePixmapFromBitmapData(disp, w, (char *)history_bits, 
					       history_width, history_height, 
					       SCOL25, SCOL8,
					       DefaultDepth(disp, DefaultScreen(disp)));

  delta_x=18;
  text_border=3;
  text_x=delta_x+text_border;

}

void   FiveCmdline::expose()
{
    XClearWindow(disp, w);
    five_prect(w, gcw, delta_x, 0, l - delta_x - 1, h-1);
    default_iconman->display_icon_from_set(w, 
					   2, h/2,
					   menu_iconset, help_icon);
    XSetForeground(disp, gcw, cols[col]);
    XDrawString(disp, w, gcw, text_x, ty, name, tl);
    tw = XTextWidth(fixfontstr, name, tl) + text_border;
    cmd_text_x=text_x+tw;
    visl = (l - cmd_text_x - text_border*2) / ll - 1;

    if(bl>visl)
    {
	cp+=base;
	base=bl-visl-1;
	cp-=base;
    }
    showbuf();
    hflg = 1;
}

///////////////////////////BookMark class///////////////////////////////////
#define        STICKY_CENTER   20
#define        MIN_PERCENT    20
#include "recycle.xbm"
#include "recmask.xbm"
#include "box.xbm"
#include "cir.xbm"
#include "cir1.xbm"
#include "cir2.xbm"
#include "cir3.xbm"
#include "cir4.xbm"
#include "cir5.xbm"
#include "cir6.xbm"
#include "cir7.xbm"
#include "a1.xbm"
#include "a2.xbm"
#include "rec2m.xbm"
#include "rec3m.xbm"
#include "rec4m.xbm"

void   FiveBookMark::init(Window ipar)
{
  GEOM_TBL *tbl;
  Sprite *skin;
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
  page_h = (h - 75) / 9;
  maxy = 25 + maxbooks * page_h;
  //  l = 30;
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  if(tbl)
  {
   skin=(Sprite*)tbl->data1;
   ra_cuts=(RecArea*)tbl->data2;
   ra_books=(RecArea*)tbl->data3;
  }
  else
   skin=NULL;
  if(skin)
  {
    back_skin=&skin[0];
    normal_skin=&skin[1];
    select_skin=&skin[2];
  }
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL25);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = SCOL25;
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  gcm = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, StructureNotifyMask | ButtonReleaseMask | PointerMotionMask |
               ExposureMask | LeaveWindowMask | ButtonPressMask |
               EnterWindowMask | OwnerGrabButtonMask);
  ty = fixfontstr->max_bounds.ascent - fixfontstr->max_bounds.descent;
  ty2 = mfixfontstr->max_bounds.ascent - mfixfontstr->max_bounds.descent;
  if (ty < 0)
    {
      ty = -ty;
      ty2 = -ty2;
    };
  rgc = XCreateGC(disp, Main, 0, NULL);
  XSetSubwindowMode(disp, rgc, IncludeInferiors);
  XSetFunction(disp, rgc, GXxor);
  XSetForeground(disp, rgc, cols[1]);
  recyclemask = 0;
  if (recyclepix == 0)
    {
      recyclepix = pixrecycle[0] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
      recycle_l = l_recycle[0] = recycle_width;
      recycle_h = h_recycle[0] = recycle_height;
      pixrecycle[1] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[1] = recycle_width;
      h_recycle[1] = recycle_height;
      pixrecycle[2] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[2] = recycle_width;
      h_recycle[2] = recycle_height;
      pixrecycle[3] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[3] = recycle_width;
      h_recycle[3] = recycle_height;

      recyclemask = maskrecycle[0] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[1] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[2] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[3] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);

    }

  if (recyclemask == 0)
    {
      recyclemask = maskrecycle[0] = XCreateBitmapFromData(disp, Main, (char *)recmask_bits, recmask_width, recmask_height);
      maskrecycle[1] = XCreateBitmapFromData(disp, Main, (char *)rec2m_bits, rec2m_width, rec2m_height);
      maskrecycle[2] = XCreateBitmapFromData(disp, Main, (char *)rec3m_bits, rec3m_width, rec3m_height);
      maskrecycle[3] = XCreateBitmapFromData(disp, Main, (char *)rec4m_bits, rec4m_width, rec4m_height);
    }
  XSetClipMask(disp, gcm, recyclemask);
  cur_recycle = 0;

  XSetClipOrigin(disp, gcm, l / 2 - recycle_l / 2, h - recycle_h - 3);
  a1pix = XCreatePixmapFromBitmapData(disp, w, (char *)a1_bits, a1_width, a1_height,
              cols[3], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
  a2pix = XCreatePixmapFromBitmapData(disp, w, (char *)a2_bits, a2_width, a2_height,
              cols[2], keyscol[1], DefaultDepth(disp, DefaultScreen(disp)));
  qh.init(Main);
  pagenum = -1;
  rw_cur = XCreateFontCursor(disp, XC_sb_h_double_arrow);
}

void   FiveBookMark::create_listers(Lister ** l1, Lister ** l2, int ix, int iy, int ny)
{
  int    mll;
  listx=ix;
  listy=iy;
  listny=ny;
  if (allow_bookmark == 0)
    l = 0;
  rw = 0;
  switch (layout)
    {
    case 1:                        //Vertical layout

      mll = (Mainl - l) * percent / 100 - ix;
      lst1 = guiplugin->new_Lister(ix, iy, mll - 1, Mainh  - ny - iy);
      lst2 = guiplugin->new_Lister(ix + mll + 1, iy, (Mainl - l - mll) - 1 - ix, Mainh - ny -iy);
      lst1->set_iname("LISTER1");
      lst2->set_iname("LISTER2");
      lst2->side = 2;

      rwx = mll - 1;
      rwy = iy;
      leftrx = (Mainl - l) * MIN_PERCENT / 100;
      rightrx = Mainl - l - leftrx;
      rw = XCreateSimpleWindow(disp, parent, rwx, rwy, 2, Mainh - ny- iy, 0, 0, SCOL25);
      gcrw = XCreateGC(disp, rw, 0, NULL);
      XSelectInput(disp, rw, PointerMotionHintMask |
                   ButtonPressMask | ExposureMask | ButtonReleaseMask | Button1MotionMask);
      break;
    case 0:                        //Horizontal layout

      XFreeCursor(disp, rw_cur);
      rw_cur = XCreateFontCursor(disp, XC_sb_v_double_arrow);
      mll = (Mainh - ny - iy) * percent / 100;
      lst1 = guiplugin->new_Lister(ix, iy, Mainl - l, mll - 1);
      lst2 = guiplugin->new_Lister(ix, iy + mll + 1, Mainl - l, Mainh - 1 - ny - iy - mll);
      lst1->set_iname("LISTER1");
      lst2->set_iname("LISTER2");
      lst2->side = 2;

      rwx = 0;
      rwy = iy -1 + mll;
      leftrx = (Mainh - 63) * MIN_PERCENT / 100;
      rightrx = Mainh - 63 - leftrx;
      rw = XCreateSimpleWindow(disp, parent, rwx, rwy, Mainl - l, 2, 0, 0, SCOL25);
      gcrw = XCreateGC(disp, rw, 0, NULL);
      XSelectInput(disp, rw, PointerMotionHintMask |
                   ButtonPressMask | ExposureMask | ButtonReleaseMask | Button1MotionMask);
      break;
    case 2:                        //Page layout

      lst1 = guiplugin->new_Lister(ix, iy, Mainl - l, Mainh - ny - iy);
      lst2 = guiplugin->new_Lister(ix, iy, Mainl - l, Mainh - ny - iy);
      lst1->set_iname("LISTER1");
      lst2->set_iname("LISTER2");
      lst1->lay = 1;
      lst2->lay = 2;
      break;
    };
  *l1 = lst1;
  *l2 = lst2;
}

void   FiveBookMark::animate()
{
}

void   FiveBookMark::expose()
{
  int    i;
  if (allow_bookmark == 0)
    return;
  if (flag_anim)
    XCopyArea(disp, flagpix[cur_flag], w, gcw, 0, 0, flagl, flagh, l / 2 - flagl / 2, 0);
  XCopyArea(disp, recyclepix, w, gcm, 0, 0, recycle_l, recycle_h, l / 2 - recycle_l / 2, h - recycle_h - 3);
  if(back_skin)
    XCopyArea(disp, back_skin->im->skin, w, gcw, back_skin->x, back_skin->y,
                  (unsigned)back_skin->l, (unsigned)back_skin->h, 
	      back_skin->tox, back_skin->toy);
  for (i = 0; i < 9; i++)
    if (i != last_used)
      draw_page(i);
  if (act_books[last_used])
    draw_selected_page(last_used);
  hflg = 1;
}

void  FiveBookMark::draw_page(int i)
{
  Sprite* skin=back_skin;
  if(act_books[i])
    skin=normal_skin;
  XCopyArea(disp,skin->im->skin, w, gcw, skin->x+ra_cuts[i].x,ra_cuts[i].y,
	    ra_cuts[i].l,ra_cuts[i].h,ra_cuts[i].x,ra_cuts[i].y-skin->y);
}

void  FiveBookMark::draw_selected_page(int i)
{
  Sprite* skin=select_skin;
  if(!act_books[i])
    skin=back_skin;
  XCopyArea(disp,skin->im->skin, w, gcw, skin->x+ra_cuts[i].x,ra_cuts[i].y,
	    ra_cuts[i].l,ra_cuts[i].h,ra_cuts[i].x,ra_cuts[i].y-skin->y);
}

int   FiveBookMark::get_pagenum(int x,int y)
{
  int i;
  for(i=0;i<9;i++)
    if(x>=ra_books[i].x && y>=ra_books[i].y-back_skin->y &&
       x<=ra_books[i].x+ra_books[i].l &&
       y<=ra_books[i].y+ra_books[i].h-back_skin->y)
      return i;
  return -1;
}

int FiveBookMark::get_page_y(int n)
{
  return ra_books[n].y-back_skin->y;
}

VFS   *FiveBookMark::get_vfs_by_coord(int iy)
{
  iy=get_pagenum(5,iy);
  if (iy > 8 || iy==-1)
    return NULL;
  if (act_books[iy] == 0)
    return NULL;
  return define_vfs(fs[iy], books[iy]);
}

void   FiveBookMark::show_dir_banner(int iy)
{
  int    k;
  if ((k=get_pagenum(5,iy))!=-1)
  {
    if (k < 9)
    {
      if (k != pagenum && act_books[k])
      {
	pagenum = k;
	qh.show_this_info(vfs_iname(fs[k], books[k]), books[k], -x, get_page_y(k)+5);
	qh.show();
      }
    }
  }
  else
  {
    qh.hide();
    pagenum = -1;
  }
}

extern int dnd_enabled;
void   FiveBookMark::click()
{
  int    k;
  int    cx, cy, rx, ry, mll;
  unsigned kmask;
  Window rrw, cw;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case MotionNotify:
//   XQueryPointer(disp,w,&rrw,&cw,&rx,&ry,&cx,&cy,&kmask);
          rx = cx = ev.xbutton.x;
          ry = cy = ev.xbutton.y;
          kmask = ev.xbutton.state;
          if (kmask & Button1Mask)
            {
              if ((abs(dnd_starty - cy) > 5 || abs(dnd_startx - cx) > 10) && dnd_enabled == 0)
                {
                  XDefineCursor(disp, w, cdnd1);
                  dnd_enabled = 1;
                  set_recycle_image(1);
                }
            }
	  k=get_pagenum(rx,ry);
	  if(k!=-1 && k != pagenum && act_books[k])
	  {
	    pagenum = k;
	    qh.show_this_info(vfs_iname(fs[k], books[k]),books[k], -x, get_page_y(k)+5);
	    qh.show();
	  }
          else if (ry > h - 51)
	  {
	    if (pagenum != -2)
	    {
	      qh.show_this_info(NULL, "Recycle Bin", -x, h - 25);
	      qh.show();
	      pagenum = -2;
	    }
	  }
          else if(pagenum!=k)
	  {
	    qh.hide();
	    pagenum = -1;
	  };
          break;
        case ButtonRelease:
          if (dnd_enabled)
            {
              XUndefineCursor(disp, w);
              if (ev.xbutton.y > h - 50)
                {
                  rx = get_pagenum(6,dnd_starty);
                  if (rx!=-1 && rx < 9)
                    if (act_books[rx])
                      del_book(rx);
                }
              else
                {
                  rx = get_pagenum(6,dnd_starty);
                  ry = get_pagenum(6,ev.xbutton.y);
                  if (rx!=-1 && ry!=-1 && rx < 9 && ry < 9)
                    if (act_books[rx] && act_books[ry] == 0)
                      {
                        if (books[ry] == NULL)
                          books[ry] = new char[1024];
                        strcpy(books[ry], books[rx]);
                        act_books[ry] = 1;
                        act_books[rx] = 0;
                        last_used = ry;
                        strcpy(fs[ry], fs[rx]);
                        XClearWindow(disp, w);
                        expose();
                      }
                }
              set_recycle_image(0);
              dnd_enabled = 0;
              break;
            }

          if (ev.xbutton.y > 0 && ev.xbutton.y < maxy)
            {
              k = get_pagenum(ev.xbutton.x,ev.xbutton.y);
              if (k!=-1 && act_books[k])
                {
                  if (k != last_used)
                    {
                      if (act_books[last_used])
                        draw_page(last_used);
                      draw_selected_page(k);
                      last_used = k;
                    }
                  if (last_used < 8)
                    switch_books(last_used);
                  else
                    {
                      swap_books(last_used);
                      qh.show_this_info(vfs_iname(fs[last_used],books[last_used]), 
					books[last_used], -x, get_page_y(last_used)+5);
                      qh.show();
                    };
                }
            };
          break;
        case ButtonPress:
          dnd_startx = ev.xbutton.x;
          dnd_starty = ev.xbutton.y;
          break;
        case MapNotify:
          mapped = 1;
          break;
        case UnmapNotify:
          mapped = 0;
          break;
        case LeaveNotify:
          qh.hide();
          pagenum = -1;
          break;
        };
    }
  else if (ev.xany.window == rw)
    switch (ev.type)
      {
      case Expose:
        rw_expose();
        break;
      case ButtonPress:
        if (layout)
          {
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx, rwy + h);
            XDrawLine(disp, Main, rgc, rwx + 1, rwy, rwx + 1, rwy + h);
          }
        else
          {
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx + Mainl - l, rwy);
            XDrawLine(disp, Main, rgc, rwx, rwy + 1, rwx + Mainl - l, rwy + 1);
          }
        break;
      case ButtonRelease:
        if (layout)
          {
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx, rwy + h);
            XDrawLine(disp, Main, rgc, rwx + 1, rwy, rwx + 1, rwy + h);
            if (abs(rwx - (Mainl - l) / 2) < STICKY_CENTER)
              rwx = (Mainl - l) / 2;
            if (rwx < (Mainl - l) * MIN_PERCENT / 100)
              rwx = (Mainl - l) * MIN_PERCENT / 100;
            percent = rwx * 100 / (Mainl - l);
            mll = (Mainl - l) * percent / 100;
            lst1->reconfigure(0, 20, mll - 1, Mainh - 63);
            lst2->reconfigure(mll + 1, 20, Mainl - l - mll - 1, Mainh - 63);
            rwx = mll - 1;
            rwy = 20;
            XMoveResizeWindow(disp, rw, rwx, rwy, 2, h);
          }
        else
          {
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx + Mainl - l, rwy);
            XDrawLine(disp, Main, rgc, rwx, rwy + 1, rwx + Mainl - l, rwy + 1);
            if (rwy < (Mainh - 63) * MIN_PERCENT / 100)
              rwy = (Mainl - 63) * MIN_PERCENT / 100 + 20;
            percent = (rwy - 20) * 100 / (Mainh - 63);
            mll = (Mainh - 63) * percent / 100;
            lst1->reconfigure(0, 20, Mainl - l, mll - 1);
            lst2->reconfigure(0, 20 + mll + 1, Mainl - l, Mainh - 64 - mll);
            rwx = 0;
            rwy = 19 + mll;
            XMoveResizeWindow(disp, rw, rwx, rwy, Mainl - l, 2);
          }
        rw_expose();
        break;
      case MotionNotify:
        XQueryPointer(disp, Main, &rrw, &cw, &rx, &ry, &cx, &cy, &kmask);
        if (layout)
          {
            if (cx > leftrx && cx < rightrx)
              {
                if (abs(cx - (Mainl - l) / 2) < STICKY_CENTER)
                  cx = (Mainl - l) / 2;
                XDrawLine(disp, Main, rgc, rwx, rwy, rwx, rwy + h);
                XDrawLine(disp, Main, rgc, rwx + 1, rwy, rwx + 1, rwy + h);
                rwx = cx;
                XDrawLine(disp, Main, rgc, rwx, rwy, rwx, rwy + h);
                XDrawLine(disp, Main, rgc, rwx + 1, rwy, rwx + 1, rwy + h);
              }
          }
        else if (cy - 20 > leftrx && cy - 20 < rightrx)
          {
            if (abs(cy - (Mainh - 63) / 2 - 20) < STICKY_CENTER)
              cy = (Mainh - 62) / 2 + 20;
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx + Mainl - l, rwy);
            XDrawLine(disp, Main, rgc, rwx, rwy + 1, rwx + Mainl - l, rwy + 1);
            rwy = cy;
            XDrawLine(disp, Main, rgc, rwx, rwy, rwx + Mainl - l, rwy);
            XDrawLine(disp, Main, rgc, rwx, rwy + 1, rwx + Mainl - l, rwy + 1);
          }
        break;
      };
}

void   FiveBookMark::animate_moving(int i)
{
  if(!allow_animation)
          return;
  int    ix = x + l - 1, iy = y + get_page_y(i) - 1;
  int    il = l - 1, ih = ra_books[i].h - 1;
  int    bh = ih, bl = il;
  int    bx = ix, by = iy;
  int    istep = 0;
  int    dx=0, dy = panel->y  - by;
  int    dl=0, dh = - ih;
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
  while (istep < 10)
    {
      istep++;
      oldx = ix - il;
      oldy = iy - ih;
      oldl = il;
      oldh = ih;
      il = bl + (dl * istep / 10);
      ih = bh + (dh * istep / 10);
      ix = bx + (dx * istep / 10);
      iy = by + (dy * istep / 10);
      XDrawRectangle(disp, Main, rgc, oldx, oldy, oldl, oldh);
      XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
      XSync(disp, 0);
      delay(20);
    }
  XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
  XSync(disp, 0);
}

void   FiveBookMark::animate_swapping(int i)
{
  if(!allow_animation)
          return;
  int    ix = x + l - 1, iy = y + get_page_y(i) - 1;
  int    il = l - 1, ih = ra_books[i].h - 1;
  int    bh = ih, bl = il;
  int    bx = ix, by = iy;
  int    istep = 0;
  int    dx=0, dy = panel->y - by;
  int    dl=0, dh = - ih;
  int    oldl, oldh, oldx, oldy;
  int    oldl2, oldh2, oldx2, oldy2;
  int    dx2, dy2, dl2, dh2, ix2, iy2, il2, ih2, bx2, by2, bl2, bh2;
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
  bx2 = ix2 = dx + bx;
  by2 = iy2 = dy + by;
  bh2 = ih2 = 25;
  bl2 = il2 = dl + bl;
  dx2 = -dx;
  dy2 = -dy;
  dl2 = -dl;
  dh2 = -dh;
  XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
  XDrawRectangle(disp, Main, rgc, ix2 - il2, iy2 - ih2, il2, ih2);
  XSync(disp, 0);
  delay(20);
  while (istep < 10)
    {
      istep++;
      oldx = ix - il;
      oldy = iy - ih;
      oldl = il;
      oldh = ih;
      oldx2 = ix2 - il2;
      oldy2 = iy2 - ih2;
      oldl2 = il2;
      oldh2 = ih2;
      il = bl + (dl * istep / 10);
      ih = bh + (dh * istep / 10);
      ix = bx + (dx * istep / 10);
      iy = by + (dy * istep / 10);
      il2 = bl2 + (dl2 * istep / 10);
      ih2 = bh2 + (dh2 * istep / 10);
      ix2 = bx2 + (dx2 * istep / 10);
      iy2 = by2 + (dy2 * istep / 10);
      XDrawRectangle(disp, Main, rgc, oldx, oldy, oldl, oldh);
      XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
      XDrawRectangle(disp, Main, rgc, oldx2, oldy2, oldl2, oldh2);
      XDrawRectangle(disp, Main, rgc, ix2 - il2, iy2 - ih2, il2, ih2);
      XSync(disp, 0);
      delay(20);
    }
  XDrawRectangle(disp, Main, rgc, ix - il, iy - ih, il, ih);
  XDrawRectangle(disp, Main, rgc, ix2 - il2, iy2 - ih2, il2, ih2);
  XSync(disp, 0);
}


void   FiveBookMark::blink_book(int n, int blinks)
{
  if (allow_bookmark == 0)
    return;
  blinks+=2;
  while (blinks)
    {
      delay(150);
      draw_selected_page(n);
      XSync(disp, 0);
      delay(150);
      draw_page(n);
      XSync(disp, 0);
      blinks--;
    }
  if (n == last_used)
    draw_selected_page(n);
  else
    draw_page(n);
}

void   FiveBookMark::add_toempty_book_by_coord(int cy, char *s, int type)
{
  int    i = get_pagenum(5,cy);
  if (act_books[i] == 0)
    {
      add_book(i, s, type);
      if (allow_bookmark)
        expose();
    }
}

