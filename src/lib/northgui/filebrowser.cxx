/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
*  email:   leo@pop.convey.ru
*  Fido:    2:5030/627.15
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
#include "filebrowser.h"

void   FileBrowser::init(Window ipar)
{
  int    tw;
  parent = ipar;
  w = create_win(name, x, y, l, h, ExposureMask);
  gcv.background = keyscol[1];
  gcv.font = fontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  tl = strlen(name);
  tm = XTextWidth(fontstr, "M", 1);
  tw = XTextWidth(fontstr, name, tl);
  if (l < tw + 40)
    l = tw + 40;
  ty = (21 + fontstr->max_bounds.ascent - fontstr->max_bounds.descent) / 2;
  prflg = 0;
  inp = new Input(10, 35, l - 25, 1);
  flt = new Input(95, h - 70, l - 110, 1);
  t1 = new Text(10, 25, "File name:", 5);
  t2 = new Text(10, h - 56, "Filter:", 5);
  kok = new KEY(10, -10, 100, 25, "OK", 1, NULL);
  kcan = new KEY(-10, -10, 100, 25, "Cancel", 2, NULL);
  inp->set_histid("FILE");
  inp->bool_expand=1;
  flt->set_histid("FILTER");
  inp->init(w);
  flt->init(w);
  t1->init(w);
  t2->init(w);
  kok->init(w);
  kcan->init(w);
  link(t1);
  t1->link(inp);
  flt->link(kok);
  kok->link(kcan);
  kcan->link(inp);
  kok->guiobj = this;
  kcan->guiobj = this;
  inp->guiobj = this;
  flt->guiobj = this;
  inp->setbuf(fname, 1023);
  flt->setbuf(filter, 255);
}


void   FileBrowser::add_tolist(char *tmp, int delta)
{
  int    i, j;
  for (i = delta; i < lmax; i++)
    if (strcmp(list[i], tmp) > 0)
      break;
  for (j = lmax; j > i; j--)
    list[j] = list[j - 1];
  list[i] = tmp;
  lmax++;
}

void   FileBrowser::show_info()
{
  struct stat st;
  char   tmp[20];
  int    tmpx, tmpl;
  XSetForeground(disp, gcw, keyscol[1]);
  XFillRectangle(disp, w, gcw, 10, h - 90, l - 20, 20);
  if (fname[0])
    {
      stat(fname, &st);
      sprintf(tmp, "%d", st.st_size);
      tmpl = strlen(tmp);
      tmpx = XTextWidth(fontstr, tmp, tmpl);
      XSetForeground(disp, gcw, cols[1]);
      XDrawString(disp, w, gcw, 10, h - 80, fname, inp->bl);
      XDrawString(disp, w, gcw, l - 13 - tmpx, h - 80, tmp, tmpl);
    }
}


int    FileBrowser::read_dir()
{
  DIR   *d = NULL;
  struct dirent *dr;
  struct stat st;
  char  *tmp;
  int    k;
// if(chdir(curdir)==-1) return 0;
  d = opendir(".");
  if (d == NULL)
    return 0;
  lmax = 0;
  while ((dr = readdir(d)) != NULL)
    {
      stat(dr->d_name, &st);
      if (strcmp(dr->d_name, ".") == 0)
        continue;
      if (S_ISDIR(st.st_mode))
        {
          tmp = new char[strlen(dr->d_name) + 4];
          sprintf(tmp, ">%s", dr->d_name);
          add_tolist(tmp, 0);
        }
    }
  rewinddir(d);
  k = lmax;
  while ((dr = readdir(d)) != NULL)
    {
      stat(dr->d_name, &st);
      if (!S_ISDIR(st.st_mode))
        filter_with_mask(dr->d_name, k);
    }
  closedir(d);
  return 1;
}


int    FileBrowser::is_filtered(char *s, char *ff)
{
  char  *f;
  while (*s != 0)
    {
      f = ff;
      if (*f == '*')
        {
          f++;
          while (*f != 0 && *s != 0)
            if (*s == *f)
              break;
            else
              s++;
          if (*f == 0)
            return 1;
        }
      while (*s != 0 && *f != 0 && *f != '*')
        if (*f == '?')
          {
            f++;
            s++;
          }
        else if (*s != *f)
          {
            if (*ff == '*')
              break;
            else
              return 0;
          }
        else
          {
            s++;
            f++;
          };

      if (*f == '*')
        ff = f;
    }
  if (*f == 0)
    return 1;
  return 0;
}

void   FileBrowser::filter_with_mask(char *na, int k)
{
  char  *tmp;
  if (is_filtered(na, filter))
    {
      tmp = new char[strlen(na) + 4];
      sprintf(tmp, " %s", na);
      add_tolist(tmp, k);
    }
}

void   FileBrowser::show()
{
  int    i;
  pan = new Panel(10, 93, l - 25, h - 180, list, 0, 1, NULL);
  pan->init(w);
  inp->link(pan);
  pan->link(flt);
  pan->guiobj = this;
  oldel = el.next;
  el.next = NULL;
  XMapRaised(disp, w);
  addto_el(this, w);
  t1->show();
  t2->show();
  inp->setbuf(fname, 1023);
  flt->setbuf(filter, 255);
  inp->show();
  flt->show();
  kok->show();
  kcan->show();
  pan->show();
  chdir(curdir);
  if (read_dir())
    {
      pan->set_max_to(lmax);
      pan->find_and_set(">..");
      pan->expose();
    }
  ffl = 1;
}

void   FileBrowser::hide()
{
  inp->hide();
  flt->hide();
  kok->hide();
  kcan->hide();
  t1->hide();
  t2->hide();
  pan->hide();
  delete pan;
  delall_el();
  delfrom_exp(this);
  el.next = oldel;
  XUnmapWindow(disp, w);
}

void   FileBrowser::guifunc(Gui * o, int nu,...)
{
  int    i;
  switch (o->guitype)
    {
    case 11:
      if (o == kok)
        {
          if (fname[0])
            {
              gen_fullname();
              hide();
              if (guiobj)
                guiobj->guifunc(this, 0);
              else if (func)
                func();
            }
          else
            XBell(disp, 0);
          break;
        };
      if (o == kcan)
        hide();
      break;
    case 7:
      if (o == inp && nu == 1)
        {
          hide();
          gen_fullname();
          if (guiobj)
            guiobj->guifunc(this, 0);
          else if (func)
            func();
        }
      else if (o == flt)
        {
          pan->reset_list();
          for (i = 0; i < lmax; i++)
            delete list[i];
          lmax = 0;
          if (read_dir() == 0)
            {
              chdir(curdir);
              read_dir();
            }
          else
            inp->flush();
          getcwd(curdir, 800);
          pan->set_max_to(lmax);
          pan->find_and_set(">..");
          pan->expose();
          expose();
          XSetInputFocus(disp, pan->w, RevertToParent, CurrentTime);
        }
      break;
    case 17:
      if (list[nu][0] != '>')
        {
          strcpy(fname, list[nu] + 1);
          inp->setbuf(fname, 1023);
          inp->expose();
          show_info();
        }
      else
        {
          if (chdir(list[nu] + 1) != -1)
            {
              pan->reset_list();
              for (i = 0; i < lmax; i++)
                delete list[i];
              lmax = 0;
              if (read_dir() == 0)
                {
                  chdir(curdir);
                  read_dir();
                }
              else
                inp->flush();
              getcwd(curdir, 800);
              pan->set_max_to(lmax);
              pan->find_and_set(">..");
              pan->expose();
              expose();
            }
          else
            XBell(disp, 0);
        }
      break;
    };
}


void   FileBrowser::gen_fullname()
{
  char   tmp[1024];
  if (fname[0] != '/')
    {
      strcpy(tmp, fname);
      sprintf(fname, "%s/%s", curdir, tmp);
    }
}

void   FileBrowser::expose()
{
  int    dirl;
  dirl = strlen(curdir);
  XSetForeground(disp, gcw, keyscol[1]);
  XFillRectangle(disp, w, gcw, 10, 60, l - 10, 30);
  XSetForeground(disp, gcw, cols[0]);
  if (dirl * tm < l - 20)
    XDrawString(disp, w, gcw, 11, 81, curdir, dirl);
  else
    XDrawString(disp, w, gcw, 11, 81, curdir + (dirl - (l - 20) / tm), (l - 20) / tm);
  XSetForeground(disp, gcw, cols[5]);
  if (dirl * tm < l - 20)
    XDrawString(disp, w, gcw, 10, 80, curdir, dirl);
  else
    XDrawString(disp, w, gcw, 10, 80, curdir + (dirl - (l - 20) / tm), (l - 20) / tm);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XDrawLine(disp, w, gcw, 1, 1, l - 1, 1);
  XDrawLine(disp, w, gcw, 1, 1, 1, h - 1);
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  XDrawLine(disp, w, gcw, 1, h - 2, l - 1, h - 2);
  XDrawLine(disp, w, gcw, l - 2, 1, l - 2, h - 1);
  t1->expose();
  t2->expose();
  show_info();
  if (ffl)
    {
      if (next)
        if (next->foc == 0 && next->w != w)
          XSetInputFocus(disp, next->w, RevertToNone, CurrentTime);
        else if (next->next)
          XSetInputFocus(disp, next->next->w, RevertToNone, CurrentTime);
      ffl = 0;
    }
  if (next)
    if (next->w == w)
      next->expose();
}

void   FileBrowser::click()
{
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case ButtonPress:
          if (hflg)
            if (ev.xbutton.x < 21 && ev.xbutton.y < 21)
              {
                XSetForeground(disp, gcw, keyscol[2]);
                XDrawLine(disp, w, gcw, 1, 20, 20, 20);
                XDrawLine(disp, w, gcw, 20, 2, 20, 20);
                XSetForeground(disp, gcw, keyscol[0]);
                XDrawLine(disp, w, gcw, 2, 2, 20, 2);
                XDrawLine(disp, w, gcw, 2, 2, 2, 20);
                prflg = 1;
              };
          break;
        case ButtonRelease:
          if (prflg)
            {
              prflg = 0;
              hide();
            };
          break;
        };
    }
}

void   FileBrowser::nonresizable()
{
  XSizeHints *xhint;
  long   lg;
  xhint = XAllocSizeHints();
  XGetWMNormalHints(disp, w, xhint, &lg);
  xhint->flags = (PMinSize | PMaxSize);
  xhint->min_width = l;
  xhint->min_height = h;
  xhint->max_width = l;
  xhint->max_height = h;
  XSetWMNormalHints(disp, w, xhint);
  XFree(xhint);
}
