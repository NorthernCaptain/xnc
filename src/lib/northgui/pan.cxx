/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
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
#include "gui.h"
#include "scroll.h"
#include "baseguiplugin.h"
const Time DBLCLICK = 250;

///////////////////////////Panel class/////////////////////////////////////
// *INDENT-OFF*        
Panel::Panel(int ix, int iy, int il, int ih, char **iname, int imax, ulong icol, int (*ifunc) (int, char *)):Gui()
// *INDENT-ON*        

{
  x = ix;
  y = iy;
  l = il;
  h = ih;
  col = icol;
  func = ifunc;
  prflg = 0;
  names = iname;
  keyfunc = NULL;
  max = imax;
  guitype = GUI_PANEL;
  foc = 0;
  hflg = 0;
  bclk = 0;
  dbtime = 0;
  every_move_func=0;
}

void   Panel::init(Window ipar)
{
  parent = ipar;
  ty = fixfontstr->max_bounds.ascent + fixfontstr->max_bounds.descent + 4;
  vh = (h - 6) / ty;
  h = vh * ty + 6;
  w = XCreateSimpleWindow(disp, parent, x, y, l, h, 0, 0, lighter_bg_color);
  gcv.background = lighter_bg_color;
  gcv.font = fixfontstr->fid;
  gcw = XCreateGC(disp, w, GCBackground | GCFont, &gcv);
  XSelectInput(disp, w, ButtonReleaseMask |
           ButtonPressMask | ExposureMask | KeyPressMask | FocusChangeMask);
  base = cur = 0;
  scr = baseguiplugin->new_ScrollBar(-1, 1, h - 4, this);
  scr->setrange(0, 1);
  scr->init(w);
}

void   Panel::show()
{
// *INDENT-OFF*        
  Gui::show();
// *INDENT-ON*        

  scr->show();
  prflg = 1;
}

void   Panel::hide()
{
  scr->hide();
// *INDENT-OFF*        
  Gui::hide();
// *INDENT-ON*        

}

void   Panel::add_element(char *el)
{
  int    prb = base;
  showcurs(0);
  names[max] = el;
  cur = max % vh;
  base = max - cur;
  max++;
  if (prb != base)
    expose();
  else
    showcurs();
}

void   Panel::reset_list()
{
  XClearWindow(disp, w);
  base = max = cur = 0;
  expose();
}

void   Panel::expose()
{
  shownames();
  XSetForeground(disp, gcw, dark_bg_color);
  XDrawLine(disp, w, gcw, 0, 0, l, 0);
  XDrawLine(disp, w, gcw, 0, 0, 0, h);
  XSetForeground(disp, gcw, light_bg_color);
  XDrawLine(disp, w, gcw, 0, h - 1, l, h - 1);
  XDrawLine(disp, w, gcw, l - 1, 0, l - 1, h);
  if (foc)
    XSetWindowBorderWidth(disp, w, 1);
  else
    XSetWindowBorderWidth(disp, w, 0);
  showcurs();
  hflg = 1;
}

void   Panel::shownames()
{
  int    as = fixfontstr->max_bounds.ascent + 7, i, k;
  for (i = 0; i < vh; i++)
    {
      XClearArea(disp, w, 1, i * ty + 5, l - 2, ty, 0);
      if (i + base < max)
        {
          k = strlen(names[i + base]);
          if(shadow)
          {
	      XSetForeground(disp, gcw, shadow_color);
	      XDrawString(disp, w, gcw, 5, i * ty + as, names[i + base], k);
          }
          XSetForeground(disp, gcw, cols[col]);
          XDrawString(disp, w, gcw, 4, i * ty + as - 1, names[i + base], k);
        }
    }
}

void   Panel::showcurs(int f)
{
  int    as = fixfontstr->max_bounds.ascent + 7, k;
  if (max > 0)
    {
      k = strlen(names[cur + base]);
      if (f)
      {
	  XSetForeground(disp, gcw, cursor_color);
	  XFillRectangle(disp, w, gcw, 3, cur * ty + 5, l - 6 - scr->get_l(), ty);
	  XSetForeground(disp, gcw, dark_cursor_color);
	  XDrawRectangle(disp, w, gcw, 3, cur * ty + 5, l - 7 - scr->get_l(), ty-1);
      }
      else
	  XClearArea(disp, w, 3, cur * ty + 5, l - 6, ty, 0);
        
      if(shadow)
      {
	  XSetForeground(disp, gcw, shadow_color);
	  XDrawString(disp, w, gcw, 5, cur * ty + as, names[cur + base], k);
      }
      XSetForeground(disp, gcw, cols[col]);
      XDrawString(disp, w, gcw, 4, cur * ty + as - 1, names[cur + base], k);
      if (f)
      {
          scr->maxval = max - 1;
          scr->range = max;
          scr->val = base + cur;
          scr->setpages(max / vh);
          scr->expose();
      }
    }
}

void   Panel::scroll(Gui * o)
{
  ScrollBar *oo = (ScrollBar *) o;
  int    i = oo->val, ibase;
  showcurs(0);
  if (i < max)
    {
      cur = i % vh;
      ibase = i - cur;
      if (ibase != base)
        {
          base = ibase;
          expose();
        }
      else
        showcurs();

      if (every_move_func)
      {
	  every_move_func(cur + base, names[cur + base]);
      }
    }
}

void   Panel::find_and_set(char *fs)
{
  int    i;
  for (i = 0; i < max; i++)
    if (strcmp(fs, names[i]) == 0)
      break;
  if (i < max)
    {
      cur = i % vh;
      base = i - cur;
      expose();
    }
}

void   Panel::activate_function()
{
  if (func)
    {
      func(cur + base, names[cur + base]);
      return;
    };
  if (guiobj)
    guiobj->guifunc(this, cur + base);
}

void   Panel::move_line_up()
{
    if (cur + base > 0)
    {
	showcurs(0);
	cur--;
	if (cur < 0)
	{
	    cur = 0;
	    base--;
	    expose();
	}
	else
	    showcurs();
	if (every_move_func)
	{
	    every_move_func(cur + base, names[cur + base]);
	}
    };
}

void Panel::move_line_down()
{
    if (cur + base + 1 < max)
    {
	showcurs(0);
	cur++;
	if (cur >= vh)
	{
	    cur--;
	    base++;
	    expose();
	}
	else
	    showcurs();
	if (every_move_func)
	{
	    every_move_func(cur + base, names[cur + base]);
	}
    };
}

void   Panel::click()
{
  KeySym ks;
  if (ev.xany.window == w)
    {
      switch (ev.type)
        {
        case Expose:
          expose();
          break;
        case FocusIn:
          if (foc == 0)
            {
              foc = 1;
	      XSetWindowBorderWidth(disp, w, 1);
              focobj = this;
            };
          break;
        case FocusOut:
          if (foc)
            {
              foc = 0;
	      XSetWindowBorderWidth(disp, w, 0);
              focobj = NULL;
            };
          break;
        case KeyPress:
          if (foc)
            {
              ks = XLookupKeysym(&ev.xkey, 0);
              switch (ks)
                {
                case XK_Execute:
                case XK_Escape:
                  if (canfunc)
                    canfunc();
                  break;
                case XK_Return:
                  if (func)
                    {
                      func(cur + base, names[cur + base]);
                      return;
                    };
                  if (guiobj)
                    guiobj->guifunc(this, cur + base);
                  break;
		  /*
                  prflg = 1;
                  if (hflg)
                    expose();
                  break;
		  */
                case XK_Prior:
                  if (base > 0)
                    {
                      base -= vh;
                      if (base < 0)
                        {
                          base = cur = 0;
                        };
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Next:
                  if (base + vh + cur < max)
                    {
                      base += vh;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Right:
                  if (cur + base + 1 != max)
                    {
                      cur = (max - 1) % vh;
                      base = max - cur - 1;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Tab:
                  if (next)
                    guiSetInputFocus(disp, next->w, RevertToParent, CurrentTime);
                  break;
                case XK_Left:
                  if (cur + base != 0)
                    {
                      base = cur = 0;
                      expose();
                      if (every_move_func)
                      {
			every_move_func(cur + base, names[cur + base]);
                      }
                    };
                  break;
                case XK_Down:
		    move_line_down();
                  break;
                case XK_Up:
		    move_line_up();
                  break;
                  break;
                default:
                  if (keyfunc)
                    keyfunc(ks, this);
                };
            };
          break;
        case ButtonPress:
          guiSetInputFocus(disp, w, RevertToNone, CurrentTime);
	  if(ev.xbutton.button == Button4)
	  {
	      move_line_up();
	      break;
	  }
	  if(ev.xbutton.button == Button5)
	  {
	      move_line_down();
	      break;
	  }

          showcurs(0);
          cur = (ev.xbutton.y - 3) / ty;
          if (cur + base >= max)
            cur = max - base - 1;
          showcurs();
	  if (every_move_func)
	  {
	    every_move_func(cur + base, names[cur + base]);
	  }

          if (ev.xbutton.button == Button3)
            {
              if (func)
                {
                  func(cur + base, names[cur + base]);
                  return;
                };
              if (guiobj)
                guiobj->guifunc(this, cur + base);
            }

          break;
        case ButtonRelease:
          if (ev.xbutton.button == Button1)
            {
              if (bclk)
                {
                  bclk = 0;
                  prflg = 1;
                  if ((ev.xbutton.time - dbtime) <= DBLCLICK &&
                      abs(ev.xbutton.x - lmx) <= 10 && abs(ev.xbutton.y - lmy) <= 10)
                    {
                      if (func)
                        {
                          func(cur + base, names[cur + base]);
                          return;
                        };
                      if (guiobj)
                        guiobj->guifunc(this, cur + base);
                    };
                }
              if (prflg)
                {
                  bclk = 1;
                  prflg = 0;
                  dbtime = ev.xbutton.time;
                  lmx = ev.xbutton.x;
                  lmy = ev.xbutton.y;
                }
//   expose();
            };
          break;
        };
    }
}

