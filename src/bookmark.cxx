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
#include "bookmark.h"
#include "recycle.xbm"
#include "recmask.xbm"
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
#include "box.xbm"
#include "image.h"
#include "qhelp.h"
#include "au_sup.h"
#include "c_externs.h"
#include "internals.h"
#include "xnimlib.h"
#include "pluginman.h"
#include "xshrloader.h"

extern Lister *dnd_creator;
extern int dnd_startx, dnd_starty;
extern VFS *define_vfs(char *type, char *d);
extern void senddir(char *d);
extern int layout;



Cursor rw_cur;
#define        STICKY_CENTER   20
#define        MIN_PERCENT    20

char  *fschr[] =
{"DFS", "ARC", "FTP"};
int    fsmax = 3;

Pixmap       recyclepix = 0;
Pixmap       recyclemask = 0;
unsigned int recycle_l, recycle_h;
Pixmap       pixrecycle[4]={0,0,0,0};
Pixmap       maskrecycle[4]={0,0,0,0};
unsigned int l_recycle[4], h_recycle[4];
int          cur_recycle = 0;



char *vfs_iname(char *vfsname, char *path)
{
 if(*vfsname!='A')
   return vfsname;
 SUP *sp=find_support(path);
 if(sp)
   return sp->name;
 return vfsname;
}



// *INDENT-OFF*        
//-------------------------------------------------------------------------
// Constructor - set up variables
//-------------------------------------------------------------------------
BookMark::BookMark(int il, int ih):Gui()
// *INDENT-ON*        

{
  x = -1;
  y = 20;
  l = il;
  h = ih;
  foc = 0;
  hflg = 0;
  dflg = 1;
  escfunc = NULL;
  guitype = GUI_BOOKMARK;
  guiobj = NULL;
  maxbooks = 9;
  last_used = 1;
  percent = 50;
  for (int i = 0; i <= 9; i++)
    {
      act_books[i] = 0;
      books[i] = NULL;
      flagpix[i]=0;
    }
  add_book(0, getenv("HOME"), DFS_TYPE);
}


//-------------------------------------------------------------------------
// Desctructor - cleans the garbage :)
//-------------------------------------------------------------------------
BookMark::~BookMark()
{
  int i;
  if(disp) 
  {
    XFreeGC(disp,gcw);
    if(rw)
      XFreeGC(disp,gcrw);
    if(flag_anim)
      for(i=0;i<7;i++)
	if(flagpix[i]) XFreePixmap(disp,flagpix[i]);
    for(i=0;i<9;i++)
      if(books[i]) delete books[i];
    XFreePixmap(disp,a1pix);
    XFreePixmap(disp,a2pix);
  };
  hflg=0;
  dflg=0;
}


//-------------------------------------------------------------------------
// Create bookmark window and pixmaps
//-------------------------------------------------------------------------
void   BookMark::init(Window ipar)
{
  Window wtemp;
  XSetWindowAttributes xswa;
  unsigned u, pl, ph;
  int    itemp;
  parent = ipar;
  page_h = (h - 75) / 9;
  maxy = 25 + maxbooks * page_h;
  l = 30;
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
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, normal_bg_color);
  XChangeWindowAttributes(disp, w, CWWinGravity, &xswa);
  gcv.background = normal_bg_color;
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
  XSetForeground(disp, rgc, normal_bg_color);
  recyclemask = 0;
  if (recyclepix == 0)
    {
      recyclepix = pixrecycle[0] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      recycle_l = l_recycle[0] = recycle_width;
      recycle_h = h_recycle[0] = recycle_height;
      pixrecycle[1] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[1] = recycle_width;
      h_recycle[1] = recycle_height;
      pixrecycle[2] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[2] = recycle_width;
      h_recycle[2] = recycle_height;
      pixrecycle[3] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
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
              cols[3], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
  a2pix = XCreatePixmapFromBitmapData(disp, w, (char *)a2_bits, a2_width, a2_height,
              cols[2], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
  qh.init(Main);
  pagenum = -1;
  rw_cur = XCreateFontCursor(disp, XC_sb_h_double_arrow);
}


//-------------------------------------------------------------------------
// Store bookmark info into ConfigManager for saving
//-------------------------------------------------------------------------
void   BookMark::save_books(ConfigManager* cfgman)
{
  char buf1[L_MAXPATH], buf2[L_MAXPATH];
  for (int i = 0; i < 9; i++)
  {
      sprintf(buf1, "bookmark.book%d", i+1);
      if (act_books[i])
      {
	  sprintf(buf2, "%s:%s", fs[i], books[i]);
	  cfgman->update_value(buf1, buf2);
      } else
	  cfgman->update_value(buf1, "");
  }
	
}


void   BookMark::del_book(int i)
{
  if (act_books[i] == 0)
    return;
  if (i == last_used)
    last_used = 8;
  act_books[i] = 0;
  if (allow_bookmark)
    {
      XClearWindow(disp, w);
      expose();
    }
}

void   BookMark::clear_and_expose()
{
  XClearWindow(disp, w);
  expose();
}

//Return VFS constructed from bookmark
//NULL if empty
VFS   *BookMark::get_vfs_by_coord(int iy)
{
  iy -= 25;
  iy /= page_h;
  if (iy > 8)
    return NULL;
  if (act_books[iy] == 0)
    return NULL;
  return define_vfs(fs[iy], books[iy]);
}

void   BookMark::free_pix()
{
  int i;
  for(i=0;i<max_anims;i++)
    if(flagpix[i]!=0)
    {
      default_loader->free_pixmap(flagpix[i]);
      flagpix[i]=0;
    }
  for(i=0;i<4;i++)
    if(pixrecycle[i]!=0)
    {
      default_loader->free_pixmap(pixrecycle[i]);
      pixrecycle[i]=0;
    }
}


int   BookMark::load_pixmap(int idx)
{
  int    r_l, r_h;
  char   tmp[128];

  if(vclass==PseudoColor)
    return 0;

  switch(idx)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:

      sprintf(tmp, "xnci_flag%d", idx + 1);
      flagpix[idx] = default_loader->load_pixmap(ResId(tmp,flags_chr[idx],flags_sizes[idx]),
						 flagl,flagh);
      return 1;

    case 7:
      pixrecycle[0] = recyclepix = default_loader->load_pixmap(
							       ResId("xnci_recycle",
								     recycle_chr,
								     recycle_chr_size),
							       r_l,r_h);
      l_recycle[0] = recycle_l = (unsigned)r_l;
      h_recycle[0] = recycle_h = (unsigned)r_h;
      recyclemask = maskrecycle[0] = XCreateBitmapFromData(disp, Main, (char *)
                                                           recmask_bits, 
                                                           recmask_width, 
                                                           recmask_height);
      return 1;

    case 8:
      pixrecycle[1] = default_loader->load_pixmap(ResId("xnci_recycle2",
							recycle2_chr,
							recycle2_chr_size),
						  r_l,r_h);
      l_recycle[1] = (unsigned)r_l;
      h_recycle[1] = (unsigned)r_h;
      maskrecycle[1] = XCreateBitmapFromData(disp, Main, (char *)rec2m_bits, 
                                             rec2m_width, rec2m_height);
      return 1;

    case 9:
      pixrecycle[2] = default_loader->load_pixmap(ResId("xnci_recycle3",
							recycle3_chr,
							recycle3_chr_size),
						  r_l,r_h);
      l_recycle[2] = (unsigned)r_l;
      h_recycle[2] = (unsigned)r_h;
      maskrecycle[2] = XCreateBitmapFromData(disp, Main, (char *)rec3m_bits, 
                                             rec3m_width, rec3m_height);
      return 1;

    case 10:
      pixrecycle[3] = default_loader->load_pixmap(ResId("xnci_recycle4",
							recycle3_chr,
							recycle3_chr_size),
						  r_l,r_h);
      l_recycle[3] = (unsigned)r_l;
      h_recycle[3] = (unsigned)r_h;
      maskrecycle[3] = XCreateBitmapFromData(disp, Main, (char *)rec4m_bits, 
                                             rec4m_width, rec4m_height);
      max_anims=3;       
      return 0;


    }
  return 0;
}

void   BookMark::load_flags(int n)
{
  char tmp[128];
  int    i = 0;
  flag_anim = 1;
  max_anims = n;
  int    r_l, r_h;
  if (vclass != PseudoColor && !is_fast_load() && !is_back_load())
    {
      pixrecycle[0] = recyclepix = default_loader->load_pixmap(
							       ResId("xnci_recycle",
								     recycle_chr,
								     recycle_chr_size),
							       r_l,r_h);
      l_recycle[0] = recycle_l = (unsigned)r_l;
      h_recycle[0] = recycle_h = (unsigned)r_h;
      show_dot();
      pixrecycle[1] = default_loader->load_pixmap(ResId("xnci_recycle2",
							recycle2_chr,
							recycle2_chr_size),
						  r_l,r_h);
      l_recycle[1] = (unsigned)r_l;
      h_recycle[1] = (unsigned)r_h;
      show_dot();
      pixrecycle[2] = default_loader->load_pixmap(ResId("xnci_recycle3",
							recycle3_chr,
							recycle3_chr_size),
						  r_l,r_h);
      l_recycle[2] = (unsigned)r_l;
      h_recycle[2] = (unsigned)r_h;
      show_dot();
      pixrecycle[3] = default_loader->load_pixmap(ResId("xnci_recycle4",
							recycle3_chr,
							recycle3_chr_size),
						  r_l,r_h);
      l_recycle[3] = (unsigned)r_l;
      h_recycle[3] = (unsigned)r_h;
      show_dot();
      recyclemask = maskrecycle[0] = XCreateBitmapFromData(disp, Main, (char *)
                                                           recmask_bits, 
                                                           recmask_width, 
                                                           recmask_height);
      maskrecycle[1] = XCreateBitmapFromData(disp, Main, (char *)rec2m_bits, 
                                             rec2m_width, rec2m_height);
      maskrecycle[2] = XCreateBitmapFromData(disp, Main, (char *)rec3m_bits, 
                                             rec3m_width, rec3m_height);
      maskrecycle[3] = XCreateBitmapFromData(disp, Main, (char *)rec4m_bits, 
                                             rec4m_width, rec4m_height);
    } else
    {
      recyclepix = pixrecycle[0] = 
        XCreatePixmapFromBitmapData(disp, Main, 
                                    (char *)recycle_bits, recycle_width,
                                    recycle_height,cols[0], 
                                    normal_bg_color, 
                                    DefaultDepth(disp, DefaultScreen(disp)));
      recycle_l = l_recycle[0] = recycle_width;
      recycle_h = h_recycle[0] = recycle_height;
      pixrecycle[1] = XCreatePixmapFromBitmapData(disp, Main, 
                                                  (char *)recycle_bits, 
                                                  recycle_width, 
                                                  recycle_height,
                                                  cols[0], normal_bg_color, 
                                                  DefaultDepth(disp, 
                                                               DefaultScreen(disp)));
      l_recycle[1] = recycle_width;
      h_recycle[1] = recycle_height;
      pixrecycle[2] = XCreatePixmapFromBitmapData(disp, Main, (char *)
                                                  recycle_bits, recycle_width,
                                                  recycle_height,
                                                  cols[0], normal_bg_color, 
                                                  DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[2] = recycle_width;
      h_recycle[2] = recycle_height;
      pixrecycle[3] = XCreatePixmapFromBitmapData(disp, Main, (char *)recycle_bits, recycle_width, recycle_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      l_recycle[3] = recycle_width;
      h_recycle[3] = recycle_height;

      recyclemask = maskrecycle[0] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[1] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[2] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);
      maskrecycle[3] = XCreateBitmapFromData(disp, Main, (char *)box_bits, box_width, box_height);

    }

  XSetClipMask(disp, gcm, recyclemask);
  cur_recycle = 0;
  XSetClipOrigin(disp, gcm, l / 2 - recycle_l / 2, h - recycle_h - 3);

  if (vclass != PseudoColor && allow_animation && !is_fast_load() && !is_back_load())
    for (i = 0; i < n; i++)
      {
        sprintf(tmp, "xnci_flag%d", i + 1);
        flagpix[i] = default_loader->load_pixmap(ResId(tmp,flags_chr[i],flags_sizes[i]),
						 flagl,flagh);
        if (flagpix[i] == 0)
          {
            flag_anim = 0;
            break;
          };
      show_dot();
      }
  if (i < n)
    {
      max_anims = 8;
      flag_anim = 1;
      flagpix[0] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[1] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir1_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[2] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir2_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[3] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir3_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[4] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir4_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[5] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir5_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[6] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir6_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagpix[7] = XCreatePixmapFromBitmapData(disp, Main, (char *)cir7_bits, cir_width, cir_height,
              cols[0], normal_bg_color, DefaultDepth(disp, DefaultScreen(disp)));
      show_dot();
      flagl = (int)cir_width;
      flagh = (int)cir_height;
    }
  cur_flag = 0;
  flag_coun = 0;
  flag_anim = allow_animation;
}

void   BookMark::animate()
{
  if (flag_anim && mapped && allow_animation)
    {
      flag_coun++;
      if (flag_coun > 40)
        {
          flag_coun = 0;
          cur_flag++;
          if (cur_flag >= max_anims)
            cur_flag = 0;
          XCopyArea(disp, flagpix[cur_flag], w, gcw, 0, 0, flagl, flagh, l / 2 - flagl / 2, 0);
          XSync(disp, 0);
        }
    }
}


void   BookMark::animate_moving(int i)
{
  if(!allow_animation)
          return;
  int    ix = x + l - 1, iy = y + 25 + page_h * (i + 1) - 1;
  int    il = l - 1, ih = page_h - 1;
  int    bh = ih, bl = il;
  int    bx = ix, by = iy;
  int    istep = 0;
  int    dx=0, dy = panel->y + 25 - by;
  int    dl=0, dh = 25 - ih;
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

void   BookMark::animate_swapping(int i)
{
  if(!allow_animation)
          return;
  int    ix = x + l - 1, iy = y + 25 + page_h * (i + 1) - 1;
  int    il = l - 1, ih = page_h - 1;
  int    bh = ih, bl = il;
  int    bx = ix, by = iy;
  int    istep = 0;
  int    dx=0, dy = panel->y + 25 - by;
  int    dl=0, dh = 25 - ih;
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


void   BookMark::blink_book(int n, int blinks)
{
  int    iy = n * page_h + 25;
  if (allow_bookmark == 0)
    return;
  while (blinks)
    {
/*XFR
      XSetForeground(disp, gcw, normal_bg_color);
      XFillRectangle(disp, w, gcw, 0, iy, l, page_h);
*/
      XClearArea(disp, w, 0, iy, l, page_h, 0);
      XSync(disp, 0);
      delay(150);
      if (n == last_used)
        draw_selected_page(n);
      else
        draw_page(n);
      XSync(disp, 0);
      delay(150);
      blinks--;
    }
}

int    detect_fs(char *s)
{
  for (int i = 0; i < fsmax; i++)
    if (strncmp(s, fschr[i], 3) == 0)
      return i;
  return 0;
}

void   BookMark::add_toempty_book(char *s, int type)
{
  for (int i = 0; i < 9; i++)
    if (act_books[i] == 0)
      {
        add_book(i, s, type);
        if (allow_bookmark)
          expose();
        break;
      }
}

void   BookMark::add_toempty_book_by_coord(int cy, char *s, int type)
{
  int    i = (cy - 25) / page_h;
  if (act_books[i] == 0)
    {
      add_book(i, s, type);
      if (allow_bookmark)
        expose();
    }
}

int    BookMark::add_book(int n, char *s, int type)
{
  int    k;
  if(s[0]==0)
    return 0;
  if ((k = is_in_bookmark(s)) != -1)
    {
      blink_book(k, 3);
      return 0;
    }
  if (books[n] == NULL)
    books[n] = new char[1024];
  if (*s == '/' && *(s + 1) == '/')
    s++;
  strcpy(books[n], s);
  act_books[n] = 1;
  last_used = n;
  strcpy(fs[n], fschr[type]);
  xncdprintf(("Bookmark adding book [%s] with type [%s]\n",s,fschr[type]));
  return 1;
}

void   BookMark::draw_page(int n)
{
  int    iy = n * page_h + 25;
  char   str[3];
  str[0] = n + '1';
  str[1] = 0;
  XClearArea(disp, w, 0, iy, l, page_h, 0);
  if (n < 8)
    XCopyArea(disp, a1pix, w, gcw, 0, 0, a1_width, a1_height, 3, iy + 6);
  else
    XCopyArea(disp, a2pix, w, gcw, 0, 0, a2_width, a2_height, 3, iy + 6);
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, 0, iy, l - 6, iy);
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, l - 5, iy + 1, l - 1, iy + 5);
  XDrawLine(disp, w, gcw, l - 1, iy + 5, l - 1, iy + page_h - 7);
// XSetForeground(disp,gcw,cols[0]);
  XDrawLine(disp, w, gcw, 0, iy + page_h - 1, l - 6, iy + page_h - 1);
  XDrawLine(disp, w, gcw, l - 5, iy + page_h - 2, l - 1, iy + page_h - 6);
  XDrawString(disp, w, gcw, l / 2, iy + 4 + fixfontstr->max_bounds.ascent, str, 1);
  if (page_h > ty + ty2 + 10)
    {
      XSetFont(disp, gcw, mfixfontstr->fid);
      XDrawString(disp, w, gcw, 3, iy + (page_h - 4 - ty) / 2 + ty + mfixfontstr->max_bounds.ascent, fs[n], 3);
      XSetFont(disp, gcw, fixfontstr->fid);
    }
}

void   BookMark::draw_selected_page(int n)
{
  int    iy = n * page_h + 25;
  char   str[3];
  str[0] = n + '1';
  str[1] = 0;
  XClearArea(disp, w, 0, iy, l, page_h, 0);
  if (n < 8)
    XCopyArea(disp, a1pix, w, gcw, 0, 0, a1_width, a1_height, 3, iy + 6);
  else
    XCopyArea(disp, a2pix, w, gcw, 0, 0, a2_width, a2_height, 3, iy + 6);
  XSetForeground(disp, gcw, window_text_color);
  XDrawString(disp, w, gcw, l / 2, iy + 3 + fixfontstr->max_bounds.ascent, str, 1);
  if (page_h > ty + ty2 + 10)
    {
      XSetFont(disp, gcw, mfixfontstr->fid);
      XDrawString(disp, w, gcw, 3, iy + ty + (page_h - 4 - ty) / 2 + mfixfontstr->max_bounds.ascent, fs[n], 3);
      XSetFont(disp, gcw, fixfontstr->fid);
    }
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, 0, iy, l - 6, iy);
  XDrawLine(disp, w, gcw, 0, iy + 1, l - 7, iy + 1);
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, l - 5, iy + 1, l - 1, iy + 5);
  XDrawLine(disp, w, gcw, l - 6, iy + 1, l - 2, iy + 5);
  XDrawLine(disp, w, gcw, l - 1, iy + 5, l - 1, iy + page_h - 8);
  XDrawLine(disp, w, gcw, l - 2, iy + 5, l - 2, iy + page_h - 8);

  XDrawLine(disp, w, gcw, 0, iy + page_h - 2, l - 6, iy + page_h - 2);
  XDrawLine(disp, w, gcw, 0, iy + page_h - 3, l - 6, iy + page_h - 3);
  XDrawLine(disp, w, gcw, l - 5, iy + page_h - 3, l - 1, iy + page_h - 7);
  XDrawLine(disp, w, gcw, l - 6, iy + page_h - 3, l - 2, iy + page_h - 7);
  XSetForeground(disp, gcw, shadow_color);
  XDrawLine(disp, w, gcw, 0, iy + page_h - 1, l - 6, iy + page_h - 1);
  XDrawLine(disp, w, gcw, l - 5, iy + page_h - 2, l - 2, iy + page_h - 5);
}

// ix - position in X coord
// iy - position int Y coord
// ny - width of down menu+commandline
void   BookMark::create_listers(Lister ** l1, Lister ** l2, int ix, int iy, int ny)
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
      rw = XCreateSimpleWindow(disp, parent, rwx, rwy, 2, Mainh - ny- iy, 0, 0, normal_bg_color);
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
      rw = XCreateSimpleWindow(disp, parent, rwx, rwy, Mainl - l, 2, 0, 0, normal_bg_color);
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


void   BookMark::set_recycle_image(int i)
{
  cur_recycle = i;
  recycle_l = l_recycle[i];
  recycle_h = h_recycle[i];
  recyclepix = pixrecycle[i];
  recyclemask = maskrecycle[i];
  XSetClipMask(disp, gcm, recyclemask);
  XSetClipOrigin(disp, gcm, l / 2 - recycle_l / 2, h - recycle_h - 3);
  XClearArea(disp, w, 0, h - 50, l, 50, 0);
  XCopyArea(disp, recyclepix, w, gcm, 0, 0, recycle_l, recycle_h, l / 2 - recycle_l / 2, h - recycle_h - 3);
  XFlush(disp);
}

void   BookMark::expose()
{
  int    i;
  if (allow_bookmark == 0)
    return;
  if (flag_anim)
    XCopyArea(disp, flagpix[cur_flag], w, gcw, 0, 0, flagl, flagh, l / 2 - flagl / 2, 0);
  XCopyArea(disp, recyclepix, w, gcm, 0, 0, recycle_l, recycle_h, l / 2 - recycle_l / 2, h - recycle_h - 3);
  for (i = 0; i < 9; i++)
    if (i != last_used && act_books[i])
      draw_page(i);
  if (act_books[last_used])
    draw_selected_page(last_used);
  hflg = 1;
}

void   BookMark::rw_expose()
{
  if (layout)
    {
      XSetForeground(disp, gcrw, light_bg_color);
      XDrawLine(disp, rw, gcrw, 0, 0, 0, h);
      XSetForeground(disp, gcrw, dark_bg_color);
      XDrawLine(disp, rw, gcrw, 1, 0, 1, h);
    }
  else
    {
      XSetForeground(disp, gcrw, light_bg_color);
      XDrawLine(disp, rw, gcrw, 0, 0, Mainl - l, 0);
      XSetForeground(disp, gcrw, dark_bg_color);
      XDrawLine(disp, rw, gcrw, 0, 1, Mainl - l, 1);
    }
}

void   BookMark::rebuild_listers()
{
  int    mll;
  if (layout == 1)
    {
      mll = (Mainl - l) * percent / 100 - listx;
      lst1->reconfigure_without_expose(listx, listy, mll - 1, Mainh - listy - listny);
      lst2->reconfigure_without_expose(mll + 1 + listx, listy, Mainl - l - mll - 1, Mainh - listy - listny);
      rwx = mll - 1;
      rwy = listy;
      leftrx = (Mainl - l) * MIN_PERCENT / 100;
      rightrx = Mainl - l - leftrx;
      XMoveResizeWindow(disp, rw, rwx, rwy, 2, h);
    }
  else if (layout == 0)
    {
      mll = (Mainh - listny - listy) * percent / 100;
      lst1->reconfigure_without_expose(listx, listy, Mainl - l, mll - 1);
      lst2->reconfigure_without_expose(listx, listy + mll + 1, Mainl - l, Mainh - 1 - listy - listny - mll);
      rwx = 0;
      rwy = listy - 1 + mll;
      leftrx = (Mainh - listny - listy) * MIN_PERCENT / 100;
      rightrx = Mainh - listny - listy - leftrx;
      XMoveResizeWindow(disp, rw, rwx, rwy, Mainl - l, 2);
    }
}

void   BookMark::reconfigure(int ix, int iy, int il, int ih)
{
  int    mll;
  h = ih;
  listy=iy;
  y=iy;
  page_h = (h - 75) / 9;
  maxy = 25 + maxbooks * page_h;
  if (allow_bookmark == 0)
    l = 0;
  switch (layout)
    {                                //3

    case 1:
      mll = (Mainl - l) * percent / 100;
      lst1->reconfigure(0, iy, mll - 1, Mainh - listy - listny);
      lst2->reconfigure(mll + 1, iy, Mainl - l - mll - 1, Mainh - listy - listny);
      rwx = mll - 1;
      rwy = iy;
      leftrx = (Mainl - l) * MIN_PERCENT / 100;
      rightrx = Mainl - l - leftrx;
      XMoveResizeWindow(disp, rw, rwx, rwy, 2, h);
      rw_expose();
      break;
    case 0:
      mll = (Mainh - listy - listny) * percent / 100;
      lst1->reconfigure(0, iy, Mainl - l, mll - 1);
      lst2->reconfigure(0, iy+1 + mll, Mainl - l, Mainh - 1 - listny - listy - mll);
      rwx = 0;
      rwy = iy + mll - 1;
      leftrx = (Mainh - listy - listny) * MIN_PERCENT / 100;
      rightrx = Mainh - listy - listny - leftrx;
      XMoveResizeWindow(disp, rw, rwx, rwy, Mainl - l, 2);
      rw_expose();
      break;
    case 2:
      lst1->reconfigure(0, iy, Mainl - l, Mainh - listy - listny);
      lst2->reconfigure(0, iy, Mainl - l, Mainh - listy - listny);
      break;
    };                                //-3

  if (x)
    x = il - l - 1;
  XSetClipOrigin(disp, gcm, l / 2 - recycle_l / 2, h - recycle_h - 3);
  if (allow_bookmark == 0)
    return;
  XMoveResizeWindow(disp, w, x, y, l, h);
  XClearWindow(disp, w);
}


void   BookMark::swap_books(int i)
{
  char   t[1024];
  int    typ = DFS_TYPE;
  if (allow_bookmark == 0)
    return;
  if (act_books[i] == 0)
    {
      add_book(i, panel->vfs->get_info_for_bookmark(), panel->vfs->fstype);
      last_used = i;
      expose();
    }
  else
    {
      strcpy(t, books[i]);
      typ=detect_fs(fs[i]);
      if (flag_anim && mapped)
        animate_swapping(i);
      strcpy(books[i], panel->vfs->get_info_for_bookmark());
      strcpy(fs[i], fschr[panel->vfs->fstype]);
      if (i != last_used)
        {
          if (act_books[last_used])
            draw_page(last_used);
          draw_selected_page(i);
          last_used = i;
        }
      else
        draw_selected_page(i);
      panel->switch_to_vfs(typ, t);
    }
}

int    BookMark::is_in_bookmark(char *s)
{
  for (int i = 0; i < 9; i++)
    if (act_books[i])
      if (strcmp(s, books[i]) == 0)
        return i;
  return -1;
}

void   BookMark::switch_books(int i)
{
  int    typ=VFS_TYPE;
  if (allow_bookmark == 0)
    return;
  if (act_books[i] == 0)
    {
      if (add_book(i, panel->vfs->get_info_for_bookmark(), panel->vfs->fstype))
        {
          last_used = i;
          expose();
        }
    }
  else
    {
      if (is_in_bookmark(panel->vfs->get_info_for_bookmark()) == -1)
        {
          if (act_books[8] == 0)
            {
              add_book(8, panel->vfs->get_info_for_bookmark(), panel->vfs->fstype);
              last_used = i;
              expose();
            }
          else
            add_book(8, panel->vfs->get_info_for_bookmark(), panel->vfs->fstype);
        }
      if (flag_anim && mapped)
        animate_moving(i);
      typ=detect_fs(fs[i]);
      panel->switch_to_vfs(typ, books[i]);
      if (i != last_used)
        {
          if (act_books[last_used])
            draw_page(last_used);
          draw_selected_page(i);
          last_used = i;
        }
      else
        draw_selected_page(i);
    }
}

void   BookMark::only_switch_to_exist_book(int i)
{
  int    typ;
  if (act_books[i] == 0)
    return;
  typ=detect_fs(fs[i]);
  panel->switch_to_vfs(typ, books[i]);
  last_used = i;
}

void   BookMark::show_dir_banner(int iy)
{
  int    k;
  if (iy > 25 && iy < h - 51)
    {
      k = (iy - 25) / page_h;
      if (k < 9)
        {
          if (k != pagenum && act_books[k])
            {
              pagenum = k;
              qh.show_this_info(vfs_iname(fs[k], books[k]), books[k], -x, 53 + page_h * k);
              qh.show();
            }
//    if(cur_recycle!=0) set_recycle_image(0);
        }
    }
  else
    {
      qh.hide();
      pagenum = -1;
//     if(iy>h-51)
   //      set_recycle_image(1);
    };
}

void   BookMark::hide_dir_banner()
{
  qh.hide();
  pagenum = -1;
}

extern int dnd_enabled;
void   BookMark::click()
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
          XQueryPointer(disp,w,&rrw,&cw,&rx,&ry,&cx,&cy,&kmask);
          rx = ev.xbutton.x;
          ry = ev.xbutton.y;
          kmask = ev.xbutton.state;
          if (kmask & Button1Mask)
            {
              if ((abs(dnd_starty - cy) > 5 || 
		   abs(dnd_startx - cx) > 10) && dnd_enabled == 0)
                {
		  start_dragging((ry - 25) / page_h);
		  break;
                }
            }

          if (ry > 25 && ry < h - 50)
	  {
              k = (ry - 25) / page_h;
              if (k < 9)
	      {
                  if (k != pagenum && act_books[k])
		  {
                      pagenum = k;
                      qh.show_this_info(vfs_iname(fs[k], books[k]),
					books[k], -x, 53 + page_h * k);
                      qh.show();
		  }
	      }
	  }
          else if (ry > h - 51)
	  {
              if (pagenum != -2)
	      {
                  qh.show_this_info(NULL, _("Recycle Bin"), -x, h - 25);
                  qh.show();
                  pagenum = -2;
	      }
	  }
          else
	  {
              qh.hide();
              pagenum = -1;
	  };
          break;

        case ButtonRelease:
	    if (ev.xbutton.y > 25 && ev.xbutton.y < maxy)
            {
		k = (ev.xbutton.y - 25) / page_h;
		if (act_books[k])
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
					  books[last_used], -x, 
					  53 + page_h * last_used);
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

void   BookMark::show()
{
  if (allow_bookmark)
    {
// *INDENT-OFF*        
      Gui::show();
// *INDENT-ON*        

      if (rw)
        {
          XMapWindow(disp, rw);
          addto_el(this, rw);
          XDefineCursor(disp, rw, rw_cur);
        }
      if(default_dnd_man)
      {
	  default_dnd_man->add_dnd_object(this,w);
	  dndwin->add_type("text/uri-list");
      }
    }
}

void   BookMark::hide()
{
  if (allow_bookmark)
    {
// *INDENT-OFF*        
      Gui::hide();
// *INDENT-ON*        

      if (rw)
        {
          XUnmapWindow(disp, rw);
          delfrom_el(this);
          delfrom_exp(this);
        }
    }
}


int  BookMark::dnd_drop(DndData* dd, int x, int y, DndAction action)
{
    xncdprintf(("BookMark got dnd_drop\n"));
    int wx,wy,k,startx,starty,wstartx,wstarty,rx,ry;

    default_dnd_man->global2win_coords(w,x,y,wx,wy);
    k = (wy - 25) / page_h;

    startx=default_dnd_man->get_drag_startx();
    starty=default_dnd_man->get_drag_starty();

    default_dnd_man->global2win_coords(w,startx,starty,wstartx,wstarty);

    if(default_dnd_man->is_dragging())
    {
	xncdprintf(("Internal dropping on bookmark\n"));
	if(default_dnd_man->get_drag_object()==this)
	{
	    xncdprintf(("BookMark is dragger and dropper\n"));
	    if (wy > h - 50)
	    {
		rx = (wstarty - 25) / page_h;
		if (rx < 9)
                    if (act_books[rx])
			del_book(rx);
	    }
	    else
	    {
		rx = (wstarty - 25) / page_h;
		ry = (wy - 25) / page_h;
		if (rx < 9 && ry < 9)
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
	} else
	{
	    if(wy>=h-50)
		panel->del();
	    else
	    {
		panel->bookmark_dropper(wy,action);
	    }
	}
    }
    delete dd;
    return 1;
}

int  BookMark::start_dragging(int n)
{
    char  str[256];
    Atom  atext=XInternAtom(disp,"text/uri-list",False);
    DndData *dat=new DndData(256,atext);
    sprintf(str,"/%d",n);
    dat->add_file_entry("book:",str);
    dnd_drag_now(dat);
    qh.hide();
    return 1;
}

int  BookMark::dnd_position(XEvent*, int globx,int globy)
{
    int wx,wy,k;
    default_dnd_man->global2win_coords(w,globx,globy,wx,wy);
    k = (wy - 25) / page_h;
    if (k < 9)
    {
	if (k != pagenum && act_books[k])
	{
	    pagenum = k;
	    qh.show_this_info(vfs_iname(fs[k], books[k]),books[k], -x, 53 + page_h * k);
	    qh.show();
	} else
	    if(!act_books[k])
		qh.hide();
    }   
    return 1;
}
