/****************************************************************************
*  Copyright (C) 1996-2002 by Leo Khramov
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
#include "xh.h"
#include "gui_menu.h"
#include "key.h"

#include "mgl.xbm"

Cursor menucr = 0;
MenuBar *current_menu_bar=0;

///////////////////////////////Menu Class//////////////////////////////////
int    Menu::max_icon_width=22;
int    Menu::window_border=2;
int    Menu::icon_delta_x=4;
unsigned int Menu::mitemh=22;
unsigned int Menu::action_space=40;
int    Menu::action_delta_x=5;

void   Menu::init(Window ipar)
{
    char action_descr[128];
    unsigned pl;
    int    tw, i;
    if (shflg == 0)
    {
	parent = ipar;
	tw = 0;
	pl = 0;
	for (i = 0; i < max; i++)
        {
	    construct_name(i, _(items[i].name));
	    action_len_x[i]=XTextWidth(fontstr, action_name(i), action_len[i]);
	    tw =XTextWidth(fontstr, item_name(i), tl[i]) +
		action_len_x[i] + Menu::action_space;
	    if (tw > pl)
		pl = tw;
        }
	l = pl + Menu::max_icon_width + Menu::window_border*3+ Menu::action_delta_x;
	ty = mitemh/2 - (fontstr->max_bounds.ascent +
			 fontstr->max_bounds.descent) / 2 +
	    fontstr->max_bounds.ascent;
    }
}

void   Menu::delete_names()
{
    int i;
    if(names) 
    {
	for(i=0;i<max;i++) 
	{
	    if(names[i])
	    {
		delete names[i]; 
		delete action_names[i];
	    }
	}
	delete tl;
	delete names;
	delete action_names;
	delete action_len;
	delete action_len_x;
	names=0;
	action_names=0;
	tl=0;
	action_len=0;
    }
}

void   Menu::new_names()
{
    int i;
    tl=new int[max];
    action_len=new int[max];
    action_len_x=new int[max];
    names=new char*[max];
    action_names=new char*[max];
    for(i=0;i<max;i++)
	names[i]=0;
}

int   Menu::construct_name(int idx, const char* name)
{
    int len=strlen(name);
    char action_descr[128]="";
    names[idx]=new char[len+1];
    strcpy(names[idx], name);
    tl[idx]=len;

    if(items[idx].action!=AEmpty)
    {
	action_key_by_id(action_descr, items[idx].action);
	action_len[idx]=strlen(action_descr);
	action_names[idx]=new char[action_len[idx]+4];
	action_names[idx][0]='[';
	strcpy(action_names[idx]+1, action_descr);
	action_names[idx][action_len[idx]+1]=']';
	action_names[idx][action_len[idx]+2]=0;
	action_len[idx]+=2;
    } else
    {
	action_len[idx]=0;
	action_names[idx]=new char[2];
	action_names[idx][0]=0;
    }
    return tl[idx];
}

void   Menu::show()
{
  if (shflg == 0)
    {
	calculate_xy_by_parent();
	w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, lighter_bg_color);
	gcv.background = lighter_bg_color;
	gcv.font = fontstr->fid;
	gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
	XSelectInput(disp, w,
		     ExposureMask | 
		     OwnerGrabButtonMask |
		     EnterWindowMask | 
		     LeaveWindowMask | 
		     PointerMotionMask | 
		     ButtonReleaseMask | 
		     ButtonPressMask);
	addto_el(this, w);
	if(main_pixmap)
	    XSetWindowBackgroundPixmap(disp, w, main_pixmap);
	XMapRaised(disp, w);
	shflg = 1;
	actfl = 0;
	cur = -1;
	xpm = XCreatePixmapFromBitmapData(disp, w, (char *)mgl_bits, mgl_width, mgl_height, cols[0], keyscol[1],
					  DefaultDepth(disp, DefaultScreen(disp)));
	if (menucr == 0)
	    menucr = XCreateFontCursor(disp, XC_arrow);
	XDefineCursor(disp, w, menucr);
	xncdprintf(("Menu::menu_to_notify set to %x\n", menu_to_notify));
       	grab_now();
    }
}

void   Menu::hide()
{
  ungrab_now();
  if (shflg)
  {
      XUnmapWindow(disp, w);
      XDestroyWindow(disp, w);
      delfrom_el(this);
      delfrom_exp(this);
      XFreeGC(disp, gcw);
      XFreePixmap(disp, xpm);
      shflg = 0;
      if(menu_to_notify)
	  menu_to_notify->notify(MENU_HIDE);
  }
}

void   Menu::calculate_xy_by_parent()
{
    Window dummywin;
    int dummy_int;
    unsigned int dummy_uint, par_width, par_height;

    XGetGeometry(disp, parent, &dummywin, &dummy_int, &dummy_int,
		 &par_width, &par_height, &dummy_uint, &dummy_uint);

    if(x<0)
	x=par_width + x - l;
    if(x<0)
	x=0;

    if(y<0)
	y=par_height + y - h;
    if(y<0)
	y=0;

    if(x + l >= par_width)
	x=par_width-l-1;
    if(y + h >= par_height)
	y=par_height-h-1;
}

void   Menu::showitem(int i)
{
    char  *name = item_name(i);
    int    iy = mitemh * i + Menu::window_border;
    XClearArea(disp, w, Menu::window_border, iy, 
	       l - Menu::window_border*2, mitemh - 1, 0);
    XSetForeground(disp, gcw, normal_bg_color);
    XFillRectangle(disp, w, gcw, Menu::window_border, iy, 
		   Menu::max_icon_width - 1, mitemh); 
    if (items[i].menu_state == menu_function)
    {
	if(shadow)
	{
	    XSetForeground(disp, gcw, cols[0]);
	    XDrawString(disp, w, gcw, Menu::max_icon_width+1+Menu::window_border,
			iy + ty + 1, name, tl[i]);
	    if(action_len[i])
	    XDrawString(disp, w, gcw, 1 + l - Menu::action_delta_x - action_len_x[i], 
			iy + ty + 1, action_name(i), action_len[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		    iy + ty, name, tl[i]);
	if(action_len[i])
	XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		    iy + ty, action_name(i), action_len[i]);
    }
    else
    {
	if (items[i].menu_state == menu_sw_set || 
	    items[i].menu_state == menu_ch_set)
	    XCopyArea(disp, xpm, w, gcw, 0, 0, mgl_width, mgl_height, 
		      Menu::window_border*2+3, 
		      iy + (mitemh - mgl_height)/2);
	if(shadow)
	{
	    XSetForeground(disp, gcw, cols[0]);
	    XDrawString(disp, w, gcw, Menu::max_icon_width+1+Menu::window_border, 
			iy + ty + 1, name, tl[i]);
	    if(action_len[i])
	    XDrawString(disp, w, gcw, 1 + l - Menu::action_delta_x - action_len_x[i], 
			iy + ty + 1, action_name(i), action_len[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		    iy + ty, name, tl[i]);
	if(action_len[i])
	XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		    iy + ty, action_name(i), action_len[i]);
    }
    
    if (name[tl[i] - 1] == ' ')
    {
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawLine(disp, w, gcw, Menu::window_border+1, iy + mitemh, 
		      l - Menu::window_border*2+1, iy + mitemh);
	}
	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw, Menu::window_border, iy + mitemh -1, 
		  l - Menu::window_border*2, iy + mitemh -1);
    }

    default_iconman->display_icon_from_set(w, 
					   Menu::icon_delta_x, iy + mitemh/2,
					   menu_iconset, items[i].icon_type);
}

void Menu::set_menu(MenuItem *ii, int maxi)
{
    delete_names();
    max=maxi;
    h=max*mitemh+Menu::window_border*2;
    items=ii;
    shflg=0;
    new_names();
    for(int i=0;i<max;i++)
	tl[i]=construct_name(i, _(items[i].name));
}

void   Menu::switches(int ncur)
{
  int    i = 0;
  char*  name;
  if(ncur==-1)
    ncur=cur;
  xncdprintf(("switches called for ncur=%d sym=%d\n",ncur,items[ncur].menu_state));
  switch (items[ncur].menu_state)
    {
    case menu_sw_unset:
	items[ncur].menu_state = menu_sw_set;
	break;
    case menu_sw_set:
	items[ncur].menu_state = menu_sw_unset;
	break;
    case menu_ch_unset:
	name=item_name(ncur);
	items[ncur].menu_state = menu_ch_set;
	if (name[tl[ncur] - 1] != ' ')
        {
	    i = ncur + 1;
	    while (i < max)
            {
		if (items[i].menu_state == menu_ch_set)
		    items[i].menu_state = menu_ch_unset;
		if (item_name(i)[tl[i] - 1] == ' ')
		    break;
		else
		    i++;
            };
        }
	i = ncur - 1;
	while (i >= 0)
	    if (item_name(i)[tl[i] - 1] == ' ')
		break;
	    else
	    {
		if (items[i].menu_state == menu_ch_set)
		    items[i].menu_state = menu_ch_unset;
		i--;
	    };
	break;
    }
}

void   Menu::expose()
{
  int    i;
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  for (i = 0; i < max; i++)
    showitem(i);
  XRaiseWindow(disp,w);
}

void   Menu::select(int i)
{
    char  *name = item_name(i);
    int    iy = mitemh * i + Menu::window_border;
    XSetForeground(disp, gcw, cursor_color);
    XFillRectangle(disp, w, gcw, Menu::window_border, iy, 
		   l - Menu::window_border*2, mitemh -1);
    XSetForeground(disp, gcw, dark_cursor_color);
    XDrawRectangle(disp, w, gcw, Menu::window_border, iy, 
		   l - Menu::window_border*2-1, mitemh-2);
    if (items[i].menu_state == menu_function)
    {
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, Menu::max_icon_width+1+Menu::window_border, 
			iy + ty + 1, name, tl[i]);
	    if(action_len[i])
	    XDrawString(disp, w, gcw, 1 + l - Menu::action_delta_x - action_len_x[i], 
			iy + ty + 1, action_name(i), action_len[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		    iy + ty, name, tl[i]);
	if(action_len[i])
	    XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
			iy + ty, action_name(i), action_len[i]);
    }
    else
    {
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, Menu::max_icon_width+1+Menu::window_border, 
			iy + ty + 1, name, tl[i]);
	    if(action_len[i])
	    XDrawString(disp, w, gcw, 1 + l - Menu::action_delta_x - action_len_x[i], 
			iy + ty + 1, action_name(i), action_len[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, Menu::max_icon_width+Menu::window_border, 
		    iy + ty, name, tl[i]);
	if(action_len[i])
	XDrawString(disp, w, gcw, l - Menu::action_delta_x - action_len_x[i], 
		    iy + ty, action_name(i), action_len[i]);
    }
    default_iconman->display_icon_from_set_with_shadow(w, 
						       Menu::icon_delta_x, iy + mitemh/2,
						       menu_iconset, items[i].icon_type);
    xncdprintf(("Menu::select - selected %d item on window %x\n",i,w));
}

void   Menu::click()
{
  int    i, mx,my;
  if (ev.xany.window == w)
  {
    switch (ev.type)
    {
    case MotionNotify:
      mx = ev.xmotion.x;
      my = ev.xmotion.y;
      if(mx<0 || my<0 || mx>l || my>h)
      {
	if(actfl)
	{
	  showitem(cur);
	  actfl=0;
	  cur=-1;
	}
	break;
      }

      i = (ev.xmotion.y - Menu::window_border) / (int)mitemh;
      if (i < 0 || i>=max)
	break;
      if (cur != i)
      {
	if(cur>=0)
	  showitem(cur);
	select(i);
	cur = i;
	actfl=1;
      };
      break;
    case Expose:
      xncdprintf(("Menu::click - expose event\n"));
      expose();
      break;
    case ButtonRelease:
      mx=ev.xbutton.x;
      my=ev.xbutton.y;
      xncdprintf(("Menu::click - mouse button release\n"));
      if(mx<0 || my<0 || mx>l || my>h)
	break;
      execute();
      break;

    case ButtonPress:
      mx=ev.xbutton.x;
      my=ev.xbutton.y;
      xncdprintf(("Menu::click - mouse button pressed\n"));
      if(mx<0 || my<0 || mx>l || my>h)
      {
	prflg = 0;
	actfl = 0;
	hide();
	break;
      }
      execute();
      break;

    case EnterNotify:
      actfl=0;
      cur=-1;
      mx = ev.xcrossing.x;
      my = ev.xcrossing.y;
      if(mx<0 || my<0 || mx>l || my>h)
	break;
      mx = (my - Menu::window_border) / (int)mitemh;
      xncdprintf(("Menu: enter_notify with x,y - %d, %d mx=%d\n",
		  ev.xcrossing.x,ev.xcrossing.y,mx));
      if (actfl == 0)
      {
	if (mx < 0 || mx>=max)
	  break;
	cur = mx;
	select(cur);
	actfl = 1;
      };
      break;
    };
  }
}

void Menu::execute()
{
  if (actfl == 1 && ev.xbutton.button == Button1)
  {
    showitem(cur);
    hide();
    switches();
    if (items[cur].func)
      items[cur].func();
    if(items[cur].metacall)
      items[cur].metacall->call(MetaCallData(this));
  }
}

void Menu::grab_now()
{
  XGrabPointer(disp,w,False,
	       EnterWindowMask | 
	       LeaveWindowMask | 
	       PointerMotionMask | 
	       ButtonReleaseMask | 
	       ButtonPressMask,
	       GrabModeAsync,
	       GrabModeAsync,
	       None,
	       None,
	       CurrentTime);
}

void Menu::ungrab_now()
{
  XUngrabPointer(disp,CurrentTime);
}


///////////////////////////////MenuBar Class/////////////////////////////

void   MenuBar::init(Window ipar)
{
    Sprite *fillskin;
    Window wtemp;
    GC fillgc;
    unsigned u, pl, ph;
    int    tw, itemp, i;
    GEOM_TBL *tbl;
    parent = ipar;
    menuh=20;
    XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
    tw = 0;
    l = pl-deltal;
    geometry_by_iname();
    tbl=geom_get_data_by_iname(guitype, in_name);
    if(tbl)
    {
	skin=(Sprite*)tbl->data1;
	skinpr=(Sprite*)tbl->data2;
	fillskin=(Sprite*)tbl->data3;
	pskin=(Sprite*)tbl->data4;
    } else
    {
	skin=NULL;
	skinpr=NULL;
	fillskin=NULL;
	pskin=NULL;
    }
    stx = 0;
    enx = l;
    calculate_sizes();

    if(skin)
    {
	fill_l=(unsigned)fillskin->l;
	fill_h=(unsigned)fillskin->h;
	fillpix=XCreatePixmap(disp, DefaultRootWindow(disp), fill_l, fill_h, 
			      DefaultDepth(disp, DefaultScreen(disp)));
	fillgc=XCreateGC(disp, fillpix, 0, NULL);
	XCopyArea(disp, fillskin->im->skin, fillpix, fillgc,
		  fillskin->x, fillskin->y, fill_l, fill_h, 0,0);
	XFreeGC(disp, fillgc);
    }
    w = XCreateSimpleWindow(disp, parent, x, y, l, menuh, 0, 0, normal_bg_color);
    gcv.background = normal_bg_color;
    gcv.font = fontstr->fid;
    gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
    XSelectInput(disp, w,
		 ButtonPressMask | 
		 ExposureMask | 
		 ButtonReleaseMask | 
		 LeaveWindowMask |
		 EnterWindowMask | 
		 PointerMotionMask | 
		 OwnerGrabButtonMask);
    ty = menuh/2 - 
	(fontstr->max_bounds.ascent + 
	 fontstr->max_bounds.descent) / 2 + fontstr->max_bounds.ascent;
    if (menucr == 0)
	menucr = XCreateFontCursor(disp, XC_arrow);
    XDefineCursor(disp, w, menucr);
    if(skin)
	XSetWindowBackgroundPixmap(disp, w, fillpix);
    cur = 0;
    highlight_idx=-1;
}

int MenuBar::geometry_by_iname()
{
    int ix=0,iy=0,il=0,ih=0;
    if(in_name[0]==0)
	return NO;
    geom_translate_by_iname(guitype,in_name, &ix, &iy, &il, &ih);
    if(ix!=DONT_CARE)
	x=ix;
    if(iy!=DONT_CARE)
	y=iy;
    if(il!=DONT_CARE)
	l=il;
    if(ih!=DONT_CARE)
	menuh=ih;
    return YES;
}

void   MenuBar::reconfigure(int ix, int iy, int pl, int ph)
{
    l = pl;
    stx = 0;
    enx = l;
    x = ix;
    y = iy;
    calculate_sizes();
    XMoveResizeWindow(disp, w, x, y, l, menuh);
    cur = 0;
}

void   MenuBar::calculate_sizes()
{
    int    tw, i;
    tw = 0;
    if(skin)
    {
	for (i = 0; i < max; i++)
	{
            dx[i]=skin[i].tox;
            tl[i]=0;
            dl[i]=skin[i].l;
            if(dx[i]==-1)
		dx[i]=l-skin[i].l;
	}
	stx=enx=0;
    } else
    {  
	for (i = 0; i < max; i++)
	{
	    dx[i] = tw;
	    tl[i] = strlen(_(items[i].name));
	    if (_(items[i].name)[0] == '-')
	    {
		tl[i]--;
		if(items[i].icon_type!=no_icon)
		    dl[i] = XTextWidth(fontstr, _(items[i].name) + 1, tl[i]) + 36;
		else
		    dl[i] = XTextWidth(fontstr, _(items[i].name) + 1, tl[i]) + 18;
		dx[i] = l - dl[i];
		enx = dx[i];
	    }
	    else
	    {
		if(items[i].icon_type!=no_icon)
		    dl[i] = XTextWidth(fontstr, _(items[i].name), tl[i]) + 36;
		else
		    dl[i] = XTextWidth(fontstr, _(items[i].name), tl[i]) + 18;
		tw = dx[i] + dl[i];
		if (tw > stx)
		    stx = tw;
	    }
	}
    }
}

void   MenuBar::showitem(int i)
{
    int    ix = dx[i], il = dl[i];
    char  *name;
    if(i<0)
	return;
    if(skin)
    {
	XCopyArea(disp, skin[i].im->skin, w, gcw, skin[i].x, skin[i].y,
		  (unsigned)skin[i].l, (unsigned)skin[i].h, dx[i], 0);
	if(last_idx==i && pskin)
	{
	    if(skin[i].tox<0)
		XCopyArea(disp, pskin->im->skin, w, gcw, pskin->x, pskin->y,
			  (unsigned)pskin->l, (unsigned)pskin->h, dx[i]+dl[i]-2-pskin->l, menuh-pskin->h+1);
	    else
		XCopyArea(disp, pskin->im->skin, w, gcw, pskin->x, pskin->y,
			  (unsigned)pskin->l, (unsigned)pskin->h, dx[i]+2, menuh-pskin->h+1);
	}
    }
    else
    {
	if (_(items[i].name)[0] == '-')
	    name = _(items[i].name) + 1;
	else
	    name = _(items[i].name);
	XClearArea(disp, w, ix, 1, dl[i], menuh - 2, 0);
	if(i==highlight_idx) //Current item, lets select it
	{
	    XSetForeground(disp, gcw, lighter_bg_color);
	    XFillRectangle(disp, w, gcw, ix, 1, dl[i], menuh - 2);
	    XSetForeground(disp, gcw, light_bg_color);
	    XDrawRectangle(disp, w, gcw, ix, 1, dl[i]-1, menuh - 3);
	}
	if(items[i].icon_type!=no_icon)
	{
	    default_iconman->display_icon_from_set(w, 
						   ix+6, menuh/2,
						   menu_iconset, 
						   items[i].icon_type);
	    ix+=18;
	    
	}
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, ix + 8, ty + 1, name, tl[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, ix + 7, ty, name, tl[i]);
    }
}

void   MenuBar::pressitem(int i)
{
    int    ix = dx[i], il = dl[i], kx;
    char  *name;
    if(skinpr)
    {
	XCopyArea(disp, skinpr[i].im->skin, w, gcw, skinpr[i].x, skinpr[i].y,
                  (unsigned)skinpr[i].l, (unsigned)skinpr[i].h, dx[i], 0);
	if (items[i].o != NULL)
	{
	    kx = x + dx[i] + 4;
	    items[i].o->setpos(kx, y + menuh);
	    items[i].o->init(parent);
	    if(kx+items[i].o->l > l+x-deltal)
	    {
		if(pskin)
		    XCopyArea(disp, pskin->im->skin, w, gcw, pskin->x, pskin->y,
			      (unsigned)pskin->l, (unsigned)pskin->h, dx[i]+dl[i]-2-pskin->l, menuh-pskin->h+1);
		kx=l+x-items[i].o->l - 4;
		items[i].o->right_justified=YES;
	    } else
	    {
		if(pskin)
		    XCopyArea(disp, pskin->im->skin, w, gcw, pskin->x, pskin->y,
			      (unsigned)pskin->l, (unsigned)pskin->h, dx[i]+2, menuh-pskin->h+1);
		items[i].o->right_justified=NO;
	    }
	    items[i].o->menu_to_notify=this;
	    last_idx=i;
	    items[i].o->setpos(kx, y + menuh);
	    items[i].o->show();
	}
    }
    else
    {
	name = _(items[i].name);
	if (name[0] == '-')
	    name++;

	XSetForeground(disp, gcw, darker_bg_color);
	XFillRectangle(disp, w, gcw, ix, 1, dl[i], menuh - 2);
	XSetForeground(disp, gcw, dark_bg_color);
	XDrawRectangle(disp, w, gcw, ix, 1, dl[i]-1, menuh - 3);
	
	if(shadow)
	{
	    XSetForeground(disp, gcw, shadow_color);
	    XDrawString(disp, w, gcw, ix + 9, ty + 2, name, tl[i]);
	}
	XSetForeground(disp, gcw, cols[items[i].col]);
	XDrawString(disp, w, gcw, ix + 7, ty, name, tl[i]);

	current_menu_bar = this;
	if (items[i].o != NULL)
	{
	    kx = x + dx[i];
	    items[i].o->menu_to_notify=this;
	    items[i].o->setpos(kx, y + menuh);
	    items[i].o->init(parent);
	    if(kx+items[i].o->l > l+x)
		kx=l-items[i].o->l;
	    items[i].o->setpos(kx, y + menuh);
	    items[i].o->show();
	}
    }
}

void   MenuBar::notify(int code)
{
    int i;
    if(code==MENU_HIDE)
    {
	i=last_idx;
	last_idx=-1;
	if(i>=0)
	    showitem(i);
    }
}

void   MenuBar::expose()
{
    Window wtemp;
    unsigned u, pl, ph;
    int    itemp, i;
    int    ix = 0, il = l;
    XGetGeometry(disp, parent, &wtemp, &itemp, &itemp, &pl, &ph, &u, &u);
    for (i = 0; i < max; i++)
	showitem(i);
    l = pl;
    XSetForeground(disp, gcw, light_bg_color);
    XDrawLine(disp, w, gcw, ix, 0, ix + il - 1, 0);
    XDrawLine(disp, w, gcw, ix, 0, ix, menuh - 1);
    XSetForeground(disp, gcw, dark_bg_color);
    XDrawLine(disp, w, gcw, ix, menuh - 1, ix + il - 1, menuh - 1);
    XDrawLine(disp, w, gcw, ix + il - 1, 0, ix + il - 1, menuh - 1);
}

void   MenuBar::click()
{
    int    i, mx, prev_idx;
    if (ev.xany.window == w)
    {
	switch (ev.type)
        {
        case MotionNotify:
	    xncdprintf(("MenuBar::click - motion event\n"));
	    if (ev.xmotion.y < menuh)
            {
		mx = ev.xmotion.x;
		for (i = 0; i < max; i++)
		    if (mx > dx[i] && mx < dl[i] + dx[i])
			break;
		if (i < max && highlight_idx != i)
                {
		    if(highlight_idx>=0)
		    {
			prev_idx=highlight_idx;
			highlight_idx=-1;
			showitem(prev_idx);
		    }
		    highlight_idx=i;
		    showitem(i);
		    xncdprintf(("MenuBar:: moving over %d item\n", i));
                };
            };
	    break;

        case Expose:
	    xncdprintf(("MenuBar::click - expose event\n"));
	    expose();
	    break;
        case ButtonPress:
	    xncdprintf(("MenuBar::click - button press event\n"));
	    if (ev.xbutton.button == Button1)
            {
		mx = ev.xbutton.x;
		for (i = 0; i < max; i++)
		    if (mx > dx[i] && mx < dl[i] + dx[i])
			break;
		if (i < max)
                {
		    for (cur = 0; cur < max; cur++)
			if (items[cur].o)
			    items[cur].o->hide();
		    cur = i;
		    pressitem(cur);
		    prflg = 1;
                }
            }
	    break;
        case ButtonRelease:
	    xncdprintf(("MenuBar::click - button release event\n"));
	    if (prflg == 1 && ev.xbutton.button == Button1)
            {
		prflg = 0;
		highlight_idx=-1;
		showitem(cur);
		if (items[cur].func)
		    items[cur].func();
            }
	    break;
        case LeaveNotify:
	    xncdprintf(("MenuBar::click - leave event\n"));
	    if (prflg == 1 && ev.xcrossing.state == Button1Mask)
            {
		prflg = 0;
		showitem(cur);
            }
	    if(highlight_idx>=0)
	    {
		prev_idx=highlight_idx;
		highlight_idx=-1;
		showitem(prev_idx);
	    }
	    break;
        case EnterNotify:
	    xncdprintf(("MenuBar::click - enter event\n"));
	    mx = ev.xcrossing.x;
	    for (i = 0; i < max; i++)
		if (mx > dx[i] && mx < dl[i] + dx[i])
		    break;
	    
	    if (i < max)
	    {
		highlight_idx=i;
		showitem(i);
	    }
	    break;
        };
    }
}


//---------------------------------- End of file ------------------------------------
