/****************************************************************************
*  Copyright (C) 1999 by Leo Khramov
*  email:     leo@xnc.dubna.su
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

/* This file contains internal images which need by XNC */
/* And routines for creating cursors, pixmaps etc...   */
#include "xh.h"
#include "gui.h"
#include "image.h"
#include "bookmark.h"
#include "infowin.h"
#include "skin.h"
#include "geomfuncs.h"
#include "c_externs.h"
#include "xnimlib.h"
#include "panel_x.h"
#include "pluginman.h"
#include "xshrloader.h"
#include "iconsets.h"

//Bitmaps and Pixmaps
#include "dnd1.xbm"
#include "dnd1m.xbm"
#include "dnd2.xbm"
#include "dnd2m.xbm"
#include "banmask.xbm"
#include "foldm.xbm"
#include "execm.xbm"
#include "arcm.xbm"
#include "assocm.xbm"
#include "linkm.xbm"
#include "quest.xbm"

extern int        xnc_ex;         //Placed in main.cxx
extern void (*xnc_run_once)();

char *flags_chr[]={
                                flag1_chr,
                                flag2_chr,
                                flag3_chr,
                                flag4_chr,
                                flag5_chr,
                                flag6_chr,
                                flag7_chr};
                                
int  flags_sizes[]={
                                flag1_chr_size,
                                flag2_chr_size,
                                flag3_chr_size,
                                flag4_chr_size,
                                flag5_chr_size,
                                flag6_chr_size,
                                flag7_chr_size};

Win*     mwin = NULL;
Text*    mtext;
Window   w_intro;
Pixmap   p_intro, intropix;
int      l_intro = 0, h_intro;
unsigned long pt_intro[256];
int      ptnum_intro;
Colormap cmp_intro;
static   SpriteWin *about_win=0;

static   int pixpipe[2]={-1, -1};

int    get_intro_size()
{
  return xncintro_chr_size;
}

void   show_about_dialog()
{
    xncdprintf(("Displaying About XNC window\n"));
    if(!about_win)
    {
	about_win=new SpriteWin(200,100,
				ResId("xnci_intro", 
				      xncintro_chr, 
				      xncintro_chr_size));
	about_win->init(Main);
    }
    about_win->set_center_to(centerx, centery);
    about_win->show();
}


IconKey *term_key=0;

extern void switch_term_fm_state();

void show_switch_term_button()
{
    if(term_key)
	return;
    term_key=new IconKey(Mainl-20,0,20,20,menu_icon,switch_term_fm_state);
    term_key->init(main_win);
    term_key->show();
}

void hide_switch_term_button()
{
    if(!term_key)
	return;
    term_key->hide();
    delete term_key;
    term_key=0;
}

Pixmap dnd1pix;
Pixmap dnd1mpix;
Cursor cdnd1, cdnd2;


Pixmap get_rootwindow_pixmap()
{
  static Atom prop=0;
  Atom type;
  int format;
  unsigned long length, after;
  unsigned char *data;

  Window root = RootWindow(disp, DefaultScreen(disp));
  
  if(!prop)
      prop = XInternAtom(disp, "_XROOTPMAP_ID", True);
  
  (void)XGetWindowProperty(disp, root, prop, 0L, 1L, False, AnyPropertyType,
                           &type, &format, &length, &after, &data);
  if ((type == XA_PIXMAP) && (format == 32) && (length == 1))
      return *((Pixmap*)data);
  return 0;
}


void   create_dnd_cursors()
{
  XColor xc1, xc2;
  xc1.pixel = cols[0];
  xc1.red = xc1.green = xc1.blue = 0l;
  xc1.flags = DoRed | DoBlue | DoGreen;
  xc2.pixel = cols[1];
  xc2.red = 65535l;
  xc2.green = 65535l;
  xc2.blue = 65535l;
  xc2.flags = DoRed | DoBlue | DoGreen;
  dnd1pix = XCreateBitmapFromData(disp, Main, (char *)dnd1_bits, dnd1_width, dnd1_height);
  dnd1mpix = XCreateBitmapFromData(disp, Main, (char *)dnd1m_bits, dnd1m_width, dnd1m_height);
  cdnd1 = XCreatePixmapCursor(disp, dnd1pix, dnd1mpix, &xc1, &xc2, 11, 8);
  XFreePixmap(disp, dnd1pix);
  XFreePixmap(disp, dnd1mpix);
  dnd1pix = XCreateBitmapFromData(disp, Main, (char *)dnd2_bits, dnd2_width, dnd2_height);
  dnd1mpix = XCreateBitmapFromData(disp, Main, (char *)dnd2m_bits, dnd2m_width, dnd2m_height);
  cdnd2 = XCreatePixmapCursor(disp, dnd1pix, dnd1mpix, &xc1, &xc2, 12, 13);
  XFreePixmap(disp, dnd1pix);
  XFreePixmap(disp, dnd1mpix);
}


void   win_nodecor(Window W)
{
  XSetWindowAttributes xswa;
  xswa.override_redirect = True;
  XChangeWindowAttributes(disp, W, CWOverrideRedirect, &xswa);
}

void   show_mes(char *head, char *mes)
{
  if (mwin == NULL)
    {
      mwin = guiplugin->new_Win(centerx - 125, 150, 250, 100, head, 3);
      mtext = guiplugin->new_Text(40, 60, mes, 5);
      mwin->init(Main);
      mtext->init(mwin->w);
      mtext->link(mtext);
      mwin->show();
      mtext->show();
      mwin->expose();
      XFlush(disp);
    }
}

void   hide_mes()
{
  mtext->hide();
  mwin->hide();
  delete mwin;
  delete mtext;
  mwin = NULL;
}


int dot_bar=0;

void show_dot()
{
     if(dot_bar && infowin)
        infowin->update_coun(1);
     else
        fprintf(stderr,".");
}

void        free_panel_pixmaps()
{
  int i;
  for(i=0;i<6;i++)
    if(panelpixs[i])
    {
      default_loader->free_pixmap(panelpixs[i]);
      XFreePixmap(disp,panelpixmask[i]);
      panelpixs[i]=0;
    }
}


void do_back_load();

void signal_back_load(int)
{
   xnc_run_once=do_back_load;
}

void do_back_load()
{
  int x,y;

  static int idx=-1;
  static int in_use=0;
   
  if(idx==-1)
    {
      idx++;
      set_xnc_alarm(signal_back_load);
      return;
    }
  switch(idx)
    {
    case 0:
      panelpixs[0] = LoadPixmapFromMem(folder_chr, F_GIF, x, y, CMP_COM,
				       folder_chr_size);
      panelpixmask[0] = XCreateBitmapFromData(disp, Main, 
					      (char *)foldm_bits, 
					      foldm_width, foldm_height);
      panelpixl[0] = x;
      panelpixh[0] = y;
      break;

    case 1:
      panelpixs[1] = LoadPixmapFromMem(exec_chr, F_GIF, x, y, CMP_COM,
				       exec_chr_size);
      panelpixmask[1] = XCreateBitmapFromData(disp, Main, (char *)execm_bits, 
					      execm_width, execm_height);
      panelpixl[1] = x;
      panelpixh[1] = y;
      break;
      
    case 2:
      panelpixs[2] = LoadPixmapFromMem(norm_chr, F_GIF, x, y, CMP_COM,
				       norm_chr_size);
      panelpixmask[2] = XCreateBitmapFromData(disp, Main, (char *)assocm_bits,
					      assocm_width, assocm_height);
      panelpixl[2] = x;
      panelpixh[2] = y;
      break;

    case 3:
      panelpixs[3] = LoadPixmapFromMem(assoc_chr, F_GIF, x, y, CMP_COM,
				       assoc_chr_size);
      panelpixmask[3] = XCreateBitmapFromData(disp, Main, (char *)assocm_bits, 
					      assocm_width, assocm_height);
      panelpixl[3] = x;
      panelpixh[3] = y;
      break;

    case 4:
      panelpixs[4] = LoadPixmapFromMem(arc_chr, F_GIF, x, y, CMP_COM,
				       arc_chr_size);
      panelpixmask[4] = XCreateBitmapFromData(disp, Main, (char *)arcm_bits, 
					      arcm_width, arcm_height);
      panelpixl[4] = x;
      panelpixh[4] = y;
      break;

    case 5:
      XFreePixmap(disp, panelpixs[5]);
      XFreePixmap(disp, panelpixmask[5]);
      panelpixs[5] = LoadPixmapFromMem(link_chr, F_GIF, x, y, CMP_COM, 
				       link_chr_size);
      panelpixmask[5] = XCreateBitmapFromData(disp, Main, (char *)linkm_bits, 
					      linkm_width, linkm_height);
      panelpixl[5] = x;
      panelpixh[5] = y;

      panel->expose();
      if (panel->lay == 0)
	panel->panel2->expose();
      XFlush(disp);
      break;

    default:
      if(!bmark->load_pixmap(idx-6))
	{
	  remove_xnc_alarm(signal_back_load);
	  bmark->set_recycle_image(0);
	  bmark->expose();
	  XFlush(disp);
	}
      break;
    }
  
  idx++;
}


int internals_set_fds(int fd, fd_set* fds)
{
  FD_SET(pixpipe[0], fds);
  return fd>pixpipe[0] ? fd : pixpipe[0];
}

int internals_check_fds(fd_set* fds)
{
  static int idx=0;
  if(FD_ISSET(pixpipe[0], fds))
     {
       Pixmap val, val2;
       int l, h;
       read(pixpipe[0], &val, sizeof(val));
       read(pixpipe[0], &val2, sizeof(val2));
       read(pixpipe[0], &l, sizeof(l));
       read(pixpipe[0], &h, sizeof(h));
       if(val)
	 {
	   panelpixs[idx]=val;
	   panelpixmask[idx]=val2;
	   panelpixl[idx]=l;
	   panelpixh[idx]=h;
	   idx++;
	 }
       fprintf(stderr, "XNC Child sent %d\n", val);
     }
  return 1;
}

void   load_pixmaps()
{
  int    x, y, i;
  int panels_pixl, panels_pixh;

  load_iconsets();

  if(is_fast_load())
  {
          allow_dir_icon=0;
          allow_file_icon=0;
	  return;
  }

  if(is_back_load())
  {
    panelpixs[0]=XCreatePixmapFromBitmapData(disp, Main, (char*)quest_bits, quest_width, 
					     quest_height, cols[0], keyscol[1],
					     DefaultDepth(disp, DefaultScreen(disp)));
    panelpixmask[0]=XCreateBitmapFromData(disp, Main, (char*)quest_bits, quest_width, 
					  quest_height);
    panelpixl[0]= quest_width;
    panelpixh[0]= quest_height;
    for(i=1;i<6;i++)
    {
      panelpixs[i]=panelpixs[0];
      panelpixmask[i]=panelpixmask[0];
      panelpixl[i]= quest_width;
      panelpixh[i]= quest_height;
    }
    do_back_load();
    return;
  }
}


void   show_intro()
{
  int    es1, es2;
  if (l_intro == 0)
    {
      p_intro = LoadPixmapFromMem(xncintro_chr, F_GIF, l_intro, h_intro, 
				  CMP_OWN, get_intro_size());
      if (p_intro)
        {
          w_intro = create_win("X Northern Captain", ScreenL / 2 - l_intro / 2,
			       ScreenH / 2 - h_intro / 2, l_intro, h_intro,
			       ExposureMask, 0);
          XSetWindowBorderWidth(disp, w_intro, 0);
          win_nodecor(w_intro);
          ptnum_intro = im_getpixels(pt_intro);
          cmp_intro = im_get_colormap();
          if (p_intro)
	      XSetWindowBackgroundPixmap(disp, w_intro, p_intro);
          if (cmp_intro)
	      XSetWindowColormap(disp, w_intro, cmp_intro);

	  intropix = 0;
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
          intropix = XCreateBitmapFromData(disp, Main, (char *)banmask_bits, 
					   banmask_width, banmask_height);
          if (XShapeQueryExtension(disp, &es1, &es2))
	      XShapeCombineMask(disp, w_intro, ShapeBounding, 0, 0, intropix, 
				ShapeSet);
          else
#endif
	      fprintf(stderr, "XShape extensions not supported.\n");

          XMapRaised(disp, w_intro);
          if (DefaultDepth(disp, DefaultScreen(disp)) <= 8)
            XInstallColormap(disp, cmp_intro);
          XFlush(disp);
        }
      else
        l_intro = 0;
    }
}

void   hide_intro()
{
  if (l_intro)
    {
      if (DefaultDepth(disp, DefaultScreen(disp)) <= 8)
        XUninstallColormap(disp, cmp_intro);
      XDestroyWindow(disp, w_intro);
      XFreePixmap(disp, p_intro);
      if(intropix!=0)
	  XFreePixmap(disp, intropix);
      XFlush(disp);
      if (cmp_intro)
        im_freecmp(cmp_intro, pt_intro, ptnum_intro);
    }
}

void   x_free_pixmaps()
{
  int    i;
  for (i = 0; i < 4; i++)
    default_loader->free_pixmap(pixrecycle[i]);
  if (allow_dir_icon)
    default_loader->free_pixmap(panelpixs[0]);
  XFreePixmap(disp, panelpixmask[0]);
  if (allow_file_icon && panelpixs[0]!=panelpixs[1])
    for (i = 1; i < 6; i++)
      {
        default_loader->free_pixmap(panelpixs[i]);
        XFreePixmap(disp, panelpixmask[i]);
      }
}

int    flush_expose(Window w)
{
  XEvent dummy;
  int    i = 0;

  while (XCheckTypedWindowEvent(disp, w, Expose, &dummy))
    i++;
  return i;
}

/*
 * This is nearly completely copied from the OffiX DragAndDrop library.
 */

#ifndef MAXINT
#define MAXINT ((1<<31)-1)
#endif

Atom DndProtocol, DndSelection;
Atom Old_DndProtocol, Old_DndSelection;

void init_dnd()
{
  DndProtocol = XInternAtom(disp, "_DND_PROTOCOL", 0);
  DndSelection = XInternAtom(disp, "_DND_SELECTION", 0);
  Old_DndProtocol = XInternAtom(disp, "DndProtocol", 0);
  Old_DndSelection = XInternAtom(disp, "DndSelection", 0);
}



