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
/* $Id: main.cxx,v 1.32 2003/02/03 15:28:22 leo Exp $ */

#include "panel.h"
#include "scroll.h"
#include "ver.h"
#include "build.h"
#include "comp.h"
#include "image.h"
#include "bookmark.h"
#include "au.h"
#include "au_sup.h"
#include "key.h"
#include "ftpfs.h"
#include "internals.h"
#include "query_windows.h"
#include "ftpvisuals.h"
#include "xncversion.h"
#include "history.h"
#include "ftpcfg.h"
#include "c_externs.h"
#include "internals.h"
#include "pluginman.h"
#include "ivesclient.h"
#include "xshrloader.h"
#include "mouse_actions.h"
#include "cmdline.h"
#include "iconman.h"
#include "config_file.h"
#include "menu_process.h"

char   ver[] = "X Northern Captain "XNCVERSION;


extern Window main_win;
extern Window vt_win;
extern Atom   wmDeleteWindow;
extern void   find_panel(int);
extern int    term_sticky;
extern int    dont_ask;        //Placed in panel.cxx
extern int    layout;
extern Viewer vw1;
extern Editor ew1;
extern int    just_at_startup;
extern int    focus_fl;
extern char   shdir[];
extern AFS    afstmp;


char   dispstr[40] = "";
int    main_show = 1;
int    resize_done = 0;
BookMark *bmark;
Switcher *fsw1;
FtpVisual *fvis;
MenuBar *emn;

int no_ives=NO;

Lister *lst1;
Lister *lst2;
MenuBar *mn;
MenuBar *mn2;
Cmdline *cmdl;

////////////////////////////////////IVES routines////////////////////////
IVES_client *iclient;

char ivesrun[256];

void   sendtoives(char *name, int type, int ret = 0)
{
  char umem[L_MAXPATH];
  Window focus_to;
  int    ipid,i=0;
  if(no_ives)
  {
          simple_mes(_("Error connecting to IVES"),
		     _("Editor and/or Viewer not available! Use 'xncsetup' for correct"));
          return;
  }
  while (iclient->Init()==0)
    {
      //      XBell(disp, 0);
      show_mes(_("Loading IVES"), _("   Please wait..."));
      psystem(ivesrun);
      sleep(1);
      hide_mes();
      i++;
      if(i>3)
      {
              simple_mes(_("Error"), _("IVE System not available!"));
              return;
      }
    };
  if (main_show)
    focus_to = Main;
  else
    focus_to = 0;


  if (name[0] != '/')
    {
      getcwd(umem, 1024);
      strcat(umem, "/");
      strcat(umem, name);
    }
  else
    strcpy(umem, name);
  iclient->send_data(DefaultRootWindow(disp),IVES_data(Main,type,focus_to,umem,ret));
}


void   editload(char *cur)
{
  sendtoives(cur, 1);
}

void   viewload(char *cur)
{
    sendtoives(cur, 2);
}

void   viewonlyload(char *cur)
{
    sendtoives(cur, 4);
}

void   viewhighlight(char *name)
{
  sendtoives(name, 3, 1);
}

/////////////////////////////IVES end////////////////////////////////////

void full_reconfig()
{
    xncdprintf(("FULL RECONFIGURE...\n"));
    mn->reconfigure(fsw1->l, 0, Mainl-fsw1->l, mn->menuh);
    mn->substract_from_l(fsw1->l);
    mn2->reconfigure(0, Mainh - mn2->menuh, Mainl, mn2->menuh);
    cmdl->reconfigure(0, Mainh - 3 - mn2->menuh - cmdl->h, Mainl, cmdl->h);
    fsw1->reconfigure(0,0, fsw1->l,fsw1->h);
    fvis->reconfigure(fsw1->l,0,Mainl-fsw1->l,fvis->h);
    bmark->reconfigure(0, mn->menuh, Mainl, Mainh - 23 - mn2->menuh - mn->menuh);
    bmark->expose();
    cmdl->expose();
    mn->expose();
    mn2->expose();
    XFlush(disp);
}

void   XEVENT()
{
};

int    evret;
int    iii = 0;
int    resize_done2 = 0;
int    Main_x = -1, Main_y = -1;

void reconfigure_main_windows(XEvent& ev)
{
    xncdprintf(("CONF_NOTIFY: %x - %d,%d %dx%d\n",ev.xany.window,
		ev.xconfigure.x,ev.xconfigure.y,
		ev.xconfigure.width,ev.xconfigure.height));
    if (Mainl != ev.xconfigure.width || Mainh != ev.xconfigure.height)
    {                        //2
	
	Mainl = ev.xconfigure.width;
	Mainh = ev.xconfigure.height;
	centerx = Mainl / 2;
	centery = Mainh / 2;
	XResizeWindow(disp, Main, Mainl, Mainh);
	full_reconfig();
	//-OUTERWIN
	resize_window();
	
	if (term_sticky && resize_done2 == 0)
	{
	    XResizeWindow(disp, main_win, Mainl, Mainh);
	    resize_done = 1;
	};
	resize_done2 = 0;
    }
#if 0
    else 
      if (ev.xconfigure.x != Main_x || ev.xconfigure.y != Main_y)
    {
	Main_x = Mainx = ev.xconfigure.x;
	Main_y = Mainy = ev.xconfigure.y;
	{
	    if (term_sticky && resize_done2 == 0 && just_at_startup == 0)        //-2,2
		
	    {
	      xncdprintf(("Moving rxvt window to %d, %d\n",
			  ev.xconfigure.x, ev.xconfigure.y));
		XMoveWindow(disp, main_win, ev.xconfigure.x, ev.xconfigure.y);        //3
		
		resize_done = 1;
	    };                //-3
	    
	    resize_done2 = 0;
	};
    };
#endif
    default_dnd_man->update_geoms();
    if(term_key)
	term_key->move_window(Mainl-20,0);
}

void set_focus_to_main_window()
{
    //    XSynchronize(disp, True);
    xncdprintf(("Setting FOCUS to: %s (%x)\n", 
		xnc_now_on_top ? "xnc-window" : "rxvt-window",
		xnc_now_on_top ? panel->w : main_win));
    if(xnc_now_on_top)
    {
	//	XRaiseWindow(disp, panel->w);
    	guiSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
    }
    else
    {
	XRaiseWindow(disp, panel->w);
    	guiSetInputFocus(disp, main_win, RevertToParent, CurrentTime);
    }
    //    XSynchronize(disp, False);
}

int    XEVENT(XEvent *event)
{
  Gui   *o;
  int    fiflg = 1;
  ev = *event;
  evret = 1;

  if(default_dnd_man->process_event(event))
      return 1;
  //  xncdprintf(("XEVENT: w=%x type=%d\n",ev.xany.window,ev.type));
  if (just_at_startup == 1 && ev.xany.window == base_win)
  {
      set_focus_to_main_window();
      just_at_startup = 0;
      full_reconfig();
      return 1;
  }

  if (ev.type == ConfigureNotify && ev.xany.window==base_win)
  {
      reconfigure_main_windows(ev);
      return 1;
  }

  if (ev.type == ClientMessage &&
      (ev.xany.window==base_win || ev.xany.window==Main))
  {
      xncdprintf(("Client message got\n"));
      if(iclient->check_event(&ev))
	  return 1;
      else
	  if(ev.xclient.format == 32 && ev.xclient.data.l[0] == wmDeleteWindow)
	      donefunc();
      return 0;
  }

  if (ev.type == FocusIn) //Skipping all focus events expect last one in the queue
  {
      do {
	  xncdprintf(("MAIN_EVENT: FOCUS - window %x\n", ev.xany.window));
      } while(XCheckTypedEvent(disp, FocusIn, &ev));
      xncdprintf(("MAIN_EVENT: FOCUS process for window %x\n", ev.xany.window));
  }

  if (ev.type == FocusOut) //Skipping all focus events expect last one in the queue
  {
      do {
	  xncdprintf(("MAIN_EVENT: FOCUS OUT - window %x\n", ev.xany.window));
      } while(XCheckTypedEvent(disp, FocusOut, &ev));
      xncdprintf(("MAIN_EVENT: FOCUS OUT process for window %x\n", ev.xany.window));
  }

  if (ev.xany.window != base_win && ev.xany.window != Main)
  {
      if (ev.type == Expose)
      {
          o = findexp(ev.xany.window);
          if (o)
	      o->click();
          else
	      return 0;
      }
      else
      {
          o = find(ev.xany.window);
          if (ev.type == FocusIn)
	  {
              if (o == panel->panel2)
	      {
		  xncdprintf(("Set FOCUS to opposite panel %x\n", panel->w));
                  guiSetInputFocus(disp, panel->w, RevertToParent, CurrentTime);
                  return 0;
	      }
	      if (o)
		  o->click();
	      else
		  return 0;
	  }
          else 
	      if (o)
		  o->click();
	      else
		  return 0;
      }
  }
#if 1
  else 
      if (ev.type == FocusIn)
      {
	  xncdprintf(("GOT FOCUS to %s window\n", 
		      ev.xany.window==base_win ? "base_win" : "main"));
	  set_focus_to_main_window();
      }
     else
	 if (ev.type == Expose && fiflg == 1 && focus_fl)
	 {
	     xncdprintf(("GOT EXPOSE to FOCUS %s window\n", 
			 ev.xany.window==base_win ? "base_win" : "main"));
	     set_focus_to_main_window();
	     fiflg = 0;
	 }
#endif
  return evret;
}

void   xnc_sdir()
{
  char tmpdir[L_MAXPATH];
  tt_printf("cd %s\n", quote_path(tmpdir,panel->curdir));
  strcpy(shdir, panel->curdir);
}

void   sig_fault(int)
{
  signal(SIGSEGV, SIG_DFL);
  fprintf(stderr, "\n\n*******************************************************************************\n");
  fprintf(stderr, 
	  _("OOPS! It seems that You found a bug in XNC!!!\n"
	    "If You can repeat this situation then send me a bug report\n"
	    "to <leo@xnc.dubna.su> with subject  'XNC - bug report'\n"));
  fprintf(stderr, 
	  _("Body of the mail:\n"
	    "   -What are You doing to produce this bug.\n"
	    "   -XNC configuration (~/.xnc/xnc.ini) file.\n"
	    "   -Output of 'ldd xnc' command.\n"
	    "   -X Server configuration (resolution and color depth)\n"
	    "   -And Your reply mail address, for more information about situation if need\n"
	    "Do NOT include 'CORE' dump file in the following letter.\nThanks, and sorry for BUG\n"));
  fprintf(stderr, "*******************************************************************************\n");
  chdir(getenv("HOME"));
}


void ives_connect(int argc, char **argv)
{
    int    i;
    fprintf(stderr, "Connecting to IVE System.................");
    if (strcmp(viewname, "internal") != 0 && strcmp(editname, "internal") != 0)
    {
	fprintf(stderr, "using externals\n");
	no_ives=YES;
    }
    else
    {
	strcpy(ivesrun, "ives ");
	if (argc > 1)
        {
	    i = 1;
	    while (i < argc - 1)
		if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-display") == 0)
		{
		    strcat(ivesrun, argv[i]);
		    strcat(ivesrun, " ");
		    strcat(ivesrun, argv[i + 1]);
		    strcpy(dispstr, argv[i + 1]);
		    break;
		}
		else
		    i++;
        }
	if (iclient->Init()==0)
	{
	    fprintf(stderr,"failed (make it later)\n");
	    no_ives=NO;
	}
	else
	{
	    fprintf(stderr,"OK\n");
	    no_ives=NO;
	}
    }
}

void   xnc_main(int argc, char **argv)
{
  signal(SIGSEGV, sig_fault);
  xncdprintf(("Initialisation:\n"));
  margc = argc;
  margv = argv;

  init_l10n();

  make_home_file_dir();
  create_cfgman(home_files_dir, "xnc.cfg");

  load_gui_plugin();

  InitXClient(argc, argv, 15, 70, 740, 455, ver);

  XSetErrorHandler(gui_dummy_x_error_handler);

  default_dnd_man=new DndManager(disp,base_win);
  default_dnd_man->set_external_xevent_processor(XEVENT);

  default_mouse_action_man=new MouseActionManager;

  init_imagengine();
  GuiLocale::init_locale();

  if(is_shared_load())
  {
    xncdprintf(("Using shared loading\n"));
    default_loader=new xsharedLoader(SharedRead);
  }
  else
    default_loader=new xsharedLoader(NonShared);
  default_loader->init();

  default_iconman=new IconManager;

  iclient=new IVES_client(ives_retval);

  if(is_show_intro() || is_new_version())
          show_intro();

  guiplugin->post_install();

  xncdprintf(("MAIN WIN is 0x%X\n",Main));

  ives_connect(argc, argv);
  compile_key_support();
  compile_AFS_supports("xnc.afs");
  compile_FTP_supports("xnc.ftp");
  init_ftp_globals();
  default_mouse_action_man->load_actions_from_file();
  create_dnd_cursors();
  bmark = guiplugin->new_BookMark(35, Mainh - 63);
  bmark->set_iname("BOOKMARK");  
}

void   xnc_exit();

void   xnc_sig()
{
  signal(SIGSEGV, sig_fault);
}

void   xnc_gofunc()
{
  char str[512];
  mainuid = getuid();
  maingid = getgid();
  init_scroll();
  init_xnc_alarm();
  init_qview();
  //  load_skins();
  mn = guiplugin->new_MenuBar(20, 0, menu, 5);
  mn->substract_from_l(20);
  mn->set_iname("MENU_UP");
  mn->init(Main);
  fvis= guiplugin->new_FtpVisual(20,0,Mainl-20,mn->menuh);
  init_submenus();
  mn2 = guiplugin->new_MenuBar(0, Mainh - mn->menuh, menu2, 10);
  emn = guiplugin->new_MenuBar(0, viewh - 20, NULL, 4);
  fsw1= guiplugin->new_Switcher(0,0,20,mn->menuh);
  mn2->set_iname("MENU_DN");
  fvis->set_iname("FVISUAL");
  fsw1->set_iname("MENU_SW");
  mn->substract_from_l(fsw1->l);
  fvis->init(Main);
  mn2->init(Main);
  cmdl = guiplugin->new_Cmdline(0, Mainh - 22 - mn2->menuh, Mainl, 6);
  cmdl->set_iname("CMDLINE");
  bmark->init(Main);
  fsw1->init(Main);
  fsw1->add_gui(mn);
#ifndef DISABLE_FTP
  fsw1->add_gui(fvis);
#endif
  bmark->create_listers(&lst1, &lst2, 0, mn->menuh, 22+mn2->menuh);
  lst1->init(Main);
  lst2->init(Main);
  cmdl->init(Main);
  lst1->setviewer(viewload, viewonlyload);
  lst2->setviewer(viewload, viewonlyload);
  lst1->seteditor(editload);
  lst2->seteditor(editload);
  fprintf(stderr, "Last version :  %s\n", created);
  fprintf(stderr, "Code compiled : %s\n", compiled);
  load_cfg(lst1, lst2);
  sprintf(str,"%s/.xnc/xnc.history",getenv("HOME"));
  fprintf(stderr,"Loading history........");
  init_history();
  hist_loadall(str,YES);

  fprintf(stderr, "Loading pixmaps.");
  load_pixmaps();
  set_current_iconset_style();
//  main_pixmap=get_rootwindow_pixmap();
  bmark->load_flags(7);
  if(is_back_load())
     fprintf(stderr, "later (back load)\n");
  else
     fprintf(stderr, "OK\n");
  unset_back_load();
  magic_init(magic_file_location, 1);
  hide_intro();
  fprintf(stderr, _("Starting %s Build%s.....\n"), ver, build_chr);

  XMapWindow(disp, base_win);
  map_vt_windows(main_pixmap);
  XMapWindow(disp, Main);
  if(xnc_now_on_top)
      XRaiseWindow(disp, Main);
  else
  {
      XRaiseWindow(disp, main_win);
      show_switch_term_button();
  }
  bmark->rebuild_listers();
  lst1->show();
  lst2->show();
  mn->show();
  mn2->show();
  cmdl->show();
  fsw1->show();
  lst1->setpanel(lst2);
  lst2->setpanel(lst1);
  lst1->setcmdl(cmdl);
  lst2->setcmdl(cmdl);
  lst1->setsubmenu(submenu1);
  lst2->setsubmenu(submenu2);
  cmdl->setpath(panel->curdir);
  bmark->show();
  afstmp.set_dir_pointer(NULL, 1024);
  wmDeleteWindow = XInternAtom(disp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(disp, base_win, &wmDeleteWindow, 1);
//  full_reconfig();
  fprintf(stderr, _("Copyright (c) 1995-2002 Leonid V. KHramov aka Leo <leo@xnc.dubna.su>\n"));
  au_out=0;  //Switch off SUP compilation output.
  //  set_focus_to_main_window();
  xncdprintf(("FOCUS WINDOW: base window    : %x\n",base_win));
  xncdprintf(("FOCUS WINDOW: term window    : %x\n",main_win));
  xncdprintf(("FOCUS WINDOW: xnc window     : %x\n",Main));
  xncdprintf(("FOCUS WINDOW: panel1 window  : %x\n",lst1->w));
  xncdprintf(("FOCUS WINDOW: panel2 window  : %x\n\n",lst2->w));
}

void   xnc_animate()
{
  bmark->animate();
}


//While we are in rxvt (terminal) window we are still can make processing
//for some functions via hot-keys.
int process_special_action_in_term(XEvent* ev)
{
  return panel->process_special_action_in_term(ev);
}

void   (*xnc_init) (int, char **) = xnc_main;
void   (*xnc_go) () = xnc_gofunc;
void   (*xnc_setsignals) () = xnc_sig;
void   (*xnc_setdir) () = xnc_sdir;
void   (*xnc_work_anim) () = xnc_animate;
int    (*xnc_look_for_key) (XEvent *) = look_for_key;
int    (*xnc_process_special_action_in_term) (XEvent *) = 
         process_special_action_in_term;
int    (*xnc_event) (XEvent *) = XEVENT;

int    xnc_ex = 1;
void   xnc_exit()
{
  char  *str;
  if (xnc_ex)
    {
      xnc_ex = 0;
      fprintf(stderr, "Exiting from XNC....");
      if (lst1 == NULL)
        {
          disp = NULL;
          fprintf(stderr, "OK\nSession aborted\n");
          return;
        };
      save_cfg(lst1, lst2);
      default_mouse_action_man->save_actions_to_file();
      deinit_users();
      del_mnt();
      deinit_all_ftp();
      delete lst1;
      delete lst2;
      delete mn;
      delete mn2;
      delete cmdl;
      if (disp)
      {
	x_free_pixmaps();
      }
      str = new char[L_MAXPATH];
      sprintf(str, "rm -rf %s/.%d.*", syspath, getpid());
      psystem(str);
      sprintf(str, "rm -rf %s/AFS/afs.%d.*", syspath, getpid());
      psystem(str);
      deinit_scroll();
      if((option_bits & IVES_EXIT_TOO) && iclient && iclient->Init())
	sendtoives("shutdown", 6); //Say IVES bye, bye :)
      deinit_keysupport();
      deinit_AFS_supports();
      ftpcfg_deinit_hosts();
      sprintf(str,"%s/.xnc/xnc.history",getenv("HOME"));
      hist_saveall(str);
      fprintf(stderr, "OK\n");
      fprintf(stderr, 
       _("------------------------------------------------------------------------------\n"
         "Notice:\n\tIf You like X Northern Captain and will use it,\n"
         "\tthen send me e-mail to <leo@xnc.dubna.su> with names of\n"
         "\tcity and country where you are using it. It's for my collection.\n"
         "\tIf You do it, i'll be happy....\n"
         "\t\t\tThanks,\n\t\t\tLeonid V. Khramov aka Leo <leo@xnc.dubna.su>\n"));
      fprintf(stderr,_("\nPS: If you have any questions, comments about xnc or you want\n"
	             "to know latest news then join to xnc mailing list <xnc@xnc.dubna.su>\n"
                     "Just mail a letter with 'SUBSCRIBE xnc' in the body of mail to\n"
		       "<majordomo@xnc.dubna.su> and you are inside.\n"));
      fprintf(stderr, "------------------------------------------------------------------------------\n");
      fprintf(stderr, _("Thank you. You have run X Northern Captain %d times!\n"), xnc_starts);
      delete str;
      delete cfgman; 
      delete default_dnd_man;
      DeinitXClient();
    }
}

void switch_term_fm_state()
{
    if(!is_term_on())
	return;
    focus_fl^=1;
    if(focus_fl) 
    {
	hide_switch_term_button();
	if(main_show)
	{
	    XLowerWindow(disp,main_win);
	    XSetInputFocus(disp,Main,RevertToNone,CurrentTime);
	} else 
	    XMapRaised(disp,Main);
	xnc_now_on_top=1;
    }
    else 
    {
	xnc_now_on_top=0;
	if(vt_show) 
	{
	    XRaiseWindow(disp,main_win);
	    XSetInputFocus(disp,main_win,RevertToNone,CurrentTime);
	}
	else 
	{
	    XMapRaised(disp,main_win);
	    XFlush(disp);
	};
	show_switch_term_button();
    };
}

void   (*xnc_kill) () = xnc_exit;
void   (*xnc_run_once) () = 0;
void   (*xnc_switch_term)()=switch_term_fm_state;
