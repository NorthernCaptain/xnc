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
/* $Id: aquaplugin.cxx,v 1.2 2002/08/05 10:43:17 leo Exp $ */
#include "aquaplugin.h"
#include "plugincln.h"
#include "aquagui.h"
#include "aquafuncs.h"
#include "aquageom.h"

KEY*    AquaPlugin::new_KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)())
{ 
  return new AquaKEY(ix,iy,il,ih,iname,icol,ifunc);
}

Text*      AquaPlugin::new_Text(int ix,int iy,char *iname,int icol)
{
  return new AquaText(ix,iy,iname,icol);
}

BookMark*  AquaPlugin::new_BookMark(int il,int ih)
{
  return new AquaBookMark(42,ih);
}

MenuBar*   AquaPlugin::new_MenuBar(int ix,int iy,MenuItem *ii,int maxi)
{
  return new AquaMenuBar(ix,iy,ii,maxi);
}

Win* AquaPlugin::new_Win(int ix,int iy,int il,int ih,char *iname,int icol,int hfl)
{ 
  return new AquaWin(ix,iy,il,ih,iname,icol,hfl);
}

Sw_panel* AquaPlugin::new_Sw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn)
{ 
  return new AquaSw_panel(ix,iy,il,ihead,mn,imax,icolumn);
}

Input*     AquaPlugin::new_Input(int ix,int iy,int il,int icol,void (*ifunc)())
{
  return new AquaInput(ix,iy,il,icol,ifunc);
}

Switch* AquaPlugin::new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)())
{ 
  return new AquaSwitch(ix,iy,il,iname,icol,ifunc);
}

Menu*      AquaPlugin::new_Menu(MenuItem *ii,int maxi)
{
  return new AquaMenu(ii,maxi);
}

Pager*     AquaPlugin::new_Pager(int ix,int iy,int il,int ih,int imax)
{
  return new AquaPager(ix,iy,il,ih,imax);
}

Lister*    AquaPlugin::new_Lister(int ix,int iy,int il,int ih,int icolumns)
{
  return new AquaLister(ix,iy,il,ih,icolumns);
}

Panel*  AquaPlugin::new_Panel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*))
{ 
  return new AquaPanel(ix,iy,il,ih,iname,imax,icol,ifunc);
}

ScrollBar* AquaPlugin::new_ScrollBar(int ix, int iy, int ih, Gui * io)
{
  return new AquaScrollBar(ix, iy, ih, io);
}

FtpVisual* AquaPlugin::new_FtpVisual(int ix,int iy,int il,int ih)
{
  return new AquaFtpVisual(ix,iy,il,ih);
}

InfoWin*   AquaPlugin::new_InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey)
{
  return new AquaInfoWin(ix,iy,il,ih,iname,icol,ikey);
}






EXWin*  AquaPlugin::new_EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl)
{
  return new EXWin(ix,iy,il,ih,iname,ifunc,hfl);
}

Separator* AquaPlugin::new_Separator(int ix,int iy,int il,Gui *ww,int ivert)
{
  return new Separator(ix,iy,il,ww,ivert);
}

Cmdline*   AquaPlugin::new_Cmdline(int ix,int iy,int il,int icol)
{
  return new Cmdline(ix,iy,il,icol);
}


Switcher*  AquaPlugin::new_Switcher(int ix,int iy,int il,int ih)
{
  return new Switcher(ix,iy,il,ih);
}


int   AquaPlugin::post_install()
{
  aqua_load_skins();
  aqua_geom_init();
  XSetWindowBackground(disp,Main,SCOL8);
  return 1;
}

extern int create_file(char *path, char *fname, char *buf, int siz);
extern char xnciniaqua_chr[];
extern int  xnciniaqua_chr_size;

const char*  AquaPlugin::get_ini_filename()
{
  char tmp[1024];
  int fp;
  xncdprintf(("Aqua: get_ini_filename() called\n"));
  sprintf(tmp,"%s/.xnc/xnc.ini.aqua",getenv("HOME"));
  xncdprintf(("Aqua: ini file [%s]\n",tmp));
  if((fp=open(tmp,O_RDONLY))==-1)
  {
    xncdprintf(("Aqua: no such file\n"));
    sprintf(tmp,"%s/.xnc",getenv("HOME"));
    create_file(tmp,"xnc.ini.aqua", xnciniaqua_chr, xnciniaqua_chr_size);
  } 
  else
  {
    xncdprintf(("Aqua: file exists\n"));
    close(fp);
  }
  return "/xnc.ini.aqua";
}


void* aqua_plugin_init(PlugInitData*)
{
  AquaPlugin *p=new AquaPlugin("AquaLook",PLUGIN_VER);
  xncdprintf(("Initialized AquaPlugin - %x\n",p));
  return p;
}

InitPluginFunc(aqua_plugin_init)







