/****************************************************************************
*  Copyright (C) 1996-98 by Leo Khramov
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
#include "query_windows.h"
#include "panel.h"
#include "infowin.h"
#include "bookmark.h"
#include "internals.h"
#include "ftpcfg.h"
#include "pager.h"
#include "c_externs.h"
#include "pluginman.h"
/////////This file contains function for different window queries
/////////needed by Listers.

Win   *ww = NULL;
Win   *ow_win;
Text  *ow_t1, *ow_t2, *ow_t3, *ow_t4;
KEY   *ow_kyes, *ow_kyes_to_all, *ow_kno, *ow_kcancel;
Input *winp = NULL;
Switch *symsw = NULL;
int    update_query_work, update_ok;


int    ow_file = 0;                //overwrite current file

int    ow_all = 0;                //overwrite all files without prompt

int    dont_ask = 0;                //don't even ask for overwriting, just do it! Very good choice for me ;)

int    ow_query_work = 0;
int    ow_cancel = 0;
int    ow_no = 0;


extern char *fschr[];                //Array of VFS names (defined in bookmark.cxx)

extern int fsmax;                //Number of VFS types

extern int dirdelay;         //Dirscan timeout in sec. Placed in panel.cxx

char   dbuf[L_MAXPATH];

int blank(char* str)
{
  while(*str)
  {
          if(*str!=' ')
                  return 0;
          str++;
  }
  return 1;
}


void   wait_for_query_done()
{
  while (ow_query_work)
    {
      XNextEvent(disp, &ev);
      process_x_event(&ev);
    }
  guiSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
}

void   to_infowin_lastused();

void   ow_f()
{
  ow_query_work = 0;
  ow_kyes->hide();
  ow_kyes_to_all->hide();
  ow_kno->hide();
  ow_kcancel->hide();
  ow_win->hide();
  delete ow_kyes;
  delete ow_kno;
  delete ow_kyes_to_all;
  delete ow_kcancel;
  delete ow_t1;
  delete ow_t2;
  delete ow_t3;
  delete ow_t4;
  delete ow_win;
  panel->expose();
  if (panel->lay == 0)
    panel->panel2->expose();
  XFlush(disp);
  to_infowin_lastused();
}

void   up_f_cancel()
{
  update_query_work = 0;
  ow_kyes->hide();
  ow_kcancel->hide();
  ow_win->hide();
  delete ow_kyes;
  delete ow_kcancel;
  delete ow_t1;
  delete ow_t2;
  delete ow_win;
  panel->expose();
  if (panel->lay == 0)
    panel->panel2->expose();
  XFlush(disp);
  to_infowin_lastused();
}

void   up_f_ok()
{
  update_ok = 1;
  up_f_cancel();
}

void   ow_f_yes()
{
  ow_file = 1;
  ow_f();
}

void   ow_f_yes_to_all()
{
  ow_all = 1;
  ow_f();
}

void   ow_f_no()
{
  ow_file = ow_all = 0;
  ow_cancel = 0;
  ow_no = 1;
  ow_f();
}


void   ow_f_cancel()
{
  ow_file = ow_all = 0;
  ow_cancel = 254;
  ow_f();
}

void   ask_for_update()
{
  ow_win = guiplugin->new_Win(centerx - 205, centery - 80, 410, 160, _("User prompt"), 1);
  ow_t1 = guiplugin->new_Text(10, 50, _("Update archive before exiting from it?"), 1);
  ow_t2 = guiplugin->new_Text(25, 85, _("Press 'Update' if You change files"), 1);
  ow_kyes = guiplugin->new_KEY(10, -20, 180, 25, _("Update"), 1, up_f_ok);
  ow_kcancel = guiplugin->new_KEY(-10, -20, 180, 25, _("Cancel"), 2, up_f_cancel);
  ow_win->init(Main);
  ow_t1->init(ow_win->w);
  ow_t2->init(ow_win->w);
  ow_kyes->init(ow_win->w);
  ow_kcancel->init(ow_win->w);
  ow_win->link(ow_kyes);
  ow_kyes->link(ow_kcancel);
  ow_kcancel->link(ow_kyes);
  ow_win->add_toexpose(ow_t1);
  ow_t1->link(ow_t2);
  ow_win->show();
  ow_t1->show();
  ow_t2->show();
  ow_kyes->show();
  ow_kcancel->show();
  XFlush(disp);
  update_query_work = 1;
  update_ok = 0;
  while (update_query_work)
    {
      XNextEvent(disp, &ev);
      process_x_event(&ev);
    }
  guiSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
}

void   init_overwrite_query(char *head, char *file)
{
  if (ow_query_work)
    return;
  ow_query_work = 1;
  ow_win = guiplugin->new_Win(centerx - 200, centery - 80, 400, 160, head, 5);
  ow_t1 = guiplugin->new_Text(10, 50, _("File:"), 5);
  ow_t2 = guiplugin->new_Text(75, 50, file, 1);
  ow_t3 = guiplugin->new_Text(65, 72, _("This file already exists!"), 1);
  ow_t4 = guiplugin->new_Text(55, 95, _("Overwrite existing file???"), 5);
  ow_kyes = guiplugin->new_KEY(10, -20, 80, 25, "Yes", 1, ow_f_yes);
  ow_kyes_to_all = guiplugin->new_KEY(100, -20, 93, 25, _("Yes to All"), 1, ow_f_yes_to_all);
  ow_kcancel = guiplugin->new_KEY(-10, -20, 80, 25, _("Cancel"), 2, ow_f_cancel);
  ow_kno = guiplugin->new_KEY(-100, -20, 93, 25, _("No"), 1, ow_f_no);
  ow_win->set_iname("OVERWIN");
  ow_kyes->set_iname("OVERYES");
  ow_kyes_to_all->set_iname("OVERALL");
  ow_kcancel->set_iname("OVERCAN");
  ow_kno->set_iname("OVERNO");
  ow_win->init(Main);
  ow_t1->init(ow_win->w);
  ow_t2->init(ow_win->w);
  ow_t3->init(ow_win->w);
  ow_t4->init(ow_win->w);
  ow_win->add_toexpose(ow_t1);
  ow_t1->link(ow_t2);
  ow_t2->link(ow_t3);
  ow_t3->link(ow_t4);
  ow_kyes->init(ow_win->w);
  ow_kyes_to_all->init(ow_win->w);
  ow_kno->init(ow_win->w);
  ow_kcancel->init(ow_win->w);
  ow_win->link(ow_kyes);
  ow_kyes->link(ow_kyes_to_all);
  ow_kyes_to_all->link(ow_kno);
  ow_kno->link(ow_kcancel);
  ow_kcancel->link(ow_kyes);
  ow_win->show();
  ow_t1->show();
  ow_t2->show();
  ow_t3->show();
  ow_t4->show();
  ow_kyes->show();
  ow_kyes_to_all->show();
  ow_kno->show();
  ow_kcancel->show();
  XFlush(disp);
}


Win   *msgw;
KEY   *msgok;
Text  *msgt1;
Text  *msgt2;
Text  *msgt3;
Text  *msgt4;
static  char   attr[] = "rwxrwxrwx";
static  char   size[40] = " ";
static  char   name[280] = " ";

void   msgdone()
{
  msgok->hide();
  msgw->hide();
  delete msgok;
  delete msgt1;
  delete msgt2;
  delete msgt3;
  delete msgt4;
  delete msgw;
  if (panel)
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   msg(char *meshead, char *mes, int col, FList * o)
{
  int    tl = XTextWidth(fontstr, mes, strlen(mes)) + 140;
  int    i, mask = S_IRUSR;
  int    al = 0, sl = 0, fl = 0;
  static char   stat_text[128];
  if (o)
    {
      sprintf(name, _("File:   %s"), o->name);
      sprintf(size, _("Owner:  %s.%s"), finduser(o->uid), findgroup(o->gid));
      for (i = 0; i < 9; i++)
        {
          if ((o->mode & mask) == 0)
            attr[i] = '-';
          mask >>= 1;
        }
      sprintf(stat_text, _("Status: %s"), attr);
      sl = XTextWidth(fontstr, stat_text, strlen(stat_text)) + 140;
      fl = XTextWidth(fontstr, name, strlen(name)) + 140;
    }
  else
      sprintf(stat_text, _("Status: %s"), attr);

  al=XTextWidth(fontstr, stat_text, strlen(stat_text)) + 140;

  if (sl > tl && sl > al && sl > fl)
    tl = sl;
  if (al > sl && al > tl && al > fl)
    tl = al;
  if (fl > tl && fl > al && fl > sl)
    tl = fl;
  tl+=20;
  msgw = guiplugin->new_Win(Mainl / 2 - tl / 2, Mainh / 2 - 90, tl, 145, meshead, 5);
  msgok = guiplugin->new_KEY(-12, -12, 80, 25, _("Agree"), 1, msgdone);
  msgt1 = guiplugin->new_Text(20, 40, mes, col);
  msgt2 = guiplugin->new_Text(20, 60, name, 1);
  msgt3 = guiplugin->new_Text(20, 80, stat_text, 1);
  msgt4 = guiplugin->new_Text(20, 100, size, 1);
  msgw->set_iname("MSGWIN");
  msgok->set_iname("MSG_OK");
  msgw->init(Main);
  msgt1->init(msgw->w);
  msgt2->init(msgw->w);
  msgt3->init(msgw->w);
  msgt4->init(msgw->w);
  msgok->init(msgw->w);
  msgw->link(msgok);
  msgw->add_toexpose(msgt1);
  msgt1->link(msgt2);
  msgt2->link(msgt3);
  msgt3->link(msgt4);
  msgw->set_escapefunc(msgdone);
  msgok->set_escapefunc(msgdone);
  msgw->show();
  msgok->show();
  msgt1->show();
  msgt2->show();
  msgt3->show();
  msgt4->show();
  
  XFlush(disp);
}




KEY   *wwk1, *wwk2, *wwk3;
Text  *wt1, *wt2, *wt3, *wt4, *wt5, *wt02;
Panel *pan, *pan2;
ManFuncs    num;
int    attrmode;
char   wpath[L_MAXPATH] = _noop("Path:  none");
char   w_to_path[L_MAXPATH] = _noop("Path:  none");
char   wfile[80] = _noop("File:  unknown");
char   wsize[40] = _noop("Size:  unknown");
char   wattr[40] = _noop("Status: rwxrwxrwx");
char   wowner[40] = _noop("Owner: *********");
char   combuffer[200];
int    comnumber;

FList *acur;
MenuItem swt[] =
    {
	menu_none, _noop("Read"), 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, _noop("Write"), 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, _noop("Execute"), 1, 0, 0, 0, no_icon, AEmpty
    };

MenuItem kills[] =
    {
	menu_none, "KILL [9]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Stop [19]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Cont [18]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Int [2]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Hup [1]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Term [15]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "Alarm [14]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "User1 [10]", 1, 0, 0, 0, no_icon, AEmpty,
	menu_none, "User2 [12]", 1, 0, 0, 0, no_icon, AEmpty
    };

int    sigkills[10] =
{SIGKILL, SIGSTOP, SIGCONT, SIGINT, SIGHUP, SIGTERM, SIGALRM, SIGUSR1, SIGUSR2, SIGKILL};
Sw_panel *wot, *wow, *wgr, *swkill = NULL;

ManFuncs    meswinnum;
void   meswinok()
{
  wwk1->hide();
  delete wwk1;
  wt1->hide();
  delete wt1;
  ww->hide();
  delete ww;
  ww = NULL;
  if (panel)
    {
      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
      panel->funcs(meswinnum);
    }
  XFlush(disp);
}

void   create_meswin(char *head, char *te, ManFuncs n)
{
  if (ww == NULL)
    {
      meswinnum = n;
      ww = guiplugin->new_Win(centerx - 140, centery - 80, 290, 125, head, 5);
      wt1 = guiplugin->new_Text(60, 50, te, 5);
      wwk1 = guiplugin->new_KEY(65, -20, 160, 25, _("OK"), 1, meswinok);
      wt1->recalc();
      ww->init(Main);
      wt1->init(ww->w);
      wwk1->init(ww->w);
      ww->link(wt1);
      wt1->link(wwk1);
      wwk1->link(wwk1);
      ww->add_toexpose(wt1);
      ww->show();
      wt1->show();
      wwk1->show();
      XFlush(disp);
    }
}

void   testok()
{
  int    i, mask = S_IRUSR;
  for (i = 0; i < 3; i++)
    {
      if (wow->get(i))
        acur->mode |= mask;
      else if (acur->mode & mask)
        acur->mode ^= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wgr->get(i))
        acur->mode |= mask;
      else if (acur->mode & mask)
        acur->mode ^= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wot->get(i))
        acur->mode |= mask;
      else if (acur->mode & mask)
        acur->mode ^= mask;
      mask >>= 1;
    }
  wwk1->hide();
  wwk2->hide();
  wow->hide();
  wot->hide();
  wgr->hide();
  ww->hide();
  wt2->hide();
  wt1->hide();
  delete ww;
  delete wwk1;
  delete wwk2;
  delete wow;
  delete wot;
  delete wgr;
  delete wt2;
  delete wt1;
  ww = NULL;
  if (panel)
    {
      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
      panel->funcs(num);
    }
}

void   testcan()
{
  wwk1->hide();
  wwk2->hide();
  wow->hide();
  wot->hide();
  wgr->hide();
  ww->hide();
  wt2->hide();
  wt1->hide();
  delete ww;
  delete wwk1;
  delete wwk2;
  delete wow;
  delete wot;
  delete wgr;
  delete wt2;
  delete wt1;
  ww = NULL;
  if (panel)
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   atcan()
{
  wwk1->hide();
  wwk2->hide();
  wwk3->hide();
  wow->hide();
  wot->hide();
  wgr->hide();
  ww->hide();
  wt2->hide();
  delete ww;
  delete wwk1;
  delete wwk2;
  delete wow;
  delete wot;
  delete wgr;
  delete wt2;
  delete wwk3;
  ww = NULL;
  if (panel)
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   atset()
{
  attrmode = 0;
  int    i, mask = S_IRUSR;
  for (i = 0; i < 3; i++)
    {
      if (wow->get(i))
        attrmode |= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wgr->get(i))
        attrmode |= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wot->get(i))
        attrmode |= mask;
      mask >>= 1;
    }
  atcan();
  panel->setattrs(1,attrmode);
  panel->reread();
}

void   atclear()
{
  attrmode = 0xffffffff;
  int    i, mask = S_IRUSR;
  for (i = 0; i < 3; i++)
    {
      if (wow->get(i))
        attrmode ^= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wgr->get(i))
        attrmode ^= mask;
      mask >>= 1;
    }
  for (i = 0; i < 3; i++)
    {
      if (wot->get(i))
        attrmode ^= mask;
      mask >>= 1;
    }
  atcan();
  panel->setattrs(0,attrmode);
  panel->reread();
}

void   attrib(FList * cur, ManFuncs inum)
{
  int    i, mask = S_IRUSR;
  if (ww == NULL)
    {
      num = inum;
      acur = cur;
      sprintf(wfile, _("File: %s"), cur->name);
      ww = guiplugin->new_Win(centerx - 140, centery - 150, 290, 305, _("Attributes"), 5);
      wwk1 = guiplugin->new_KEY(150, -70, 120, 25, _("OK"), 1, testok);
      wwk2 = guiplugin->new_KEY(150, -20, 120, 25, _("Cancel"), 2, testcan);
      wow = guiplugin->new_Sw_panel(15, 70, 120, _("Owner:"), swt, 3);
      wgr = guiplugin->new_Sw_panel(150, 70, 120, _("Group:"), swt, 3);
      wot = guiplugin->new_Sw_panel(15, 190, 120, _("Other:"), swt, 3);
      wt2 = guiplugin->new_Text(10, 40, wfile, 1);
      wt2->recalc();
      sprintf(wowner, _("Owner: %s.%s"), finduser(cur->uid), findgroup(cur->gid));
      wt1 = guiplugin->new_Text(10, 60, wowner, 1);
      wt1->recalc();
      ww->set_iname("ATTRWIN");
      wwk1->set_iname("ATTR_OK");
      wwk2->set_iname("ATTR_CAN");
      ww->init(Main);
      wwk1->init(ww->w);
      wwk2->init(ww->w);
      wow->init(ww->w);
      wot->init(ww->w);
      wgr->init(ww->w);
      wt2->init(ww->w);
      wt1->init(ww->w);
      ww->add_toexpose(wt1);
      wt1->link(wt2);
      ww->link(wwk1);
      wwk1->link(wwk2);
      wwk2->link(wow->ar[0]);
      wow->link(wot->ar[0]);
      wot->link(wgr->ar[0]);
      wgr->link(wwk1);
      wwk1->set_escapefunc(testcan);
      wwk2->set_escapefunc(testcan);
      wow->set_escapefunc(testcan);
      wgr->set_escapefunc(testcan);
      wot->set_escapefunc(testcan);
      for (i = 0; i < 3; i++)
        {
          if ((cur->mode & mask) == 0)
            wow->set(i, 0);
          else
            wow->set(i, 1);
          mask >>= 1;
        }
      for (i = 0; i < 3; i++)
        {
          if ((cur->mode & mask) == 0)
            wgr->set(i, 0);
          else
            wgr->set(i, 1);
          mask >>= 1;
        }
      for (i = 0; i < 3; i++)
        {
          if ((cur->mode & mask) == 0)
            wot->set(i, 0);
          else
            wot->set(i, 1);
          mask >>= 1;
        }
      ww->show();
      wwk1->show();
      wwk2->show();
      wow->show();
      wgr->show();
      wot->show();
      wt2->show();
      wt1->show();
    }
}

void   attrib2()
{
  if (ww == NULL)
    {
      strcpy(wfile, _("For all selected files:"));
      ww = guiplugin->new_Win(centerx - 140, centery - 150, 290, 305, _("Set/Clear attributes"), 5);
      wwk1 = guiplugin->new_KEY(150, -90, 120, 25, _("Set"), 1, atset);
      wwk2 = guiplugin->new_KEY(150, -50, 120, 25, _("Clear"), 1, atclear);
      wwk3 = guiplugin->new_KEY(150, -15, 120, 25, _("Cancel"), 2, atcan);
      wow = guiplugin->new_Sw_panel(15, 70, 120, _("Owner:"), swt, 3);
      wgr = guiplugin->new_Sw_panel(150, 70, 120, _("Group:"), swt, 3);
      wot = guiplugin->new_Sw_panel(15, 190, 120, _("Other:"), swt, 3);
      wt2 = guiplugin->new_Text(10, 50, wfile, 1);
      wt2->recalc();
      ww->set_iname("ATTR2WIN");
      wwk1->set_iname("ATTR2_SET");
      wwk2->set_iname("ATTR2_CLE");
      wwk3->set_iname("ATTR2_CAN");
      ww->init(Main);
      wwk1->init(ww->w);
      wwk2->init(ww->w);
      wwk3->init(ww->w);
      wow->init(ww->w);
      wot->init(ww->w);
      wgr->init(ww->w);
      wt2->init(ww->w);
      ww->link(wwk1);
      wwk1->link(wwk2);
      wwk2->link(wwk3);
      wwk3->link(wow->ar[0]);
      wow->link(wot->ar[0]);
      wot->link(wgr->ar[0]);
      wgr->link(wwk1);
      wwk1->set_escapefunc(atcan);
      wwk2->set_escapefunc(atcan);
      wwk3->set_escapefunc(atcan);
      wow->set_escapefunc(atcan);
      wgr->set_escapefunc(atcan);
      wot->set_escapefunc(atcan);
      ww->add_toexpose(wt2);
      ww->show();
      wwk1->show();
      wwk2->show();
      wwk3->show();
      wow->show();
      wgr->show();
      wot->show();
      wt2->show();
    }
}

void   okquery()
{
  if (winp)                        //Check dbuf for directory existance

    {
      char   type[200];
      struct stat st;
      char  *b = strchr(dbuf, ':');
      if (b == NULL)
        strcpy(type, "DFS");
      else
        strncpy(type, dbuf, b - dbuf);
      if((num==FCOPY || num==FMOVE) &&
	 detect_fs(type) == DFS_TYPE)        //Currently  do checking only on DFS

        {
          if (stat(b ? b + 1 : dbuf, &st) == -1 || S_ISDIR(st.st_mode) == 0)
            {
              XBell(disp, 0);
              return;
            }
        }
    }
  wwk2->hide();
  wwk1->hide();
  if (symsw)
    {
      symlink_as_is = symsw->sw;
      symsw->hide();
      delete symsw;
      symsw = NULL;
      winp->hide();
      delete winp;
      winp = NULL;
    }
  else
    {
      delete wt02;
      delete wt5;
    }
  ww->hide();
  delete ww;
  delete wwk1;
  delete wwk2;
  delete wt1;
  delete wt2;
  delete wt3;
  delete wt4;
  ww = NULL;
  if (panel)
    {
      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
      panel->funcs(num);
    }
}

void   canquery()
{
  wwk2->hide();
  wwk1->hide();
  if (symsw)
    {
      symsw->hide();
      delete symsw;
      symsw = NULL;
      winp->hide();
      delete winp;
      winp = NULL;
    }
  else
    {
      delete wt5;
      delete wt02;
    }
  ww->hide();
  delete ww;
  delete wwk1;
  delete wwk2;
  delete wt1;
  delete wt2;
  delete wt3;
  delete wt4;
  if (afstmp_defined)
    {
      afstmp.delete_vfs_list();
      afstmp_defined = 0;
    }
  bmark->set_recycle_image(0);
  if (panel)
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
  ww = NULL;
}

void   initquery(char *head, Lister * p1, VFS * vfs, ManFuncs inum)
{
    char   tmp1[256];
    int    tl;
    int    i, mask = S_IRUSR;
    if (ww == NULL)
    {
	num = inum;
	sprintf(wpath, _("From: %s"), p1->curdir);
	if (inum == 0)
	    sprintf(w_to_path, _("To:   Recycle Bin"));
	else if (inum == 8)
	    sprintf(w_to_path, _("To:   Printer"));
	else if (vfs->need_dir_prefix)
        {
	    sprintf(dbuf, "%s:%s", vfs->vfs_prefix, vfs->curdir);
	    sprintf(w_to_path, _("To:   %s"), dbuf);
        }
	else
        {
	    sprintf(w_to_path, _("To:   %s"), vfs->curdir);
	    strcpy(dbuf, vfs->curdir);
        }
	strcpy(wattr, _("Status: rwxrwxrwx"));
	if (p1->selfiles)
        {
	    dig2ascii(tmp1, p1->selsize);
	    sprintf(wfile, _("File:   %d selected files"), p1->selfiles);
	    sprintf(wsize, _("Size:   %s"), tmp1);
	    strcpy(wattr, _("Status: *********"));
	    strcpy(wowner, _("Owner:  *********"));
        }
	else
        {
	    dig2ascii(tmp1, p1->cur->size);
	    sprintf(wfile, _("File:   %s"), p1->cur->name);
	    sprintf(wsize, _("Size:   %s"), tmp1);
	    for (i = 0; i < 9; i++)
            {
		if ((p1->cur->mode & mask) == 0)
		    wattr[i + 8] = '-';
		mask >>= 1;
            }
	    sprintf(wowner, _("Owner:  %s.%s"), finduser(p1->cur->uid), findgroup(p1->cur->gid));
        }
	wt1 = guiplugin->new_Text(10, 45, wpath, 1);
	wt02 = guiplugin->new_Text(10, 65, w_to_path, 1);
	wt2 = guiplugin->new_Text(10, 85, wfile, 5);
	wt3 = guiplugin->new_Text(10, 105, wowner, 1);
	wt4 = guiplugin->new_Text(10, 125, wattr, 1);
	wt5 = guiplugin->new_Text(10, 145, wsize, 1);
	wt1->recalc();
	wt2->recalc();
	wt02->recalc();
	wt3->recalc();
	wt4->recalc();
	wt5->recalc();
	tl = (wt2->l > wt1->l ? wt2->l : wt1->l);
	tl = (wt02->l > tl ? wt02->l : tl);
	tl += 40;
	tl= tl > 400 ? tl : 400;
	ww = guiplugin->new_Win(centerx - tl / 2, Mainh / 2 - 100, tl, 160, head, 5);
	wwk1 = guiplugin->new_KEY(-20, -60, 80, 25, _("OK"), 1, okquery);
	wwk2 = guiplugin->new_KEY(-20, -20, 80, 25, _("Cancel"), 2, canquery);
	wwk1->set_escapefunc(canquery);
	wwk2->set_escapefunc(canquery);
	ww->link(wwk1);
	wwk1->link(wwk2);
	wwk2->link(wwk1);
	ww->set_iname("QUERYWIN");
	wwk1->set_iname("QUERY_OK");
	wwk2->set_iname("QUERY_CAN");
	ww->init(Main);
	wwk1->init(ww->w);
	wwk2->init(ww->w);
	wt1->init(ww->w);
	wt02->init(ww->w);
	wt2->init(ww->w);
	wt3->init(ww->w);
	wt4->init(ww->w);
	wt5->init(ww->w);
	ww->add_toexpose(wt1);
	wt1->link(wt2);
	wt2->link(wt02);
	wt02->link(wt3);
	wt3->link(wt4);
	wt4->link(wt5);
	ww->show();
	wwk1->show();
	wwk2->show();
	wt1->show();
	wt2->show();
	wt02->show();
	wt3->show();
	wt4->show();
	wt5->show();
    }
}

void   init_copymove_query(char *head, Lister * p1, VFS * vfs, ManFuncs inum, char* content)
{
  int    tl;
  char tmpst[200];
  if (ww == NULL)
    {
      num = inum;
      sprintf(wpath, _("From:  %s"), p1->curdir);
      if (vfs->need_dir_prefix)
        {
          sprintf(dbuf, "%s:%s", vfs->vfs_prefix, vfs->curdir);
        }
      else
        {
          strcpy(dbuf, vfs->curdir);
        }
      if(content)
      {
	  strcat(dbuf,"/");
	  strcat(dbuf, content);
      }
      dig2ascii(tmpst, p1->selsize);
      sprintf(wfile, _("File:   %d selected files"), p1->selfiles);
      sprintf(wsize, _("Size:   %s"), tmpst);

      wt1 = guiplugin->new_Text(10, 45, wpath, 1);
      wt2 = guiplugin->new_Text(10, 135, wfile, 5);
      wt3 = guiplugin->new_Text(10, 155, wsize, 1);
      wt4 = guiplugin->new_Text(10, 72, _("To:"), 1);
      wt1->recalc();
      wt2->recalc();
      wt3->recalc();
      wt4->recalc();
      tl = (wt2->l > wt1->l ? wt2->l : wt1->l) + 40;
      if (tl < 400)
        tl = 400;
      if(tl > Mainl-40)
	  tl=Mainl-40;
      ww = guiplugin->new_Win(centerx - tl / 2, Mainh / 2 - 100, tl, 170, head, 5);
      wwk1 = guiplugin->new_KEY(-20, -50, 80, 25, _("OK"), 1, okquery);
      wwk2 = guiplugin->new_KEY(-20, -15, 80, 25, _("Cancel"), 2, canquery);
      winp = guiplugin->new_Input(50, 55, tl - 70, 1, okquery);
      sprintf(tmpst,"FUNC%02d",inum);
      winp->set_histid(tmpst);
      if(inum==FSYMLINK)
	  symsw = guiplugin->new_Switch(55, 90, 250, _("Make symbolic link"), 1);
      else
	  symsw = guiplugin->new_Switch(55, 90, 250, _("Translate symlinks as is."), 1);
      symsw->sw= (inum==FMOVE || inum==FSYMLINK) ? 1 : 0;
      symsw->set_disable(inum==FMOVE);
      winp->set_escapefunc(canquery);
      wwk1->set_escapefunc(canquery);
      wwk2->set_escapefunc(canquery);
      ww->link(winp);
      winp->link(symsw);
      symsw->link(wwk1);
      wwk1->link(wwk2);
      wwk2->link(winp);
      ww->set_iname("COPYWIN");
      wwk1->set_iname("COPY_OK");
      wwk2->set_iname("COPY_CAN");
      ww->init(Main);
      winp->init(ww->w);
      symsw->init(ww->w);
      wwk1->init(ww->w);
      wwk2->init(ww->w);
      wt1->init(ww->w);
      wt2->init(ww->w);
      wt3->init(ww->w);
      wt4->init(ww->w);
      winp->setbuf(dbuf, L_MAXPATH-2);
      winp->bool_expand=YES;        //Allow TAB expansion.
      ww->add_toexpose(wt1);
      wt1->link(wt2);
      wt2->link(wt3);
      wt3->link(wt4);
      ww->show();
      winp->show();
      symsw->show();
      wwk1->show();
      wwk2->show();
      wt1->show();
      wt2->show();
      wt3->show();
      wt4->show();
    }
}

//////////////////////////////////////////////////////////////////////////////
Win   *dw = NULL;
KEY   *dok;
KEY   *dcan;
Input *din;
Text  *dt1, *dt2, *dt3, *dt4, *dt5;

void   dialogok()
{
  if (din->bl)
  {
      din->hide();
      dok->hide();
      dcan->hide();
      if (symsw)
      {
          symlink_as_is = symsw->sw;
          symsw->hide();
          delete symsw;
          symsw = NULL;
      }
      dw->hide();
      delete dw;
      delete din;
      delete dok;
      delete dcan;
      delete dt1;
      delete dt2;
      delete dt3;
      dw = NULL;
      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
      panel->funcs(num);
  }
}

void   dialogcan()
{
  din->hide();
  dok->hide();
  dcan->hide();
  if (symsw)
    {
      symsw->hide();
      delete symsw;
      symsw = NULL;
    }
  dw->hide();
  delete dw;
  delete din;
  delete dok;
  delete dcan;
  delete dt1;
  delete dt2;
  delete dt3;
  dw = NULL;
  if (afstmp_defined)
    {
      afstmp.delete_vfs_list();
      afstmp_defined = 0;
    }
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   nfok()
{
  if (din->bl)
    {
      din->hide();
      dok->hide();
      dcan->hide();
      dw->hide();
      delete dw;
      delete din;
      delete dok;
      delete dcan;
      delete dt1;
      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
      dw = NULL;
      panel->funcs(num);
    }
}

void   nfcan()
{
  din->hide();
  dok->hide();
  dcan->hide();
  dw->hide();
  delete dw;
  delete din;
  delete dok;
  delete dcan;
  delete dt1;
  dw = NULL;
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   mes_cancel()
{
  dok->hide();
  dw->hide();
  delete dw;
  delete dok;
  delete dt1;
  dw = NULL;
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   fchowncan()
{
  din->hide();
  dok->hide();
  dcan->hide();
  pan->hide();
  pan2->hide();
  dw->hide();
  delete dw;
  delete din;
  delete dok;
  delete dcan;
  delete dt1;
  delete dt2;
  delete dt3;
  delete dt4;
  delete dt5;
  delete pan;
  delete pan2;
  dw = NULL;
  for (int i = 0; i < 200; i++)
    panlist[i] = NULL;
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

void   fchownok()
{
  fchowncan();
  panel->funcs(FCHOWN);
}

int    panusr(int, char *m)
{
  char   str[30] = "";
  int    i = 0;
  while (dbuf[i] != '.' && dbuf[i] != 0)
    i++;
  if (dbuf[i] == '.')
    strcpy(str, dbuf + i);
  strcpy(dbuf, m);
  strcat(dbuf, str);
  din->setbuf(dbuf, 56);
  din->expose();
  return 0;
}

int    pangrp(int, char *m)
{
  int    i = 0;
  while (dbuf[i] != '.' && dbuf[i] != 0)
    i++;
  if (dbuf[i] == '.')
    strcpy(dbuf + i + 1, m);
  else
    {
      strcat(dbuf, ".");
      strcat(dbuf, m);
    }
  din->setbuf(dbuf, 56);
  din->expose();
  return 0;
}

int test_strcmp(char *s1, char *s2)
{
  return strcmp(s1,s2);
}

void char_sort(char **list, int max)
{
  int i,j;
  char *tmp;
  for(i=0;i<max-1;i++)
    for(j=i+1;j<max;j++)
    {
            if(strcmp(list[i],list[j])>0)
            {
                    tmp=list[i];
                    list[i]=list[j];
                    list[j]=tmp;
            }
    }
}

void   init_chown_dialog(FList * cur)
{
  USR   *o = users.next;
  char  *at = "rwxrwxrwx";
  int    mask = 0400, u = 0, g = 0, i;
  if (dw == NULL)
    {
      while (o != NULL)
        {
          panlist[u] = o->name;
          u++;
          o = o->next;
        }
      char_sort(panlist, u);
      o = groups.next;
      while (o != NULL)
        {
          panlist2[g] = o->name;
          g++;
          o = o->next;
        }
      char_sort(panlist2, g);
      if (panel->selfiles == 0)
        {
          sprintf(wfile, _("File:  %s"), cur->name);
          sprintf(dbuf, "%s.%s", finduser(cur->uid), findgroup(cur->gid));
          strcpy(wowner, _("Attributes: "));
          for (i = 0; i < 9; i++)
            {
              if (cur->mode & mask)
                wowner[12 + i] = at[i];
              else
                wowner[12 + i] = '-';
              mask >>= 1;
            }
          wowner[12 + i] = 0;
        }
      else
        {
          sprintf(wfile, _("File: %d selected"), panel->selfiles);
          strcpy(wowner, _("Attributes: *********"));
          sprintf(dbuf, "%s.%s", finduser(cur->uid), findgroup(cur->gid));
        }
      dw = guiplugin->new_Win(centerx - 200, centery - 180, 403, 340, _("Change Owner"), 5);
      dok = guiplugin->new_KEY(40, -20, 110, 25, _("Change"), 1, fchownok);
      dcan = guiplugin->new_KEY(-40, -20, 110, 25, _("Cancel"), 2, fchowncan);
      din = guiplugin->new_Input(10, 89, 380, 1, fchownok);
      din->set_histid("CHOWN");
      dt1 = guiplugin->new_Text(10, 40, wfile, 1);
      dt2 = guiplugin->new_Text(10, 60, wowner, 1);
      dt3 = guiplugin->new_Text(10, 80, _("Owner:"), 5);
      dt4 = guiplugin->new_Text(10, 130, _("Users:"), 5);
      dt5 = guiplugin->new_Text(210, 130, _("Groups:"), 5);
      pan = guiplugin->new_Panel(10, 135, 180, 150, panlist, u, 1, panusr);
      pan->escfunc(fchowncan);
      pan2 = guiplugin->new_Panel(210, 135, 180, 150, panlist2, g, 1, pangrp);
      pan2->escfunc(fchowncan);
      din->set_escapefunc(fchowncan);
      dok->set_escapefunc(fchowncan);
      dcan->set_escapefunc(fchowncan);
      dw->set_iname("OWNERWIN");
      dok->set_iname("OWNERCHG");
      dcan->set_iname("OWNERCAN");
      dw->init(Main);
      din->init(dw->w);
      pan->init(dw->w);
      pan2->init(dw->w);
      dok->init(dw->w);
      dcan->init(dw->w);
      dt1->init(dw->w);
      dt2->init(dw->w);
      dt3->init(dw->w);
      dt4->init(dw->w);
      dt5->init(dw->w);
      dw->link(din);
      din->link(pan);
      pan->link(pan2);
      pan2->link(dok);
      dok->link(dcan);
      dcan->link(din);
      din->setbuf(dbuf, 56);
      dw->add_toexpose(dt1);
      dt1->link(dt2);
      dt2->link(dt3);
      dt3->link(dt4);
      dt4->link(dt5);
      dw->show();
      din->show();
      dok->show();
      dcan->show();
      pan->show();
      pan2->show();
      dt1->show();
      dt2->show();
      dt3->show();
      dt4->show();
      dt5->show();
      pan->find_and_set(finduser(cur->uid));
      pan2->find_and_set(findgroup(cur->gid));
    }
}

/**************************FTP configuration windows**************************/
FTPCFG_S ftprec;

Win *ftpw = NULL;
Pager *ftpwp;
KEY *ftpok, *ftpcan, *ftpk1;
Text *ftpt1, *ftpt2, *ftpt3, *ftpt4, *ftpt5, *ftpt6, *ftpt7, *ftpt8, *ftpt9,
    *ftpt10, *ftpt11, *ftpt12, *ftpt13;
Input *ftpin1, *ftpin2, *ftpin3, *ftpin4, *ftpin5, *ftpin6;
char ftp_port_chr[10]="21";
Switch *proxsw, *ftpsw;

Switch *ftpsw1;
Input *fpathin,*flogin,*fpwdin;
Text *ft4,*ft5;

Win *proxw = NULL;
Input *proxin,*proxin2,*proxin3;
Text *proxt1, *proxt2, *proxt3;
KEY *proxok, *proxcan;
int bool_anon=YES;


void ftp_can()
{
  ftpok->hide();
  ftpcan->hide();
  ftpwp->hide();
  ftpw->hide();

  ftpin1->hide();
  ftpin2->hide();
  ftpin3->hide();
  ftpin4->hide();
  ftpin5->hide();
  ftpin6->hide();

  delete ftpok;
  delete ftpcan;
  delete ftpwp;
  delete pan;
  delete ftpt1;
  delete ftpt2;
  delete ftpt3;
  delete ftpt4;
  delete ftpt5;
  delete ftpt6;
  delete ftpt7;
  delete ftpt8;
  delete ftpt9;
  delete ftpt10;
  delete ftpt11;
  delete ftpt12;
  delete ftpt13;
  delete ftpin1;
  delete ftpin2;
  delete ftpin3;
  delete ftpin4;
  delete ftpin5;
  delete ftpin6;
  delete ftpk1;
  delete ftpsw;
  delete proxsw;
  delete proxt1;
  delete proxt2;
  delete proxt3;
  delete proxin;
  delete proxin2;
  delete proxin3;
  delete ftpw;
  ftpw = NULL;
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}


void ftp_translate_values()
{
    ftprec.ftpport=atoi(ftp_port_chr);
    if(proxsw->sw && ftprec.proxhost[0]!=0 &&
       ftprec.proxlogin[0]!=0)
          ftprec.bool_prox=1;
    else
        ftprec.bool_prox=0;
}


void ftp_ok()
{

  ftp_translate_values();

  ftp_can();
  panel->funcs(num);

}

void ftp_opt()
{
    simple_mes(_("Sorry"),_("Not implemented yet!"));
}


void prox_can()
{
   proxt1->hide();
   proxt2->hide();
   proxt3->hide();
   proxin->hide();
   proxin2->hide();
   proxin3->hide();
   proxok->hide();
   proxcan->hide();
   proxw->hide();
   
   delete proxt1;
   delete proxt2;
   delete proxt3;
   delete proxin;
   delete proxin2;
   delete proxin3;
   delete proxok;
   delete proxcan;
   delete proxw;
   
   proxsw->sw=0;
   proxsw->expose();
   guiSetInputFocus(disp,ftpw->w,RevertToNone,CurrentTime);
   ftprec.bool_prox=0;
}
   
void prox_ok()
{
     if(blank(ftprec.proxhost))
     {
             simple_mes(_("Error"),_("Hostname can't be blank!"));
             return;
     }
     if(blank(ftprec.proxlogin))
     {
             simple_mes(_("Error"),_("Login can't be blank!"));
             return;
     }
     if(blank(ftprec.proxpasswd))
     {
             simple_mes(_("Error"),_("Password can't be blank!"));
             return;
     }
     prox_can();
     proxsw->sw=1;
     proxsw->expose();
     ftprec.bool_prox=1;
}

void   ftp_prox()
{
      proxw = guiplugin->new_Win(centerx - 150, centery - 100, 300, 220, _("Ftp Proxy Config"), 5);
      proxt1 = guiplugin->new_Text(10, 40, _("Ftp Proxy host:"), 5);
      proxin = guiplugin->new_Input(10, 49, 280, 1, prox_ok);
      proxt2 = guiplugin->new_Text(10, 92, _("FTP Proxy Login:"), 5);
      proxin2 = guiplugin->new_Input(10, 99, 280, 1, prox_ok);
      proxt3 = guiplugin->new_Text(10, 142, _("Password:"), 1);
      proxin3 = guiplugin->new_Input(10, 149, 280, 1, prox_ok);
      proxok = guiplugin->new_KEY(15, -10, 120, 25, _("OK"), 1, prox_ok);
      proxcan = guiplugin->new_KEY(-15, -10, 120, 25, _("Cancel"), 2, prox_can);
      
      proxw->init(Main);
      proxt1->init(proxw->w);
      proxin->init(proxw->w);
      proxt2->init(proxw->w);
      proxin2->init(proxw->w);
      proxt3->init(proxw->w);
      proxin3->init(proxw->w);
      proxok->init(proxw->w);
      proxcan->init(proxw->w);
      
      proxw->link(proxin);
      proxin->link(proxin2);
      proxin2->link(proxin3);
      proxin3->link(proxok);
      proxok->link(proxcan);

      proxin->set_escapefunc(prox_can);
      proxin2->set_escapefunc(prox_can);
      proxin3->set_escapefunc(prox_can);
      proxcan->set_escapefunc(prox_can);
      proxok->set_escapefunc(prox_can);

      proxin->set_histid("PROXH");
      proxin2->set_histid("PROXL");
      proxin3->set_histid("PROXP");
      
      proxin3->passwd=1;

      proxin->setbuf(ftprec.proxhost, 80);
      proxin2->setbuf(ftprec.proxlogin, 80);
      proxin3->setbuf(ftprec.proxpasswd, 80);
      
      proxt1->link(proxt2);
      proxt2->link(proxt3);
      proxw->add_toexpose(proxt1);
      
      proxw->show();
      proxt1->show();
      proxt2->show();
      proxt3->show();
      proxin->show();
      proxin2->show();
      proxin3->show();
      proxok->show();
      proxcan->show();      
}




void ftp_anon()
{
  strcpy(ftprec.ftplogin, "anonymous");
  strcpy(ftprec.ftppasswd, "xnc_user@host.domain");
  ftpin4->setbuf(ftprec.ftplogin, 40);
  ftpin5->setbuf(ftprec.ftppasswd, 40);
  ftpin4->expose();
  ftpin5->expose();
  XFlush(disp);
}

int ftp_list_to_host(int idx, char*)
{
  FTPCFG_S *frec=(FTPCFG_S*)hosts_list[idx];
  if(frec==NULL)
     return 0;
  ftprec=*frec;
  ftpt5->recalc();
  ftpt5->clear_text_area(260);
  ftpt5->expose();
  ftpt6->recalc();
  ftpt6->clear_text_area(260);
  ftpt6->expose();
  ftpt7->recalc();
  ftpt7->clear_text_area(260);
  ftpt7->expose();
  ftpin1->setbuf(ftprec.ftpbookmark, 80); 
  ftpin2->setbuf(ftprec.ftphost, 80); 
  ftpin3->setbuf(ftprec.ftppath, 1024); 
  ftpin4->setbuf(ftprec.ftplogin, 40); 
  ftpin5->setbuf(ftprec.ftppasswd, 40);
  proxin->setbuf(ftprec.proxhost, 80);
  proxin2->setbuf(ftprec.proxlogin, 80);
  proxin3->setbuf(ftprec.proxpasswd, 80);
  sprintf(ftp_port_chr, "%d", ftprec.ftpport);
  ftpin6->setbuf(ftp_port_chr, 10);
  if(strcmp(ftprec.ftplogin, "anonymous")==0)
      ftpsw->sw=1;
  else
      ftpsw->sw=0;

  proxsw->sw=ftprec.bool_prox;
  return 0;
}

int ftp_list_to_host_and_go(int idx, char* ptr)
{
  ftp_list_to_host(idx,ptr);
  ftp_ok();
  return 0;
}


void   ftp_update_rec()
{
    FTPCFG_S* frec=ftpcfg_findhost(&ftprec, FCFG_FBOOK);

    ftp_translate_values();
    
    if(frec)
    {
        *frec=ftprec;
        ftpcfg_save_all_hosts();
        pan->find_and_set((char*)&ftprec);
    }
    else
    {
        ftpcfg_addhost(&ftprec);
        pan->set_max_to(max_hosts);
        pan->cursor_to(max_hosts-1);
    }
}


void   new_ftp_prompt(Lister *l1, ManFuncs inum)
{
  if (ftpw == NULL)
    {
      num=inum;
      ftpcfg_reinit_hosts();
      ftpw   = guiplugin->new_Win(centerx - 200, centery - 220, 403, 440, _("Ftp Connection"), 5);
      ftpwp  = guiplugin->new_Pager(10, 30, 380, 370, 3);
      ftpok  = guiplugin->new_KEY(-105, -10, 80, 25, _("Connect"), 1, ftp_ok);
      ftpcan = guiplugin->new_KEY(-15, -10, 80, 25, _("Cancel"), 2, ftp_can);
      ftpw->set_iname("FTPWIN");
      ftpok->set_iname("FTPOK");
      ftpcan->set_iname("FTPCAN");
      ftpwp->set_iname("FTPPAGE");
      ftpt1  = guiplugin->new_Text(10, 50, _("FTP Sessions:"), 5);
      pan    = guiplugin->new_Panel(10, 60, 360, 220, hosts_list, max_hosts, 1, 
				    ftp_list_to_host_and_go);
      
      if(max_hosts)
          ftprec=*((FTPCFG_S*)hosts_list[0]);
      
      ftpt2  = guiplugin->new_Text(10, 307, _("Host:"), 1);
      ftpt3  = guiplugin->new_Text(10, 330, _("Login:"), 1);
      ftpt4  = guiplugin->new_Text(10, 353, _("Path:"), 1);

      ftpt5  = guiplugin->new_Text(100, 305, ftprec.ftphost, 1);
      ftpt6  = guiplugin->new_Text(100, 330, ftprec.ftplogin, 1);
      ftpt7  = guiplugin->new_Text(100, 355, ftprec.ftppath, 1);

      ftpt8  = guiplugin->new_Text(10, 50,  _("Session name:"), 5);
      ftpin1 = guiplugin->new_Input(20, 60, 325, 1, ftp_ok);

      ftpt9  = guiplugin->new_Text(10, 100, _("Host name:"), 1);
      ftpin2 = guiplugin->new_Input(20, 110, 325, 1, ftp_ok);

      ftpt10 = guiplugin->new_Text(10, 150, _("Remote path:"), 1);
      ftpin3 = guiplugin->new_Input(20, 160, 325, 1, ftp_ok);

      ftpt11 = guiplugin->new_Text(10, 200, _("Login name:"), 1);
      ftpin4 = guiplugin->new_Input(20, 210, 325, 1, ftp_ok);

      ftpt12 = guiplugin->new_Text(10, 250, _("Password:"), 1);
      ftpin5 = guiplugin->new_Input(20, 260, 325, 1, ftp_ok);

      ftpt13 = guiplugin->new_Text(235, 305, _("Port:"), 1);
      ftpin6 = guiplugin->new_Input(295, 290, 50, 1, ftp_ok);

      ftpk1  = guiplugin->new_KEY(-15, -10, 140, 25, _("Add/Upd"), 5, ftp_update_rec);
      ftpk1->set_iname("FTPUPD");
      ftpsw  = guiplugin->new_Switch(20, 290, 130, _("Anonymous"), 1, ftp_anon);

      proxsw = guiplugin->new_Switch(20, 325, 130, _("Use proxy"), 1);

      proxt1 = guiplugin->new_Text(10, 50, _("Ftp Proxy host:"), 5);
      proxin = guiplugin->new_Input(10, 59, 325, 1, ftp_ok);
      proxt2 = guiplugin->new_Text(10, 102, _("FTP Proxy Login:"), 5);
      proxin2 = guiplugin->new_Input(10, 109, 325, 1, ftp_ok);
      proxt3 = guiplugin->new_Text(10, 152, _("Password:"), 1);
      proxin3 = guiplugin->new_Input(10, 159, 325, 1, ftp_ok);

      ftpw->init(Main);
      ftpwp->init(ftpw->w);
      
      ftpok->init(ftpw->w);
      ftpcan->init(ftpw->w);
      
      ftpt1->init(ftpwp->w);
      ftpt2->init(ftpwp->w);
      ftpt3->init(ftpwp->w);
      ftpt4->init(ftpwp->w);
      proxt1->init(ftpwp->w);
      proxt2->init(ftpwp->w);
      proxt3->init(ftpwp->w);
      ftpt5->init(ftpwp->w);
      ftpt6->init(ftpwp->w);
      ftpt7->init(ftpwp->w);
      ftpt8->init(ftpwp->w);
      ftpt9->init(ftpwp->w);
      ftpt10->init(ftpwp->w);
      ftpt11->init(ftpwp->w);
      ftpt12->init(ftpwp->w);
      ftpt13->init(ftpwp->w);
      proxin->init(ftpwp->w);
      proxin2->init(ftpwp->w);
      proxin3->init(ftpwp->w);
      ftpin1->init(ftpwp->w);
      ftpin2->init(ftpwp->w);
      ftpin3->init(ftpwp->w);
      ftpin4->init(ftpwp->w);
      ftpin5->init(ftpwp->w);
      ftpin6->init(ftpwp->w);
      ftpk1->init(ftpwp->w);
      ftpsw->init(ftpwp->w);
      proxsw->init(ftpwp->w);
      pan->init(ftpwp->w);
      pan->escfunc(ftp_can);
      pan->set_every_move_to(ftp_list_to_host);
      ftpin1->set_escapefunc(ftp_can);
      ftpin2->set_escapefunc(ftp_can);
      ftpin3->set_escapefunc(ftp_can);
      ftpin4->set_escapefunc(ftp_can);
      ftpin5->set_escapefunc(ftp_can);
      ftpin6->set_escapefunc(ftp_can);
      
      proxin->set_escapefunc(ftp_can);
      proxin2->set_escapefunc(ftp_can);
      proxin3->set_escapefunc(ftp_can);

      ftpwp->setpage(0);
      ftpwp->setpagename(_("Sites"));
      ftpwp->setpage(1);
      ftpwp->setpagename(_("New site"));
      ftpwp->setpage(2);
      ftpwp->setpagename(_("Proxy"));
      
      ftpwp->setpage(0);
      ftpwp->pagemaxobj(8);
      ftpwp->addobj(pan);
      ftpwp->addobj(ftpt1);
      ftpwp->addobj(ftpt2);
      ftpwp->addobj(ftpt3);
      ftpwp->addobj(ftpt4);
      ftpwp->addobj(ftpt5);
      ftpwp->addobj(ftpt6);
      ftpwp->addobj(ftpt7);
      
      ftpwp->setpage(1);
      ftpwp->pagemaxobj(14);
      ftpwp->addobj(ftpin1);
      ftpwp->addobj(ftpt8);
      ftpwp->addobj(ftpt9);
      ftpwp->addobj(ftpin2);
      ftpwp->addobj(ftpt10);
      ftpwp->addobj(ftpin3);
      ftpwp->addobj(ftpt11);
      ftpwp->addobj(ftpin4);
      ftpwp->addobj(ftpt12);
      ftpwp->addobj(ftpin5);
      ftpwp->addobj(ftpt13);
      ftpwp->addobj(ftpin6);
      ftpwp->addobj(ftpk1);
      ftpwp->addobj(ftpsw);
      ftpwp->addobj(proxsw);
      ftpin1->link(ftpin2);
      ftpin2->link(ftpin3);
      ftpin3->link(ftpin4);
      ftpin4->link(ftpin5);
      ftpin5->link(ftpin6);
      ftpin6->link(ftpsw);
      ftpsw->link(proxsw);
      proxsw->link(ftpk1);
      ftpk1->link(ftpin1);


      ftpwp->setpage(2);
      ftpwp->pagemaxobj(6);
      ftpwp->addobj(proxt1);
      ftpwp->addobj(proxt2);
      ftpwp->addobj(proxt3);
      ftpwp->addobj(proxin);
      ftpwp->addobj(proxin2);
      ftpwp->addobj(proxin3);
      proxin->link(proxin2);
      proxin2->link(proxin3);
      proxin3->link(proxin);

      ftpwp->setpage(0);

      ftpin1->set_iname("FTPB");
      ftpin1->set_histid("FTPB");
      ftpin1->setbuf(ftprec.ftpbookmark, 80); 
      ftpin2->set_iname("FTPH");
      ftpin2->set_histid("FTPH");
      ftpin2->setbuf(ftprec.ftphost, 80); 
      ftpin4->set_iname("FTPL");
      ftpin4->set_histid("FTPL");
      ftpin4->setbuf(ftprec.ftplogin, 40); 
      ftpin3->set_iname("FTPP");
      ftpin3->set_histid("FTPP");
      ftpin3->setbuf(ftprec.ftppath, 1024);
      ftpin5->set_iname("FTPPW");
      ftpin5->set_histid("FTPPW");
      ftpin5->passwd=1;
      ftpin5->setbuf(ftprec.ftppasswd, 40);
      ftpin6->set_iname("FTPPRT");
      ftpin6->set_histid("FTPPRT");
      sprintf(ftp_port_chr, "%d", ftprec.ftpport);
      ftpin6->setbuf(ftp_port_chr, 10);
      if(strcmp(ftprec.ftplogin, "anonymous")==0)
          ftpsw->sw=1;
      else
          ftpsw->sw=0;
          
      proxsw->sw=ftprec.bool_prox;

      proxin->set_histid("PROXH");
      proxin2->set_histid("PROXL");
      proxin3->set_histid("PROXP");
      
      proxin3->passwd=1;

      proxin->setbuf(ftprec.proxhost, 80);
      proxin2->setbuf(ftprec.proxlogin, 80);
      proxin3->setbuf(ftprec.proxpasswd, 80);
      

      ftpok->link(ftpcan);
      ftpcan->link(ftpok);
      ftpw->show();
      ftpwp->show();
      ftpok->show();
      ftpcan->show();
      pan->set_selffocus();
    }
}

void   set_dbuf(VFS* vf, char* content)
{
      if (vf)
        {
          if (vf->need_dir_prefix)
            sprintf(dbuf, "%s:%s", vf->vfs_prefix, vf->curdir);
          else
            strcpy(dbuf, vf->curdir);
          if (dbuf[strlen(dbuf) - 1] != '/')
            strcat(dbuf, "/");
        }
      if(content)
	  strcat(dbuf, content);
}

void   initdialog(char *head, Lister * l1, VFS * vf, ManFuncs inum, char* content)
{
  char tmpst[200];
  int  tl;
  if (dw == NULL)
    {
      num = inum;
      sprintf(wfile,  _("File:  %s"), l1->cur->name);
      sprintf(wowner, _("Owner: %s.%s"), finduser(l1->cur->uid), findgroup(l1->cur->gid));
      dok = guiplugin->new_KEY(40, -20, 80, 25, _("OK"), 1, dialogok);
      dcan = guiplugin->new_KEY(-40, -20, 80, 25, _("Cancel"), 2, dialogcan);
      sprintf(tmpst,"FUNC%02d",inum);
      dt1 = guiplugin->new_Text(10, 40, wfile, 5);
      dt2 = guiplugin->new_Text(10, 60, wowner, 1);
      dt3 = guiplugin->new_Text(10, 80, _("To directory:"), 1);

      dt1->recalc();
      tl = dt1->l  + 40;
      tl= tl > 400 ? tl : 400;
      dw = guiplugin->new_Win(centerx - tl/2, centery - 120, tl, 195, head, 5);
      din = guiplugin->new_Input(10, 89, tl-20, 1, dialogok);
      din->set_histid(tmpst);
      din->bool_expand=YES;
      
      if(inum==FSYMLINK)
	  symsw = guiplugin->new_Switch(20, 120, 230, _("Make symbolic link"), 5);
      else
	  symsw = guiplugin->new_Switch(20, 120, 230, _("Translate symlinks as is."), 5);
      symsw->sw = (inum==FMOVE || inum==FSYMLINK);
      symsw->set_disable(inum==FMOVE);
      din->set_escapefunc(dialogcan);
      dok->set_escapefunc(dialogcan);
      dcan->set_escapefunc(dialogcan);
      dw->set_iname("DIALWIN");
      dok->set_iname("DIAL_OK");
      dcan->set_iname("DIAL_CAN");
      dw->init(Main);
      din->init(dw->w);
      dok->init(dw->w);
      dcan->init(dw->w);
      dt1->init(dw->w);
      dt2->init(dw->w);
      dt3->init(dw->w);
      symsw->init(dw->w);
      dw->link(din);
      din->link(symsw);
      symsw->link(dok);
      dok->link(dcan);
      dcan->link(din);
      dbuf[0] = 0;
      if (vf)
        {
          if (vf->need_dir_prefix)
            sprintf(dbuf, "%s:%s", vf->vfs_prefix, vf->curdir);
          else
            strcpy(dbuf, vf->curdir);
          if (dbuf[strlen(dbuf) - 1] != '/')
            strcat(dbuf, "/");
        }
      if(content)
	  strcat(dbuf, content);
      din->setbuf(dbuf, L_MAXPATH-2);
      dw->add_toexpose(dt1);
      dt1->link(dt2);
      dt2->link(dt3);
      dw->show();
      din->show();
      dok->show();
      dcan->show();
      dt1->show();
      dt2->show();
      dt3->show();
      symsw->show();
    }
}

void   newtextfile(char *head, Lister * l1, ManFuncs inum, int showtext,
		   char *text_title)
{
  char tmpst[20];
  if (dw == NULL)
    {
	if(text_title==0)
	    text_title=_noop("New Name:");
	num = inum;
	dw = guiplugin->new_Win(centerx - 150, centery-70, 303, 130, head, 5);
	dok = guiplugin->new_KEY(40, -20, 80, 25, _("OK"), 1, nfok);
	dcan = guiplugin->new_KEY(-40, -20, 80, 25, _("Cancel"), 2, nfcan);
	din = guiplugin->new_Input(10, 50, 280, 1, nfok);
	sprintf(tmpst,"FUNC%02d",inum);
	din->set_histid(tmpst);
	dt1 = guiplugin->new_Text(10, 40, _(text_title), 5);
	dok->set_escapefunc(nfcan);
	dcan->set_escapefunc(nfcan);
	din->set_escapefunc(nfcan);
	dw->set_iname("NTEXTWIN");
	dok->set_iname("NTEXT_OK");
	dcan->set_iname("NTEXT_CAN");
	dw->init(Main);
	din->init(dw->w);
	dok->init(dw->w);
	dcan->init(dw->w);
	dt1->init(dw->w);
	dw->link(din);
	din->link(dok);
	dok->link(dcan);
	dcan->link(din);
	dbuf[0] = 0;
	din->setbuf(dbuf, 255);
	if(inum!=FMANPAGE)        //Skip man pages
	    din->bool_expand=YES;
	if(!showtext)
	    din->set_passwd_mode();
	dw->add_toexpose(dt1);
	dw->show();
	din->show();
	dok->show();
	dcan->show();
	dt1->show();
    }
}

void   simple_mes(char *head, char *mes)
{
  if (dw == NULL)
    {
      if(strstr(head,"rror"))
              dt1 = guiplugin->new_Text(20, 50, mes, 2);
      else
              dt1 = guiplugin->new_Text(20, 50, mes, 1);
      dt1->recalc();
      dw = guiplugin->new_Win(centerx - dt1->l / 2 - 20, centery - 80, dt1->l + 40, 110, head, 5);
      dok = guiplugin->new_KEY(30, -20, dt1->l - 30, 25, _("OK"), 1, mes_cancel);
      dok->set_escapefunc(mes_cancel);
      dw->set_iname("SIMPWIN");
      dok->set_iname("SIMP_OK");
      dw->init(Main);
      dw->add_toexpose(dt1);
      dok->init(dw->w);
      dt1->init(dw->w);
      dw->link(dok);
      dok->link(dok);
      dw->add_toexpose(dt1);
      dw->show();
      dok->show();
      dt1->show();
    }
}


void   simple_input_window(char *head, char *mes)
{
  if (dw == NULL)
    {
	dt1 = guiplugin->new_Text(20, 50, mes, 1);
	dt1->recalc();
	dw = guiplugin->new_Win(centerx - 220, centery - 80, 400, 80, head, 5);
	dok->set_escapefunc(mes_cancel);
	dw->set_iname("CDWIN");
	dw->init(Main);
	dw->add_toexpose(dt1);
	dt1->init(dw->w);
	dw->show();
	dt1->show();
    }
}


void   show_vfs_error()
{
  simple_mes(vfshead, last_error_message);
}

void   show_file_error(char *head,FList *o)
{
  msg(head, last_error_message, 2, o);
}

void   pancan()
{
  dw->hide();
  pan->hide();
  if (swkill)
    {
      swkill->hide();
      dok->hide();
      dcan->hide();
      delete swkill;
      delete dok;
      delete dcan;
      swkill = NULL;
    }
  delete pan;
  delete dw;
  dw = NULL;
  if (psmax > 0)
    {
      delps(psmax);
      psmax = 0;
    };
  guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
}

int    panexec(int i, char *m)
{
  pancan();
  panel->ext_exec(m);
  return 0;
}

int    panexec2(int i, char *m)
{
  pancan();
  comnumber = i;
  panel->menu_exec(menucom[i]);
  return 0;
}

char   mntmnt[80];
int    panmnt(int i, char *m)
{
  pancan();
  tt_printf("umount %s\n", m);
  strcpy(mntmnt, m);
  create_meswin(_("Remounting"), _("Change DEVICE NOW"), FMOUNT);
  return 0;
}

int    panmntmnt()
{
  tt_printf("mount %s\n", mntmnt);
  panel->reread();
  if (panel->lay == 0)
    panel->panel2->reread();
  return 0;
}

int    pankill(int i, char *m)
{
  int    pid, ii;
#if defined (__svr4__)
  char dummy[256];
#endif

  if (i > 0)
    {
#if defined (__svr4__)
      sscanf(m,"%s %d",dummy,&pid);
#else
      sscanf(m, "%d", &pid);
#endif
      for (ii = 0; ii < 9; ii++)
        if (swkill->get(ii))
          break;
      pid = kill(pid, sigkills[ii]);
      pancan();
      if (pid == -1)
        simple_mes(_("Error"), _("Kill failed"));
    }
  return 0;
}

void   menukey(KeySym ks, Panel *)
{
  int    i = findmenukey(ks);
  if (i != -1)
    panexec2(i, NULL);
}

void   newpanel(char *head, int n, int (*prfunc) (int, char *), int wl)
{
  if (dw == NULL)
    {
      dw = guiplugin->new_Win(centerx - wl / 2, centery - 100, wl, 200, head, 5);
      pan = guiplugin->new_Panel(20, 30, wl - 40, 150, panlist, n, 1, prfunc);
      dw->set_escapefunc(pancan);
      pan->escfunc(pancan);
      dw->init(Main);
      pan->init(dw->w);
      dw->link(pan);
      pan->link(pan);
      dw->show();
      pan->show();
    }
}

void   pankillok()
{
  pan->activate_function();
}

void   newkillpanel(char *head, int n, int (*prfunc) (int, char *), int wl)
{
  if (dw == NULL)
    {
      dw = guiplugin->new_Win(centerx - wl / 2, centery - 200, wl, 320, head, 5);
      pan = guiplugin->new_Panel(20, 30, wl - 40, 150, panlist, n, 1, prfunc);
      swkill = guiplugin->new_Sw_panel(20, 196, 150, _("Signals for sending:"), kills, 9, 3);
      dok = guiplugin->new_KEY(-15, 220, 100, 25, _("Send"), 1, pankillok);
      dcan = guiplugin->new_KEY(-15, 255, 100, 25, _("Cancel"), 2, pancan);
      dw->set_iname("KILLWIN");
      dok->set_iname("KILLOK");
      dcan->set_iname("KILLCAN");
      pan->escfunc(pancan);
      dw->set_escapefunc(pancan);
      dw->init(Main);
      pan->init(dw->w);
      swkill->init(dw->w);
      dok->init(dw->w);
      dcan->init(dw->w);
      dw->link(pan);
      pan->link(swkill->ar[0]);
      swkill->link(dok);
      dok->link(dcan);
      dcan->link(pan);
      swkill->excl();
      swkill->set(0, 1);
      dw->show();
      pan->show();
      swkill->show();
      dok->show();
      dcan->show();
    }
}

void   dfs_info(int wl)
{
  if (dw == NULL)
    {
      dw = guiplugin->new_Win(centerx - wl / 2, centery - 100, wl, 200, _("Disk Free Space"), 5);
      pan = guiplugin->new_Panel(20, 30, wl - 40, 150, panlist, 0, 1, NULL);
      pan->escfunc(pancan);
      dw->set_escapefunc(pancan);
      dw->init(Main);
      pan->init(dw->w);
      dw->link(pan);
      pan->link(pan);
      dw->show();
      pan->show();
      psmax = getdf(pan);
    }
}

void   free_info(int wl)
{
  if (dw == NULL)
    {
      dw = guiplugin->new_Win(centerx - wl / 2, centery - 80, wl, 160, _("Memory Information"), 5);
      pan = guiplugin->new_Panel(20, 30, wl - 40, 110, panlist, 0, 1, NULL);
      pan->escfunc(pancan);
      dw->set_escapefunc(pancan);
      dw->init(Main);
      pan->init(dw->w);
      dw->link(pan);
      pan->link(pan);
      dw->show();
      pan->show();
      psmax = getfree(pan);
    }
}

void   menupanel(int (*prfunc) (int, char *))
{
  if (dw == NULL)
    {
      dw = guiplugin->new_Win(centerx - 225, centery - 100, 453, 200, _("Menu commands"), 5);
      pan = guiplugin->new_Panel(20, 30, 410, 150, menunames, menumax, 1, prfunc);
      dw->set_escapefunc(pancan);
      pan->escfunc(pancan);
      pan->setkeyfunc(menukey);
      dw->init(Main);
      pan->init(dw->w);
      dw->link(pan);
      pan->link(pan);
      dw->show();
      pan->show();
    }
}

void   selectmask(Lister * l1, ManFuncs inum)
{
  if (dw == NULL)
    {
      num = inum;
      if (num == 5)
        dw = guiplugin->new_Win(centerx - 125, centery-80, 253, 130, _("Select files"), 5);
      else
        dw = guiplugin->new_Win(centerx - 125, centery-80, 253, 130, _("Deselect files"), 5);
      dok = guiplugin->new_KEY(20, -20, 80, 25, _("OK"), 1, nfok);
      dcan = guiplugin->new_KEY(-20, -20, 80, 25, _("Cancel"), 2, nfcan);
      din = guiplugin->new_Input(10, 50, 230, 1, nfok);
      din->set_histid("MASK");
      dt1 = guiplugin->new_Text(10, 40, _("Mask:"), 5);
      din->set_escapefunc(nfcan);
      dw->set_iname("SELWIN");
      dok->set_iname("SEL_OK");
      dcan->set_iname("SEL_CAN");
      dw->init(Main);
      din->init(dw->w);
      dok->init(dw->w);
      dcan->init(dw->w);
      dt1->init(dw->w);
      dw->link(din);
      din->link(dok);
      dok->link(dcan);
      dcan->link(din);
      strcpy(dbuf, l1->maskchr);
      din->setbuf(dbuf, 255);
      dw->add_toexpose(dt1);
      dw->show();
      din->show();
      dok->show();
      dcan->show();
      dt1->show();
    }
}


