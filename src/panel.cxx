/****************************************************************************
*  Copyright (C) 1996-99 by Leo Khramov
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
/* $Id: panel.cxx,v 1.37 2003/12/25 13:27:30 leo Exp $ */
#include "panel.h"
#include "bookmark.h"
#include "query_windows.h"
#include "infowin.h"
#include "ftpfs.h"
#include "commonfuncs.h"
#include "systemfuncs.h"
#include "ftpcfg.h"
#include "c_externs.h"
#include "internals.h"
#include "baseguiplugin.h"
#include "iconman.h"
#include "config_file.h"
#include "ftpvisuals.h"
#include "main.h"

#include "key.h"
#include "x_actions.h"
#include "au_sup.h"


const int DIRCOL = 7;
const int EXECOL = 8;
const int NORMCOL = 6;
const int SELECTCOL = 5;
const int EXTCOL = 9;
const int LINKCOL = 10;
const int AFSCOL = 11;


Pixmap panelpixs[6]={0,0,0,0,0,0};
Pixmap panelpixmask[6];
int    panelpixl[6];
int    panelpixh[6];

int    just_at_startup = 1;

extern int qvflag;

extern int evret, vt_show;
extern Window main_win;
extern int focus_fl;
extern void donefunc();



char  *panlist[2000];
char  *panlist2[1000];
int    psmax = 0;

///////////////////////////////////Lister class///////////////////////////////
Lister::Lister(int ix,int iy,int il,int ih,int icolumns):Gui(),BaseCaptain(icolumns)
{
    x=ix;y=iy;l=il;h=ih;
    bclk=lmx=lmy=mouse=0;
    guitype=GUI_LISTER;
    prflg=0;
    lastn=0;
    cmdl=NULL;
    lowfl=0;
    viewfunc=viewonlyfunc=NULL;
    editorfunc=NULL;
    side=0;
    subm=NULL;
    display_mode=BriefMode;
    display_format[BriefMode].set_format("F");
    display_format[FullMode].set_format("F|S|D|A|U");
    display_format[CustomMode].set_format("O|F");
}


void   Lister::delall_l(FList * n)
{
}

//----------------------------------------------------------------------------
// Insert given string into terminal and commandline widget
//----------------------------------------------------------------------------
void Lister::cmdl_insert_name(char *name)
{
  char *n2=name;
  char term[]=" ;()[]<>'?\"|*&^%$#@!`";
  while (*n2 != 0)
    if(strchr(term, *n2))
    {
      cmdl->insert('\'');
      n2=name;
      while(*n2)
	cmdl->insert(*n2++);
      tt_printf("'%s'", name);
      cmdl->insert('\'');
      return;
    } else n2++;
  n2=name;
  while(*n2)
    cmdl->insert(*n2++);
  tt_printf("%s", name);
}

//----------------------------------------------------------------------------
// Init class, create windows, texts....
//----------------------------------------------------------------------------
void   Lister::init(Window ipar)
{
  foc = 0;
  refindchr[0] = 0;
  XWindowAttributes xwa;
  XSetWindowAttributes xswa;
  parent = ipar;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, normal_bg_color);
  gcv.background = normal_bg_color;
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  gcv.font = lfontstr->fid;
  wgcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  xpmgc = XCreateGC(disp, w, 0, NULL);
  if (XGetWindowAttributes(disp, w, &xwa) != 0)
  {
    xswa.do_not_propagate_mask = xwa.do_not_propagate_mask | ButtonPressMask |
      KeyPressMask;
    XChangeWindowAttributes(disp, w, CWDontPropagate, &xswa);
  }
  else
  {
    xncdprintf(("Propagation failed\n"));
  }
  XSelectInput(disp, w, 
	       ExposureMask | 
	       ButtonPressMask | 
	       ButtonReleaseMask | 
               PointerMotionMask |
               KeyPressMask |
               FocusChangeMask | 
	       StructureNotifyMask);
  fattr1 = XTextWidth(lfontstr, "-rwxrwxrwx", 9);
  attdx = 8 + (43 - XTextWidth(fixfontstr, "0000", 4)) / 2; //FIXME
  repanel(col);
  foc = 0;

  init_dnd();

  scr=baseguiplugin->new_ScrollBar(l-24,28,h-28-35,this); //FIXME
  scr->setrange(0,10);
  scr->init(w);
  
}

//----------------------------------------------------------------------------
// Recalc all sizes according to new one (resize windows event)
//----------------------------------------------------------------------------
void   Lister::repanel(int ic)
{
    int dummy, to_x, to_y, i;

    col = ic;
    main_area.text_h = lfontstr->max_bounds.ascent + lfontstr->max_bounds.descent;
    main_area.text_y = lfontstr->max_bounds.ascent;
    fixl = XTextWidth(fixfontstr,"M",1);
    fixh = fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent;
    fixy = fixfontstr->max_bounds.ascent;
    pixels_per_column = (l - Lister::main_area_delta_x*2) / col;
    
    vlook_recalc_all_data();
    
    prflg = 0;
    
    items_per_column = main_area.active_h / main_area.item_h;
    items_per_page=items_per_column*col;
    
    get_xy_of_item_by_number(0, to_x, dummy, to_y);
    rect.x=0;
    rect.y=to_y;
    rect.width = main_area.item_l;
    rect.height = main_area.item_h*items_per_column;
    XSetClipRectangles(disp, wgcw, 0, 0, &rect, 1, Unsorted);
    
    lastn = -1;
    lcurn = 0;
    if (curn > 0)
	curn = 0;
    base = cur = dl.next;

    for(i=0;i<MAX_FORMATS;i++)
	display_format[i].init(allow_dir_icon ? 
			       main_area.item_l - Lister::main_area_icons_width :
			       main_area.item_l, lfontstr);
}

//----------------------------------------------------------------------------
// Calculate offsets for status bar info
//----------------------------------------------------------------------------
void Lister::calc_statusbar_offsets()
{
  int stlx, stll;
  stlx=status_area.active_from_x;
  stll=status_area.active_l;

  xncdprintf(("CALCULATING status bar offsets\n"));

  if(option_bits & STATUS_ATTR)
  {
    STL_ATTR_PTR.x=stlx;
    STL_ATTR_PTR.y=status_area.active_from_y;
    STL_ATTR_PTR.ty=status_area.active_text_y;
    STL_ATTR_PTR.tx=STL_ATTR_PTR.x+STL_SPACE2+1;
    STL_ATTR_PTR.tlen=STL_ATTR_NCHAR;
    STL_ATTR_PTR.len=STL_ATTR_NCHAR*fixl+STL_SPACE;
    STL_ATTR_PTR.enabled=1;
    stlx+=STL_ATTR_NCHAR*fixl+STL_SPACE+1;
    stll-=STL_ATTR_NCHAR*fixl+STL_SPACE+1;
  } else
      STL_ATTR_PTR.enabled=0;

  if(stll<0)
    stll=0;
  if(option_bits & STATUS_SIZE)
  {
    STL_SIZE_PTR.x=stlx+stll-STL_SIZE_NCHAR*fixl-STL_SPACE;
    STL_SIZE_PTR.y=status_area.active_from_y;
    STL_SIZE_PTR.ty=status_area.active_text_y;
    STL_SIZE_PTR.tx=STL_SIZE_PTR.x+STL_SPACE2+1;
    STL_SIZE_PTR.tlen=STL_SIZE_NCHAR;
    STL_SIZE_PTR.len=STL_SIZE_NCHAR*fixl+STL_SPACE;
    stll-=STL_SIZE_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
	STL_SIZE_PTR.enabled=0;
    else
	STL_SIZE_PTR.enabled=1;
  } else
      STL_SIZE_PTR.enabled=0;
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_TIME)
  {
    STL_TIME_PTR.x=stlx+stll-STL_TIME_NCHAR*fixl-STL_SPACE;
    STL_TIME_PTR.y=status_area.active_from_y;
    STL_TIME_PTR.ty=status_area.active_text_y;
    STL_TIME_PTR.tx=STL_TIME_PTR.x+STL_SPACE2+1;
    STL_TIME_PTR.tlen=STL_TIME_NCHAR;
    STL_TIME_PTR.len=STL_TIME_NCHAR*fixl+STL_SPACE;
    stll-=STL_TIME_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
	STL_TIME_PTR.enabled=0;
    else
	STL_TIME_PTR.enabled=1;
  } else
      STL_TIME_PTR.enabled=0;

  if(stll<0)
    stll=0;
  if(option_bits & STATUS_OWNER)
  {
    STL_OWNER_PTR.x=stlx+stll-STL_OWNER_NCHAR*fixl-STL_SPACE;
    STL_OWNER_PTR.y=status_area.active_from_y;
    STL_OWNER_PTR.ty=status_area.active_text_y;
    STL_OWNER_PTR.tx=STL_OWNER_PTR.x+STL_SPACE2+1;
    STL_OWNER_PTR.tlen=STL_OWNER_NCHAR;
    STL_OWNER_PTR.len=STL_OWNER_NCHAR*fixl+STL_SPACE;
    stll-=STL_OWNER_NCHAR*fixl+STL_SPACE+1;
    if(stll<0)
	STL_OWNER_PTR.enabled=0;
    else
	STL_OWNER_PTR.enabled=1;
    xncdprintf(("Changing status bar owner position to %d, %d\n",
		STL_OWNER_PTR.x, STL_OWNER_PTR.y));
  } else
      STL_OWNER_PTR.enabled=0;
  
  if(stll<0)
    stll=0;
  if(option_bits & STATUS_NAME)
  {
    STL_NAME_PTR.x=stlx;
    STL_NAME_PTR.y=status_area.active_from_y;
    STL_NAME_PTR.ty=status_area.active_text_y;
    STL_NAME_PTR.tx=STL_NAME_PTR.x+STL_SPACE2+1;
    STL_NAME_PTR.tlen=(stll-STL_SPACE)/fixl;
    STL_NAME_PTR.len=stll;
    STL_NAME_PTR.enabled=1;
  } else
      STL_NAME_PTR.enabled=0;

}

//----------------------------------------------------------------------------
// Calls when ConfigureNotify event appears
//----------------------------------------------------------------------------
void   Lister::reconfigure(int ix, int iy, int il, int ih)
{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  XMoveWindow(disp, w, x, y);
  XResizeWindow(disp, w, l, h);
  repanel(col);
  expose();
}

//----------------------------------------------------------------------------
// Same as above but don't redraw window contents 
//----------------------------------------------------------------------------
void   Lister::reconfigure_without_expose(int ix, int iy, int il, int ih)
{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  XMoveWindow(disp, w, x, y);
  XResizeWindow(disp, w, l, h);
  repanel(col);
}

//----------------------------------------------------------------------------
// Return geometry of the panel window
//----------------------------------------------------------------------------
void Lister::get_geometry(int& ix, int& iy, 
			  unsigned int& il, unsigned int& ih)
{
  ix=x;
  iy=y;
  il=l;
  ih=h;
}

//----------------------------------------------------------------------------
// Swap two panels between each other. Actually don't make real swap
// just update window coords and redraw them :) 
//----------------------------------------------------------------------------
void   Lister::swap_panels()
{

  int x1,x2,y1,y2;
  unsigned int l1,l2,h1,h2;

  xncdprintf(("Swapping panels between each other\n"));
  switch(layout)
  {
  case 0: //Horizontal layout
  case 1: //Vertical layout
    get_geometry(x1,y1,l1,h1);
    panel2->get_geometry(x2,y2,l2,h2);
    reconfigure(x2,y2,l2,h2);
    panel2->reconfigure(x1,y1,l1,h1);
    break;

  case 2: //Page layout
    orientation= orientation==LeftPage ? RightPage : LeftPage;
    lay=orientation;
    repanel(col);
    clean_expose();
    break;
  }
}

//----------------------------------------------------------------------------
// Switching opposite panel to the same VFS as we are now
// NOTE: for AFS switching to / dir not current one... :( sorry
//----------------------------------------------------------------------------
void   Lister::set_opposite_to_current()
{
  xncdprintf(("Setting opposite panel to current VFS\n"));
  panel2->push_n_switch(vfs->fstype, vfs->get_info_for_bookmark());
  if(layout==2) //Page layout
    clean_expose();
}

//----------------------------------------------------------------------------
// Clean and Draw entire widget, make directory reread if need
//----------------------------------------------------------------------------
void   Lister::clean_expose()
{
    XClearWindow(disp,w);
    expose();
}

//----------------------------------------------------------------------------
// Draw entire widget, make directory reread if need
//----------------------------------------------------------------------------
void   Lister::expose()
{
    int    i, px;
    char  *cdir;
    
    xncdprintf(("====Expose for panel %d====\n",lay));
    if(lay>0 && panel!=this) //We are under next panel window -> don't need expose
    {
	xncdprintf(("==== >>  SKIPPED  << ====\n",lay));
	return;
    }
    
    cdir = panel2->vfs->get_dir_header();
    
    vlook_draw_frame();
    vlook_draw_head();
    vlook_display_header_inactive_string(cdir,strlen(cdir),dark_bg_color);
    vlook_draw_column_separators();
    vlook_draw_status_buttons();
    
    try_reread_dir();
    /*
    if (just_at_startup == 1 && panel == this)
    {
	guiSetInputFocus(disp, Main, RevertToNone, CurrentTime);
	just_at_startup = 2;
    };
    */
    if (disable_reread != 2 && cur)
	showfinfo(cur, selfiles);
    else
	disable_reread = 0;
    xncdprintf(("====End expose for panel %d====\n",lay));
}


//----------------------------------------------------------------------------
//Convert item number to xy coords
//return 1 if n inside page, 0 otherwise
//----------------------------------------------------------------------------
int    Lister::get_xy_of_item_by_number(int n, int& to_x, 
					int& to_y_text,
					int& to_y)
{
  to_y = n % items_per_column * main_area.item_h + 
      main_area.active_from_y;
  to_y_text = to_y + main_area.text_y + Lister::main_area_item_border;
  to_x = n / items_per_column * pixels_per_column + Lister::main_area_column_border 
      + main_area.active_from_x;
  return n<items_per_page;
}

//----------------------------------------------------------------------------
//Convert xy coords to item number
//return 1 if n inside page, 0 otherwise
//----------------------------------------------------------------------------
int    Lister::get_item_number_by_xy(int x, int y, int& k) 
{
    k = (y - main_area.active_from_y - Lister::main_area_column_border) / 
	main_area.item_h + 
	(x - Lister::main_area_column_border - main_area.active_from_x) / 
	pixels_per_column * items_per_column;
    return k<items_per_page;
}

//-------------------------------------------------------------------------
// Calculate item number and set cursor to it by xy coords.
// return 1 if success
//-------------------------------------------------------------------------
int Lister::set_cursor_by_xy_no_redraw(int x, int y)
{
    int k,i;
    if (get_item_number_by_xy(x,y,k))
    {
	xncdprintf(("Setting cursor to item %d\n",k));
	if (curn != k)
	{
	    i = curn;
	    curn = k;
	    showitem(cur, i);
	    if (setcur(&cur, k - i))
	    {
		xncdprintf(("Setting cursor to item %d - OK\n",k));
		curn = k;
	    }
	    else
	    {
		xncdprintf(("Setting cursor to item %d - FAIL, back to %d\n",k,i));
		curn = i;
	    }
	    return 1;
	} 
    }
    return 0;
}

//-------------------------------------------------------------------------
// Calculate item number and set cursor to it by xy coords.
// return 1 if success
//-------------------------------------------------------------------------
int Lister::set_cursor_by_xy_with_redraw(int x, int y)
{
    int k,i;
    if (get_item_number_by_xy(x,y,k))
    {
	if (curn != k)
	{
	    i = curn;
	    curn = k;
	    showitem(cur, i);
	    if (setcur(&cur, k - i))
		curn = k;
	    else
		curn = i;
	    showitem(cur,curn);
	    return 1;
	} 
    }
    return 0;
}

//----------------------------------------------------------------------------
// Highlight fast find item in list 
//----------------------------------------------------------------------------
void   Lister::show_ff_item(int n, int fflen)
{
  int    iy, ix, piy, delta = 0;
  get_xy_of_item_by_number(n, ix, iy, piy);
  if ((cur->mode & S_IFDIR) == S_IFDIR)
  {
    if (allow_dir_icon)
	delta = Lister::main_area_icons_width;
  }
  else 
      if (allow_file_icon)
	  delta = Lister::main_area_icons_width;
  if (lastn != n)
  {
    XSetClipOrigin(disp, wgcw, ix, 0);
    lastn = n;
  };
  XSetForeground(disp, wgcw, f_select_color);

  //FIXME for display_format
  XDrawString(disp, w, wgcw, ix + delta, iy, ff_buf, fflen);

  XFlush(disp);
}

//----------------------------------------------------------------------------
// Show empty item
//----------------------------------------------------------------------------
void   Lister::showempty(int n)
{
  int    iy, ix, dummy;
  get_xy_of_item_by_number(n,ix,dummy,iy);

  if (lastn != n)
  {
    XSetClipOrigin(disp, wgcw, ix, 0);
    lastn = n;
  };
  XClearArea(disp, w, ix, iy, 
	     main_area.item_l, main_area.item_h, 0);
}

//----------------------------------------------------------------------------
// Show one item in list
// Display file info in position n according to format.
// Format:  'F|A|S|U|G' or 'F.A.S.U.G'
//      F - File name
//      A - File attributes (-rwxr-x-r-x)
//      O - File attributes in octal ( 0755 )
//      S - File size
//      D - File date/time (last modification)
//      U - File owner (user)
//      G - File group
//      | - Draw separator
//      . - Don't draw separator
//----------------------------------------------------------------------------

void   Lister::showitem(FList * ol, int n)
{
    unsigned int color;

    int    cn, iy, ix, piy, pixy, delta = 0, pixn = 0, i;
    char   str[40];
    char   str2[80];
    char  *str3;
    
    get_xy_of_item_by_number(n, ix, iy, piy);
    
    XSetClipOrigin(disp, wgcw, ix, 0);
    
    lastn = n;
    
    if (curn == n) //Under cursor
    {
	showfinfo(ol, selfiles);
	if ((ol->mode & S_IFDIR) == S_IFDIR)
	    if (qvflag)
	    {
		qv_update(ol);
	    };
	
	if(option_bits & FILLED_CURSOR)
	{
	    XSetForeground(disp, wgcw, cursor_color);
	    XFillRectangle(disp, w, wgcw, ix, piy, 
			   main_area.item_l, main_area.item_h);
	    XSetForeground(disp, wgcw, dark_cursor_color);
	    XDrawRectangle(disp, w, wgcw, ix, piy, 
			   main_area.item_l - 1, main_area.item_h - 1);
	} else
	{
	    XClearArea(disp, w, ix, piy, main_area.item_l, main_area.item_h, 0);
	    XSetForeground(disp, wgcw, cursor_color);
	    XDrawRectangle(disp,w,wgcw, ix, piy, 
			   main_area.item_l-1, main_area.item_h-1);
	}
    }
    else
    {
	XClearArea(disp, w, ix, piy, main_area.item_l, main_area.item_h, 0);
    }


    if (allow_dir_icon || allow_file_icon)
	delta = Lister::main_area_icons_width;
    
    if (ol->options & S_SELECT)
	color=f_select_color;
    else
	color=ol->file_color;
    
    XSetForeground(disp, wgcw, color);
    
    for(i=0;i<display_format[display_mode].max_interval;i++)
	(this->*display_format[display_mode].intervals[i].draw_method)
	    (ix+delta,piy,iy,color,
	     display_format[display_mode].intervals[i],
	     ol);
   
    if((allow_dir_icon && ol->file_type==file_is_dir) ||
       (allow_file_icon && ol->file_type!=file_is_dir))
	default_iconman->display_icon_from_set(w, xpmgc,
					       ix, piy + main_area.item_h/2,
					       file_type_iconset, ol->icon_type);

    XSetClipRectangles(disp, wgcw, 0, 0, &rect, 1, Unsorted);
}

//----------------------------------------------------------------------------
// Show Lister widget (just mapping window)
//----------------------------------------------------------------------------
void   Lister::show()
{
  if(main_pixmap)
    XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  XMapRaised(disp, w);
  addto_el(this, w);
  if(default_dnd_man)
  {
    default_dnd_man->add_dnd_object(this,w);
    dndwin->add_type("text/uri-list");
  }
          //  scr->show();
}


void   draw_filled_urect(Window w, GC gcw, int x, int y, int l, int h)
{
          /*
                    XClearArea(disp, w, x+1, y+1, l-1, h-1, 0);

                    XSetForeground(disp, gcw, keyscol[1]);
                    XFillRectangle(disp, w, gcw, x+1, y+1, l-1, h-1);
          */
}

void   Lister::redraw_statusbar()
{
    if(cur)
	showfinfo(cur, selfiles);
}

void   Lister::show_string_info(char *str)
{
    //FIXME
    int stlx=status_area.active_from_x+1;
    int stll=status_area.active_l-2;
    int ll;
    int ty=status_area.active_text_y;

    if(selfiles && cur)
    {
	showfinfo(cur, 0);
	return;
    }

    ll=strlen(str);
    if(ll>stll/fixl)
	ll=stll/fixl;
    XClearArea(disp, w, 
	       status_area.active_from_x+1,
	       status_area.active_from_y+1, 
	       status_area.active_l-2, status_area.active_h-2, 0);

    urect(w, gcw,
	  status_area.active_from_x,
	  status_area.active_from_y, 
	  status_area.active_l, status_area.active_h);
    
    if(shadow)
    {
	XSetForeground(disp, gcw, shadow_color);
	XDrawString(disp, w, gcw, stlx+1,
		    ty+1, str, ll);
    }
    XSetForeground(disp, gcw, statusbar_color);
    XDrawString(disp, w, gcw, stlx,
		ty, str, ll);
    XFlush(disp);
}


void   Lister::showfinfo(FList * fo, int files)
{
    static char fname[512];
    static char flink[512];
    int    i = 0, sl, vl = 0, dl = 0, ll;
    char   chr2[30];
    char   chr1[30];
    char   tchr[256];

    XClearArea(disp, w, 
	       status_area.active_from_x+1,
	       status_area.active_from_y+1, 
	       status_area.active_l-2, status_area.active_h-2, 0);

    urect(w, gcw,
	  status_area.active_from_x,
	  status_area.active_from_y, 
	  status_area.active_l, status_area.active_h);
    
    if(STL_ATTR_PTR.enabled)
    {
	dig2ascii_r(chr1, fo->mode & 07777, 4, 8, '0');
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, STL_ATTR_PTR.tx+1,
			STL_ATTR_PTR.ty+1, chr1, 4);
	}
	XSetForeground(disp, gcw, statusbar_color);
	XDrawString(disp, w, gcw, STL_ATTR_PTR.tx,
		    STL_ATTR_PTR.ty, chr1, 4);
	
    }
    if(STL_TIME_PTR.enabled)
    {
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, STL_TIME_PTR.tx+1,
			STL_TIME_PTR.ty+1, fo->chr_time, STL_TIME_NCHAR);
	}
	XSetForeground(disp, gcw, statusbar_color);
	XDrawString(disp, w, gcw, STL_TIME_PTR.tx,
		    STL_TIME_PTR.ty, fo->chr_time, STL_TIME_NCHAR);
	
    }
    if(STL_OWNER_PTR.enabled)
    {
	ll=sprintf(tchr,"%s.%s", fo->user, fo->group);
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, STL_OWNER_PTR.tx+1,
			STL_OWNER_PTR.ty+1, tchr, ll);
	}
	XSetForeground(disp, gcw, statusbar_color);
	XDrawString(disp, w, gcw, STL_OWNER_PTR.tx,
		    STL_OWNER_PTR.ty, tchr, ll);
	
    }
    if(STL_SIZE_PTR.enabled)
    {
	if (files == 0)
	{
	    if (fo->mode & S_IFDIR)
	    {
		strcpy(chr2, ">DIR<");
		dig2ascii_r(chr2+5, fo->size, 6);
	    }
	    else
		dig2ascii(chr2, fo->size);
	}
	else
	{
	    dig2ascii(chr2, selsize);
	}
	vl=strlen(chr2);
	dl=STL_SIZE_PTR.tlen;
	
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, STL_SIZE_PTR.tx+1+(dl-vl)*fixl,
			STL_SIZE_PTR.ty+1, chr2, vl);
	}
	XSetForeground(disp, gcw, statusbar_color);
	XDrawString(disp, w, gcw, STL_SIZE_PTR.tx+(dl-vl)*fixl,
		    STL_SIZE_PTR.ty, chr2, vl);
    }
    
    if(STL_NAME_PTR.enabled)
    {
	int tlen=STL_NAME_PTR.tlen;
	vl=STL_NAME_PTR.tlen;
	dl=0;
	
	if ((fo->mode & S_IFLNK) == S_IFLNK && files == 0)
	{
	    vfs->ch_curdir();
	    ll=vfs->readlink(fo, flink, 512);
	    flink[ll]=0;
	    if (ll < tlen-3)
	    {
		sl = strlen(fo->name);
		if (sl + ll > tlen)
		{
		    for (i = ll; i >= 0; i--)
			fname[tlen - 1 - ll + i] = flink[i];
		    fname[tlen - 2 - ll] = '>';
		    fname[tlen - 3 - ll] = '-';
		    for (i = sl - 1; i >= 0; i--)
			if (tlen - 4 - sl + i < 0)
			    break;
			else
			    fname[tlen - sl + i] = fo->name[i];
		}
		else
		{
		    strcpy(fname, fo->name);
		    strcat(fname, "->");
		    strcat(fname, flink);
		}
	    }
	    else
		strcpy(fname, flink + ll - tlen - 3);
	    sl = strlen(fname);
	    if (sl > vl)
		dl = sl - vl;
	    vl = sl - dl;
	    if(shadow)
	    {
		XSetForeground(disp, gcw, shadow_color);
		XDrawString(disp, w, gcw, STL_NAME_PTR.tx+1, STL_NAME_PTR.ty + 1, fname + dl, vl);
	    }
	    XSetForeground(disp, gcw, statusbar_color);
	    XDrawString(disp, w, gcw, STL_NAME_PTR.tx, STL_NAME_PTR.ty, fname + dl, vl);
	}
	else
	{
	    char *pname;
	    if(files)
	    {
		pname=tchr;
		dig2ascii_r(pname, files, 4);
		strcat(pname, _(" file(s) selected"));
	    }
	    else
	    {
		if(fo->get_dir())
		    pname=fo->get_dir();
		else
		    pname=fo->name;
	    }

	    sl = strlen(pname);
	    if (sl > vl)
		dl = sl - vl;
	    vl = sl - dl;
	    if(shadow)
	    {
		XSetForeground(disp, gcw, shadow_color);
		XDrawString(disp, w, gcw, STL_NAME_PTR.tx+1, STL_NAME_PTR.ty + 1, pname + dl, vl);
	    }
	    XSetForeground(disp, gcw, statusbar_color);
	    XDrawString(disp, w, gcw, STL_NAME_PTR.tx, STL_NAME_PTR.ty, pname + dl, vl);
	}
    }
    if(option_bits & STATUS_INODE)
	show_inodeinfo(fo);
}


void   Lister::header_blink_other()
{
    char *cdir;
    int sl,i;

    if(orientation==OnePage)
    {
	panel2->header_blink();
	return;
    }
    
    cdir = panel2->vfs->get_dir_header();
    sl = strlen(cdir);

    for(i=0;i<3;i++)
    {
	vlook_display_header_inactive_string(cdir,sl,normal_bg_color);
	XSync(disp,0);
	delay(150);
	
	vlook_display_header_inactive_string(cdir,sl,dark_bg_color);
	XSync(disp,0);
	delay(150);
    }
}

void   Lister::header_blink()
{
    int i;
    char  *curdir = vfs->get_dir_header();
    int    sl = strlen(curdir);

    for(i=0;i<3;i++)
    {
	vlook_display_header_string(curdir,sl,normal_bg_color);
	XSync(disp,0);
	delay(150);
	
	vlook_display_header_string(curdir,sl,panel_info_color);
	XSync(disp,0);
	delay(150);
    }
}

void   Lister::showdirname()
{
    char  *curdir = vfs->get_dir_header();
    int    sl = strlen(curdir);
    vlook_display_header_string(curdir,sl,panel_info_color);
}

void   Lister::showdir()
{
    FList *fo = base;
    int    i = 0, al = items_per_page;
    while (fo != NULL && i < al)
    {
	showitem(fo, i);
	fo = fo->next;
	i++;
    }
    xncdprintf(("Lister:showdir items %d\n",i));
    while (i < al)
    {
	showempty(i);
	i++;
    };
    showdirname();
    if (cmdl != NULL && foc)
	cmdl->setpath(curdir);
}


//--------------------------------------------------------------------------
//Return pointer to entry in listing by coords in panel
//return 0 if missed
//--------------------------------------------------------------------------
FList* Lister::find_entry_by_coord(int x, int y)
{
  FList *obj;
  int k;
  if (main_area.get_area_by_xy(x,y) == areaMainActive &&
      get_item_number_by_xy(x,y,k))
  {
      obj=base;
      while(obj && k)
      {
	  k--;
	  obj=obj->next;
      }
      return obj;
  }
  return 0;
}


///////////////////////////////////FS operations//////////////////////////////
void   Lister::fserror(char *head, FList * o)
{
  show_file_error(head,o);
}

void   Lister::set_recycle_state(int state)
{
  bmark->set_recycle_image(0);
}

void   Lister::create_infowin(char *head)
{
  ::create_infowin(head);
}

void   Lister::del_infowin()
{
  ::del_infowin();
}

void   Lister::to_infowin(char *mes)
{
  ::to_infowin(mes);
}

void   Lister::infowin_set_coun(int n)
{
  infowin->set_coun(n);
}

void   Lister::infowin_update_coun(int n)
{
  infowin->set_coun(n);
}

void   Lister::flush_screen()
{
  XFlush(disp);
}

void   Lister::expose_panel()
{
  expose();
}

int   Lister::am_i_visible()
{
  return panel==this;
}


void   Lister::total_expose()
{
  if (lay == 0)
    panel2->expose();
  expose();
}

void   Lister::total_reread()
{
  if (lay == 0)
    panel2->reread();
  reread();
}

BaseCaptain* Lister::other()
{
  return panel2;
}

VFS* Lister::other_vfs()
{
  return panel2->vfs;
}

int    Lister::is_panel_vfs(VFS* vfstmp)
{
  return vfstmp == panel2->vfs;
}

void   Lister::show_item(FList * o)
{
  FList *t = base;
  int    coun = 0;
  while (t != o && t != NULL)
  {
    t = t->next;
    coun++;
  };
  if (t == o && coun < items_per_page)
    showitem(o, coun);
}





void   Lister::remount()
{
  char  *mntptr = findmntent(curdir);
  if (mntptr != NULL)
  {
    chdir("..");
    senddir("..");
    tt_printf("umount %s\n", mntptr);
    tt_printf("mount %s\n", mntptr);
    reread();
  }
}

void   Lister::ext_exec(char *exec)
{
  char   findchr[2*L_MAXPATH];
  char   fname[L_MAXPATH];
  char   fdir[L_MAXPATH];
  char   execdup[L_MAXPATH];
  char  *s, c, *b;

  if (vfs->is_exec_supported())
  {
    char  *name = vfs->get_file_for_execute(cur);
    strcpy(execdup,exec);
    if (name == NULL)
    {
      show_vfs_error();
      return;
    }
    char  *execdir = vfs->get_execute_dir(cur);

    s = execdup;
    while (*++s)
    {
      if (*s == '#')
      {
	*s = 0;
	break;
      }
      /*          if (*s == '\\')
		  {
		  if (++*s)
		  break;
		  }
      */  
      if (*s == '"' || *s == '\'')
      {
	c = *s;
	while (*++s)
	{
	  if (*s == c)
	  {
	    break;
	  }
	}
	if (!*s)
	  break;
      }
    }

    if (findsubstr(execdup, "%s/%s"))
    {
      chdir(panel2->curdir);
      senddir(panel2->curdir);
      sprintf(fdir, "%s/%s", execdir, name);
      quote_path(fname, fdir);
      b=strchr(fname, '/');
      *b=0;b++;
      sprintf(findchr, execdup, fname, b);
    }
    else
    {
      senddir(execdir);
      quote_path(fname, name);
      sprintf(findchr, execdup, fname);
    }
    raise_terminal_if_need();
    flush_screen();
    tt_printf("%s\n", findchr);
    cmdl->flush();
    senddir(curdir);
    total_reread();
    flush_screen();
  }
  else
    XBell(disp, 0);
}

void   Lister::menu_exec(char *mecom)
{
  char   findchr[L_MAXPATH];
  char  *suc;
  xncdprintf(("Menu command: [%s]\n",mecom));
  if (vfs->is_exec_supported())
  {
    char  *name = vfs->get_file_for_execute(cur);
    if (name == NULL)
    {
      show_vfs_error();
      return;
    }
    char  *execdir = vfs->get_execute_dir(cur);
    int    i = 0;
    if ((suc = findsubstr(mecom, "%p(")))
    {
      suc += 3;
      while (*suc != ')' && *suc != 0)
	wfile[i++] = *suc++;
      wfile[i] = 0;
      newtextfile(wfile, this, FWIDEEXEC);
    }
    else
    {
      if (findsubstr(mecom, "%s/%s"))
      {
	chdir(panel2->curdir);
	senddir(panel2->curdir);
	sprintf(findchr, mecom, execdir, name);
      }
      else
      {
	senddir(execdir);
	sprintf(findchr, mecom, name);
      };
      xncdprintf(("Executing menu command [%s]\n", findchr));
      raise_terminal_if_need();
      flush_screen();
      tt_printf("%s\n", findchr);
      cmdl->flush();
      senddir(curdir);
      total_reread();
      flush_screen();
    }
  }
  else
    XBell(disp, 0);
}

void   Lister::wide_exec()
{
  char  *suc;
  int    i = 0;
  suc = findsubstr(menucom[comnumber], "%p(");
  while (menucom[comnumber][i] != *suc)
    combuffer[i] = menucom[comnumber][i++];
  combuffer[i] = 0;
  i = 0;
  strcat(combuffer, dbuf);
  suc += 3;
  while (*suc != ')' && *suc != 0)
    suc++;
  if (*suc != 0)
    strcat(combuffer, suc + 1);
  menu_exec(combuffer);
}

void   Lister::raise_terminal_if_need()
{
  if(!is_term_on())
    return;
  if (allow_t_raising)
  {
      switch_term_fm_state();
//     focus_fl = 0;
//     if (vt_show)
//     {
//       XRaiseWindow(disp, main_win);
//       guiSetInputFocus(disp, main_win, RevertToNone, CurrentTime);
//     }
//     else
//       XMapRaised(disp, main_win);
//     xnc_now_on_top=0;
  }
  else
  {
    delay(100);
  }
}

void   Lister::ps_info()
{
  int    psmax = getps();
  if (psmax > 0)
    newkillpanel(_("Kill process!"), psmax, pankill, 600);
}

void   Lister::lpr()
{
  char   dest[L_MAXPATH];
  if(cur->get_dir())
      chdir(cur->get_dir());
  else
      chdir(curdir);
  quote_path(dest, cur->name);
  tt_printf("lpr %s\n", dest);
}



void   Lister::view(int v)
{
  char   str[L_MAXPATH];
  char   dest[L_MAXPATH];
  char  *name;
  chdir(curdir);
  VFS *pvfs=vfs;
  if (strcmp(viewname, "internal") != 0)
  {
    name = vfs->get_file_for_execute(cur);
    if (name == NULL && pvfs->bgbit==0)
    {
      show_vfs_error();
      return;
    }
    if(pvfs->bgbit!=0)
    {
      if(pvfs->bgbit==FF_GET)
	ftp_set_bgcode((FTP*)pvfs, FF_VIE);
      return;
    }
    sprintf(str, "%s/%s", vfs->get_execute_dir(cur), name);
    quote_path(dest, str);
    tt_printf("%s %s\n", viewname, dest);
    raise_terminal_if_need();
  }
  else if (viewfunc != NULL && ((cur->mode & S_IFDIR) == 0) && (cur->size != 0) && v != 0 && vfs->is_exec_supported())
  {
    name = vfs->get_file_for_execute(cur);
    if (name == NULL && pvfs->bgbit==0)
    {
      show_vfs_error();
      return;
    }
    if(pvfs->bgbit!=0)
    {
      if(pvfs->bgbit==FF_GET)
	ftp_set_bgcode((FTP*)pvfs, FF_VIE);
      return;
    }
    sprintf(str, "%s/%s", vfs->get_execute_dir(cur), name);
    viewfunc(str);
  }
  else if (viewonlyfunc != NULL && ((cur->mode & S_IFDIR) == 0) && (cur->size != 0) && v == 0 && vfs->is_exec_supported())
  {
    name = vfs->get_file_for_execute(cur);
    if (name == NULL && pvfs->bgbit==0)
    {
      show_vfs_error();
      return;
    }
    if(pvfs->bgbit!=0)
    {
      if(pvfs->bgbit==FF_GET)
	ftp_set_bgcode((FTP*)pvfs, FF_SVI);
      return;
    }
    sprintf(str, "%s/%s", vfs->get_execute_dir(cur), name);
    viewonlyfunc(str);
  }
}

void   Lister::bg_view(char *name)
{
  char   dest[L_MAXPATH];
  if (strcmp(viewname, "internal") != 0)
  {
      quote_path(dest, name);
      tt_printf("%s %s\n", viewname, dest);
      raise_terminal_if_need();
  }
  else
  {
    if(vfs->bgbit==FF_VIE)
      viewfunc(name);
    else
      viewonlyfunc(name);
  }
}

void   Lister::bg_edit(char *name)
{
  char   dest[L_MAXPATH];
  if (strcmp(editname, "internal") != 0)
  {
      quote_path(dest, name);
      tt_printf("%s %s\n", editname, dest);
      raise_terminal_if_need();
  }
  else
  {
    editorfunc(name);
  }
}

void   Lister::edit()
{
  char   str[L_MAXPATH];
  char   dest[L_MAXPATH];
  char  *name;
  chdir(curdir);
  VFS *pvfs=vfs;
  if (strcmp(editname, "internal") != 0 && vfs->is_exec_supported())
  {
    name = vfs->get_file_for_execute(cur);
    if (name == NULL && pvfs->bgbit==0)
    {
      show_vfs_error();
      return;
    }
    if(pvfs->bgbit!=0)
    {
      if(pvfs->bgbit==FF_GET)
	ftp_set_bgcode((FTP*)pvfs, FF_EDI);
      return;
    }
    sprintf(str, "%s/%s", vfs->get_execute_dir(cur), name);
    quote_path(dest, str);
    tt_printf("%s %s\n", editname, dest);
    raise_terminal_if_need();
  }
  else if (editorfunc != NULL && ((cur->mode & S_IFDIR) == 0) && vfs->is_exec_supported())
  {
    name = vfs->get_file_for_execute(cur);
    if (name == NULL && pvfs->bgbit==0)
    {
      show_vfs_error();
      return;
    }
    if(pvfs->bgbit!=0)
    {
      if(pvfs->bgbit==FF_GET)
	ftp_set_bgcode((FTP*)pvfs, FF_EDI);
      return;
    }
    sprintf(str, "%s/%s", vfs->get_execute_dir(cur), name);
    editorfunc(str);
  }
}

void   Lister::del()
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_DEL)
    {
      bmark->set_recycle_image(0);
      initquery(_("Delete file?"), this, panel2->vfs, FDELETE);
    } else
    {
      set_dbuf(panel2->vfs);
      funcs(FDELETE);
    }
      
  }
}

void   Lister::copy()
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_COPY)
    {
      if (selfiles)
        init_copymove_query(_("Copy files?"), this, panel2->vfs, FCOPY);
      else
        initdialog(_("Copy file"), this, panel2->vfs, FCOPY);
    } else
    {
      set_dbuf(panel2->vfs);
      funcs(FCOPY);
    }
  }
}

void   Lister::make_link()
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_LINK)
    {
      if (selfiles)
        init_copymove_query(_("Link files?"), this, panel2->vfs, FSYMLINK);
      else
        initdialog(_("Make a link to file"), this, panel2->vfs, FSYMLINK);
    } else
    {
      set_dbuf(panel2->vfs);
      funcs(FSYMLINK);
    }
  }
}

void   Lister::pack()
{
    char tmpstr[L_MAXPATH];
    if (strcmp(cur->name, "..") != 0 || selfiles != 0)
    {
	if(use_prompt & PROMPT_PACK)
	{
	    vfs->ch_curdir();
	    
	    AFS::construct_archive_name(tmpstr, 
					selfiles ? vfs->get_dir_header() : cur->name, 
					selfiles ? 1 : (cur->file_type==file_is_dir),
					".tar.gz");
	    
	    if(selfiles)
		init_copymove_query(_("Create archive?"), this, panel2->vfs, FPACK, tmpstr);
	    else
		initdialog(_("Create archive?"), this, panel2->vfs, FPACK, tmpstr);
	    
	} else
	{
	    vfs->ch_curdir();
	    
	    AFS::construct_archive_name(tmpstr, 
					selfiles ? vfs->get_dir_header() : cur->name, 
					selfiles ? 1 : (cur->file_type==file_is_dir),
					".tar.gz");
	    set_dbuf(panel2->vfs, tmpstr);
	    funcs(FPACK);
	}
    }
}

void   Lister::copy(VFS * vf)
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_COPY)
    {
      if (selfiles)
        init_copymove_query(_("Copy files?"), this, vf, FCOPY);
      else
        initdialog(_("Copy file"), this, vf, FCOPY);
    } else
    {
      set_dbuf(vf);
      funcs(FCOPY);
    }
  }
}

//edit for movedir...

void   Lister::move()
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_MOVE)
    {
      if (selfiles)
        init_copymove_query(_("Move files?"), this, panel2->vfs, FMOVE);
      else
        initdialog(_("Move file"), this, panel2->vfs, FMOVE);
    } else
    {
      set_dbuf(panel2->vfs);
      funcs(FMOVE);
    }
  }
}

//-------------------------------------------------------------------------
// Open rename file dialog
//-------------------------------------------------------------------------
void   Lister::rename()
{
  if (strcmp(cur->name, "..") != 0 && selfiles == 0)
  {
    vfs->ch_curdir();
    initdialog(_("Rename file"), this, 0, FMOVE, cur->name);
  }
}


void   Lister::move(VFS * vf)
{
  if (strcmp(cur->name, "..") != 0 || selfiles != 0)
  {
    vfs->ch_curdir();
    if(use_prompt & PROMPT_MOVE)
    {
      if (selfiles)
        init_copymove_query(_("Move files?"), this, vf, FMOVE);
      else
        initdialog(_("Move file"), this, vf, FMOVE);
    } else
    {
      set_dbuf(panel2->vfs);
      funcs(FMOVE);
    }
  }
}


void   Lister::insert_left_dir()
{
  if(this<panel2)
  {
    cmdl_insert_name(curdir);
    cmdl->insert('/');
    tt_printf("/");
  } else
    panel2->insert_left_dir();
}

void   Lister::insert_right_dir()
{
  if(this>panel2)
  {
    cmdl_insert_name(curdir);
    cmdl->insert('/');
    tt_printf("/");
  } else
    panel2->insert_right_dir();
}

void   Lister::activate()
{
    ActionKey defexact;
    char   findchr[L_MAXPATH];
    char  *name;
    char   tname[1024];
    StoreBuffer   command_buf;
    int    findfl = 0, kn, kk, pid, ef, efn;
    FList *dub;
    ActionKey *ac = action_by_name(AExecute);
    lastscan = 0;
    if (ac == NULL)
    {
	ac = &defexact;
	ac->mods[1] = ShiftMask;
	ac->mods[2] = ControlMask;
	ac->action = AExecute;
    }
    vfs->ch_curdir();
    dub = new FList(cur->name, cur->mode, cur->size, cur->uid, cur->gid, cur->time);
    if ((ev.xkey.state & ac->mods[2]) == ac->mods[2])
    {                                /*+1 */
	pid = 0;
	if ((ev.xkey.state & ac->mods[1]) == ac->mods[1])
	{                        /*+2 */
	    cmdl_insert_name(curdir);
	    cmdl->insert('/');
	    tt_printf("/");
	}
	else
	    /*-2*/
	{                        /*+3 */
	    if (cur->mode & S_IFDIR)
	    {
		cmdl_insert_name(cur->name);
		cmdl->insert(' ');
		tt_printf(" ");
          
	    } else
		if (vfs->is_exec_supported())
		{
		    name = vfs->get_file_for_execute(cur);
		    if (name == NULL)
		    {
			show_vfs_error();
			return;
		    }
		    if(vfs->fstype!=DFS_TYPE)
		    {
			cmdl_insert_name(vfs->get_execute_dir(cur));
			cmdl->insert('/');
			tt_printf("/");
		    }
		    cmdl_insert_name(name);
		    cmdl->insert(' ');
		    tt_printf(" ");
		}
	}
	/*-3*/
    }
    /*-1*/
    else 
	if (cmdl->bl != 0 && mouse==0)
	{                                /*+4 */
	    if(!cmdl->flush())
		raise_terminal_if_need();
	    evret = (!vfs->pass_cd_to_terminal() && (masks & PANEL_CD_DONE));
	    masks&=~PANEL_CD_DONE;
	    XFlush(disp);
	}
	else
	{
	    cmdl->save_input(&command_buf);
	    
	    /*-4*/ 
	    if ((cur->mode & S_IFDIR) && cmdl->bl == 0)
	    {                                /*+5 */
		reset_branch_view_mode();
		if(cur->get_dir())
		    vfs->chdir(cur->get_dir());
		else
		    vfs->ch_curdir();
		get_last_content(vfs->curdir, findchr);
		if (vfs->chdir(cur->name) == 0)
		{                        /*+6 */
		    if (vfs->need_change_vfs)
		    {
			if (strcmp(cur->name, "..") == 0)
			{                /*+7 */
			    findfl = 1;
			    if (vfs->fstype == AFS_TYPE)
				strcpy(findchr, afs.arcname);
			    else
				get_last_content(curdir, findchr);
			};
			/*-7*/
			vfs->delete_vfs_list();
			vfs->close_fs();
			vfs = pop_vfs();
			vfs->ch_curdir();
		    }
		    else if (strcmp(cur->name, "..") == 0)
		    {                        /*+7 */
			findfl = 1;
		    };
		    /*-7*/
		    vfs->getcwd(curdir, 1020);
		    if (vfs == &dfs)
			senddir(curdir);
		    delall_l(&dl);
		    dl.next = fl.next = NULL;
		    if (direc(vfs->curdir) == 0)
		    {                        /*+8 */
			errno2mes();
			fserror(_("Reading directory"), dub);
			vfs->chdir("..");
			vfs->getcwd(curdir, 1020);
			direc(curdir);
		    }
		    /*-8*/
		    if (findfl != 0 && findchr[0] != 0)
		    {                        /*+9 */
			kn = findentry(findchr);
			if (kn != -1)
			{                /*+10 */
			    curn = kn % items_per_page;
			    kk = kn - curn;
			    setcur(&base, kk);
			    setcur(&cur, kn);
			}
			/*-10*/
		    }
		    /*-9*/
		    showdir();
		}
		else
		{
		    errno2mes();
		    fserror(_("Changing directory"), dub);
		}
		/*-6*/
	    }
	    else
		/*-5*/ if (cur->options & S_EXTEND)
		{                                /*+15 */
		    if (is_it_afs_file(cur->name) && (ev.xkey.state & ac->mods[1]) == 0)
		    {
			name = vfs->get_file_for_execute(cur);
			if (name)
			{
			    // *INDENT-OFF*        
			    ::chdir(vfs->get_execute_dir(cur));
			    // *INDENT-ON*        


			    strcpy(tname, name);
			}
			vfs->delete_vfs_list();
			push_vfs(vfs);
			vfs = &afs;
			if (name == NULL)
			{
			    show_vfs_error();
			    strcpy(findchr, afs.arcname);
			    vfs = pop_vfs();
			    findfl = 1;
			    // *INDENT-OFF*        
			    ::chdir(curdir);
			    // *INDENT-ON*        


			}
			else
			{
			    if (afs.init_support(tname) == 0)                //If error we return to panels DFS

			    {
				show_vfs_error();
				strcpy(findchr, afs.arcname);
				vfs = pop_vfs();
				findfl = 1;
			    }
			    // *INDENT-OFF*        
			    ::chdir(curdir);
			    // *INDENT-ON*        


			}
			vfs->ch_curdir();
			if (direc(curdir) == 0)
			{                        /*+8 */
			    fserror(_("Reading directory"), dub);
			    vfs->chdir("..");
			    vfs->getcwd(curdir, 1020);
			    direc(curdir);
			}
			/*-8*/
			if (findfl != 0 && findchr[0] != 0)
			{                        /*+9 */
			    kn = findentry(findchr);
			    if (kn != -1)
			    {                /*+10 */
				curn = kn % items_per_page;
				kk = kn - curn;
				setcur(&base, kk);
				setcur(&cur, kn);
			    }
			    /*-10*/
			}
			/*-9*/
			showdir();
		    }
		    else
		    {
			efn = 0;
			while ((ef = ext_find2(cur->name, efn)) != -1)
			    panlist[efn++] = extcom[ef];
			if (efn == 1)
			    ext_exec(panlist[0]);
			else
			    newpanel(_("Which command?"), efn, panexec);
		    }
		}
		else
		    /*-15*/ if (is_exec(cur))
		    {                                /*+11 */
			if ((ev.xkey.state & ac->mods[1]) == 0)
			{                        /*+12 */
			    evret = 0;
			    if (cmdl->bl == 0)
			    {
				if (vfs->is_exec_supported())
				{
				    name = vfs->get_file_for_execute(cur);
				    if (name == NULL)
				    {
					show_vfs_error();
					return;
				    }
				    if (vfs->fstype != DFS_TYPE)
					senddir(vfs->get_execute_dir(cur));
				    tt_printf("./");
				    cmdl_insert_name(name);
				    if (mouse)
					tt_printf("\n");
				    pid = 0;
				    XFlush(disp);
				}
			    }
			    raise_terminal_if_need();
			    cmdl->flush();
			    if (vfs->fstype != DFS_TYPE)
			    {
				if (mouse == 0)
				    tt_printf("\n");
				senddir(curdir);
			    }
			    XFlush(disp);
			}
			/*-12*/
			else
			{                        /*+14 */
			    evret = 0;
			    if (cmdl->bl == 0 && vfs->is_exec_supported())
			    {
				cmdl_insert_name(vfs->get_file_for_execute(cur));
				tt_printf(" &");
			    }
			    cmdl->flush();
			    XFlush(disp);
			}
			/*-14*/
		    }
		    else
			/*-11*/
			run_with_extension_menu();
	    cmdl->restore_input(&command_buf);
	    cmdl->expose();
	}
    delete dub;
}

//Construct menu from extension commands
void Lister::run_with_extension_menu()
{
    int ef, idx=0, i, skip_it;
    for (ef = 0; ef < extmax; ef++)
    {
	skip_it=0;
	for(i=0;i<idx;i++)
	    if(!strcmp(panlist[i],extcom[ef]))
	    {
		skip_it=1;
		break;
	    }
	if(!skip_it)
	    panlist[idx++] = extcom[ef];
    }
    newpanel(_("Execute with..."), idx, panexec);
}

void   Lister::funcs(ManFuncs funcnum)
{
  switch (funcnum)
  {
  case FDELETE:
    fdelete();
    break;
  case FCOPY:
    fcopy();
    break;
  case FMOVE:
    fmove();
    break;
  case FMKDIR:
    fmkdir();
    break;
  case FEDIT:
    fedit();
    break;
  case FSELBYMASK:
    select_by_mask(1);
    break;
  case FDESELMASK:
    select_by_mask(0);
    break;
  case FSETATTR:
    setattr();
    break;
  case FLPR:
    lpr();
    break;
  case FWIDEEXEC:
    wide_exec();
    break;
  case FMANPAGE:
    man_page();
    break;
  case FFTPCON:
    do_ftp_link();
    break;
  case FFTPCONPWD:
    do_ftp_link_passwd(dbuf);
    break;
  case FCHOWN:
    chowns();
    break;
  case FMOUNT:
    panmntmnt();
    break;
  case FPACK:
    pack_files(dbuf);
    break;
  case FCDDIR:
    change_dir(dbuf);
    break;
  case FSYMLINK:
    flink();
    break;
  default:
    break;
  }
}


void   Lister::mkdir()
{
  vfs->ch_curdir();
  newtextfile(_("Make directory"), this, FMKDIR);
}


void   Lister::scrollup(Gui*)
{
  go_upline();
}

void   Lister::scrolldown(Gui*)
{
  go_downline();
}

void   Lister::fedit()
{
  strcpy(cur->name, dbuf);
  cur->size = 0;
  cur->mode = 0666;
  edit();
  reread();
}

void   Lister::attrib()
{
  // *INDENT-OFF*        
    if(selfiles==0) 
	::attrib(cur,FSETATTR);
    else 
	attrib2();
  // *INDENT-ON*        
	
	
}

void  Lister::do_simple_dialog(char* head, ManFuncs fnum, int show_text)
{	
  newtextfile(head,this,fnum,show_text);
}

//-------------------------------------------------------------------------
// Switch on/off branch view of the panel
//-------------------------------------------------------------------------
void   Lister::switch_branch_view()
{
    if(is_branch_view_mode())
	reset_branch_view_mode();
    else
	set_branch_view_mode();
    reread();
}

void   Lister::try_to_add_to_bookmark(int cy)
{
  char strtmp[L_MAXPATH];
  if ((cur->mode & S_IFDIR) && strcmp("..", cur->name) != 0)
  {
      strcpy(strtmp, vfs->get_info_for_bookmark());
      add_path_content(strtmp, cur->name, 0);
      bmark->add_toempty_book_by_coord(cy, strtmp, vfs->fstype);
      return;
  }
  else
    bmark->add_toempty_book_by_coord(cy, vfs->get_info_for_bookmark(), vfs->fstype);
}

extern int disable_modifiers;


void   Lister::fast_find(XEvent * ev)
{
  FList *o = dl.next;
  int    kn = 0, kk = 0, l;
  KeySym ks;
  XComposeStatus cs;
  char   sym[8];
  ev->xkey.state ^= Mod1Mask;
  XLookupString(&ev->xkey, sym, 4, &ks, &cs);
  if (ks == XK_BackSpace)
  {
    l = strlen(ff_buf);
    if (l)
    {
      ff_buf[l - 1] = 0;
    }
    showitem(cur, curn);
    show_ff_item(curn, l - 1);
    return;
  }
  sym[1] = 0;
  strcat(ff_buf, sym);
  l = strlen(ff_buf);
  while (o)
    if (strncmp(o->name, ff_buf, l) == 0)
      break;
    else
    {
      o = o->next;
      kn++;
    };
  if (o != NULL)
  {
    if (o != last_ff)
    {
      base = dl.next;
      cur = o;
      lcurn = curn = kn % items_per_page;
      kk = kn - curn;
      setcur(&base, kk);
      showfilelist();
      last_ff = o;
    }
  }
  else
  {
    last_ff = NULL;
    if (l)
      ff_buf[--l] = 0;
  };
  show_ff_item(curn, l);
}

void   Lister::create_ftp_link()
{
  if(vfs->fstype==FTP_TYPE)
  {
    simple_mes(_("Pusher"),_("Can't switch to FTP over another one!"));
    return;
  }
  vfs->ch_curdir();
  new_ftp_prompt(this,FFTPCON);
          //  newtextfile("Connect to Ftp Host", this, 12);
}

void Lister::switch_to_me()
{
  panel = this;
  focus_fl = 1;
  XRaiseWindow(disp, w);
  default_dnd_man->move_to_top(w);
  xncdprintf(("Chdir to %s\n",vfs->curdir));
  vfs->ch_curdir();
  if (curn == -1)
  {
    curn = lcurn;
    if (cur == NULL)
      reread();
    showitem(cur, curn);
  }
  if (cmdl)
  {
    cmdl->save_input();
    cmdl->setpath(curdir);
  }
  senddir(curdir);
  if(cmdl)
  {
    cmdl->restore_input();
    cmdl->expose();
  }
  XFlush(disp);
  xncdprintf(("End of switch_to_me\n"));
}


void Lister::focus_out()
{
    xncdprintf(("Panel::FOCUS_OUT - window %x, object=%x\n",w,this));
    if (curn != -1)
    {
	lcurn = curn;
	curn = -1;
	if (cur != NULL)
	    showitem(cur, lcurn);
    };
    foc = 0;
}

void Lister::focus_in()
{
    panel2->focus_out();
    xncdprintf(("Panel::FOCUS_IN  - window %x, object=%x\n",w,this));
    switch_to_me();
    foc = 1;
}
////////////////////////////////End of file/////////////////////////////////////////



