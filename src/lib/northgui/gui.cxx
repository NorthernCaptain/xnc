/****************************************************************************
*  Copyright (C) 1996-99 by Leo Khramov
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
#include "gui.h"

Gui   *focobj = NULL;
//MenuBar *mb;
char   host[80];

Pixmap main_pixmap=0;

int shadow=YES;        //Use shadow text? YES

int gui_options=0;    //THICK_BORDER for thick buttons

static int gui_x_error=0;

int gui_dummy_x_error_handler(Display*, XErrorEvent*)
{
    char *ptr=0;
    gui_x_error=1;
    xncdprintf(("XERROR HANDLER got Xlib Error!!\n\n\n"));
    //    *ptr=0;
    return 0;
}

void   guiSetInputFocus(Display* disp, Window w, int revert_to, Time t)
{
  XWindowAttributes xwa;
  int (*prev_handler)(Display*, XErrorEvent*);

  xncdprintf(("GUISETFOCUS: window %x\n",w));

  gui_x_error=0;
  //  prev_handler=XSetErrorHandler(gui_dummy_x_error_handler);
  XGetWindowAttributes(disp, w, &xwa);
  if(gui_x_error==0)
  {
      if(xwa.map_state==IsViewable)
	  XSetInputFocus(disp, w, revert_to, t);
      else
	  xncdprintf(("try to XSetInputFocus while window in %s state\n",
		  xwa.map_state==IsUnmapped ? "unmapped" : "unviewable"));
  } else
      xncdprintf(("try to XSetInputFocus while window in ERROR state\n"));
  //  XSetErrorHandler(prev_handler);
  XFlush(disp);
}

void   tohex(char *str, int num, int dig)
{
  int    n;
  char  *sym = "0123456789ABCDEF";
  str[dig--] = 0;
  while (dig >= 0)
    {
      n = num & 0xf;
      num >>= 4;
      str[dig] = sym[n];
      dig--;
    }
}

int str_casecmp(char *src1, char *src2)
{
        while(*src1!=0 && *src2!=0)
                if(tolower(*src1)!=tolower(*src2))
                        break;
                else
                        src1++,src2++;
        return tolower(*src1)-tolower(*src2);
}

int str_ncasecmp(char *src1, char *src2, int l)
{
        int i=0;
        while(*src1!=0 && *src2!=0 && i<l-1)
        {
                if(tolower(*src1)!=tolower(*src2))
                        break;
                else
                        src1++,src2++;
                i++;
        }
        return tolower(*src1)-tolower(*src2);
}

void   prect(Window w, GC & gcw, int x, int y, int l, int h)
{
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, x, y, x + l, y);
  XDrawLine(disp, w, gcw, x, y, x, y + h);
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, x + 1, y + h, x + l, y + h);
  XDrawLine(disp, w, gcw, x + l, y, x + l, y + h);
}

void   urect(Window w, GC & gcw, int x, int y, int l, int h)
{
  XSetForeground(disp, gcw, keyscol[2]);
  XDrawLine(disp, w, gcw, x, y, x + l - 1, y);
  XDrawLine(disp, w, gcw, x, y, x, y + h - 1);
  XSetForeground(disp, gcw, keyscol[0]);
  XDrawLine(disp, w, gcw, x + 1, y + h - 1, x + l - 1, y + h - 1);
  XDrawLine(disp, w, gcw, x + l - 1, y, x + l - 1, y + h - 1);
}

//Delay with milliseconds precision
void   delay(int usec)
{
  struct timeval time;
  usec *= 1000;
  time.tv_sec = usec / 1000000L;
  time.tv_usec = usec % 1000000L;
  select(0, (fd_set *) NULL, (fd_set *) NULL, (fd_set *) NULL, &time);
}


void init_l10n()
{
#ifdef ENABLE_NLS
#ifdef HAVE_SETLOCALE
    setlocale(LC_ALL, "");
#endif
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE); 
#endif
}


/////////////////////////Gui methods//////////////////////////////////

void   Gui::show()
{
  if(main_pixmap)
      XSetWindowBackgroundPixmap(disp, w, main_pixmap);
  XMapWindow(disp, w);
  addto_el(this, w);
}

void   Gui::hide()
{
  XUnmapWindow(disp, w);
  foc = 0;
  delfrom_el(this);
  delfrom_exp(this);
}

int Gui::set_selffocus()
{
  guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
  return 1;
}

int Gui::geometry_by_iname()
{
   int ix=0,iy=0,il=0,ih=0;
   if(in_name[0]==0)
           geom_translate_by_guitype(guitype,&ix, &iy, &il, &ih);
   else
           geom_translate_by_iname(guitype,in_name, &ix, &iy, &il, &ih);
   if(ix!=DONT_CARE)
           x=ix;
   if(iy!=DONT_CARE)
           y=iy;
   if(il!=DONT_CARE)
           l=il;
   if(ih!=DONT_CARE)
           h=ih;
   return YES;
}


////////////////////////////////////////class Separator///////////////////////////////////////////
void   Separator::expose()
{
 if(!vertical)
  prect(w, gcw, x, y, l, 1);
 else
  prect(w,gcw,x,y,1,l);
}

void   Separator::click()
{
  if (ev.xany.window == w && ev.type == Expose)
    expose();
}

////////////////////////////////End of file/////////////////////////////////////////



