/****************************************************************************
*  Copyright (C) 2001 by Leo Khramov
*  email:     leo@xnc.dubna.su
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
// $Id: xshrloader.cxx,v 1.4 2002/09/26 13:34:05 leo Exp $

#include "xshrloader.h"
#include "xncdebug.h"

static int dummy_x_error_handler(Display*, XErrorEvent*)
{
  return 0;
}



//Initializing data for Loader
void xsharedLoader::init()
{
  xncdprintf(("Initing xshared vars\n"));
  screen = DefaultScreen(disp);
  defcmp = DefaultColormap(disp,screen);
  root = RootWindow(disp, screen);
}

//Load pixmap from shared res or load it from descr in ResId and make it shared
Pixmap xsharedLoader::load_pixmap(ResId res, int& x, int& y)
{
  Pixmap pix=0;
  
  switch(mode)
  {
  case SharedRead:
  case SharedWrite:
    pix=get_shared_pixmap(res,x,y);
    if(pix==0)
    {
      xncdprintf(("res[%s] - not shared yet\n",res.name));
      pix=real_load_pixmap(res,x,y);
      if(pix)
	make_shared_pixmap(res,x,y,&pix);
	
      return pix;
    }
    xncdprintf(("load_pixmap loaded as %x\n",pix));
    break;
  case SharedOverride:
    pix=get_shared_pixmap(res,x,y);
    /*    if(pix)
    {
      ignore_xerror();
      xncdprintf(("Free_pixmap: %d\n",pix));
      XFreePixmap(disp,pix);
      restore_xerror();
      pix=0;
      }*/
    xncdprintf(("res[%s] - loading and sharing\n",res.name));
    pix=real_load_pixmap(res,x,y);
    if(pix)
    {
      make_shared_pixmap(res,x,y,&pix);
      xncdprintf(("load_pixmap loaded as %x\n",pix));
    }
    break;
  case NonShared:
    xncdprintf(("res[%s] - loading non-shared\n",res.name));
    pix=real_load_pixmap(res,x,y);
    break;
  }

  return pix;
}


//Load xSprite from shared res or load it from descr in ResId and make it shared
xSprite xsharedLoader::load_xsprite(ResId res)
{
  xSprite sprite;
  Pixmap  pix=0;
  
  switch(mode)
  {
  case SharedRead:
  case SharedWrite:
    sprite=get_shared_xsprite(res);
    if((sprite.flags & XSPRITE_IMAGE_LOADED)==0)
    {
      xncdprintf(("res[%s] - not shared yet\n",res.name));
      sprite=real_load_xsprite(res);
      if((sprite.flags & XSPRITE_IMAGE_LOADED)!=0)
      {
	  xncdprintf(("res[%s] - loaded ok, sharing\n",res.name));
	  make_shared_xsprite(res,sprite);
      }
      return sprite;
    }
    xncdprintf(("load_xsprite loaded as %s\n",res.name));
    break;

  case SharedOverride:
    sprite=get_shared_xsprite(res);
    xncdprintf(("res[%s] - loading and sharing\n",res.name));
    sprite=real_load_xsprite(res);
    if(sprite.flags & XSPRITE_IMAGE_LOADED)
    {
      make_shared_xsprite(res,sprite);
      xncdprintf(("load_pixmap loaded as %s\n",res.name));
    }
    break;

  case NonShared:
    xncdprintf(("res[%s] - loading non-shared\n",res.name));
    sprite=real_load_xsprite(res);
    break;
  }

  return sprite;
}


//Load xSprite from mem don't use any shared
xSprite xsharedLoader::real_load_xsprite(ResId res)
{
  Pixmap pix;
  int    x,y;
  xSprite sprite(XSPRITE_LOAD_TRANSPARENT);
  if(res.type!=resPixmap)
  {
    pix=real_load_pixmap(res, x, y);
    sprite.set_image(pix, (unsigned int) x, (unsigned int) y);
  } else
    LoadPixmapFromMem(res.img_data, res.img_size, F_GIF, sprite, CMP_COM);
  return sprite;
}

//Load pixmap from mem don't use any shared
Pixmap xsharedLoader::real_load_pixmap(ResId res, int& x, int& y)
{
  Pixmap pix;
  if(res.type==resBitmap) //loading bitmap from data
  {
      xncdprintf(("Loading bitmap res[%s] from mem\n",res.name));
      x=(int)res.img_l;
      y=(int)res.img_h;
      return XCreateBitmapFromData(disp, DefaultRootWindow(disp),
				   res.img_data, res.img_l, res.img_h);
  }

  if(res.type==resBitPixmap)
  {
      xncdprintf(("Loading bit pixmap res[%s] from mem\n",res.name));
      x=(int)res.img_l;
      y=(int)res.img_h;
      return XCreatePixmapFromBitmapData(disp, DefaultRootWindow(disp),
					 res.img_data, res.img_l, res.img_h,
					 res.fg_color, res.bg_color,
					 DefaultDepth(disp, DefaultScreen(disp)));
  }

  if(res.img_size>0 && res.img_data)
  {
      save_pixmap_to_tmp(res);
      xncdprintf(("Loading res[%s] from mem\n",res.name));
      pix=LoadPixmapFromMem(res.img_data, F_GIF, x, y, CMP_COM,
			    res.img_size);
      if(pix!=0)
      {
	  xncdprintf(("loaded successfuly\n"));
	  return pix;
      }
      xncdprintf(("ERROR: failed to load\n")); 
  }
  return None;
}


void xsharedLoader::save_pixmap_to_tmp(ResId res)
{
#ifdef DEBUG_XNC
    char fname[L_MAXPATH];
    FILE* fp;
    if(res.type!=resPixmap)
	return;
    sprintf(fname,"/tmp/xnc_images/%s.gif",res.name);
    fp=fopen(fname,"w");
    if(!fp)
	return;
    fwrite(res.img_data, res.img_size, 1, fp);
    fclose(fp);
#endif
}

void xsharedLoader::free_pixmap(Pixmap pix)
{
  if(mode==NonShared)
  {
    ignore_xerror();
    xncdprintf(("Free_pixmap: %d",pix));
    XFreePixmap(disp,pix);
    restore_xerror();
  }
}


Pixmap xsharedLoader::get_shared_pixmap(ResId res,int& x, int& y)
{
  Pixmap *ppix,pix;
  Atom resa;
  char tmp[128];
  int *pi;
  resa=XInternAtom(disp,res.name,False);
  ppix=(Pixmap*)get_shared_res_by_atom(resa,XA_PIXMAP);
  if(ppix==0)
    return None;
  pix=*ppix;
  XFree(ppix);
  sprintf(tmp,"%s_x",res.name);
  resa=XInternAtom(disp,tmp,False);
  pi=(int*)get_shared_res_by_atom(resa,XA_CARDINAL);
  x=*pi;
  XFree(pi);
  sprintf(tmp,"%s_y",res.name);
  resa=XInternAtom(disp,tmp,False);
  pi=(int*)get_shared_res_by_atom(resa,XA_CARDINAL);
  y=*pi;
  XFree(pi);
  return pix;
}

xSprite xsharedLoader::get_shared_xsprite(ResId res)
{
  unsigned int x,y;
  xSprite sprite;
  Pixmap *ppix,pix;
  Atom resa;
  char tmp[128];
  unsigned int *pi;

  resa=XInternAtom(disp,res.name,False);
  ppix=(Pixmap*)get_shared_res_by_atom(resa,XA_PIXMAP);
  if(ppix==0)
    return sprite;
  pix=*ppix;
  XFree(ppix);
  sprintf(tmp,"%s_x",res.name);
  resa=XInternAtom(disp,tmp,False);
  pi=(unsigned int*)get_shared_res_by_atom(resa,XA_CARDINAL);
  x=*pi;
  XFree(pi);
  sprintf(tmp,"%s_y",res.name);
  resa=XInternAtom(disp,tmp,False);
  pi=(unsigned int*)get_shared_res_by_atom(resa,XA_CARDINAL);
  y=*pi;
  XFree(pi);

  sprite.set_image(pix, x, y);

  sprintf(tmp,"%s_mask",res.name);
  resa=XInternAtom(disp,tmp,False);
  ppix=(Pixmap*)get_shared_res_by_atom(resa,XA_PIXMAP);
  if(ppix==0)
    return sprite;
  pix=*ppix;
  XFree(ppix);
  sprite.set_mask(pix);

  return sprite;
}

int xsharedLoader::make_shared_pixmap(ResId res,int x, int y,void* data)
{
  char tmp[128];
  Atom resa=XInternAtom(disp,res.name,False);
  make_shared(resa,data,XA_PIXMAP,32);
  sprintf(tmp,"%s_x",res.name);
  resa=XInternAtom(disp,tmp,False);
  make_shared(resa,(void*)&x,XA_CARDINAL,32);
  sprintf(tmp,"%s_y",res.name);
  resa=XInternAtom(disp,tmp,False);
  make_shared(resa,(void*)&y,XA_CARDINAL,32);
  return 1;
}

int xsharedLoader::make_shared_xsprite(ResId res, xSprite sprite)
{
  char tmp[128];
  Atom resa=XInternAtom(disp,res.name,False);
  make_shared(resa,(void*)&sprite.image,XA_PIXMAP,32);
  sprintf(tmp,"%s_x",res.name);
  resa=XInternAtom(disp,tmp,False);
  make_shared(resa,(void*)&sprite.l,XA_CARDINAL,32);
  sprintf(tmp,"%s_y",res.name);
  resa=XInternAtom(disp,tmp,False);
  make_shared(resa,(void*)&sprite.h,XA_CARDINAL,32);
  if(sprite.flags & XSPRITE_MASK_LOADED)
  {
    sprintf(tmp,"%s_mask",res.name);
    resa=XInternAtom(disp,tmp,False);
    make_shared(resa,(void*)&sprite.image_mask,XA_PIXMAP,32);
  }
  return 1;
}

//Create property on name xa_prop on root window and stores in it given data
int xsharedLoader::make_shared(Atom xa_prop, void* data,Atom xa_type, int quant)
{
  if(mode!=SharedWrite && mode!=SharedOverride)
    return 0;
  ignore_xerror();
  XChangeProperty(disp, root, xa_prop, xa_type, quant, PropModeReplace,
		  (unsigned char *) data, 1);
  xncdprintf(("Property [%d] shared ok\n",xa_prop));
  restore_xerror();
  return 1;
}

//Return 0 if can't get property and pointer if get.
//!!!!Don't forget to make XFree after use of return value.
void* xsharedLoader::get_shared_res_by_atom(Atom xa_res,Atom xa_type)
{
  unsigned char* buf;
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_remain;
  ignore_xerror();
  if(XGetWindowProperty(disp,root,xa_res,0,1,False,xa_type,
			&actual_type,&actual_format,&nitems,&bytes_remain,
			&buf)==Success && nitems==1)
  {
    xncdprintf(("Property checked and data received\n"));
    restore_xerror();
    return buf;
  }
  xncdprintf(("Check property failed\n"));
  restore_xerror();
  return 0;
}

//Do some staff at exit
void xsharedLoader::exit_now()
{
  if(mode==SharedWrite || mode==SharedOverride)
  {
    XSetCloseDownMode(disp, RetainPermanent);
    xncdprintf(("Setting closing mode to store shared at server\n"));
  }
}

//Set dummy handler for ignoring Xlib errors
void xsharedLoader::ignore_xerror()
{
  prev_handler=XSetErrorHandler(dummy_x_error_handler);
}

//Restoring previous handler
void xsharedLoader::restore_xerror()
{
  XSetErrorHandler(prev_handler);
}


xsharedLoader *default_loader;
