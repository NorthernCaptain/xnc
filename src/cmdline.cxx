/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
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

#include "panel.h"
#include "c_externs.h"
#include "ftpcfg.h"
#include "history.h"
#include "pluginman.h"

#include "history.xbm"

///////////////////////////////Cmdline class/////////////////////////////

void   Cmdline::show()
{
  if(main_pixmap)
    XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  Gui::show();
  if(default_dnd_man)
  {
    default_dnd_man->add_dnd_object(this,w);
    dndwin->add_type("text/uri-list");
  }
  load_history();
}

void   Cmdline::setpath(char *path)
{
  strcpy(name, host);
  strcat(name, path);
  if (getuid() != 0)
    strcat(name, "$");
  else
    strcat(name, "#");
  tl = strlen(name);
  if (hflg)
    expose();
}

void   Cmdline::init(Window ip)
{
  parent = ip;
  bbp = bp = 0;
  h=20;
  for (int i = 0; i < max_cmd_history; i++)
  {
      bak[i][0] = 0;
  }
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, cmdline_bg_color);
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fixfontstr->fid);
  XSetForeground(disp, gcw, cols[col]);

  gl.init(w, ExposureMask | ButtonPressMask);
  tl = strlen(name);
  ty = h/2 - (fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent)/2 +
      fixfontstr->max_bounds.ascent;
  ll = XTextWidth(fixfontstr, "MMMMMMMMMM",10)/10;
  hflg = 0;

  history_pixmap = XCreatePixmapFromBitmapData(disp, w, (char *)history_bits, 
					       history_width, history_height, 
					       cols[col], cmdline_bg_color,
					       DefaultDepth(disp, DefaultScreen(disp)));

  delta_x=19;
  text_border=3;
  text_x=delta_x+text_border;
}


void   clear_cmdbuf(int bl)
{
  char chr[L_MAXPATH];
  for(int n=0;n<bl;n++)
    chr[n]='\b';
  tt_write((unsigned char*)chr, bl);
}

//Looking for 'cd' command in command line and process it
//return 1 if 'cd' command found, and 0 otherwise 
int   Cmdline::look_for_cd()
{
    int    i, j;
    i = 0;
    char   tmp[L_MAXPATH];
    while (buf[i])
    {
	if (buf[i] == 'c' && buf[i + 1] == 'd') // 'cd' command
        {
	    if (i > 0 && buf[i - 1] != ';' && buf[i - 1] != ' ')
	    {
		i++;
		continue;
	    };
	    if (buf[i + 2] == 0)
	    {
		panel->change_dir(getenv("HOME"));
		if(!panel->vfs->pass_cd_to_terminal())
		    clear_cmdbuf(bl);
		return 1;
	    }
	    else 
		if (buf[i + 2] == ' ')
		{
		    j = 0;
		    i += 2;
		    while (buf[i] == ' ')
			i++;
		    if(strncmp(&buf[i],"ftp://",6)==0)  //make cd to FTP
		    {
			char ftpsite[L_MAXPATH];
			int  j=0;
			clear_cmdbuf(bl);
			i+=6; //Skipping 'ftp://'
			while(buf[i] && buf[i]!=' ' && buf[i]!=';')
			    ftpsite[j++]=buf[i++];
			ftpsite[j]=0;
			xncdprintf(("Got chdir to FTP site: [%s]\n",ftpsite));
			if(ftprec.init(ftpsite))
			    panel->funcs(FFTPCON);
			else
			    newtextfile(_("Enter your password"),panel,FFTPCONPWD,0);
			return 1;

		    } else
			if (buf[i] == 0)
			{
			    panel->change_dir(getenv("HOME"));
			    if(!panel->vfs->pass_cd_to_terminal())
				clear_cmdbuf(bl);
			    return 1;
			}
			else
			{
			    while (buf[i + j] != ';' && buf[i + j] != 0)
			    {
				tmp[j] = buf[i + j];
				j++;
			    };
			    tmp[j] = 0;
			    while(j>0)
			    {
				j--;
				if(tmp[j]!=' ')
				{
				    j++;
				    break;
				}
				tmp[j]=0;
			    }
			    if (j == 0)
				panel->change_dir(getenv("HOME"));
			    else
				panel->change_dir(tmp);
			    if(!panel->vfs->pass_cd_to_terminal())
				clear_cmdbuf(bl);
			    return 1;
			}
		}
        }
	i++;
    }
    return 0; //CD command not found
}

//Flushing buffer, and expose empty command line
//Return result from look_for_cd()
int   Cmdline::flush()
{
    int ret;
    ret=look_for_cd();
    if (bl != 0 && !ret)
	backup();
    bl = cp = base = 0;
    buf[0] = 0;
    expose();
    return ret;
}

void   Cmdline::backup()
{
    add_string_to_backup(buf);
    hist_addstring(in_name,buf);
}

void   Cmdline::load_history()
{
    int count=0;
    History *history=hist_gethistory(in_name);
    while(history && count<max_cmd_history)
    {
	add_string_to_backup(history->string);
	history=history->next;
	count++;
    }
}

void   Cmdline::add_string_to_backup(char* str)
{
    int i;
    xncdprintf(("Cmdline::add_string_to_backup: %s\n",str));
    for(i=0;i<max_cmd_history;i++)
	if(strcmp(bak[i],str)==0)
	    return;
    bp++;
    if (bp > max_cmd_history-1)
	bp = 0;
    bbp = bp;
    strcpy(bak[bp], str);
    bbp++;
    if (bbp > max_cmd_history-1)
	bbp = 0;
}

void   Cmdline::reconfigure(int ix, int iy, int il, int ih)
{
    x = ix;
    y = iy;
    l = il;
    //    h = ih;
    ty = h/2 - (fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent)/2 +
	fixfontstr->max_bounds.ascent;
    XMoveWindow(disp, w, x, y);
    XResizeWindow(disp, w, l, h);
    expose();
}

void   Cmdline::hide()
{
    XUnmapWindow(disp, w);
    foc = 0;
    hflg = 0;
    delfrom_exp(this);
    if(default_dnd_man)
	default_dnd_man->del_dnd_win(w);
}

int  Cmdline::dnd_drop(DndData* dd, int, int, DndAction)
{
  int i=0;
  if(!strncmp(dd->data,"file:",5))
      i=5;
  for(;dd->data[i] && dd->data[i]!='\r' && dd->data[i]!='\n';i++)
      insert(dd->data[i]);
  delete dd;
  return 1;
}

void   Cmdline::expose()
{
    XClearWindow(disp, w);
    prect(w, gcw, delta_x, 0, l - delta_x - 1, h-1);
    default_iconman->display_icon_from_set(w, 
					   2, h/2,
					   menu_iconset, help_icon);
    
    //    XCopyArea(disp, history_pixmap, w, gcw, 0, 0, history_width, history_height,
    //	      delta_x/2 - history_width/2, h/2 - history_height/2);
    XSetForeground(disp, gcw, cols[col]);
    XDrawString(disp, w, gcw, text_x, ty, name, tl);
    tw = XTextWidth(fixfontstr, name, tl) + text_border;
    cmd_text_x=text_x+tw;
    visl = (l - cmd_text_x - text_border*2) / ll - 1;

    if(bl>visl)
    {
	cp+=base;
	base=bl-visl-1;
	cp-=base;
    }
    showbuf();
    hflg = 1;
}

void   Cmdline::save_input(StoreBuffer *pbuf)
{
   int i;
   StoreBuffer *ptr = &store_buf;
   if(pbuf)
       ptr=pbuf;
   ptr->store[0]=0;
   if(bl)
   {
       strncpy(ptr->store, buf, bl);
       ptr->store_cp=cp; ptr->store_base=base;
       for(i=0;i<bl;i++)
	   tt_printf("%c", 8);
       bl=cp=base=0;
   }
}

void   Cmdline::restore_input(StoreBuffer *pbuf)
{
   StoreBuffer *ptr = &store_buf;
   if(pbuf)
       ptr=pbuf;
   if(ptr->store[0])
     {
	 strcpy(buf, ptr->store);
	 bl=strlen(buf);
	 cp=ptr->store_cp;
	 base=ptr->store_base;
	 tt_write((unsigned char*)buf, bl);
     }
}

void   Cmdline::insert(char ch)
{
  int    i;
  for (i = (int)bl; i >= cp+base; i--)
    buf[i + 1] = buf[i];
  buf[cp+base] = ch;
  cp++;
  if(cp>visl)
  {
      cp--;
      base++;
  }
  bl++;
  xncdprintf(("Cmdline::insert - %d/%d %d/%d\n",base,cp,visl,el));
  showbuf();
}

void   Cmdline::press(int k)
{
  int    i;
  KeySym ks;
  XComposeStatus cs;
  Status st;
  char   sym[4];
  gl.LookupString(&ev.xkey, sym, 4, &ks, &cs, &st);
  if( (unsigned char)sym[0] >= (unsigned char)' ' && 
      bl < el && (ev.xkey.state & ControlMask)==0)
      insert(sym[0]);
  else
      switch (k)
      {
      case XK_BackSpace:
      case XK_Delete:
	  delete_one_symbol();
	  break;
      case XK_Left:
	  move_cursor_left();
	  break;
      case XK_Right:
	  move_cursor_right();
	  break;

      case XK_Up:
	  history_up_line();
	  break;

      case XK_Down:
	  history_down_line();
	  break;

      case XK_u:
      case XK_U:
	  flush();
	  break;

      }
}

void Cmdline::show_history_menu()
{
    int i, lines=0;
    for(i=0;i<max_cmd_history;i++)
    {
	if(bak[i][0]==0)
	    continue;
	xncdprintf(("Cmdline::show_history_menu [%s]\n",bak[i]));
	history_callers[lines].init(this,&Cmdline::menu_choose_history,i);
	history_menu_items[lines].init(bak[i],menu_text_color_idx,
				       0,0,&history_callers[lines]);
	lines++;
    }
    if(lines==0)
	return;
    if(!history_menu)
	history_menu=guiplugin->new_Menu(history_menu_items, lines);
    else
	history_menu->set_menu(history_menu_items, lines);
    history_menu->setpos(x,-(Mainh - y));
    history_menu->init(Main);
    history_menu->show();
}

int  Cmdline::menu_choose_history(int idx, MetaCallData)
{
    int i;
    for (i = 0; i < bl; i++)
	tt_printf("%c", 8);

    strcpy(buf, bak[idx]);
    bl = cp = strlen(buf);
    if(bl>visl)
	base=bl-visl-1;
    else
	base=0;
    cp=bl-base;
    tt_write((unsigned char*)buf, bl);
    showbuf();
    return 1;
}

void Cmdline::move_cursor_right()
{
    if (cp+base != bl)
    {
	XSetForeground(disp, gcw, cmdline_bg_color);
	XDrawRectangle(disp, w, gcw, cmd_text_x + cp * ll, 1, ll, h-3);
	cp++;
	if(cp>visl)
	{
	    cp--;
	    base++;
	    showbuf();
	} else
	{
	    XSetForeground(disp, gcw, cols[col]);
	    XDrawRectangle(disp, w, gcw, cmd_text_x + cp * ll, 1, ll, h-3);
	}
    } 
}

void Cmdline::move_cursor_left()
{
    if (cp != 0)
    {
	XSetForeground(disp, gcw, cmdline_bg_color);
	XDrawRectangle(disp, w, gcw, cmd_text_x + cp * ll, 1, ll, h-3);
	cp--;
	XSetForeground(disp, gcw, cols[col]);
	XDrawRectangle(disp, w, gcw, cmd_text_x + cp * ll, 1, ll, h-3);
    } else
	if(base!=0)
	{
	    base--;
	    showbuf();
	}    
}

void Cmdline::history_down_line()
{
    int i;
    for (i = 0; i < bl; i++)
	tt_printf("%c", 8);
    bbp++;
    if (bbp > max_cmd_history-1)
	bbp = 0;
    strcpy(buf, bak[bbp]);
    bl = cp = strlen(buf);
    if(bl>visl)
	base=bl-visl-1;
    else
	base=0;
    cp=bl-base;
    tt_write((unsigned char*)buf, bl);
    showbuf();
}

void Cmdline::history_up_line()
{
    int i;
    for (i = 0; i < bl; i++)
	tt_printf("%c", 8);
    bbp--;
    if (bbp < 0)
	bbp = max_cmd_history-1;
    strcpy(buf, bak[bbp]);
    bl = strlen(buf);
    tt_write((unsigned char*)buf, bl);
    if(bl>visl)
	base=bl-visl-1;
    else
	base=0;
    cp=bl-base;
    showbuf();
}

void Cmdline::delete_one_symbol()
{
    int i;
    if(base+cp==0)
	return;

    if(base)
	base--;
    else
	cp--;

    for (i = cp+base; i < bl; i++)
	buf[i] = buf[i + 1];
    bl--;
    showbuf();
}

void   Cmdline::showbuf()
{
  XClearArea(disp, w, cmd_text_x, 1, l - cmd_text_x - 1, h-2, 0);
  XSetForeground(disp, gcw, cols[col]);
  XDrawString(disp, w, gcw, cmd_text_x, ty, buf+base, bl-base);
  XDrawRectangle(disp, w, gcw, cmd_text_x + cp * ll, 1, ll, h-3);
}

void   Cmdline::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();

  if (ev.xany.window == w && ev.type == ButtonPress)
  {
      if(ev.xbutton.button != Button1)
      {
	  char *cutbuf;
	  int cl;
	  cutbuf=XFetchBytes(disp,&cl);
	  if(cl>0)
	  {
	      for(int i=0;cutbuf[i]!=0 && cutbuf[i]!='\n' && cutbuf[i]!='\r';i++)
		  insert(cutbuf[i]);
	  }
      } else
	  if(ev.xbutton.x<delta_x) //we are in history part
	      show_history_menu();
  }
}
