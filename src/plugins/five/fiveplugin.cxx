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
/* $Id: fiveplugin.cxx,v 1.2 2002/08/05 10:43:18 leo Exp $ */
#include "fiveplugin.h"
#include "plugincln.h"
#include "fivegeom.h"
#include "fivefuncs.h"
#include "fivegui.h"

Panel*  FivePlugin::new_Panel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*))
{ 
  return new FivePanel(ix,iy,il,ih,iname,imax,icol,ifunc);
}

EXWin*  FivePlugin::new_EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl)
{
  return new FiveEXWin(ix,iy,il,ih,iname,ifunc,hfl);
}

KEY*    FivePlugin::new_KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)())
{ 
  return new FiveKEY(ix,iy,il,ih,iname,icol,ifunc);
}

Switch* FivePlugin::new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)())
{ 
  return new FiveSwitch(ix,iy,il,iname,icol,ifunc);
}

Sw_panel* FivePlugin::new_Sw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn)
{ 
  return new FiveSw_panel(ix,iy,il,ihead,mn,imax,icolumn);
}

Win* FivePlugin::new_Win(int ix,int iy,int il,int ih,char *iname,int icol,int hfl)
{ 
  return new FiveWin(ix,iy,il,ih,iname,icol,hfl);
}

Input*     FivePlugin::new_Input(int ix,int iy,int il,int icol,void (*ifunc)())
{
  return new FiveInput(ix,iy,il,icol,ifunc);
}

Text*      FivePlugin::new_Text(int ix,int iy,char *iname,int icol)
{
  return new FiveText(ix,iy,iname,icol);
}

MenuBar*   FivePlugin::new_MenuBar(int ix,int iy,MenuItem *ii,int maxi)
{
  return new FiveMenuBar(ix,iy,ii,maxi);
}

Menu*      FivePlugin::new_Menu(MenuItem *ii,int maxi)
{
  return new FiveMenu(ii,maxi);
}

Pager*     FivePlugin::new_Pager(int ix,int iy,int il,int ih,int imax)
{
  return new FivePager(ix,iy,il,ih,imax);
}

InfoWin*   FivePlugin::new_InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey)
{
  return new FiveInfoWin(ix,iy,il,ih,iname,icol,ikey);
}

Lister*    FivePlugin::new_Lister(int ix,int iy,int il,int ih,int icolumns)
{
  return new FiveLister(ix,iy,il,ih,icolumns);
}

Separator* FivePlugin::new_Separator(int ix,int iy,int il,Gui *ww,int ivert)
{
  return new FiveSeparator(ix,iy,il,ww,ivert);
}

FtpVisual* FivePlugin::new_FtpVisual(int ix,int iy,int il,int ih)
{
  return new FiveFtpVisual(ix,iy,il,ih);
}

Cmdline*   FivePlugin::new_Cmdline(int ix,int iy,int il,int icol)
{
  return new FiveCmdline(ix,iy,il,icol);
}

BookMark*  FivePlugin::new_BookMark(int il,int ih)
{
  return new FiveBookMark(44,ih);
}




Switcher*  FivePlugin::new_Switcher(int ix,int iy,int il,int ih)
{
  return new Switcher(ix,iy,il,ih);
}


int   FivePlugin::post_install()
{
  five_load_skins();
  five_geom_init();
  XSetWindowBackground(disp,Main,SCOL8);
  return 1;
}

extern int create_file(char *path, char *fname, char *buf, int siz);
extern char xncinifive_chr[];
extern int  xncinifive_chr_size;

const char*  FivePlugin::get_ini_filename()
{
  char tmp[1024];
  int fp;
  sprintf(tmp,"%s/.xnc/xnc.ini.five",getenv("HOME"));
  if((fp=open(tmp,O_RDONLY))==-1)
  {
    sprintf(tmp,"%s/.xnc",getenv("HOME"));
    create_file(tmp,"xnc.ini.five", xncinifive_chr, xncinifive_chr_size);
  } 
  else
    close(fp);
  return "/xnc.ini.five";
}


void* five_plugin_init(PlugInitData*)
{
  return new FivePlugin("LookFive",PLUGIN_VER);
}

InitPluginFunc(five_plugin_init)
