/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
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
#include <config.h>
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "ivesclient.h"

Display *disp;
IVES_client *iclient;
char data[2048];

int main(int argc,char** argv)
{
  int i;
  char *dispchr=0;
  if(argc>1)
    {
      for(i=1;i<argc;i++)
	if(strcmp("-display",argv[i])==0 || strcmp("-d",argv[i])==0) dispchr=argv[i+1];
      if((disp=XOpenDisplay(dispchr))==0)
	{
	  fprintf(stderr,"%s: connection to X server failed on display %s.....\n",argv[0],dispchr);
	  exit(1);
	}
      iclient=new IVES_client;
      if(iclient->Init()==0)
	{
	  fprintf(stderr,"IVES is not running.... Run it first\n");
	  XCloseDisplay(disp);
	  return 0;
	}
      if(argv[argc-1][0]!='/')
	{
	  getcwd(data,1500);
	  strcat(data,"/");
	  strcat(data,argv[argc-1]);
	}
      else
	strcpy(data,argv[argc-1]);
      iclient->send_data(DefaultRootWindow(disp),IVES_data(1,data));
      XCloseDisplay(disp);
    } else fprintf(stderr,"es - IVES editor.\n Usage : es filename\n");
  return 0;
}

        
