/****************************************************************************
*  Copyright (C) 1999 by Leo Khramov
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
#include "panel.h"
#include "exwin.h"
#include "internals.h"
#include "pluginman.h"
#include "systemfuncs.h"
#include "config_file.h"

#define INODEWIN_H  328

static EXWin *inodewin=NULL;

static FList cur_i;

static int maxtx, iwinx, iwiny;

static int maxsymb=0;

void magic_scanner();

int magic_need_scan=0;

void kill_inodewin()
{
    inodewin->hide();
    option_bits&=~STATUS_INODE;
}


int get_inodewin_position(int *x, int *y)
{
    if(inodewin==NULL)
        return 0;
    *x=inodewin->x; *y=inodewin->y;
    if(inodewin->ffl)
       *y-=wm_y_correction;
    return 1;
}

void draw_multi_text(Window win, GC& gcw, int ix, int iy, char *str, int maxs)
{
  int l,i;
  char chr;

  if(maxs<=0 || *str==0)
      return;

  while(1)
  {
  l=0;
  for(i=0;i<maxs;i++)
    if(str[i]==0)
        break;
    else
      if(str[i]==' ')
         l=i;
  if(i<maxs)
  {
     XDrawString(disp, win, gcw, ix, iy, str, i);
     return;
  }
  if(l==0)
     l=i;
  XDrawString(disp, win, gcw, ix, iy, str, l);
  iy+=22;
  str+=l;
  if(*str==' ')
      str++;
  }
}
  



void   inode_redraw_body(Window win, GC& gcw)
{
  char buf[64];
  struct tm *tt;
  XRaiseWindow(disp, win);
  XClearArea(disp, win, maxtx+20, 0, 180-maxtx, 330, 0);
  XSetForeground(disp, gcw, guiplugin->get_exwin_foreground());
  dig2ascii2(buf, cur_i.st.st_dev);
  XDrawString(disp, win, gcw, 20+maxtx, 20, buf, strlen(buf));
  dig2ascii2(buf, cur_i.st.st_ino);
  XDrawString(disp, win, gcw, 20+maxtx, 42, buf, strlen(buf));
  dig2ascii2(buf, cur_i.st.st_mode, 8);
  XDrawString(disp, win, gcw, 20+maxtx, 64, buf, strlen(buf));

  dig2ascii2(buf, cur_i.st.st_nlink);
  XDrawString(disp, win, gcw, 20+maxtx, 89, buf, strlen(buf));
  sprintf(buf, "%-4d (%s)", cur_i.st.st_uid, finduser(cur_i.st.st_uid));
  XDrawString(disp, win, gcw, 20+maxtx, 114, buf, strlen(buf));
  sprintf(buf, "%-4d (%s)", cur_i.st.st_gid, findgroup(cur_i.st.st_gid));
  XDrawString(disp, win, gcw, 20+maxtx, 139, buf, strlen(buf));
  sprintf(buf,"(%d, %d)", major(cur_i.st.st_rdev), minor(cur_i.st.st_rdev));
  XDrawString(disp, win, gcw, 20+maxtx, 164, buf, strlen(buf));
  dig2ascii2(buf, cur_i.st.st_size);
  XDrawString(disp, win, gcw, 20+maxtx, 189, buf, strlen(buf));
  dig2ascii2(buf, cur_i.st.st_blksize);
  XDrawString(disp, win, gcw, 20+maxtx, 214, buf, strlen(buf));
  dig2ascii2(buf, cur_i.st.st_blocks);
  XDrawString(disp, win, gcw, 20+maxtx, 239, buf, strlen(buf));
  tt=localtime(&cur_i.st.st_atime);
  strftime(buf, 64, "%d-%b-%Y %H:%M", tt);
  XDrawString(disp, win, gcw, 20+maxtx, 264, buf, strlen(buf));
  tt=localtime(&cur_i.st.st_mtime);
  strftime(buf, 64, "%d-%b-%Y %H:%M", tt);
  XDrawString(disp, win, gcw, 20+maxtx, 289, buf, strlen(buf));
  tt=localtime(&cur_i.st.st_ctime);
  strftime(buf, 64, "%d-%b-%Y %H:%M", tt);
  XDrawString(disp, win, gcw, 20+maxtx, 314, buf, strlen(buf));
/*  
  XClearArea(disp, win, 0, 332, 200, INODEWIN_H-332, 0);
  draw_multi_text(win, gcw, 10, 350, cur_i.magic, maxsymb);
*/
}

void   inode_redraw_head(Window win, GC& gcw)
{
  maxsymb=(INODEWIN_H-20)*10/XTextWidth(fontstr, "MMMMMMMMMM", 10);
  XClearWindow(disp, win);
  XSetForeground(disp, gcw, cols[5]);
  XDrawString(disp, win, gcw, 10, 20, "Device:", 7);
  XDrawString(disp, win, gcw, 10, 42, "Inode:", 6);
  XDrawString(disp, win, gcw, 10, 64, "Attributes:", 11);
  maxtx=XTextWidth(fontstr, "Attributes:", 11);
  XDrawString(disp, win, gcw, 10, 89, "Links:", 6);
  XDrawString(disp, win, gcw, 10, 114,"Uid:", 4);
  XDrawString(disp, win, gcw, 10, 139,"Gid:", 4);
  XDrawString(disp, win, gcw, 10, 164,"Dev type:", 9);
  XDrawString(disp, win, gcw, 10, 189,"Size:", 5);
  XDrawString(disp, win, gcw, 10, 214,"Blksize:", 8);
  XDrawString(disp, win, gcw, 10, 239,"Blocks:", 7);
  XDrawString(disp, win, gcw, 10, 264,"ATime:", 6);
  XDrawString(disp, win, gcw, 10, 289,"MTime:", 6);
  XDrawString(disp, win, gcw, 10, 314,"CTime:", 6);
  XDrawLine(disp, win, gcw, 10, 330, 190, 330);
}

void inode_redraw(Window win, GC& gcw)
{
    inode_redraw_head(win, gcw);
    inode_redraw_body(win, gcw);
}


void   show_inodeinfo(FList *o)
{
    memcpy(&cur_i, o, sizeof(cur_i));
    inode_redraw_body(inodewin->w, inodewin->gcw);
}

void   init_inodewin()
{
  inodewin = guiplugin->new_EXWin(Mainx + Mainl - 200, Mainy + Mainh - INODEWIN_H, 200, INODEWIN_H,
                      "Stat Info", inode_redraw);
  inodewin->init(Main);
  inodewin->on_destroy(kill_inodewin);
}


void init_inodewin_with_position(int x, int y)
{
  inodewin = guiplugin->new_EXWin(x, y, 200, INODEWIN_H,
                      "Stat Info", inode_redraw);
  inodewin->init(Main);
  inodewin->on_destroy(kill_inodewin);
}


void magic_scan_files(int)
{
  if(option_bits & STATUS_INODE)
  {
      if(panel->vfs->magic_scan())
         panel->panel2->vfs->magic_scan();
      if(magic_need_scan)
          magic_scanner();
  } 
}
    
void toggle_inodewin()
{
    option_bits^=STATUS_INODE;
    if(option_bits & STATUS_INODE)
    {
        if(inodewin==NULL)
            init_inodewin();
        inodewin->show();
        magic_scanner();
    }
    else
    {
        inodewin->hide();
//        remove_xnc_alarm(magic_scan_files);
    }
}
    
void magic_scanner()
{
/*  if(option_bits & STATUS_INODE)
  {
      set_xnc_alarm(magic_scan_files);
      magic_need_scan=0;
      panel->vfs->options&=~MAGIC_WORK;
      panel->panel2->vfs->options&=~MAGIC_WORK;
  }
*/
}
