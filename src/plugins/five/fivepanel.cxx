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
/* $Id: fivepanel.cxx,v 1.3 2002/10/30 12:47:42 leo Exp $ */
#include "guiplugin.h"
#include "fivegui.h"
#include "scroll.h"
#include "internals.h"
#include "fivefuncs.h"
#include "config_file.h"

////////////////////////////Lister class///////////////////////////////////
#define HEADH      20

void   five_drawhead(Window w, GC gcw, int x, int y, int l, int h, int fl = 0)
{
    XSetForeground(disp, gcw, SCOL26);
    XDrawLine(disp, w, gcw, x, y + h, x, y + 5);
    XDrawLine(disp, w, gcw, x, y + 5, x + 5, y);
    XDrawLine(disp, w, gcw, x + 5, y, x + l - 5, y);
    if (fl)
    {
      XDrawLine(disp, w, gcw, x + 1, y + h + 1, x + 1, y + 5);
      XDrawLine(disp, w, gcw, x + 1, y + 5, x + 5, y + 1);
      XDrawLine(disp, w, gcw, x + 5, y + 1, x + l - 5, y + 1);
    }
    XSetForeground(disp, gcw, SCOL2);
    XDrawLine(disp, w, gcw, x + l - 5, y, x + l, y + 5);
    XDrawLine(disp, w, gcw, x + l, y + 5, x + l, y + h);
    if (fl)
    {
      XDrawLine(disp, w, gcw, x + l - 5, y + 1, x + l - 1, y + 5);
      XDrawLine(disp, w, gcw, x + l - 1, y + 5, x + l - 1, y + h + 1);
      XSetForeground(disp, gcw, cols[0]);
      XDrawLine(disp, w, gcw, x + l - 3, y + 1, x + l + 1, y + 5);
      XDrawLine(disp, w, gcw, x + l + 1, y + 6, x + l + 1, y + h - 1);
    }
}

void   FiveLister::init(Window ipar)
{
  foc = 0;
  refindchr[0] = 0;
  XWindowAttributes xwa;
  XSetWindowAttributes xswa;
  GEOM_TBL *tbl;
  Sprite *skin;
  parent = ipar;
  geometry_by_iname();
  tbl=geom_get_data_by_iname(guitype, in_name);
  if(tbl)
  {
   skin=(Sprite*)tbl->data1;
   spr_arrow1=&skin[0];  //arrow < for scroll
   spr_arrow2=&skin[1];  //arrow > for scroll
   spr_statcon1=&skin[2];  //status bar right corner (
   spr_statcon2=&skin[3];  //status bar left corner )
   spr_pancon1=&skin[4];
   spr_pancon2=&skin[5];
   spr_pancon3=&skin[6];   

   spr_pancon4=&skin[7];
   spr_pancon5=&skin[8];
   spr_pancon6=&skin[9];
  }
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, SCOL25);
  gcv.background = SCOL25;
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  gcv.font = lfontstr->fid;
  wgcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  xpmgc = XCreateGC(disp, w, 0, NULL);
  if (XGetWindowAttributes(disp, w, &xwa) != 0)
  {
    xswa.do_not_propagate_mask = xwa.do_not_propagate_mask | ButtonPressMask |
      KeyPressMask;
    XChangeWindowAttributes(disp, w, CWDontPropagate, &xswa);
  }
  else
  {
    xncdprintf(("Propagation failed\n"));
  }
  XSelectInput(disp, w, ExposureMask | ButtonPressMask | ButtonReleaseMask | 
               PointerMotionMask |
               KeyPressMask |
               FocusChangeMask | StructureNotifyMask);
  fattr1 = XTextWidth(lfontstr, "-rwxrwxrwx", 9);
  attdx = 8 + (43 - XTextWidth(fixfontstr, "0000", 4)) / 2;
  repanel(col);
  foc = 0;

  init_dnd();

  scr=new ScrollBar(l-24,28,h-28-35,this);
  scr->setrange(0,10);
  scr->init(w);
}

void   FiveLister::expose()
{
  int    i, px, lx = l / 2 + 5, l2 = l / 2;
  int    lx2 = l2 - 15;
  char  *cdir = panel2->vfs->get_dir_header();
  int    sl = strlen(cdir), vl, fl;
  fl = (l - 70) / 2;
  vl = fl / fixl;
  int    dl = 0;
  if (sl > vl)
    dl = sl - vl;
  vl = sl - dl;
  switch (lay)
  {
  case 0:
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,11,0,l-22,spr_pancon1->h);
    five_show_sprite(w, gcw,l,h,spr_pancon1);
    spr_pancon5->tox=l-spr_pancon5->l+1;
    five_show_sprite(w, gcw,l,h,spr_pancon5);
    break;
  case 1:
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,11,0,l-11,spr_pancon1->h);
    five_show_sprite(w, gcw,l,h,spr_pancon1);
    spr_pancon2->tox=lx-spr_pancon2->l+1;
    five_show_sprite(w, gcw,l,h,spr_pancon2);
    spr_pancon3->tox=l-spr_pancon3->l+1;
    five_show_sprite(w, gcw,l,h,spr_pancon3);
    XSetForeground(disp,gcw,cols[0]);
    XDrawLine(disp,w,gcw,lx+1,spr_pancon3->h-2,l-1,spr_pancon3->h-2);
    XSetForeground(disp,gcw,SCOL16);
    XDrawLine(disp,w,gcw,lx+1,2,l-spr_pancon3->l,2);
    XSetForeground(disp,gcw,SCOL17);
    XDrawLine(disp,w,gcw,lx+1,1,l-spr_pancon3->l,1);
    XSetForeground(disp,gcw,SCOL24);
    XDrawLine(disp,w,gcw,lx+1,0,l-spr_pancon3->l,0);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, l2 + 31, 5+fixy, cdir + dl, vl);
    }
    XSetForeground(disp, gcw, SCOL24);
    XDrawString(disp, w, gcw, l2 + 30, 4+fixy, cdir + dl, vl);
    break;
  case 2:
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,11,0,l-11,spr_pancon1->h);
    spr_pancon4->tox=lx-5;
    five_show_sprite(w,gcw,l,h,spr_pancon4);
    spr_pancon5->tox=l-spr_pancon5->l+1;
    five_show_sprite(w, gcw,l,h,spr_pancon5);
    five_show_sprite(w, gcw,l,h,spr_pancon6);
    XSetForeground(disp,gcw,cols[0]);
    XDrawLine(disp,w,gcw,10,spr_pancon6->h-2,lx-5,spr_pancon6->h-2);
    XSetForeground(disp,gcw,SCOL16);
    XDrawLine(disp,w,gcw,10,2,lx-5,2);
    XSetForeground(disp,gcw,SCOL17);
    XDrawLine(disp,w,gcw,10,1,lx-5,1);
    XSetForeground(disp,gcw,SCOL24);
    XDrawLine(disp,w,gcw,10,0,lx-5,0);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, 12, 5+fixy, cdir + dl, vl);
    }
    XSetForeground(disp, gcw, SCOL24);
    XDrawString(disp, w, gcw, 11, 4+fixy, cdir + dl, vl);
    break;
  };

  vlook_draw_column_separators();

  XSetForeground(disp,gcw,SCOL22);
  XDrawLine(disp,w,gcw,0,HEADH,0,h-20);
  XDrawLine(disp,w,gcw,l-3,HEADH,l-3,h-25);
  XSetForeground(disp,gcw,cols[0]);
  XDrawLine(disp,w,gcw,1,HEADH,1,h-20);
  XDrawLine(disp,w,gcw,l-2,HEADH,l-2,h-24);
  XSetForeground(disp,gcw,SCOL8);
  XDrawLine(disp,w,gcw,2,HEADH,2,h-20);
  XDrawLine(disp,w,gcw,l-1,HEADH,l-1,h-23);

  try_reread_dir();

  five_show_sprite(w,gcw,l,h,spr_arrow1);
  five_show_sprite(w,gcw,l,h,spr_arrow2);

  if (just_at_startup == 1 && panel == this)
  {
    guiSetInputFocus(disp, Main, RevertToNone, CurrentTime);
    just_at_startup = 2;
  };
  if (disable_reread != 2)
    showfinfo(cur,selfiles);
  else
    disable_reread = 0;
}

//----------------------------------------------------------------------------
// Draw column separators on panel if columns > 1
//----------------------------------------------------------------------------
void FiveLister::vlook_draw_column_separators()
{
    int i,px;

    if (col > 1)
    {
	XSetForeground(disp, gcw, SCOL8);
	for (i = 0, px = pixels_per_column+main_area.active_from_x; 
	     i < col - 1; 
	     i++, px += pixels_per_column)
	{
	    //	    XDrawLine(disp, w, gcw, px - 1, 
	    //		      main_area.active_from_y + Lister::main_area_column_border, 
	    //		      px - 1, main_area.active_from_y + main_area.active_h - 
	    //		      Lister::main_area_column_border*2);
	    XDrawLine(disp, w, gcw, px, 
		      main_area.active_from_y + Lister::main_area_column_border, 
		      px, main_area.active_from_y + main_area.active_h - 
		      Lister::main_area_column_border*2);
	}
    }
}


//----------------------------------------------------------------------------
// Recalculate all nessesary data for proper drawing
//----------------------------------------------------------------------------
void  FiveLister::vlook_recalc_all_data()
{
    xncdprintf(("Aqua: recalc_all_data called\n"));
    orientation=PageOrientation(lay);
    Lister::header_active_percent=50;
    Lister::header_height= 25 -
	(orientation==OnePage ? 0 : Lister::main_area_delta_y);
    Lister::header_menu_button_width=Lister::header_height;
    head.recalc_data(orientation,0,0,l,Lister::header_height);

    Lister::status_area_height=23;
    Lister::status_area_delta_y=0;
    Lister::status_area_button_l=40;

    status_area.recalc_data(0,h-1,l,fixh,fixy);

    calc_statusbar_offsets();

    main_area.recalc_data(0,head.h+head.y,
			  l,h-head.h-head.y-status_area.h,
			  pixels_per_column);
}

void   FiveLister::showdirname()
{
  char  *curdir = vfs->get_dir_header();
  int    sl = strlen(curdir), vl, fl, l2 = l / 2;
  int    dl = 0;
  switch (lay)
  {
  case 0:
    fl = l - 30;
    vl = fl / fixl;
    if (sl > vl)
      dl = sl - vl;
    vl = sl - dl;
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,11,0,fl+5,spr_pancon1->h);
    XSetForeground(disp,gcw,SCOL22);
    XDrawLine(disp,w,gcw,11,0,15+fl,0);
    XSetForeground(disp,gcw,SCOL7);
    XDrawLine(disp,w,gcw,11,1,11+fl,1);
    XSetForeground(disp,gcw,SCOL4);
    XDrawLine(disp,w,gcw,11,16,15+fl,16);
    XDrawLine(disp,w,gcw,11,17,15+fl,17);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, 15, 3+fixy, curdir + dl, vl);
    }
    XSetForeground(disp, gcw, cols[1]);
    XDrawString(disp, w, gcw, 14, 2+fixy, curdir + dl, vl);
    break;
  case 1:  //panel layout left-top side
    fl = (l - 40) / 2;
    vl = fl / fixl;
    if (sl > vl)
      dl = sl - vl;
    vl = sl - dl;
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,11,0,fl,spr_pancon1->h);
    XSetForeground(disp,gcw,SCOL22);
    XDrawLine(disp,w,gcw,11,0,11+fl,0);
    XSetForeground(disp,gcw,SCOL7);
    XDrawLine(disp,w,gcw,11,1,11+fl,1);
    XSetForeground(disp,gcw,SCOL4);
    XDrawLine(disp,w,gcw,11,16,11+fl,16);
    XDrawLine(disp,w,gcw,11,17,11+fl,17);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, 15, 3+fixy, curdir + dl, vl);
    }
    XSetForeground(disp, gcw, cols[1]);
    XDrawString(disp, w, gcw, 14, 2+fixy, curdir + dl, vl);
    break;
  case 2:
    fl = (l - 70) / 2;
    vl = fl / fixl;
    if (sl > vl)
      dl = sl - vl;
    vl = sl - dl;
    XSetForeground(disp,gcw,SCOL8);
    XFillRectangle(disp,w,gcw,l2+20,0,fl,spr_pancon1->h);
    XSetForeground(disp,gcw,SCOL22);
    XDrawLine(disp,w,gcw,l2+16,0,l2+20+fl,0);
    XSetForeground(disp,gcw,SCOL7);
    XDrawLine(disp,w,gcw,l2+16,1,l2+20+fl,1);
    XSetForeground(disp,gcw,SCOL4);
    XDrawLine(disp,w,gcw,l2+16,16,l2+20+fl,16);
    XDrawLine(disp,w,gcw,l2+16,17,l2+20+fl,17);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, l2 + 22, 3+fixy, curdir + dl, vl);
    }
    XSetForeground(disp, gcw, cols[1]);
    XDrawString(disp, w, gcw, l2 + 21, 2+fixy, curdir + dl, vl);
    break;
  }
}

void   FiveLister::redraw_statusbar()
{
  if(cur)
    showfinfo(cur,selfiles);
}

void   FiveLister::show_string_info(char *str)
{
  int    con1_x,con1_y,con2_x,con2_y;
  int stlx=7;
  int stll=l - 17 - spr_arrow1->l - spr_arrow2->l - 5;
  int ll=strlen(str);
  int ty=spr_statcon1->toy+4+fixy+h;

  if(spr_statcon1->tox<0)
    con1_x=spr_statcon1->tox+l;
  else
    con1_x=spr_statcon1->tox;

  if(spr_statcon2->tox<0)
    con2_x=spr_statcon2->tox+l;
  else
    con2_x=spr_statcon2->tox;
  if(spr_statcon1->toy<0)
    con1_y=spr_statcon1->toy+h;
  else
    con1_y=spr_statcon1->toy;
  if(spr_statcon2->toy<0)
    con2_y=spr_statcon2->toy+h;
  else
    con2_y=spr_statcon2->toy;

  XSetForeground(disp,gcw,SCOL8);
  XFillRectangle(disp, w, gcw, con1_x, con1_y, con2_x-con1_x, spr_statcon1->h);
  XSetForeground(disp,gcw,SCOL17);
  XDrawLine(disp, w, gcw,con1_x,con1_y,con2_x,con2_y);
  XSetForeground(disp,gcw,SCOL5);
  XDrawLine(disp, w, gcw,con1_x,con1_y+spr_statcon1->h-1,con2_x,con2_y+spr_statcon1->h-1);
  

  five_show_sprite(w,gcw,l,h,spr_statcon1);
  five_show_sprite(w,gcw,l,h,spr_statcon2);

  if(ll>stll/fixl)
    ll=stll/fixl;
  if(shadow)
  {
    XSetForeground(disp, gcw, cols[0]);
    XDrawString(disp, w, gcw, stlx+1,
		ty+1, str, ll);
  }
  XSetForeground(disp, gcw, SCOL21);
  XDrawString(disp, w, gcw, stlx,
	      ty, str, ll);
  XFlush(disp);
}

void   FiveLister::header_blink_other()
{
  if(lay==0)
  {
    panel2->header_blink();
    return;
  }
  char  *cdir = panel2->vfs->get_dir_header();
  int    sl = strlen(cdir), vl, fl, i, l2=l/2;
  fl = (l - 70) / 2;
  vl = fl / fixl;
  int    dl = 0;
  if (sl > vl)
    dl = sl - vl;
  vl = sl - dl;
  for(i=0;i<3;i++)
    if(lay==1)
    {
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, l2 + 31, 5+fixy, cdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[5]);
      XDrawString(disp, w, gcw, l2 + 30, 4+fixy, cdir + dl, vl);
      XSync(disp,0);
      delay(150);
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, l2 + 31, 5+fixy, cdir + dl, vl);
      }
      XSetForeground(disp, gcw, SCOL24);
      XDrawString(disp, w, gcw, l2 + 30, 4+fixy, cdir + dl, vl);
      XSync(disp,0);
      delay(150);
    } else
    {
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 12, 5+fixy, cdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[5]);
      XDrawString(disp, w, gcw, 11, 4+fixy, cdir + dl, vl);
      XSync(disp,0);
      delay(150);
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 12, 5+fixy, cdir + dl, vl);
      }
      XSetForeground(disp, gcw, SCOL24);
      XDrawString(disp, w, gcw, 11, 4+fixy, cdir + dl, vl);
      XSync(disp,0);
      delay(150);
    }
}

void   FiveLister::header_blink()
{
  int i;
  char  *curdir = vfs->get_dir_header();
  int    sl = strlen(curdir), vl, fl, l2 = l / 2;
  if (lay == 0)
    fl = l - 38;
  else
    fl = (l - 38) / 2;
  vl = fl / fixl;
  int    dl = 0;
  if (sl > vl)
    dl = sl - vl;
  vl = sl - dl;
  for(i=0;i<3;i++)
  {
    switch (lay)
    {
    case 0:
      XSetForeground(disp, gcw, keyscol[1]);
      XDrawString(disp, w, gcw, 31, 22, curdir + dl, vl);
      XDrawString(disp, w, gcw, 30, 21, curdir + dl, vl);
      XSync(disp,0);
      delay(150);
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 31, 22, curdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[5]);
      XDrawString(disp, w, gcw, 30, 21, curdir + dl, vl);
      XSync(disp,0);
      delay(150);
      break;
    case 1:
      fl = (l - 40) / 2;
      vl = fl / fixl;
      if (sl > vl)
	dl = sl - vl;
      vl = sl - dl;
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 15, 3+fixy, curdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[5]);
      XDrawString(disp, w, gcw, 14, 2+fixy, curdir + dl, vl);
      XSync(disp,0);
      delay(150);
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, 15, 3+fixy, curdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[1]);
      XDrawString(disp, w, gcw, 14, 2+fixy, curdir + dl, vl);
      XSync(disp,0);
      delay(150);
      break;
    case 2:
      fl = (l - 70) / 2;
      vl = fl / fixl;
      if (sl > vl)
	dl = sl - vl;
      vl = sl - dl;
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, l2 + 22, 3+fixy, curdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[5]);
      XDrawString(disp, w, gcw, l2 + 21, 2+fixy, curdir + dl, vl);

      XSync(disp,0);
      delay(150);
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, l2 + 22, 3+fixy, curdir + dl, vl);
      }
      XSetForeground(disp, gcw, cols[1]);
      XDrawString(disp, w, gcw, l2 + 21, 2+fixy, curdir + dl, vl);
      XSync(disp,0);
      delay(150);
      break;
    }
  }
}



//----------------------------------------------------------------------------
// Calculate offsets for status bar info
//----------------------------------------------------------------------------
void FiveLister::calc_statusbar_offsets()
{
  int stlx, stll,stly;
  stlx=7;
  stll=l - 17 - spr_arrow1->l - spr_arrow2->l - 5;
  stly=spr_statcon1->toy+4;
  if(stly<0)
    stly+=h;
  if(option_bits & STATUS_ATTR)
  {
    STL_ATTR_PTR.x=stlx;
    STL_ATTR_PTR.y=stly;
    STL_ATTR_PTR.ty=stly+fixy;
    STL_ATTR_PTR.tx=STL_ATTR_PTR.x+STL_SPACE2+1;
    STL_ATTR_PTR.tlen=STL_ATTR_NCHAR;
    STL_ATTR_PTR.len=STL_ATTR_NCHAR*fixl+STL_SPACE;
    stlx+=STL_ATTR_NCHAR*fixl+STL_SPACE+1;
    stll-=STL_ATTR_NCHAR*fixl+STL_SPACE+1;
  }
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_SIZE)
  {
    STL_SIZE_PTR.x=stlx+stll-STL_SIZE_NCHAR*fixl-STL_SPACE;
    STL_SIZE_PTR.y=stly;
    STL_SIZE_PTR.ty=stly+fixy;
    STL_SIZE_PTR.tx=STL_SIZE_PTR.x+STL_SPACE2+1;
    STL_SIZE_PTR.tlen=STL_SIZE_NCHAR;
    STL_SIZE_PTR.len=STL_SIZE_NCHAR*fixl+STL_SPACE;
    stll-=STL_SIZE_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
      option_bits&=~STATUS_SIZE;
  }
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_TIME)
  {
    STL_TIME_PTR.x=stlx+stll-STL_TIME_NCHAR*fixl-STL_SPACE;
    STL_TIME_PTR.y=stly;
    STL_TIME_PTR.ty=stly+fixy;
    STL_TIME_PTR.tx=STL_TIME_PTR.x+STL_SPACE2+1;
    STL_TIME_PTR.tlen=STL_TIME_NCHAR;
    STL_TIME_PTR.len=STL_TIME_NCHAR*fixl+STL_SPACE;
    stll-=STL_TIME_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
      option_bits&=~STATUS_TIME;
  }
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_OWNER)
  {
    STL_OWNER_PTR.x=stlx+stll-STL_OWNER_NCHAR*fixl-STL_SPACE;
    STL_OWNER_PTR.y=stly;
    STL_OWNER_PTR.ty=stly+fixy;
    STL_OWNER_PTR.tx=STL_OWNER_PTR.x+STL_SPACE2+1;
    STL_OWNER_PTR.tlen=STL_OWNER_NCHAR;
    STL_OWNER_PTR.len=STL_OWNER_NCHAR*fixl+STL_SPACE;
    stll-=STL_OWNER_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
      option_bits&=~STATUS_OWNER;
  }
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_NAME)
  {
    STL_NAME_PTR.x=stlx;
    STL_NAME_PTR.y=stly;
    STL_NAME_PTR.tx=STL_NAME_PTR.x+STL_SPACE2+1;
    STL_NAME_PTR.ty=stly+fixy;
    STL_NAME_PTR.tlen=(stll-STL_SPACE)/fixl;
    STL_NAME_PTR.len=stll;
  }
}

void   FiveLister::showfinfo(FList * fo, int files)
{
  static char fname[L_MAXPATH];
  static char flink[L_MAXPATH];
  int    con1_x,con1_y,con2_x,con2_y;
  int    i = 0, sl, vl = 0, dl = 0, ll;
  char   chr2[30];
  char   chr1[30];
  char   tchr[256];
  
  if(spr_statcon1->tox<0)
    con1_x=spr_statcon1->tox+l;
  else
    con1_x=spr_statcon1->tox;

  if(spr_statcon2->tox<0)
    con2_x=spr_statcon2->tox+l;
  else
    con2_x=spr_statcon2->tox;
  if(spr_statcon1->toy<0)
    con1_y=spr_statcon1->toy+h;
  else
    con1_y=spr_statcon1->toy;
  if(spr_statcon2->toy<0)
    con2_y=spr_statcon2->toy+h;
  else
    con2_y=spr_statcon2->toy;

  XSetForeground(disp,gcw,SCOL8);
  XFillRectangle(disp, w, gcw, con1_x, con1_y, con2_x-con1_x, spr_statcon1->h);
  XSetForeground(disp,gcw,SCOL17);
  XDrawLine(disp, w, gcw,con1_x,con1_y,con2_x,con2_y);
  XSetForeground(disp,gcw,SCOL5);
  XDrawLine(disp, w, gcw,con1_x,con1_y+spr_statcon1->h-1,con2_x,con2_y+spr_statcon1->h-1);
  

  five_show_sprite(w,gcw,l,h,spr_statcon1);
  five_show_sprite(w,gcw,l,h,spr_statcon2);

  if(option_bits & STATUS_ATTR)
  {
    dig2ascii_r(chr1, fo->mode & 07777, 4, 8, '0');
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, STL_ATTR_PTR.tx+1,
		  STL_ATTR_PTR.ty+1, chr1, 4);
    }
    XSetForeground(disp, gcw, SCOL21);
    XDrawString(disp, w, gcw, STL_ATTR_PTR.tx,
		STL_ATTR_PTR.ty, chr1, 4);
      
  }
  if(option_bits & STATUS_TIME)
  {
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, STL_TIME_PTR.tx+1,
		  STL_TIME_PTR.ty+1, fo->chr_time, STL_TIME_NCHAR);
    }
    XSetForeground(disp, gcw, SCOL21);
    XDrawString(disp, w, gcw, STL_TIME_PTR.tx,
		STL_TIME_PTR.ty, fo->chr_time, STL_TIME_NCHAR);
      
  }
  if(option_bits & STATUS_OWNER)
  {
    ll=sprintf(tchr,"%s.%s", fo->user, fo->group);
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, STL_OWNER_PTR.tx+1,
		  STL_OWNER_PTR.ty+1, tchr, ll);
    }
    XSetForeground(disp, gcw, SCOL21);
    XDrawString(disp, w, gcw, STL_OWNER_PTR.tx,
		STL_OWNER_PTR.ty, tchr, ll);
      
  }
  if(option_bits & STATUS_SIZE)
  {
    if (files == 0)
    {
      if (fo->mode & S_IFDIR)
      {
	strcpy(chr2, ">DIR<");
	dig2ascii_r(chr2+5, fo->size, 6);
      }
      else
	dig2ascii(chr2, fo->size);
    }
    else
    {
      dig2ascii(chr2, selsize);
    }
    vl=strlen(chr2);
    dl=STL_SIZE_PTR.tlen;
      
    if(shadow)
    {
      XSetForeground(disp, gcw, cols[0]);
      XDrawString(disp, w, gcw, STL_SIZE_PTR.tx+1+(dl-vl)*fixl,
		  STL_SIZE_PTR.ty+1, chr2, vl);
    }
    XSetForeground(disp, gcw, SCOL21);
    XDrawString(disp, w, gcw, STL_SIZE_PTR.tx+(dl-vl)*fixl,
		STL_SIZE_PTR.ty, chr2, vl);
  }

  if(option_bits & STATUS_NAME)
  {
    int tlen=STL_NAME_PTR.tlen;
    vl=STL_NAME_PTR.tlen;
    dl=0;
    if ((fo->mode & S_IFLNK) == S_IFLNK && files == 0)
    {
      vfs->ch_curdir();
      ll=vfs->readlink(fo, flink, 512);
      flink[ll]=0;
      if (ll < tlen-3)
      {
	sl = strlen(fo->name);
	if (sl + ll > tlen)
	{
	  for (i = ll; i >= 0; i--)
	    fname[tlen - 1 - ll + i] = flink[i];
	  fname[tlen - 2 - ll] = '>';
	  fname[tlen - 3 - ll] = '-';
	  for (i = sl - 1; i >= 0; i--)
	    if (tlen - 4 - sl + i < 0)
	      break;
	    else
	      fname[tlen - sl + i] = fo->name[i];
	}
	else
	{
	  strcpy(fname, fo->name);
	  strcat(fname, "->");
	  strcat(fname, flink);
	}
      }
      else
        strcpy(fname, flink + ll - tlen - 3);
      sl = strlen(fname);
      if (sl > vl)
        dl = sl - vl;
      vl = sl - dl;
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, STL_NAME_PTR.tx+1, STL_NAME_PTR.ty + 1, fname + dl, vl);
      }
      XSetForeground(disp, gcw, SCOL21);
      XDrawString(disp, w, gcw, STL_NAME_PTR.tx, STL_NAME_PTR.ty, fname + dl, vl);
    }
    else
    {
      char *pname;
      if(files)
      {
        pname=tchr;
        dig2ascii_r(pname, files, 4);
        strcat(pname, " file(s) selected");
      }
      else
 	    {
		if(fo->get_dir())
		    pname=fo->get_dir();
		else
		    pname=fo->name;
	    }
      
      sl = strlen(pname);
      if (sl > vl)
        dl = sl - vl;
      vl = sl - dl;
      if(shadow)
      {
	XSetForeground(disp, gcw, cols[0]);
	XDrawString(disp, w, gcw, STL_NAME_PTR.tx+1, STL_NAME_PTR.ty + 1, pname + dl, vl);
      }
      XSetForeground(disp, gcw, SCOL21);
      XDrawString(disp, w, gcw, STL_NAME_PTR.tx, STL_NAME_PTR.ty, pname + dl, vl);
    }
  }
  if(option_bits & STATUS_INODE)
    show_inodeinfo(fo);
}
