/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:     leo@pop.convey.ru
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
#include "globals.h"
#include "xheaders.h"
#include "image.h"
#define JROOT_VERSION  "1.1.6"
Display *disp;
int screen;
Window root;
void SetRootWindow(char *tline,char* opt);
Pixmap pix;
Colormap defcmp;

int main(int argc, char **argv)
{
  Atom prop, type;
  int format;
  unsigned long length, after;
  unsigned char *data;

  fprintf(stderr,"XJPEGroot Version %s\n",JROOT_VERSION);
  if(argc <3)
    {
      fprintf(stderr,"Usage: xjpegroot -c | -t jpegfile\n");
      exit(1);
    }
  disp = XOpenDisplay(NULL);
  if (!disp) 
    {
      fprintf(stderr, "XJPEGroot:  unable to open display '%s'\n",
              XDisplayName (NULL));
      exit (2);
    }
  screen = DefaultScreen(disp);
  defcmp = DefaultColormap(disp,screen);
  root = RootWindow(disp, screen);
  
  SetRootWindow(argv[2],argv[1]);

  prop = XInternAtom(disp, "_XROOTPMAP_ID", False);
  
  (void)XGetWindowProperty(disp, root, prop, 0L, 1L, True, AnyPropertyType,
                           &type, &format, &length, &after, &data);
  if ((type == XA_PIXMAP) && (format == 32) && (length == 1) && (after == 0))
    XKillClient(disp, *((Pixmap *)data));

  XChangeProperty(disp, root, prop, XA_PIXMAP, 32, PropModeReplace,
                  (unsigned char *) &pix, 1);
  XSetCloseDownMode(disp, RetainPermanent);
  XCloseDisplay(disp);
  return 0;
}


void SetRootWindow(char *tline,char *opt)
{
 int l,h;
 GC gcw;
  int mode=0;
  if(strcmp(opt,"-c")==0) mode=1;
  XWindowAttributes root_attr;
  XGetWindowAttributes(disp,root,&root_attr);
  init_imagengine();
  fprintf(stderr,"Loading image [%s].....\n",tline);
  XImage* iim=LoadXImage(tline,CMP_COM,F_AUTO);
  if(iim==NULL) 
 { fprintf(stderr,"XJPEGroot Error: Can't identify file format!\n");
    XCloseDisplay(disp);exit(1);
  }
  if(mode) {l=root_attr.width;h=root_attr.height;}
  else {l=iim->width;h=iim->height;};
   pix=XCreatePixmap(disp,root,
     l,h,DefaultDepth(disp,DefaultScreen(disp)));
   gcw=XCreateGC(disp,pix,0,NULL);
   XSetForeground(disp,gcw,BlackPixel(disp,screen));
   XFillRectangle(disp,pix,gcw,0,0,l,h);
   XPutImage(disp,pix,gcw,iim,0,0,l/2-iim->width/2,h/2-iim->height/2,
    iim->width,iim->height);
   XFreeGC(disp,gcw);
   delete iim->data;iim->data=NULL;XDestroyImage(iim);
  XSetWindowBackgroundPixmap(disp, root, pix);
  XClearWindow(disp,root);
}

