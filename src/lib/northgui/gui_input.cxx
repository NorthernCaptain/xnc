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
#include "gui_input.h"
///////////////////////////////Input class/////////////////////////////

const unsigned tw = 5;

#define INP_PULDOWN   0

void   Input::init(Window ip)
{
  parent = ip;
  w = XCreateSimpleWindow(disp, parent, x, y, l, 21, 0, 0, lighter_bg_color);
  gcw = XCreateGC(disp, w, 0, NULL);
  XSetFont(disp, gcw, fixfontstr->fid);
  XSetForeground(disp, gcw, cols[col]);
  gl.init(w, ExposureMask | KeyPressMask | FocusChangeMask |
	  ButtonPressMask);
  ty = (21 + fixfontstr->max_bounds.ascent - fixfontstr->max_bounds.descent) / 2;
  ll = XTextWidth(fixfontstr, "MMMMMMMMMM",10)/10;
  hflg = foc = 0;
  dl = 0;
  tl = (l - tw - 1- INP_PULDOWN) / ll;
  firstfl = 1;
}

void   Input::flush()
{
  bl = cp = 0;
  dl = 0;
  buf[0] = 0;
  expose();
}

void   Input::expose()
{
// XClearWindow(disp,w);
    showbuf();
    if (foc)
    {
	XSetWindowBorderWidth(disp, w, 1);
	show_cursor();
    }
    else
    {
	XSetWindowBorderWidth(disp, w, 0);
	prect(w, gcw, 0, 0, l - 1, 20);
    }
#if INP_PULDOWN
  urect(w,gcw,l-INP_PULDOWN,1,INP_PULDOWN-2,18);
#endif
  hflg = 1;
}

void   Input::show()
{
  Gui::show();
  hist=hist_gethistory(hist_id);
  first_hist=1;
  if(default_dnd_man)
  {
    default_dnd_man->add_dnd_object(this,w);
    dndwin->add_type("text/plain");
  }
}


void  Input::hide()
{
  Gui::hide();
  if(default_dnd_man)
    default_dnd_man->del_dnd_win(w);
}

int  Input::dnd_drop(DndData* dd, int, int, DndAction)
{
  int i;
  for(i=0;dd->data[i] && dd->data[i]!='\r' && dd->data[i]!='\n';i++)
    insert(dd->data[i]);
  delete dd;
  return 1;
}

void Input::extract_path_part(char *buf, int pos, char *topath, char *part)
{
        int i=pos-1,j=0;
        *part=0;
        while(i>=0 && buf[i]!='/')
                i--,j++;
        if(j)
        {
                strncpy(part,&buf[i+1],j);
                part[j]=0;
        }
        if(i<0 || *buf!='/')
        {
                getcwd(topath,L_MAXPATH);
                if(i>0)
                {
                        buf[i]=0;
                        if (topath[strlen(topath) - 1] != '/')
                        strcat(topath, "/");
                        strcat(topath, buf);
                        buf[i]='/';
                }
        } else
        {
                strncpy(topath,buf,i+1);
                topath[i+1]=0;
       }
       if(topath[strlen(topath)-1] != '/')
               strcat(topath, "/");
}

void   Input::partial_completion(char *compl1, char *name, int *completed1, int *bool_first1)
{
        int i=0;
        while(compl1[i]==name[i] && name[i]!=0)
                i++;
        if(*bool_first1 && name[i]!=0)
        {
                while(name[i])
                 {
                        compl1[i]=name[i];
                        i++;
                 }
                *bool_first1=NO;
        }
        compl1[i]=0;
        *completed1=i;
}
                
int     Input::do_tab_expansion(char *buf, int pos, int maxl)
{
        DIR *d;
        struct dirent *dr;
        struct stat dstat;
        char path[L_MAXPATH];
        char part[L_MAXPATH];
        char found[L_MAXPATH]="";
        char compl1[L_MAXPATH]="";
        int l,pl=0;
        int another=NO;
        int bool_first=YES;
        int coun=0;
        extract_path_part(buf,pos,path,part);
        l=strlen(part);
        if((d=opendir(path))==NULL)
        {
                XBell(disp,0);
                XFlush(disp);
                return NO;
       }
       while((dr=readdir(d))!=NULL)
       {
               if(strcmp(dr->d_name,".")==0 ||
                   strcmp(dr->d_name,"..")==0)
                           continue;
                if(strncmp(dr->d_name,part,l)==0)
                {
                   if(found[0]==0)
                           strcpy(found,dr->d_name);
                   else
                           another=YES;
                   partial_completion(compl1,dr->d_name, &pl, &bool_first);
                }
                coun++;
       }
       closedir(d);
       if(another)        //Partially completed
       {
                   while(compl1[l]!=0)
                   {
                           insert(compl1[l]);
                           l++;
                   }
          XBell(disp,-50);
          XFlush(disp);
          return NO;
       }
       if(found[0])        //Full completion
       {
                   while(found[l]!=0)
                   {
                           insert(found[l]);
                           l++;
                   }
                   strcat(path,found);
                   stat(path,&dstat);
                   if(S_ISDIR(dstat.st_mode))
                           insert('/');
                   return YES;
       }
       XBell(disp,100);        //Not found :(
       XFlush(disp);
       return NO;
}

void   Input::insert(char ch)
{
  int    i;
  if (firstfl)
    {
      flush();
      firstfl = 0;
    };
  for (i = (int)bl; i >= cp + (int)dl; i--)
    buf[i + 1] = buf[i];
  buf[cp + dl] = ch;
  cp++;
  bl++;
  if (cp >= tl)
    {
      cp = tl - 1;
      dl++;
    }
  showbuf();
}

void   Input::press()
{
  int    i;
  KeySym ks;
  XComposeStatus cs;
  Status status;
  char   sym[4]="";
  gl.LookupString(&ev.xkey, sym, 4, &ks, &cs, &status);
  switch (ks)
      {
      case XK_Return:
        if(bl==0)
            return;
        if(!passwd)
        {
                hist_add_to_top(hist_id,buf);
                hist=hist_gethistory(hist_id);
        }
        if (func != NULL)
          {
            func();
            return;
          };
        if (guiobj)
          guiobj->guifunc(this, 1);
        break;
      case XK_Tab:
        if(bool_expand && (ev.xkey.state & ControlMask)==0)
              do_tab_expansion(buf,cp+dl,bl);
        else        
        if (next)
        {
                if(!passwd)
                {
                        hist_add_to_top(hist_id,buf);
                        hist=hist_gethistory(hist_id);
                }
          guiSetInputFocus(disp, next->w, RevertToNone, CurrentTime);
        }
        break;
      case XK_Execute:
      case XK_Escape:
        if (escfunc != NULL)
          {
            escfunc();
            return;
          };
        if (guiobj)
          guiobj->guifunc(this, 0);
        break;
      case XK_Delete:
        if (cp+dl<bl)
          {
            firstfl = 0;
            for (i = cp + dl; i < bl; i++)
              buf[i] = buf[i + 1];
            bl--;
            showbuf();
          };
        break;
      case XK_BackSpace:
        if (cp != 0 || dl != 0)
          {
            firstfl = 0;
            if (dl != 0)
              dl--;
            else
              cp--;
            for (i = cp + dl; i < bl; i++)
              buf[i] = buf[i + 1];
            bl--;
            showbuf();
          };
        break;
      case XK_Left:
        if (cp != 0 || dl != 0)
          {
            firstfl = 0;
	    hide_cursor();
            cp--;
            if (cp <= 0)
              {
                cp = 0;
                if (dl != 0)
                  {
                    dl--;
                    showbuf();
                  };
              };
	    show_cursor();
          };
        break;
      case XK_Right:
        if (cp + dl != bl)
          {
            firstfl = 0;
	    hide_cursor();
            cp++;
            if (cp >= tl)
              {
                cp = tl - 1;
                dl++;
                showbuf();
              }
	    show_cursor();
          };
        break;
       case XK_u:
       case XK_U:
	 if((ev.xkey.state & ControlMask) !=0)
	   flush();
	 else
	   insert(sym[0]);
	 show_cursor();
	 break;
       case XK_e:
       case XK_E:
	 if((ev.xkey.state & ControlMask) !=0)
	 {
	     goto_end();
            break;
	 } else
	   insert(sym[0]);
	 break;
       case XK_Home:
	  go_home();
	  break;
       case XK_End:
	  goto_end();
	  break;

       case XK_a:
       case XK_A:
	 if((ev.xkey.state & ControlMask) !=0)
	 {
	     go_home();
	     break;
	 } else
	   insert(sym[0]);
	 break;
       case XK_Down:
            if(passwd)            //Skip history on password inputs
                    break;
            if(first_hist)
            {
               hist_addstring(hist_id,buf);
               hist=hist_gethistory(hist_id);
               if(hist==NULL)
                 break;
               strcpy(buf,hist->string);
               cp=dl=0;
               bl=strlen(buf);
               expose();
               first_hist=0;
            } else
            if(hist && hist->next)
            {
                  hist=hist->next;
                  strcpy(buf,hist->string);
                  cp=dl=0;
                  bl=strlen(buf);
                  expose();
            }
            break;
       case XK_Up:
            if(passwd)            //Skip history on password inputs
                    break;
            if(first_hist)
            {
               hist_addstring(hist_id,buf);
               hist=hist_gethistory(hist_id);
               if(hist==NULL)
                 break;
               strcpy(buf,hist->string);
               cp=dl=0;
               bl=strlen(buf);
               expose();
               first_hist=0;
            } else
            if(hist && hist->prev)
            {
                  hist=hist->prev;
                  strcpy(buf,hist->string);
                  cp=dl=0;
                  bl=strlen(buf);
                  expose();
            }
            break;
      default:
	if ((unsigned char)sym[0] >= (unsigned char)' ' && 
	    bl < el 
	    && (ev.xkey.state & ControlMask)==0
	    )
	  insert(sym[0]);
	break;
      }
  show_cursor();
}

void   Input::goto_end()
{
    hide_cursor();
    cp=tl<bl ? tl-1 : bl;
    dl=bl-cp;
    if ((int)dl<0)
	dl = 0;
    showbuf();
    show_cursor();
    firstfl = 0;
}

void   Input::go_home()
{
    hide_cursor();
    cp=dl=0;
    showbuf();
    show_cursor();
    firstfl = 0;
}

void   Input::showbuf()
{
  char tmp[L_MAXPATH];
  int i;
  prect(w, gcw, 0, 0, l - 1, 20);
  XClearArea(disp, w, tw, 1, l - tw - 1 - INP_PULDOWN, 19, 0);
  XSetForeground(disp, gcw, cols[col]);
  if(passwd)
  {
      for(i=0;i< (cp==0 ? bl : cp);i++)
	  tmp[i]='*';
      tmp[i]=0;
      XDrawString(disp, w, gcw, tw, ty, tmp, i);
  } else         
      XDrawString(disp, w, gcw, tw, ty, buf + dl, (bl - dl)<tl ? bl-dl : tl);
}

void   Input::click()
{
  if (ev.xany.window == w)
    switch (ev.type)
      {
      case Expose:
        expose();
        break;
      case ButtonPress:
	guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
	hide_cursor();
	cp = (ev.xbutton.x - tw) / ll;
	if (cp + dl > bl)
	  cp = bl - dl;
	show_cursor();
	firstfl = 0;
        if (ev.xbutton.button != Button1)
	{
	  char *cutbuf;
	  int cl;
	  cutbuf=XFetchBytes(disp,&cl);
	  if(cl>0)
	  {
	    for(int i=0;cutbuf[i]!=0 && cutbuf[i]!='\n' && cutbuf[i]!='\r';i++)
	      insert(cutbuf[i]);
	  }
	}
        break;
      case KeyPress:
        if (foc)
          press();
        break;
      case FocusIn:
	set_focus();
        break;
      case FocusOut:
	unset_focus();
        break;

      };
}



//----------------------------------------------------------------------------
// Hide/show cursor in the widget using current state
//----------------------------------------------------------------------------
void Input::hide_cursor()
{
    XSetForeground(disp, gcw, lighter_bg_color);
    XDrawRectangle(disp, w, gcw, tw + cp * ll, 1, ll, 18);
    showbuf();
}

void Input::show_cursor()
{
  XSetForeground(disp, gcw, cols[col]);
  XDrawRectangle(disp, w, gcw, tw + cp * ll, 1, ll, 18);
}


//----------------------------------------------------------------------------
// Set/unset focus in the widget
//----------------------------------------------------------------------------
void Input::set_focus()
{
  if (foc == 0)
  {
    foc = 1;
    XSetWindowBorderWidth(disp, w, 1);
    focobj = this;
    show_cursor();
    if (cp == 0)
      firstfl = 1;
    gl.setFocus();
  }
}

void Input::unset_focus()
{
  if (foc)
  {
    foc = 0;
    XSetWindowBorderWidth(disp, w, 0);
    focobj = NULL;
    hide_cursor();
    gl.unsetFocus();
  }
}
