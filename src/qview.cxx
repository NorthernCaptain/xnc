/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
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
/* $Id: qview.cxx,v 1.5 2002/11/10 16:18:31 leo Exp $ */
#include "panel.h"
#include "exwin.h"
#include "internals.h"
#include "c_externs.h"
#include "commonfuncs.h"
#include "pluginman.h"
#include "systemfuncs.h"

void   qredraw(Window, GC &);
EXWin *qwin;
int    qvflag = 0;
char   qvdir[L_MAXPATH] = "";
char   qvname[L_MAXPATH] = "";
int    qvsubdir = -1;
int    qvfiles = -1;
long_size_t    qvsize = -1;
int    qvwork = 0;
struct stat qvstat;
char   qtmp[L_MAXPATH];

int    DFS::qvrecscan(char *dir)
{
  DIR   *d;
  struct dirent *dr;
  int    dcoun = 0, ret = 1;
  if (qvwork == 0)
    return 0;
// *INDENT-OFF*        
  if (::chdir(dir) == -1)
    return 0;
// *INDENT-ON*        

  d = opendir(".");
  if (d == NULL)
    return 0;
  while ((dr = readdir(d)) != NULL)
    {
      dcoun++;
      process_X();
      if (qvwork == 0)
        {
          closedir(d);
          return 0;
        };
      if (strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0)
        {
          stat(dr->d_name, &qvstat);
//   (dr->d_name,qvstat.st_mode,qvstat.st_size,qvstat.st_uid,qvstat.st_gid,qvstat.st_mtime);
          if (qvstat.st_mode & S_IFDIR)
            {
              qvsubdir++;
              closedir(d);
              strcpy(qtmp, dr->d_name);
              ret = qvrecscan(qtmp);
// *INDENT-OFF*        
              ::chdir("..");
// *INDENT-ON*        

              if (ret == 0)
                return 0;
              d = opendir(".");
              xnc_seekdir(d, dcoun);
            }
          else
            {
              qvfiles++;
              qvsize += qvstat.st_size;
            };
        }
    }
  closedir(d);
  return 1;
}

int    DFS::qvscan()
{
// *INDENT-OFF*        
  ::chdir(qvdir);
  int    r = qvrecscan(qvname);
  ::chdir(curdir);
// *INDENT-ON*        

  return r;
}


int    AFS::qvscan()
{
  char strtmp[L_MAXPATH];
  int    mode, fsiz, coun;
  int    t;
  char   name[1024];
  add_path_content(curdir, qvname);
  FILE  *fp = fopen(listpath, "r");
  if (fp == NULL)
    {
      upper_path(curdir);
      return 0;
    }
  while (fgets(strtmp, 1000, fp) != NULL)
    {
      sscanf(strtmp, "%d %X %d %d %[^\n]\n", &coun, &mode, &fsiz, &t, name);
      if (is_in_this_dir(name))
        {
          qvsize += fsiz;
          if (mode & S_IFDIR)
            qvsubdir++;
          else
            qvfiles++;
        }
      process_X();
      if (qvwork == 0)
        break;
    }
  fclose(fp);
  upper_path(curdir);
  return 0;
}

void   qv_lets_scan()
{
  panel->vfs->qvscan();
  qredraw(qwin->w, qwin->gcw);
}

void   qv_alarm(int)
{
  set_xnc_alarm(qv_alarm);
  if(qvflag)
  {
    qvsubdir = 0;
    qvsize = 0;
    qvfiles = 0;
    qvwork = 1;
    xnc_run_once=qv_lets_scan;
  }
}

void   qv_signal()
{
  set_xnc_alarm(qv_alarm);
}

void   qredraw(Window win, GC & gcw)
{
  char   tmp1[125];
  char   tmp2[125];
  char   tmp3[125];
  char   tmp4[125];
  char   tmp5[125];
  char   tmp6[125];
  int    qksize = qvsize / (long_size_t)1024;
  xncdprintf(("QREDRAW: redraw window while qvflag=%d, disp=%x, win=%x\n",
	      qvflag, disp, win));
  if(!qvflag)
      return;
  if (qvsubdir != -1)
    sprintf(tmp1, "%d", qvsubdir);
  else
    sprintf(tmp1, "************");
  if (qvfiles != -1)
    sprintf(tmp2, "%d", qvfiles);
  else
    sprintf(tmp2, "************");
  
  if (qvsize != -1)
    {
	dig2ascii(tmp6, qvsize);
	sprintf(tmp3, "%s bytes", tmp6);
	sprintf(tmp4, "%d Kbytes", qksize);
	sprintf(tmp5, "%d Mbytes", qksize / 1024);
    }
  else
    {
      sprintf(tmp3, "************");
      sprintf(tmp4, "************");
      sprintf(tmp5, "************");
    }
  XClearWindow(disp, win);
  XSetForeground(disp, gcw, cols[5]);
  XDrawString(disp, win, gcw, 10, 20, "Directory:", 10);
  XDrawString(disp, win, gcw, 18, 42, "Subdirs:", 8);
  XDrawString(disp, win, gcw, 18, 64, "Files:", 6);
  XDrawString(disp, win, gcw, 10, 89, "Total Size:", 11);
  XSetForeground(disp, gcw, guiplugin->get_exwin_foreground());
  XDrawString(disp, win, gcw, 110, 20, qvname, strlen(qvname));
  XDrawString(disp, win, gcw, 110, 42, tmp1, strlen(tmp1));
  XDrawString(disp, win, gcw, 110, 64, tmp2, strlen(tmp2));
  XDrawString(disp, win, gcw, 110, 89, tmp3, strlen(tmp3));
  XDrawString(disp, win, gcw, 110, 109, tmp4, strlen(tmp4));
  XDrawString(disp, win, gcw, 110, 129, tmp5, strlen(tmp5));
  XRaiseWindow(disp, win);
}

void   qv_update(FList * o)
{
  if ((strcmp(qvdir, panel->curdir) != 0 || strcmp(o->name, qvname) != 0) && strcmp(o->name, "..") != 0)
    {
      strcpy(qvdir, panel->curdir);
      strcpy(qvname, o->name);
      qvsubdir = -1;
      qvsize = -1;
      qvfiles = -1;
      qvwork = 0;
      qredraw(qwin->w, qwin->gcw);
      set_xnc_alarm(qv_alarm);
    }
}

void kill_qview()
{
      qvflag = 0;
      qwin->hide();
      remove_xnc_alarm(qv_alarm);
}

void   init_qview()
{
  qwin = guiplugin->new_EXWin(Mainx + Mainl - 270, Mainy + Mainh - 140, 270, 140, "Quick ScanDir Info", qredraw);
  qwin->init(Main);
  qwin->on_destroy(kill_qview);
//  win_nodecor(qwin->w);
}


void   qview()
{
  if (qvflag)
    {
      kill_qview();
    }
  else
    {
      qvflag = 1;
      if (qwin->x != Mainx + Mainl - 270 || qwin->y != Mainy + Mainh - 140)
        XMoveWindow(disp, qwin->w, Mainx + Mainl - 270, Mainy + Mainh - 140);
      qwin->show();
      if ((panel->cur->mode & S_IFMT) == S_IFDIR)
        qv_update(panel->cur);
      XFlush(disp);
      qv_signal();
    }
}
