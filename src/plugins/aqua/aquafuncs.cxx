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
/* $Id: aquafuncs.cxx,v 1.3 2002/09/09 14:23:11 leo Exp $ */
#include "aquaplugin.h"
#include "colors.h"
#include "internals.h"
#include "image.h"
#include "aquagui.h"
#include "xshrloader.h"

extern char aquaskin_chr[];
extern int  aquaskin_chr_size;

void   aqua_load_skins()
{
  int x,y;
  fprintf(stderr,"Loading skins..");
  show_dot();
  im_populate_pallete(skincol);
  show_dot();
  skin.skin=default_loader->load_pixmap(ResId("xnci_p_aqua_skin",
					      aquaskin_chr,
					      aquaskin_chr_size
					      ),x,y);
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

void aqua_show_sprite(Window w, GC & gcw, int l, int h, Sprite* spr)
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


Pixmap aqua_skin_to_pixmap(Sprite* skin)
{
  GC gcpix;
  Pixmap pix;
  pix=XCreatePixmap(disp,Main,(unsigned)skin->l,(unsigned)skin->h,
		      DefaultDepth(disp,DefaultScreen(disp)));
  gcpix = XCreateGC(disp, pix, 0, 0);
  XCopyArea(disp, skin->im->skin, pix, gcpix, skin->x, skin->y,
	    (unsigned)skin->l, (unsigned)skin->h, 
	    0, 0);
  XFree(gcpix);
  return pix;
}



