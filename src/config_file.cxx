/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
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
/* $Id: config_file.cxx,v 1.14 2003/11/27 15:36:43 leo Exp $ */
#include "xncversion.h"
#include "config_file.h"
#include "bookmark.h"
#include "panel.h"
#include "inodewin.h"
#include "c_externs.h"
#include "pluginman.h"

ConfigManager*  cfgman=0;

char       home_files_dir[L_MAXPATH];
int        use_prompt=~0;        //By default activate all prompt windows.
//Additinal options for XNC at startup. Overwrites by xnc.cfg
int        option_bits=FILLED_CURSOR | IVES_EXIT_TOO |
                       STATUS_ATTR | STATUS_NAME | STATUS_TIME | STATUS_SIZE | STATUS_OWNER; 
int        xnc_ver=0,               //XNC version stored in config file
           xnc_orig_ver=0;          //Original XNC version
int        init_starts=0, xnc_starts=0;
int        branch_depth=5;
int        wm_y_correction=21;      //Correction for Window manager title window
char       magic_file_location[L_MAXPATH]="/usr/share/magic";
int        iconset_number=1;        //Iconset style number
char       guiplugin_name[L_MAXPATH]="none"; //Name of gui plugin to load
char       html_browser[L_MAXPATH]="netscape %s"; //Command to run html browser

ConfigDefault  confref[] =
{
  /*  0 */  {0,0},
  /*  1 */  {"panel1.path",                           "/"},
  /*  2 */  {"panel2.path",                           "/"},
  /*  3 */  {"panel.active",                          "panel1"},
  /*  4 */  {"panel1.columns",                        "2"},
  /*  5 */  {"panel2.columns",                        "3"},
  /*  6 */  {"panel1.sortby",                         "name"},
  /*  7 */  {"panel2.sortby",                         "extension"},
  /*  8 */  {"panel1.style",                          "brief"},
  /*  9 */  {"panel2.style",                          "brief"},
  /* 10 */  {"panel1.sort_order",                     "normal"},
  /* 11 */  {"panel2.sort_order",                     "normal"},
  /* 12 */  {"bookmark.book1",                        "/"},
  /* 13 */  {"bookmark.book2",                        ""},
  /* 14 */  {"bookmark.book3",                        ""},
  /* 15 */  {"bookmark.book4",                        ""},
  /* 16 */  {"bookmark.book5",                        ""},
  /* 17 */  {"bookmark.book6",                        ""},
  /* 18 */  {"bookmark.book7",                        ""},
  /* 19 */  {"bookmark.book8",                        ""},
  /* 20 */  {"bookmark.book9",                        ""},
  /* 21 */  {"panels.show_hidden_files",              "yes"},
  /* 22 */  {"panels.percentage",                     "50"},
  /* 23 */  {"panels.prompt.copy",                    "yes"},
  /* 24 */  {"panels.prompt.move",                    "yes"},
  /* 25 */  {"panels.prompt.delete",                  "yes"},
  /* 26 */  {"panels.prompt.overwrite",               "yes"},
  /* 27 */  {"panels.prompt.exit",                    "yes"},
  /* 28 */  {"xnc.term.raise",                        "yes"},
  /* 29 */  {"xnc.bookmark.animation",                "yes"},
  /* 30 */  {"xnc.icons.dir",                         "yes"},
  /* 31 */  {"xnc.icons.file",                        "yes"},
  /* 32 */  {"xnc.version.current",                   "4.9.2   0"}, 
  /* 33 */  {"xnc.total.starts",                      "1"},
  /* 34 */  {"xnc.dirscan.timeout",                   "1"},
  /* 35 */  {"xnc.text.shadow",                       "yes"},
  /* 36 */  {"xnc.compare_by.size",                   "yes"},
  /* 37 */  {"xnc.compare_by.time",                   "yes"},
  /* 38 */  {"panels.cursor.filled",                  "yes"},
  /* 39 */  {"xnc.ives.exit.together",                "yes"},
  /* 40 */  {"xnc.statusbar.show.name",               "yes"},
  /* 41 */  {"xnc.statusbar.show.attr",               "yes"},
  /* 42 */  {"xnc.statusbar.show.time",               "yes"},
  /* 43 */  {"xnc.statusbar.show.size",               "yes"},
  /* 44 */  {"xnc.statusbar.show.owner",              "yes"},
  /* 45 */  {"xnc.statinfo.position",                 "100, 100"},
  /* 46 */  {"xnc.magicfile.path",                    "/etc/magic"},
  /* 47 */  {"panels.format.brief",                   "f"},
  /* 48 */  {"panels.format.full",                    "f|s|d|a|u"},
  /* 49 */  {"panels.format.custom",                  "o|f"},
  /* 50 */  {"xnc.start.top_window",                  "xnc"},
  /* 51 */  {"xnc.iconset.style",                     "1"},
  /* 52 */  {"xnc.plugin.name",                       "none"},
  /* 53 */  {"panels.prompt.pack",                    "yes"},
  /* 54 */  {"xnc.branch_view.depth",                 "3"},
  /* 55 */  {"xnc.html.browser",                      "netscape %s"},
  /* 56 */  {"xnc.navigation.style",                  "xnc"},
  /* 57 */  {"panels.format.maxlen",                  "20"}
};

const int confmax = sizeof(confref)/sizeof(ConfigDefault);

extern Menu *submenu1, *submenu2, *submenu3;

//-------------------------------------------------------------------------
// Create config manager for xnc.cfg file
//-------------------------------------------------------------------------
void create_cfgman(char *dir, char* fname)
{
  char buf[L_MAXPATH];
  sprintf(buf, "%s/%s", dir, fname);
  cfgman=new ConfigManager(buf);
}

//-------------------------------------------------------------------------
// Set values from cfgman to system :)
//-------------------------------------------------------------------------
static int load_config(char *fname, Lister * panel1, Lister * panel2)
{
  char   *w, w1[L_MAXPATH];
  char   *s=XNCVERSION;
  int    i, nn, j, value_int;

  str2version(s+2,&xnc_orig_ver);

  for(i=1;i<confmax;i++)
  {
    w=cfgman->get_value(confref[i].name, confref[i].default_value);
    switch (i)
    {
    case 1:
      panel1->setpath(w);
      break;
    case 2:
      panel2->setpath(w);
      break;
    case 3:
      if (strcmp("panel2", w) == 0)
	panel = panel2;
      else
	panel = panel1;
      break;
    case 4:
      nn=cfgman->get_value_int(confref[i].name, confref[i].default_value);
      if (nn == 0 || nn > 4)
	nn = 2;
      panel1->repanel(nn);
      submenu1->set(2 + nn);
      break;
    case 5:
      nn=cfgman->get_value_int(confref[i].name, confref[i].default_value);
      if (nn == 0 || nn > 4)
	nn = 2;
      panel2->repanel(nn);
      submenu2->set(2 + nn);
      break;
    case 6:
      if (strcmp(w,"extension") == 0)
      {
	panel1->comp = extcmp;
	submenu1->set(8);
      }
      else if (strcmp(w, "size") == 0)
      {
	panel1->comp = sizecmp;
	submenu1->set(9);
      }
      else if (strcmp(w, "time") == 0)
      {
	panel1->comp = timecmp;
	submenu1->set(10);
      }
      else if(strcmp(w, "name") == 0)
      {
	panel1->comp = mystrcmp;
	submenu1->set(7);
      }
      else
      {
	panel1->comp = unsortcmp;
	submenu1->set(11);
      }
      break;
    case 7:
      if (strcmp(w, "extension") == 0)
      {
	panel2->comp = extcmp;
	submenu2->set(8);
      }
      else if (strcmp(w, "size") == 0)
      {
	panel2->comp = sizecmp;
	submenu2->set(9);
      }
      else if (strcmp(w, "time") == 0)
      {
	panel2->comp = timecmp;
	submenu2->set(10);
      }
      else if (strcmp(w, "name") == 0)
      {
	panel2->comp = mystrcmp;
	submenu2->set(7);
      }
      else
      {
	panel2->comp = unsortcmp;
	submenu2->set(11);
      }
      break;
    case 8:
      if (strcmp(w, "brief") == 0)
      {
	panel1->set_display_mode(BriefMode);
	submenu1->set(0);
	break;
      }
      if (strcmp(w, "full") == 0)
      {
	panel1->set_display_mode(FullMode);
	submenu1->set(1);
	break;
      }
      panel1->set_display_mode(CustomMode);
      submenu1->set(2);
      break;

    case 9:
      if (strcmp(w, "brief") == 0)
      {
	panel2->set_display_mode(BriefMode);
	submenu2->set(0);
	break;
      }

      if (strcmp(w, "full") == 0)
      {
	panel2->set_display_mode(FullMode);
	submenu2->set(1);
	break;
      }
      panel2->set_display_mode(CustomMode);
      submenu2->set(2);
      break;
    case 10:
      if(strcmp(w,"reverse") == 0)
      {
	panel1->rev_order=1;
	submenu1->set(12);
      } else
	panel1->rev_order=0;
      break;
    case 11:
      if(strcmp(w,"reverse") == 0)
      {
	panel2->rev_order=1;
	submenu2->set(12);
      } else
	panel2->rev_order=0;
      break;

    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
      w=getseq(w,w1,0);
      j = detect_fs(w1);
      getrest(w, w1, 0);
      bmark->add_book(i-12, w1, j);
      break;

    case 21:
      if (strcmp(w, "no") == 0)
      {
	hide_hidden = 1;
      }
      else
      {
	hide_hidden = 0;
      }
      break;
    case 22:
      bmark->percent=cfgman->get_value_int(confref[i].name, confref[i].default_value);
      if (bmark->percent < 15)
	bmark->percent = 15;
      break;
    case 23:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_COPY;
      }
      break;
    case 24:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_MOVE;
      }
      break;
    case 25:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_DEL;
      }
      break;
    case 26:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_OVR;
	dont_ask=1;
      }
      break;
    case 27:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_EXIT;
      }
      break;
    case 28:
      if(strcmp(w,"no") == 0)
      {
	allow_t_raising=NO;
      } else
	allow_t_raising=YES;
      break;
    case 29:
      if(strcmp(w,"no") == 0)
      {
	allow_animation=NO;
      } else
	allow_animation=YES;
      if(!allow_bookmark)
	allow_animation=NO;
      break;
    case 30:
      if(strcmp(w,"no") == 0)
      {
	allow_dir_icon=NO;
      } else
	allow_dir_icon=YES;
      break;
    case 31:
      if(strcmp(w,"no") == 0)
      {
	allow_file_icon=NO;
      } else
	allow_file_icon=YES;
      break;
    case 32:
      str2version(w,&xnc_ver);
      sscanf(w+6,"%d",&init_starts);
      break;
    case 33:
      xnc_starts=cfgman->get_value_int(confref[i].name, confref[i].default_value);
      xnc_starts++;
      break;
    case 34:
      dirdelay=cfgman->get_value_int(confref[i].name, confref[i].default_value);
      break;
    case 35:
      if(strcmp(w,"no") == 0)
      {
	shadow=NO;
      } else
	shadow=YES;
      break;
    case 36:
      if(strcmp(w,"no") == 0)
      {
	compare_by_size=0;
	submenu3->set(21);
      } else
	compare_by_size=1;
      break;
    case 37:
      if(strcmp(w,"yes") == 0)
      {
	compare_by_time=1;
	submenu3->set(22);
      } else
	compare_by_time=0;
      break;
    case 38:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~FILLED_CURSOR;
      } else
	option_bits|=FILLED_CURSOR;
      break;
    case 39:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~IVES_EXIT_TOO;
      } else
	option_bits|=IVES_EXIT_TOO;
      break;
    case 40:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~STATUS_NAME;
      } else
	option_bits|=STATUS_NAME;
      break;
    case 41:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~STATUS_ATTR;
      } else
	option_bits|=STATUS_ATTR;
      break;
    case 42:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~STATUS_TIME;
      } else
	option_bits|=STATUS_TIME;
      break;
    case 43:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~STATUS_SIZE;
      } else
	option_bits|=STATUS_SIZE;
      break;
    case 44:
      if(strcmp(w,"no") == 0)
      {
	option_bits&=~STATUS_OWNER;
      } else
	option_bits|=STATUS_OWNER;
      break;
    case 45:
      {
	int ix, iy;
	sscanf(w, "%d ,%d", &ix, &iy);
	init_inodewin_with_position(ix, iy);
	break;
      }
    case 46:
      strcpy(magic_file_location, w);
      break;
    case 47: //brief_mode_format
      panel1->set_list_format(BriefMode,w);
      panel2->set_list_format(BriefMode,w);
      break;
    case 48: //full_mode_format
      panel1->set_list_format(FullMode,w);
      panel2->set_list_format(FullMode,w);
      break;
    case 49: //custom_mode_format
      panel1->set_list_format(CustomMode,w);
      panel2->set_list_format(CustomMode,w);
      break;
    case 50: //xnc_now_on_top (startup window on top)
      if(strcmp(w, "xterm")==0)
	xnc_now_on_top=0;
      else
	xnc_now_on_top=1;
      break;
    case 51:
	iconset_number=cfgman->get_value_int(confref[i].name, 
					     confref[i].default_value);
	break;
    case 52: //Nothing to de here, plugin loads before this function
	break;
    case 53:
      if(strcmp(w,"no") == 0)
      {
	use_prompt&=~PROMPT_PACK;
      }
      break;
    case 54:
	branch_depth=cfgman->get_value_int(confref[i].name, 
					   confref[i].default_value);
	break;
    case 55:
	getrest(w, html_browser, 0);
	break;
    case 56:
	if(strcmp(w, "lynx")==0)
	    option_bits |= LYNX_NAVIGATION;
	break;
    case 57: //custom_mode_format
	value_int=cfgman->get_value_int(confref[i].name, 
					confref[i].default_value);
	panel1->set_display_max_file_len(BriefMode,value_int);
	panel2->set_display_max_file_len(BriefMode,value_int);
	panel1->set_display_max_file_len(FullMode,value_int);
	panel2->set_display_max_file_len(FullMode,value_int);
	panel1->set_display_max_file_len(CustomMode,value_int);
	panel2->set_display_max_file_len(CustomMode,value_int);
	break;
    };
  }
  return 1;
}


//-------------------------------------------------------------------------
// Load cfg, set names
//-------------------------------------------------------------------------
void   load_cfg(Lister * l1, Lister * l2)
{
  XTextProperty wname;
  char *winname;

  init_users();
  load_mnt();
  fprintf(stderr, "Loading extension file......");
  load_ext();
  fprintf(stderr, "OK\nLoading menu file......");
  load_menu();
  fprintf(stderr, "OK\nLoading config......");
  strcpy(rfile, home_files_dir);
  strcat(rfile, "/xnc.cfg");
  panel = l1;
  l1->repanel(3);
  submenu1->set(5);
  l2->repanel(3);
  submenu2->set(5);
  l1->comp = extcmp;
  submenu1->set(8);
  l2->comp = extcmp;
  submenu2->set(8);
  l2->set_display_mode(BriefMode);
  submenu2->set(0);
  l1->set_display_mode(BriefMode);
  submenu1->set(0);
  if (load_config(rfile, l1, l2) == 0)
    {
      fprintf(stderr, "\n!!!XNC Panic!!!: Can't open configuration file.\n");
    }
  strcat(winame, "    User: ");
  strcat(winame, finduser(getuid()));
  strcat(winame, "  Group: ");
  strcat(winame, findgroup(getgid()));
  winname=winame;
  if (XStringListToTextProperty(&winname, 1, &wname) == 0)
    {
      fprintf(stderr, "XNC: Error creating TextProperty\n");
      exit(1);
    }
  else
    XSetWMName(disp, base_win, &wname);
}


//-------------------------------------------------------------------------
//Loading plugin from command line of from config
//-------------------------------------------------------------------------
void load_gui_plugin()
{
    char *w=cfgman->get_value(confref[52].name, confref[52].default_value);
    strcpy(guiplugin_name, w);
    if(rs_plugin_name)
	strcpy(guiplugin_name, (char*)rs_plugin_name);
    if(strcmp(guiplugin_name, "none"))
	pluginman.load_look(guiplugin_name);    
}


//-------------------------------------------------------------------------
// Save cfg to file (xnc.cfg)
//-------------------------------------------------------------------------
void   save_cfg(Lister * l1, Lister * l2)
{
  FILE  *fp;
  int ix, iy;
  char buf[L_MAXPATH];

  if (l1 == 0 || l2 == 0)
    return;

  sprintf(buf, "%d.%d.%d    %d", xnc_orig_ver/100,
	  (xnc_orig_ver%100)/10, xnc_orig_ver%10, init_starts);
  cfgman->update_value("xnc.version.current",      buf);
  cfgman->update_value("xnc.total.starts",         xnc_starts);

  cfgman->update_value("panel1.path",              l1->curdir);
  cfgman->update_value("panel1.columns",           l1->col);
  cfgman->update_value("panel1.sort_order",        l1->rev_order ? "reverse" : "normal");
  if (l1->comp == mystrcmp)
    cfgman->update_value("panel1.sortby",          "name");
  else if (l1->comp == extcmp)
    cfgman->update_value("panel1.sortby",          "extension");
  else if (l1->comp == sizecmp)
    cfgman->update_value("panel1.sortby",          "size");
  else if (l1->comp == timecmp)
    cfgman->update_value("panel1.sortby",          "time");
  else
    cfgman->update_value("panel1.sortby",          "unsorted");
  if (l1->get_display_mode()==BriefMode)
    cfgman->update_value("panel1.style",           "brief");
  if (l1->get_display_mode()==FullMode)
    cfgman->update_value("panel1.style",           "full");
  if (l1->get_display_mode()==CustomMode)
    cfgman->update_value("panel1.style",           "custom");

  cfgman->update_value("panel2.path",              l2->curdir);
  cfgman->update_value("panel2.columns",           l2->col);
  cfgman->update_value("panel2.sort_order",        l2->rev_order ? "reverse" : "normal");
  if (l2->comp == mystrcmp)
    cfgman->update_value("panel2.sortby",          "name");
  else if (l2->comp == extcmp)
    cfgman->update_value("panel2.sortby",          "extension");
  else if (l2->comp == sizecmp)
    cfgman->update_value("panel2.sortby",          "size");
  else if (l2->comp == timecmp)
    cfgman->update_value("panel2.sortby",          "time");
  else
    cfgman->update_value("panel2.sortby",          "unsorted");
  if (l2->get_display_mode()==BriefMode)
    cfgman->update_value("panel2.style",           "brief");
  if (l2->get_display_mode()==FullMode)
    cfgman->update_value("panel2.style",           "full");
  if (l2->get_display_mode()==CustomMode)
    cfgman->update_value("panel2.style",           "custom");

  cfgman->update_value("panel.active",             panel == l1 ? "panel1" : "panel2");
  bmark->save_books(cfgman);
  cfgman->update_value("panels.format.brief",      l1->get_list_format(BriefMode));
  cfgman->update_value("panels.format.full",       l1->get_list_format(FullMode));
  cfgman->update_value("panels.format.custom",     l1->get_list_format(CustomMode));
  cfgman->update_value("panels.show_hidden_files", hide_hidden ? "no" : "yes");
  cfgman->update_value("panels.percentage",        bmark->percent);
  cfgman->update_value("panels.prompt.copy",       (use_prompt & PROMPT_COPY) ? "yes" : "no");
  cfgman->update_value("panels.prompt.move",       (use_prompt & PROMPT_MOVE) ? "yes" : "no");
  cfgman->update_value("panels.prompt.delete",     (use_prompt & PROMPT_DEL)  ? "yes" : "no");
  cfgman->update_value("panels.prompt.overwrite",  (use_prompt & PROMPT_OVR)  ? "yes" : "no");
  cfgman->update_value("panels.prompt.exit",       (use_prompt & PROMPT_EXIT) ? "yes" : "no");
  cfgman->update_value("panels.prompt.pack",       (use_prompt & PROMPT_PACK) ? "yes" : "no");
  cfgman->update_value("xnc.term.raise",           allow_t_raising ? "yes" : "no");
  cfgman->update_value("xnc.bookmark.animation",   allow_animation ? "yes" : "no");
  cfgman->update_value("xnc.icons.dir",            allow_dir_icon  ? "yes" : "no");
  cfgman->update_value("xnc.icons.file",           allow_file_icon ? "yes" : "no");
  cfgman->update_value("xnc.text.shadow",          shadow          ? "yes" : "no");
  cfgman->update_value("xnc.compare_by.size",      compare_by_size ? "yes" : "no");
  cfgman->update_value("xnc.compare_by.time",      compare_by_time ? "yes" : "no");
  cfgman->update_value("xnc.dirscan.timeout",      dirdelay);
  cfgman->update_value("panels.cursor.filled",     (option_bits & FILLED_CURSOR) ? "yes" : "no");
  cfgman->update_value("xnc.ives.exit.together",   (option_bits & IVES_EXIT_TOO) ? "yes" : "no");
  cfgman->update_value("xnc.statusbar.show.name",  (option_bits & STATUS_NAME)   ? "yes" : "no");
  cfgman->update_value("xnc.statusbar.show.attr",  (option_bits & STATUS_ATTR)   ? "yes" : "no");
  cfgman->update_value("xnc.statusbar.show.time",  (option_bits & STATUS_TIME)   ? "yes" : "no");
  cfgman->update_value("xnc.statusbar.show.size",  (option_bits & STATUS_SIZE)   ? "yes" : "no");
  cfgman->update_value("xnc.statusbar.show.owner", (option_bits & STATUS_OWNER)  ? "yes" : "no");
  if(get_inodewin_position(&ix, &iy))
  {
    sprintf(buf, "%d, %d", ix, iy);
    cfgman->update_value("xnc.statinfo.position",  buf);
  }
  cfgman->update_value("xnc.magicfile.path",       magic_file_location);
  cfgman->update_value("xnc.start.top_window",     xnc_now_on_top ? "xnc" : "xterm");
  cfgman->update_value("xnc.iconset.style",        iconset_number);
  cfgman->update_value("xnc.plugin.name",          guiplugin_name);
  cfgman->update_value("xnc.branch_view.depth",    branch_depth);
  cfgman->update_value("xnc.html.browser",         html_browser);
  cfgman->update_value("xnc.navigation.style",     
		       (option_bits & LYNX_NAVIGATION) ? "lynx" : "xnc");
  cfgman->update_value("panels.format.maxlen",     
		       panel->get_display_max_file_len(FullMode));

  cfgman->save_config("Do NOT edit it by hand, please. Use Options/Configuration in xnc.");
}



//============================== End of file ==================================


