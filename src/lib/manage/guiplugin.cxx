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
/* $Id: guiplugin.cxx,v 1.2 2002/08/05 10:43:10 leo Exp $ */
#include "guiplugin.h"

EXWin*  GuiPlugin::new_EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl)
{
  return new EXWin(ix,iy,il,ih,iname,ifunc,hfl);
}

InfoWin* GuiPlugin::new_InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey)
{
  return new InfoWin(ix,iy,il,ih,iname,icol,ikey);
}

Cmdline*   GuiPlugin::new_Cmdline(int ix,int iy,int il,int icol)
{
  return new Cmdline(ix,iy,il,icol);
}

BookMark*  GuiPlugin::new_BookMark(int il,int ih)
{
  return new BookMark(il,ih);
}

Lister*    GuiPlugin::new_Lister(int ix,int iy,int il,int ih,int icolumns)
{
  return new Lister(ix,iy,il,ih,icolumns);
}

FtpVisual* GuiPlugin::new_FtpVisual(int ix,int iy,int il,int ih)
{
  return new FtpVisual(ix,iy,il,ih);
}

Switcher*  GuiPlugin::new_Switcher(int ix,int iy,int il,int ih)
{
  return new Switcher(ix,iy,il,ih);
}

const char* GuiPlugin::get_ini_filename()
{
  return "/xnc.ini";
}


extern GuiPlugin* guiplugin;
Switch* guiplugin_new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)())
{
  return guiplugin->new_Switch(ix,iy,il,iname,icol,ifunc);
}

