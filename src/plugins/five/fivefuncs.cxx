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
/* $Id: fivefuncs.cxx,v 1.2 2002/08/05 10:43:18 leo Exp $ */
#include "fiveplugin.h"
#include "internals.h"
#include "image.h"
#include "fivegui.h"
#include "xshrloader.h"

extern char fiveskin_chr[];
extern int  fiveskin_chr_size;

void   five_load_skins()
{
  int x,y;
  LoaderMode oldmode;
  fprintf(stderr,"Loading skins..");
  show_dot();
  im_populate_pallete(skincol);
  show_dot();
  oldmode=default_loader->set_workmode(NonShared);
  skin.skin=default_loader->load_pixmap(ResId("xnci_p_five_skin",
					      fiveskin_chr,
					      fiveskin_chr_size
					      ),x,y);
  default_loader->set_workmode(oldmode);
  show_dot();
  im_clear_global_pal();
  show_dot();
  skin.l=x;
  show_dot();
  skin.h=y;
  show_dot();
  show_dot();
  fprintf(stderr,".OK\n");
}

void   five_prect(Window w, GC & gcw, int x, int y, int l, int h)
{
  XSetForeground(disp, gcw, SCOL2);
  XDrawLine(disp, w, gcw, x, y, x + l, y);
  XDrawLine(disp, w, gcw, x, y, x, y + h);
  XSetForeground(disp, gcw, SCOL26);
  XDrawLine(disp, w, gcw, x + 1, y + h, x + l, y + h);
  XDrawLine(disp, w, gcw, x + l, y, x + l, y + h);
}

void   five_urect(Window w, GC & gcw, int x, int y, int l, int h)
{
  XSetForeground(disp, gcw, SCOL26);
  XDrawLine(disp, w, gcw, x, y, x + l, y);
  XDrawLine(disp, w, gcw, x, y, x, y + h);
  XSetForeground(disp, gcw, SCOL2);
  XDrawLine(disp, w, gcw, x + 1, y + h, x + l, y + h);
  XDrawLine(disp, w, gcw, x + l, y, x + l, y + h);
}

void five_show_sprite(Window w, GC & gcw, int l, int h, Sprite* spr)
{
  if(spr==0)
    return;
  int tox=spr->tox, toy=spr->toy;
  if(spr->tox<0)
    tox+=l;
  if(spr->toy<0)
    toy+=h;
  XCopyArea(disp,spr->im->skin,w,gcw,spr->x,spr->y,
	    (unsigned int)spr->l,(unsigned int)spr->h,tox,toy);
}

