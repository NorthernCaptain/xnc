/****************************************************************************
*  Copyright (C) 1996-98 by Leo Khramov
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
/* $Id: menu_process.cxx,v 1.4 2003/12/25 13:27:30 leo Exp $ */

#include "panel.h"
#include "config_file.h"
#include "internals.h"
#include "c_externs.h"
#include "pluginman.h"

extern Lister *lst1, *lst2;

void   sendtoives(char *name, int type, int ret = 0);

void   defret()
{
  if (panel->lay == 0)
    panel->panel2->reread();
  panel->reread();
// guiSetInputFocus(disp,panel->w,RevertToNone,CurrentTime);
}

void   manret()
{
  char   tmps[L_MAXPATH];
  strcpy(tmps, syspath);
  strcat(tmps, "/xnc.tmp");
  chdir(panel->curdir);
  unlink(tmps);
  defret();
}

void   extret()
{
  load_ext();
  defret();
}

void   menuret()
{
  load_menu();
  defret();
}

void   (*funcs[]) () =
{
  defret, manret, extret, menuret
};


void   ives_retval(int ret)
{
  funcs[ret]();
}

void   p_info()
{
  panel->ps_info();
};
void   m_info()
{
  free_info(centerx * 2 - 40);
};
void   d_info()
{
  dfs_info(centerx * 2 - 40);
};
void   ownerfunc()
{
  init_chown_dialog(panel->cur);
};
void   findfunc()
{
  find_panel(centerx * 2 - 40);
};

void   packfunc()
{
    panel->pack();
}

void   donefunc();
void   viewfunc();
void   editfunc();
void   copyfunc();
void   movefunc();
void   delfunc();
void   mkdirfunc();
void   remntfunc();
void   manfunc();
void   revfunc1();
void   revfunc2();
void   p1col1();
void   p1col2();
void   p1col3();
void   p2col1();
void   p2col2();
void   p2col3();
void   p2col4();
void   p1col4();
void   p1ext();
void   p2ext();
void   p1name();
void   p2name();
void   p1time();
void   p2time();
void   p1size();
void   p2size();
void   p1unsort();
void   p2unsort();
void   p1brief();
void   p2brief();
void   p1full();
void   p2full();
void   p1custom();
void   p2custom();
void   configure_func() {show_config_win();};
void   edit_ext();
void   edit_menu();
void   attrfunc();
void   menu_func();
void   branchfunc1();
void   branchfunc2();
void   show_hidden_files();
void   try_clean_afs_cache();
void   ftp_callfunc();
void   compare_dirs() {panel->compare_panels();panel->panel2->compare_panels();};
void   use_sizes() {compare_by_size^=1;};
void   use_times() {compare_by_time^=1;};

MenuBar *ftpmn;
FList *node;

char   so1[15][25] =
{
    _noop("Brief"), 
    _noop("Full"), 
    _noop("Custom "), 
    _noop("One column"), 
    _noop("Two columns"), 
    _noop("Three columns"),
    _noop("Four columns "), 
    _noop("Sort by Names"), 
    _noop("Sort by Extensions"), 
    _noop("Sort by Size"), 
    _noop("Sort by Time"), 
    _noop("Unsorted "), 
    _noop("Reverse sort "),
    _noop("Toggle branch mode "), 
    _noop("Quit")};

MenuItem subm1[] =
{
    menu_ch_set,    so1[0],  1, p1brief,   0, 0, no_icon, ABriefMode,
    menu_ch_unset,  so1[1],  1, p1full,    0, 0, no_icon, AFullMode,
    menu_ch_unset,  so1[2],  1, p1custom,  0, 0, no_icon, ACustomMode,
    menu_ch_unset,  so1[3],  1, p1col1,    0, 0, no_icon, AEmpty,
    menu_ch_set,    so1[4],  1, p1col2,    0, 0, no_icon, AEmpty,
    menu_ch_unset,  so1[5],  1, p1col3,    0, 0, no_icon, AEmpty,
    menu_ch_unset,  so1[6],  1, p1col4,    0, 0, no_icon, AEmpty,
    menu_ch_set,    so1[7],  1, p1name,    0, 0, no_icon, ASortByName,
    menu_ch_unset,  so1[8],  1, p1ext,     0, 0, no_icon, ASortByExt,
    menu_ch_unset,  so1[9],  1, p1size,    0, 0, no_icon, ASortBySize,
    menu_ch_unset,  so1[10], 1, p1time,    0, 0, no_icon, ASortByTime,
    menu_ch_unset,  so1[11], 1, p1unsort,  0, 0, no_icon, ASortByUnsort,
    menu_sw_unset,  so1[12], 1, revfunc1,  0, 0, no_icon, AReverseSort,
    menu_function,  so1[13], 1, branchfunc1,  0, 0, no_icon, ABranchView,
    menu_function,  so1[14], 2, donefunc,  0, 0, delete_icon, AQuit
};

MenuItem optmenu[] =
{
    menu_function,  _noop("Edit menu file"), 1, edit_menu, 0, 0, edit_icon, AEmpty,
    menu_function,  _noop("Edit extension file "), 1, edit_ext, 0, 0, edit_icon, AEmpty,
    menu_function,  _noop("Clean AFS cache "), 1, try_clean_afs_cache, 0, 0, no_icon, ACleanCache,
    menu_function,  _noop("Configuration "), 1, configure_func, 0, 0, config_icon, AEmpty,
    menu_function,  _noop("Help in documentation"), 1, browse_documentation, 0, 0, help_icon, AEmpty,
    menu_function,  _noop("About XNC"), 1, show_about_dialog, 0, 0, owner_icon, AEmpty
};

static ListerVoidCaller rereadcall(&panel,&Lister::reread_all);
static ListerVoidCaller targetsrccall(&panel,&Lister::set_opposite_to_current);
static ListerVoidCaller swapcall(&panel,&Lister::swap_panels);
static ListerVoidCaller leftdircall(&panel,&Lister::insert_left_dir);
static ListerVoidCaller rightdircall(&panel,&Lister::insert_right_dir);
static ListerVoidCaller invertcall(&panel,&Lister::invert_mask_selection);
static ListerVoidCaller addselmaskcall(&panel,&Lister::add_selection_by_mask);
static ListerVoidCaller delselmaskcall(&panel,&Lister::del_selection_by_mask);
static ListerVoidCaller addselextcall(&panel,&Lister::add_selection_by_ext);
static ListerVoidCaller delselextcall(&panel,&Lister::del_selection_by_ext);
static ListerVoidCaller addbookcall(&panel,&Lister::add_to_bookmark);

MenuItem opermenu[] =
{
    menu_function,  _noop("Reread current directory"), 1, 0, 0, &rereadcall, no_icon, ARereadDir,
    menu_function,  _noop("Switch to terminal "), 1, xnc_switch_term, 0, 0, no_icon, ASwitchtoTerm,
    menu_function,  _noop("Add entry to bookmark "), 1, 0, 0, &addbookcall, no_icon, ABookAdd, 
    menu_function,  _noop("Add selection by mask"), 1, 0, 0, &addselmaskcall, no_icon, ASelectMask,
    menu_function,  _noop("Clean selection by mask"), 1, 0, 0, &delselmaskcall, no_icon, ADeselectMask,
    menu_function,  _noop("Invert selection"), 1, 0, 0, &invertcall, no_icon, AInvertMask,
    menu_function,  _noop("Add selection by extension"), 1, 0, 0, &addselextcall, no_icon, ASelectFileMask,
    menu_function,  _noop("Clean selection by extension "), 1, 0, 0, &delselextcall, no_icon, ADeselectFileMask,
    menu_function,  _noop("Insert left directory name"), 1, 0, 0, &leftdircall, no_icon, AInsertLeftDir, 
    menu_function,  _noop("Insert right directory name "), 1, 0, 0, &rightdircall, no_icon, AInsertRightDir,
    menu_function,  _noop("Source <-> Target"), 1, 0, 0, &swapcall, no_icon, ASwapPanels,
    menu_function,  _noop("Target = Source"), 1, 0, 0, &targetsrccall, no_icon, ASetPanelToCurrent
};

char   so2[14][25] =
{
    _noop("Brief"), 
    _noop("Full"), 
    _noop("Custom "), 
    _noop("One column"), 
    _noop("Two columns"), 
    _noop("Three columns"),
    _noop("Four columns "), 
    _noop("Sort by Names"), 
    _noop("Sort by Extensions"), 
    _noop("Sort by Size"), 
    _noop("Sort by Time"), 
    _noop("Unsorted "), 
    _noop("Reverse sort "),
    _noop("Toggle branch mode ")
};

MenuItem subm2[] =
{
    menu_ch_set,    so2[0], 1, p2brief,0, 0, no_icon, ABriefMode,
    menu_ch_unset,  so2[1], 1, p2full, 0, 0, no_icon, AFullMode,
    menu_ch_unset,  so2[2], 1, p2custom, 0, 0, no_icon, ACustomMode,
    menu_ch_unset,  so2[3], 1, p2col1, 0, 0, no_icon, AEmpty,
    menu_ch_set,    so2[4], 1, p2col2, 0, 0, no_icon, AEmpty,
    menu_ch_unset,  so2[5], 1, p2col3, 0, 0, no_icon, AEmpty,
    menu_ch_unset,  so2[6], 1, p2col4, 0, 0, no_icon, AEmpty,
    menu_ch_unset,  so2[7], 1, p2name, 0, 0, no_icon, ASortByName,
    menu_ch_set,    so2[8], 1, p2ext,  0, 0, no_icon, ASortByExt,
    menu_ch_unset,  so2[9], 1, p2size, 0, 0, no_icon, ASortBySize,
    menu_ch_unset,  so2[10], 1, p2time, 0, 0, no_icon, ASortByTime,
    menu_ch_unset,  so2[11], 1, p2unsort, 0, 0, no_icon, ASortByUnsort,
    menu_sw_unset,  so2[12], 1, revfunc2, 0, 0, no_icon, AReverseSort,
    menu_function,  so2[13], 1, branchfunc2, 0, 0, no_icon, ABranchView
};

char so3[2][25]={_noop("Compare by sizes"), _noop("Compare by times")};

static ListerPtrCaller copycall(&panel,&Lister::metacall_copy);
static ListerVoidCaller renamecall(&panel,&Lister::rename);
static ListerVoidCaller linkcall(&panel,&Lister::make_link);

MenuItem submnu2[] =
{
    menu_function,  _noop("Remount current directory"), 1, remntfunc, 0, 0,  remount_icon, ARemount,
    menu_function,  _noop("Menu"), 1, menu_func, 0, 0, menu_icon, AMenu,
    menu_function,  _noop("View file"), 1, viewfunc, 0, 0, view_icon, AFView,
    menu_function,  _noop("Edit file"), 1, editfunc, 0, 0, edit_icon, AFEdit,
    menu_function,  _noop("Copy file"), 1, 0, 0, &copycall, copy_icon, AFCopy,
    menu_function,  _noop("Move/Rename file"), 1, movefunc, 0, 0, move_icon, AFMove,
    menu_function,  _noop("Rename file"), 1, 0, 0, &renamecall, no_icon, ARenameFile,
    menu_function,  _noop("Make directory"), 1, mkdirfunc, 0, 0, mkdir_icon, AMakeDir,
    menu_function,  _noop("Delete file"), 1, delfunc, 0, 0, delete_icon, AFDelete,
    menu_function,  _noop("Make link"), 1, 0, 0, &linkcall, no_icon, ASymLink,
    menu_function,  _noop("Attributes of file"), 1, attrfunc, 0, 0, attr_icon, AAttributes,
    menu_function,  _noop("Change Owner"), 1, ownerfunc, 0, 0, owner_icon, AChown,
    menu_function,  _noop("Create Archive"), 1, packfunc, 0, 0, pack_icon, ACreateArchive,
    menu_function,  _noop("Find files"), 1, findfunc, 0, 0, find_icon, AFind,
    menu_function,  _noop("Quick ScanDir Info"), 1, qview, 0, 0, no_icon, AQuickView,
    menu_function,  _noop("Processes Info"), 1, p_info, 0, 0, proc_icon, AKill,
    menu_function,  _noop("Memory Info"), 1, m_info, 0, 0, no_icon, AMemory,
    menu_function,  _noop("Disk Info"), 1, d_info, 0, 0, disk_icon, ADiskInfo,
    menu_function,  _noop("Stat Info"), 1, toggle_inodewin, 0, 0, no_icon, AInodeInfo,
    menu_function,  _noop("Ftp connection "), 1, ftp_callfunc, 0, 0, ftp_icon, AFtpLink,
    menu_function,  _noop("Compare directories"), 1, compare_dirs, 0, 0, no_icon, AEmpty,
    menu_sw_set,    so3[0], 1, use_sizes, 0, 0, no_icon, AEmpty,
    menu_sw_unset,  so3[1], 1, use_times, 0, 0, no_icon, AEmpty
};

Menu   *submenu1=0;
Menu   *submenu2=0;
Menu   *submenu3=0;
Menu   *submenu4=0;
Menu   *submenu5=0;

MenuItem menu[] =
    {menu_menu, _noop("Left"), 1, 0, submenu1, 0, no_icon, AEmpty,
     menu_menu, _noop("Commands"), 1, 0, submenu3, 0, no_icon, AEmpty,
     menu_menu, _noop("Operations"), 1, 0, submenu5, 0, no_icon, AEmpty,
     menu_menu, _noop("Options"), 1, 0, submenu4, 0, no_icon, AEmpty,
     menu_menu, _noop("-Right"), 1, 0, submenu2, 0, no_icon, AEmpty
};

MenuItem ftpmenu[] =
{menu_menu, _noop("FTP"), 1, 0, submenu1, 0, no_icon};

MenuItem menu2[] =
    {menu_function, _noop("1.Man"), 1, manfunc, 0, 0,  help_icon, AEmpty,
     menu_function, _noop("2.Menu"), 1, menu_func, 0, 0, menu_icon, AEmpty,
     menu_function, _noop("3.View"), 1, viewfunc, 0, 0, view_icon, AEmpty,
     menu_function, _noop("4.Edit"), 1, editfunc, 0, 0, edit_icon, AEmpty,
     menu_function, _noop("5.Copy"), 1, copyfunc, 0, 0, copy_icon, AEmpty,
     menu_function, _noop("6.Move"), 1, movefunc, 0, 0, move_icon, AEmpty,
     menu_function, _noop("7.Mkdir"), 1, mkdirfunc, 0, 0, mkdir_icon, AEmpty,
     menu_function, _noop("8.Delete"), 1, delfunc, 0, 0, delete_icon, AEmpty,
     menu_function, _noop("9.Attr"), 1, attrfunc, 0, 0, attr_icon, AEmpty,
     menu_function, _noop("-0.Quit"), 2, donefunc, 0, 0, no_icon, AEmpty
};

void init_submenus()
{
  submenu1=guiplugin->new_Menu(subm1, 15);
  submenu2=guiplugin->new_Menu(subm2, 14);
  submenu3=guiplugin->new_Menu(submnu2, 23);
  submenu4=guiplugin->new_Menu(optmenu, 6);
  submenu5=guiplugin->new_Menu(opermenu, 12);
  menu[0].o=submenu1;
  menu[1].o=submenu3;
  menu[2].o=submenu5;
  menu[3].o=submenu4;
  menu[4].o=submenu2;
  ftpmenu[0].o=submenu1;
}

void   fqok();
void   fqcan();

void ftp_callfunc()
{
    new_ftp_prompt(panel,FFTPCON);
}

void   menu_func()
{
  menupanel(panexec2);
}

Win    *w1;
KEY    *qok;
KEY    *qcan;

void   donefunc()
{
  if(use_prompt & PROMPT_EXIT)
  {
    w1=guiplugin->new_Win(300, 80, 200, 80, _("Really, QUIT?"), 5);
    qok=guiplugin->new_KEY(10, 35, 80, 25, _("Yes"), 2, fqok);
    qcan=guiplugin->new_KEY(110, 35, 80, 25, _("Never"), 1, fqcan);
    qok->set_iname("Q_YES");
    qcan->set_iname("Q_NEVER");
    w1->set_iname("QWIN");
    w1->init(Main);
    qok->init(w1->w);
    qcan->init(w1->w);
    w1->link(qok);
    qok->link(qcan);
    qcan->link(qok);
    qok->set_escapefunc(fqcan);
    qcan->set_escapefunc(fqcan);
    XMoveWindow(disp, w1->w, Mainl / 2 - 100, Mainh / 2 - 70);
    w1->show();
    qok->show();
    qcan->show();
   }
   else
   {
     clean_exit();
     exit(0);
   }
}

void   attrfunc()
{
  panel->attrib();
}

void   fqok()
{
  qok->hide();
  qcan->hide();
  w1->hide();
  clean_exit();
  exit(0);
}

void   fqcan()
{
  qok->hide();
  qcan->hide();
  w1->hide();
  delete qcan;
  delete qok;
  delete w1;
  guiSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
}

void   p1brief()
{
    lst1->switch_to_brief();
}

void revfunc1()
{
    submenu1->switches(12);
    lst1->switch_reverse();
}

void revfunc2()
{
    submenu2->switches(12);
    lst2->switch_reverse();
}

void   branchfunc1()
{
    lst1->switch_branch_view();
}

void   branchfunc2()
{
    lst2->switch_branch_view();
}

void   p2brief()
{
    lst2->switch_to_brief();
}

void   p1full()
{
    lst1->switch_to_full();
    p1col1();
}

void   p2full()
{
    lst2->switch_to_full();
    p2col1();
}

void   p1custom()
{
    lst1->switch_to_custom();
}

void   p2custom()
{
    lst2->switch_to_custom();
}

void   p1ext()
{
    lst1->sort_by_ext();
}
void   p1name()
{
    lst1->sort_by_name();
}

void   p1time()
{
    lst1->sort_by_time();
}

void   p1unsort()
{
    lst1->sort_by_unsort();
}

void   p2unsort()
{
    lst2->sort_by_unsort();
}
void   p1size()
{
  lst1->sort_by_size();
}

void   p2ext()
{
    lst2->sort_by_ext();
}

void   p2name()
{
    lst2->sort_by_name();
}

void   p2time()
{
    lst2->sort_by_time();
}
void   p2size()
{
    lst2->sort_by_size();
}

void   p1col1()
{
    lst1->switch_to_one_column();
}

void   p1col2()
{
    lst1->switch_to_two_columns();
}

void   p1col3()
{
    lst1->switch_to_three_columns();
}

void   p2col1()
{
    lst2->switch_to_one_column();
}

void   p1col4()
{
    lst1->switch_to_four_columns();
}

void   p2col4()
{
    lst2->switch_to_four_columns();
}

void   p2col2()
{
    lst2->switch_to_two_columns();
}

void   p2col3()
{
    lst2->switch_to_three_columns();
}

void   editfunc()
{
    panel->edit();
}

void   viewfunc()
{
    panel->view(1);
}

void   edit_ext()
{
  char   name[120];
  strcpy(name, getenv("HOME"));
  strcat(name, "/.xnc/xnc.ext");
  if (strcmp(editname, "internal") == 0)
    sendtoives(name, 1, 2);
  else
    {
      tt_printf("%s %s\n", editname, name);
      panel->raise_terminal_if_need();
    }
}

void   edit_menu()
{
  char   name[120];
  strcpy(name, getenv("HOME"));
  strcat(name, "/.xnc/xnc.menu");
  if (strcmp(editname, "internal") == 0)
    sendtoives(name, 1, 3);
  else
    {
      tt_printf("%s %s\n", editname, name);
      panel->raise_terminal_if_need();
    }
}

void   copyfunc()
{
  panel->copy();
}

void   movefunc()
{
  panel->move();
}

void   delfunc()
{
  panel->del();
}

void   mkdirfunc()
{
  panel->mkdir();
}

void   remntfunc()
{
  panel->remount();
}

void   manfunc()
{
  newtextfile(_("Man page"), panel, FMANPAGE);
}
