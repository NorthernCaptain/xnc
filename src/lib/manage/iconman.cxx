/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
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
/* $Id: iconman.cxx,v 1.6 2002/09/26 13:34:05 leo Exp $ */
#include "xheaders.h"
#include "colors.h"
#include "iconman.h"

IconManager *default_iconman=0;


//========================================================================


//  --------===========   Icon Manager methods   ===========-----------


//========================================================================

//-------------------------------------------------------------------------
// Create icons from iconset
//-------------------------------------------------------------------------
void IconManager::IconSet::create_icons(int ifrom_x, int ifrom_y,
					unsigned int il, unsigned int ih,
					int max_icons)
{
    int i;
    Pixmap bitmap, shadow;
    GC gc;
    delete_icons();
    total_icons=max_icons;
    from_x=ifrom_x;
    from_y=ifrom_y;
    icon_l=il;
    icon_h=ih;
    if(!icons)
	icons=new Icon[total_icons];
    for(i=0; i<total_icons; i++)
    {
	bitmap=XCreatePixmap(disp, DefaultRootWindow(disp), icon_l, icon_h, 1);
	gc=XCreateGC(disp,bitmap, 0l, 0);
	XCopyArea(disp, mask, bitmap, gc, from_x + i*icon_l, from_y,
		  icon_l, icon_h, 0, 0);
	XFreeGC(disp, gc);
	shadow=XCreatePixmap(disp, DefaultRootWindow(disp), icon_l, icon_h, 
			     DefaultDepth(disp, DefaultScreen(disp)));
	gc=XCreateGC(disp,shadow, 0l, 0);
	XSetForeground(disp, gc, shadow_color);
	XFillRectangle(disp, shadow, gc, 0, 0, icon_l, icon_h);
	XFreeGC(disp, gc);
	icons[i].init(pixmap, bitmap, shadow,
		      from_x + i*icon_l, from_y, 
		      icon_l, icon_h);
    }
}

//-------------------------------------------------------------------------
// Delete all icons from iconset
//-------------------------------------------------------------------------
void IconManager::IconSet::delete_icons()
{
    int i;
    if(!icons)
	return;
    for(i=0; i<total_icons; i++)
    {
	if(icons[i].image)
	{
	    icons[i].image=0;
	    XFreePixmap(disp, icons[i].mask);
	    XFreePixmap(disp, icons[i].shadow);
	}
    }
}

//-------------------------------------------------------------------------
// Constructor of Icon Manager, inits variables
//-------------------------------------------------------------------------

IconManager::IconManager()
{
    total_isets=0;
}


//-------------------------------------------------------------------------
// Load iconset from default_loader and create icons
// return iconset idx on success, -1 on fail
//-------------------------------------------------------------------------
int IconManager::load_iconset(int idx, char *iname, ResId respixmap, ResId resmask, 
			      int ifrom_x, int ifrom_y, 
			      unsigned int icon_l, unsigned int icon_h,
			      int max_icons)
{
    int pix_l, pix_h;
    xSprite  sprite;
    strncpy(iset[idx].name, iname, L_ICONSET_NAME);
    iset[idx].name[L_ICONSET_NAME-1]=0;
#if 0
    iset[idx].pixmap=default_loader->load_pixmap(respixmap, 
							 iset[idx].l,
							 iset[idx].h);
    iset[idx].mask=default_loader->load_pixmap(resmask, pix_l, pix_h);
#else
    sprite=default_loader->load_xsprite(respixmap);
    iset[idx].pixmap=sprite.image;
    iset[idx].mask=sprite.image_mask;
    iset[idx].l=(int)sprite.l;
    iset[idx].h=(int)sprite.h;
#endif
    iset[idx].create_icons(ifrom_x, ifrom_y, icon_l, icon_h, max_icons);

    return total_isets++;
}

//-------------------------------------------------------------------------
// Delete old created icons from set and creates new one with given from_x
//-------------------------------------------------------------------------
void IconManager::change_iconset_style(int idx, int ifrom_x, int ifrom_y)
{
  iset[idx].delete_icons();
  iset[idx].create_icons(ifrom_x, ifrom_y, 
			 iset[idx].icon_l, iset[idx].icon_h, 
			 iset[idx].total_icons);  
}


//-------------------------------------------------------------------------
// Draw icon on given window with GC
//-------------------------------------------------------------------------
void IconManager::display_icon_from_set(Window w, GC xpmgc, int left_x, int center_y,
					int iconset_idx, int icon_idx)
{
    int top_y;

    if(icon_idx<0)
	return;

    Icon& icon=iset[iconset_idx].icons[icon_idx];

    top_y=center_y - icon.h/2;
    XSetClipMask(disp, xpmgc, icon.mask);
    XSetClipOrigin(disp, xpmgc, left_x, top_y);
    XCopyArea(disp, icon.image, w, xpmgc, icon.from_x, icon.from_y, 
		icon.l, icon.h, 
		left_x, top_y);
    
}

//-------------------------------------------------------------------------
// Draw icon with shadow on given window with GC
//-------------------------------------------------------------------------
void IconManager::display_icon_from_set_with_shadow(Window w, GC xpmgc, 
						    int left_x, int center_y,
						    int iconset_idx, int icon_idx)
{
    int top_y;

    if(icon_idx<0)
	return;

    Icon& icon=iset[iconset_idx].icons[icon_idx];

    top_y=center_y - icon.h/2;
    XSetClipMask(disp, xpmgc, icon.mask);
    XSetClipOrigin(disp, xpmgc, left_x+1, top_y+1);
    XCopyArea(disp, icon.shadow, w, xpmgc, 0, 0, 
		icon.l, icon.h, 
		left_x+1, top_y+1);
    
    XSetClipOrigin(disp, xpmgc, left_x-1, top_y-1);
    XCopyArea(disp, icon.image, w, xpmgc, icon.from_x, icon.from_y, 
		icon.l, icon.h, 
		left_x-1, top_y-1);
    
}

//-------------------------------------------------------------------------
// Draw icon on given window with autocreated GC
//-------------------------------------------------------------------------
void IconManager::display_icon_from_set(Window w, int left_x, int center_y,
					int iconset_idx, int icon_idx)
{
    GC xpmgc;
    if(icon_idx<0)
	return;
    xpmgc=XCreateGC(disp,w,0l,0);
    display_icon_from_set(w, xpmgc, left_x, center_y,
			  iconset_idx, icon_idx);
    XFreeGC(disp, xpmgc);
}
    
//-------------------------------------------------------------------------
// Draw icon on given window with autocreated GC
//-------------------------------------------------------------------------
void IconManager::display_icon_from_set_with_shadow(Window w, int left_x, int center_y,
						    int iconset_idx, int icon_idx)
{
    GC xpmgc;
    if(icon_idx<0)
	return;
    xpmgc=XCreateGC(disp,w,0l,0);
    display_icon_from_set_with_shadow(w, xpmgc, left_x, center_y,
				      iconset_idx, icon_idx);
    XFreeGC(disp, xpmgc);
}
    
//============================== End of file ==================================
