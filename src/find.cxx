/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:   leo@xnc.dubna.su
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
/* $Id: find.cxx,v 1.4 2002/11/01 15:37:23 leo Exp $ */

#include "panel.h"
#include "pluginman.h"
#include "systemfuncs.h"
//////////////////////Find files in filesystems//////////////////////

extern "C" void process_X();

Win   *fwin = NULL;
KEY   *ffind, *ffind2, *fstop, *fquit;
Panel *fpan;
Input *fin;
extern char *panlist[];
char   f_buf[256] = "";
int    findfl;
int    findmax;
int    retflag, qflag;

int    is_it(char *fn)
{
  int    i = 0, bad = 0;
  int    l = strlen(fn);
  while (fn[l] != '/' && l > 0)
    l--;
  fn = &fn[l];
  if (*fn == '/')
    fn++;
  while (f_buf[i] != 0 && *fn != 0)
    {
      switch (f_buf[i])
        {
        case '*':
          while (*fn != 0 && *fn != '.')
            fn++;
          break;
        case '?':
          *fn++;
          break;
        default:
          if (*fn != f_buf[i])
            bad = 1;
          else
            fn++;
          break;
        }
      if (bad)
        break;
      else
        i++;
    }
  if (f_buf[i - 1] == '*' || f_buf[i] == '*')
    while (*fn)
      fn++;
  if (bad == 0 && f_buf[i] == 0 && *fn == 0)
    return 1;
  return 0;
}

char   fullpath[1024];

void   genfullpath(char *n)
{
  getcwd(fullpath, 800);
  strcat(fullpath, "/");
  strcat(fullpath, n);
}

//return -1 on error;

int    ft_coun = 0;
int    workcoun;
char  *workstr = "Working... Please, be patient!!!";
int    ftw_func(char *file, struct stat *sb, int flag)
{
  char  *f;
  int    ty = fontstr->max_bounds.ascent;
  if (file[0] == file[1] && file[0] == '/')
    file++;
  if (is_it(file))
    {
      f = new char[strlen(file) + 1];
      strcpy(f, file);
      fpan->add_element(f);
      findmax++;
      if (findmax >= 200)
        retflag = 1;
    }
//   if(XEventsQueued(disp,QueuedAfterReading))  XEVENT();
  ft_coun++;
  if (ft_coun > 10)
    {
      ft_coun = 0;
      XSetForeground(disp, fwin->gcw, cols[2]);
      XDrawString(disp, fwin->w, fwin->gcw, 20, 325 + ty, workstr, workcoun);
      workcoun++;
      if (workcoun > 32)
        {
          XSetForeground(disp, fwin->gcw, keyscol[1]);
          XFillRectangle(disp, fwin->w, fwin->gcw, 20, 325, XTextWidth(fontstr, workstr, 32) + 10,
                         fontstr->max_bounds.descent + ty);
          workcoun = 1;
        }
    }
  process_X();
  return retflag;
}

char   drdir[80];
int    myftw(char *dir)
{
  struct stat dstat;
  if (chdir(dir) == -1)
    return -1;
  DIR   *d;
  struct dirent *dr;
  int    dcoun = 0, ret = 1;
  d = opendir(".");
  if (d == NULL)
    {
      chdir("..");
      return 0;
    }
  while ((dr = readdir(d)) != NULL)
    {
      dcoun++;
      if (strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0)
        {
          stat(dr->d_name, &dstat);
          genfullpath(dr->d_name);
          ret = ftw_func(fullpath, &dstat, 1);
          if (ret != 0)
            {
              closedir(d);
              chdir("..");
              return 0;
            };
          ret = 1;
          if (dstat.st_mode & S_IFDIR)
            {
              strcpy(drdir, dr->d_name);
              closedir(d);
              ret = myftw(drdir);
              d = opendir(".");
              xnc_seekdir(d, dcoun);
            }
        }
      if (retflag)
        ret = 0;
      if (ret != 1)
        {
          closedir(d);
          chdir("..");
          return ret;
        };
    }
  closedir(d);
  chdir("..");
  return 1;
}

void   findcan()
{
  retflag = 1;
  fwin->hide();
  ffind->hide();
  ffind2->hide();
  fstop->hide();
  fquit->hide();
  fin->hide();
  fpan->hide();
  delete fpan;
  delete fwin;
  delete ffind;
  delete ffind2;
  delete fstop;
  delete fquit;
  delete fin;
  for (int i = 0; i < findmax; i++)
    delete panlist[i];
  fwin = NULL;
  XSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
}

void   findq()
{
  if (retflag == 0)
    {
      retflag = 1;
      qflag = 1;
    }
  else
    findcan();
}

int    find_choice(int, char *path)
{
  char  *n;
  if (path != NULL)
    {
      if (chdir(path) == 0)
        {
          strcpy(panel->curdir, path);
        }
      else
        {
          n = strrchr(path, '/');
          *n = 0;
          n++;
          strcpy(panel->curdir, path);
//    delete panel->cur->name;
       //    panel->cur->name=new char[strlen(n)+1];
          strcpy(panel->cur->name, n);
        }
      panel->reread();
    }
  findcan();
  return 1;
}

void   findfind()
{
  if (findfl)
    {
      findfl = 0;
      fpan->set_max_to(0);
      fpan->cursor_to(0);
      fpan->expose();
      for (int i = 0; i < 200; i++)
        {
          if (panlist[i])
            delete panlist[i];
          panlist[i] = NULL;
        }
      retflag = 0;
      workcoun = 1;
      //      ftw(panel->curdir,ftw_func,5);
      myftw(panel->curdir);
      retflag = 1;
      XBell(disp, 0);
      if (qflag)
        findcan();
      else
        XSetInputFocus(disp, fpan->w, RevertToParent, CurrentTime);
      findfl = 1;
    }
}

void   findstop()
{
  retflag = 1;
}

void   findfind2()
{
  if (findfl)
    {
      findfl = 0;
      fpan->set_max_to(0);
      fpan->cursor_to(0);
      fpan->expose();
      for (int i = 0; i < 200; i++)
        {
          if (panlist[i])
            delete panlist[i];
          panlist[i] = NULL;
        }
      retflag = 0;
      workcoun = 1;
//  ftw("/",ftw_func,5);
      myftw("/");
      retflag = 1;
      XBell(disp, 0);
      if (qflag)
        findcan();
      else
        XSetInputFocus(disp, fpan->w, RevertToParent, CurrentTime);
      findfl = 1;
    }
}

void   find_panel(int wl)
{
  if (fwin == NULL)
    {
      findfl = 1;
      qflag = 0;
      retflag = 1;
      findmax = 0;
      for (int i = 0; i < 200; i++)
        panlist[i] = NULL;
      fwin = guiplugin->new_Win(centerx - wl / 2, centery - 190, wl, 390, "Find file", 5);
      fpan = guiplugin->new_Panel(20, 30, wl - 40, 250, panlist, 0, 1, find_choice);
      ffind = guiplugin->new_KEY(20, -20, 100, 25, _("Find"), 1, findfind);
      ffind2 = guiplugin->new_KEY(140, -20, 100, 25, _("From '/'"), 1, findfind2);
      fstop = guiplugin->new_KEY(-140, -20, 100, 25, _("Stop"), 5, findstop);
      fquit = guiplugin->new_KEY(-20, -20, 100, 25, _("Quit"), 2, findq);
      fin = guiplugin->new_Input(20, 290, wl - 40, 1, findfind);
      ffind->set_iname("FFIND");
      ffind2->set_iname("FINDFR");
      fstop->set_iname("FSTOP");
      fquit->set_iname("FQUIT");
      fwin->set_iname("FINDWIN");
      fpan->escfunc(findcan);
      fwin->init(Main);
      fpan->init(fwin->w);
      ffind->init(fwin->w);
      ffind2->init(fwin->w);
      fstop->init(fwin->w);
      fquit->init(fwin->w);
      fin->init(fwin->w);
      fwin->link(fin);
      fin->link(ffind);
      ffind->link(ffind2);
      ffind2->link(fstop);
      fstop->link(fquit);
      fquit->link(fpan);
      fpan->link(fin);
      fin->setbuf(f_buf, 255);
      fwin->show();
      fpan->show();
      fin->show();
      ffind->show();
      ffind2->show();
      fstop->show();
      fquit->show();
    }
}
