/****************************************************************************
*  Copyright (C) 1996-2003 by Leo Khramov
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
/* $Id: panel_click.cxx,v 1.5 2003/12/25 13:27:30 leo Exp $ */
#include "panel.h"
#include "bookmark.h"
#include "query_windows.h"
#include "infowin.h"
#include "internals.h"
#include "key.h"
#include "x_actions.h"
#include "au_sup.h"
#include "config_file.h"
#include "menu_process.h"

extern void try_clean_afs_cache();
extern int evret, vt_show;
extern int disable_modifiers;

//-------------------------------------------------------------------------
// Process XEvents for this window (panel)
//-------------------------------------------------------------------------
void   Lister::click()
{
  int    action;
  KeySym ks;
  FList *ko;
  VFS   *ivfs;
  PanelArea area;

  if (ev.xany.window == w)
  {
      switch (ev.type)
      {
      case Expose:
	  flush_expose(w);
	  expose();
	  break;

      case ButtonPress:
	  process_mouse_press_event(ev);
	  break;

      case MotionNotify:
	  process_mouse_motion_event(ev);
	  break;

      case ButtonRelease:
	  process_mouse_release_event(ev);
	  break;

      case UnmapNotify:
	  break;

      case MapNotify:
	  break;

      case FocusIn:
	  focus_in();
	  break;

      case FocusOut:
	  focus_out();
	  break;

      case KeyPress:
	  action = look_for_key(&ev);
	  switch (action)
	  {
	  case ASwitchTab:
	      clear_ff();
	      panel = panel2;
	      guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);
	      break;
	  case AExecute:
	      clear_ff();
	      activate();
	      break;
	  case AInsert:
	      if (strcmp(cur->name, "..") != 0)
	      {
		  selected(cur);
		  showitem(cur, curn);
	      };
	  case AMoveDown:
	      go_downline();
	      break;
	  case AMoveUp:
	      go_upline();
	      break;
	  case AMoveLeft:
	      clear_ff();
	      if(option_bits & LYNX_NAVIGATION)
	      {
		  if(cur->file_type==file_is_dir)
		  {
		     go_home();
		     activate();
		  } else
		      left();
	      } else
		  left();
	      break;
	  case AMoveRight:
	      clear_ff();
	      if(option_bits & LYNX_NAVIGATION)
	      {
		  if(cur->file_type==file_is_dir && 
		     cur->icon_type!=f_updir_icon)
		      activate();
		  else
		      right();
	      }
	      else
		  right();
	      break;
	  case AMoveHome:
	      go_home();
	      break;
	  case AMoveEnd:
	      go_end();
	      break;
	  case AMovePageDn:
	      page_down();
	      break;
	  case AMovePageUp:
	      page_up();
	      break;
	  case ACmdDown:
	      cmdl->press(XK_Down);
	      break;
	  case ACmdUp:
	      cmdl->press(XK_Up);
	      break;
	  case ACmdRight:
	      cmdl->press(XK_Right);
	      disable_modifiers = 1;
	      evret = 0;
	      break;
	  case ACmdLeft:
	      cmdl->press(XK_Left);
	      disable_modifiers = 1;
	      evret = 0;
	      break;
	  case AInvertMask:
	      invert_mask_selection();
	      break;
	  case ASelectMask:
	      add_selection_by_mask();
	      break;
	  case ADeselectMask:
	      del_selection_by_mask();
	      break;
	  case ARemountList:
	      show_remount_device_list();
	      break;
	  case ARemount:
      	      remount();
	      break;
	  case AMan:
      	      newtextfile(_("Man page"), this, FMANPAGE);
	      break;
	  case AFDelete:
	      clear_ff();
	      del();
	      break;
	  case AFCopy:
	      clear_ff();
	      copy();
	      break;
	  case AFMove:
	      clear_ff();
	      move();
	      break;
	  case AQuit:
	      donefunc();
	      break;
	  case APrint:
	      initquery(_("Print file"), this, panel2->vfs, FLPR);
	      break;
	  case AAttributes:
	      change_attributes();
	      // *INDENT-OFF*        
	      // *INDENT-ON*
	      break;
	  case AKill:
	      clear_ff();
	      ps_info();
	      break;
#ifdef NO_INTERNAL_VT
	  case ARaise:
	      if (lowfl)
		  XRaiseWindow(disp, Main);
	      else
		  XLowerWindow(disp, Main);
	      lowfl ^= 1;
	      XFlush(disp);
	      if (foc)
		  cmdl->setpath(curdir);
	      chdir(curdir);
	      break;
#endif
	  case AFNewEdit:
	      clear_ff();
	      newtextfile(_("New text file"), this, FEDIT);
	      break;
	  case AFEdit:
	      vfs->ch_curdir();
	      clear_ff();
	      edit();
	      break;
	  case AFView:
	      view(1);
	      break;
	  case AFSimpleView:
	      view(0);
	      break;
	  case AMenu:
	      menupanel(panexec2);
	      break;
	  case AMakeDir:
	      vfs->ch_curdir();
	      newtextfile(_("Make directory"), this, FMKDIR);
	      break;
	  case ARereadDir:
	      lastscan = 0;
	      if(vfs->fstype==FTP_TYPE)
	      {
		  vfs->need_reread=1;
		  vfs->options|=REALREAD_BIT;
	      }
	      reread();
	      break;
	  case AFind:
	      find_panel();
	      break;
	  case ADiskInfo:
	      dfs_info(centerx * 2 - 40);
	      break;
	  case AQuickView:
	      qview();
	      break;
	  case AMemory:
	      free_info(centerx * 2 - 40);
	      break;
	  case ABookAdd:
	      add_to_bookmark();
	      break;
	  case ABook1:
	      bmark->switch_books(0);
	      break;
	  case ABook2:
	      bmark->switch_books(1);
	      break;
	  case ABook3:
	      bmark->switch_books(2);
	      break;
	  case ABook4:
	      bmark->switch_books(3);
	      break;
	  case ABook5:
	      bmark->switch_books(4);
	      break;
	  case ABook6:
	      bmark->switch_books(5);
	      break;
	  case ABook7:
	      bmark->switch_books(6);
	      break;
	  case ABook8:
	      bmark->switch_books(7);
	      break;
	  case ABook9:
	      bmark->swap_books(8);
	      break;
	  case ADelBook:
	      bmark->del_book(bmark->last_used);
	      break;
	  case AChown:
	      init_chown_dialog(cur);
	      break;
	  case ACleanCache:
	      try_clean_afs_cache();
	      break;
#ifndef DISABLE_FTP
	  case AFtpLink:
	      create_ftp_link();
	      break;
#endif
	  case AInsertLeftDir:
	      insert_left_dir();
	      break;
	  case AInsertRightDir:
	      insert_right_dir();
	      break;
	  case ASelectFileMask:
	      add_selection_by_ext();
	      break;
	  case ADeselectFileMask:
	      del_selection_by_ext();
	      break;
	  case AInodeInfo:
	      toggle_inodewin();
	      break;
	  case AQuickFInfo:
	      do_quick_file();
	      break;
	  case ABranchView:
	      switch_branch_view();
	      break;
	  case ASwapPanels:
	      swap_panels();
	      break;
	  case ACreateArchive:
	      pack();
	      break;
	  case ASymLink:
	      make_link();
	      break;
	  case ARenameFile:
	      rename();
	      break;
	  case ASetPanelToCurrent:
	      set_opposite_to_current();
	      break;
	  case AShowHiddenFiles:
	      switch_hidden_files_showing();
	      break;
	  case ABriefMode:
	      switch_to_brief();
	      break;
	  case AFullMode:
	      switch_to_full();
	      break;
	  case ACustomMode:
	      switch_to_custom();
	      break;
	  case ASortByName:
	      sort_by_name();
	      break;
	  case ASortByExt:
	      sort_by_ext();
	      break;
	  case ASortBySize:
	      sort_by_size();
	      break;
	  case ASortByTime:
	      sort_by_time();
	      break;
	  case ASortByUnsort:
	      sort_by_unsort();
	      break;
	  case AReverseSort:
	      switch_reverse();
	      break;
	  case AChangeDir:
	      show_cd_window();
	      break;

	  default:
	      //              if ((ev.xkey.state & ControlMask) == 0)
	      {
		  ks = XLookupKeysym(&ev.xkey, 0);
		  if ((ev.xkey.state & Mod1Mask) == 0)
		      cmdl->press(ks);
		  else
		  {
		      fast_find(&ev);
		      evret = 1;
		      break;
		  }
	      };
	      evret = 0;
	  };
	  break;
      }
  }
}

//While we are in terminal mode we can make special action processing
//here it is. return 1 if special action called.
int Lister::process_special_action_in_term(XEvent* ev)
{
  int action = look_for_key(ev);
  switch (action)
  {
  case ABook1:
    bmark->only_switch_to_exist_book(0);
    return 1;
  case ABook2:
    bmark->only_switch_to_exist_book(1);
    return 1;
  case ABook3:
    bmark->only_switch_to_exist_book(2);
    return 1;
  case ABook4:
    bmark->only_switch_to_exist_book(3);
    return 1;
  case ABook5:
    bmark->only_switch_to_exist_book(4);
    return 1;
  case ABook6:
    bmark->only_switch_to_exist_book(5);
    return 1;
  case ABook7:
    bmark->only_switch_to_exist_book(6);
    return 1;
  case ABook8:
    bmark->only_switch_to_exist_book(7);
    return 1;
  }
  return 0;
}

////////////////////////////////End of file/////////////////////////////////////////
