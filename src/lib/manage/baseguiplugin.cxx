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
/* $Id: baseguiplugin.cxx,v 1.2 2002/08/05 10:43:10 leo Exp $ */
#include "baseguiplugin.h"

/****************** Base wrappers for plugin objects ******************/

KEY*    BaseGuiPlugin::new_KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)())
{ 
  return new KEY(ix,iy,il,ih,iname,icol,ifunc);
}

BKey*   BaseGuiPlugin::new_BKey(int ix, int iy, int il, int ih, Gui * io, int f)
{
  return new BKey(ix, iy, il, ih, io, f);
}

Panel*  BaseGuiPlugin::new_Panel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*))
{ 
  return new Panel(ix,iy,il,ih,iname,imax,icol,ifunc);
}

Pager*  BaseGuiPlugin::new_Pager(int ix,int iy,int il,int ih,int imax)
{
  return new Pager(ix,iy,il,ih,imax);
}

Switch* BaseGuiPlugin::new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)())
{ 
  return new Switch(ix,iy,il,iname,icol,ifunc);
}

Sw_panel* BaseGuiPlugin::new_Sw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn)
{ 
  return new Sw_panel(ix,iy,il,ihead,mn,imax,icolumn);
}

Win* BaseGuiPlugin::new_Win(int ix,int iy,int il,int ih,char *iname,int icol,int hfl)
{ 
  return new Win(ix,iy,il,ih,iname,icol,hfl);
}

Separator* BaseGuiPlugin::new_Separator(int ix,int iy,int il,Gui *ww,int ivert)
{
  return new Separator(ix,iy,il,ww,ivert);
}

Input*     BaseGuiPlugin::new_Input(int ix,int iy,int il,int icol,void (*ifunc)())
{
  return new Input(ix,iy,il,icol,ifunc);
}

Text*      BaseGuiPlugin::new_Text(int ix,int iy,char *iname,int icol)
{
  return new Text(ix,iy,iname,icol);
}

MenuBar*   BaseGuiPlugin::new_MenuBar(int ix,int iy,MenuItem *ii,int maxi)
{
  return new MenuBar(ix,iy,ii,maxi);
}

Menu*      BaseGuiPlugin::new_Menu(MenuItem *ii,int maxi)
{
  return new Menu(ii,maxi);
}

ScrollBar* BaseGuiPlugin::new_ScrollBar(int ix, int iy, int ih, Gui * io)
{
  return new ScrollBar(ix, iy, ih, io);
}

BaseGuiPlugin *baseguiplugin;

void init_basegui_plugin()
{
  baseguiplugin=new BaseGuiPlugin("base_internal","0.6.0");
}

