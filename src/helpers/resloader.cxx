/****************************************************************************
*  Copyright (C) 2001 by Leo Khramov
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
#include "xshrloader.h"
#include "xncdebug.h"

#define RES_VERSION  "1.0.1"

Display *disp;

int screen;
Window root;
Pixmap pix;
Colormap defcmp;

// For test only -->
extern int lcd_chr_size;
extern char lcd_chr[];

// <--- end test


extern int     recycle_chr_size,
               flag1_chr_size,
               flag2_chr_size,
               flag3_chr_size,
               flag4_chr_size,
               flag5_chr_size,
               flag6_chr_size,
               flag7_chr_size,
               iconset1_chr_size,
               iconset2_chr_size,
               aquaskin_chr_size,
               fiveskin_chr_size;

extern char    recycle_chr[],
               recycle2_chr[],
               recycle3_chr[],
               flag1_chr[],
               flag2_chr[],
               flag3_chr[],
               flag4_chr[],
               flag5_chr[],
               flag6_chr[],
               flag7_chr[],
               iconset1_chr[],
               iconset2_chr[],
               fiveskin_chr[],
               aquaskin_chr[];


ResId res[]=
{
  ResId("xnci_recycle",recycle_chr,recycle_chr_size),
  ResId("xnci_recycle2",recycle2_chr,recycle_chr_size),
  ResId("xnci_recycle3",recycle3_chr,recycle_chr_size),
  ResId("xnci_recycle4",recycle3_chr,recycle_chr_size),
  ResId("xnci_flag1",flag1_chr,flag1_chr_size),
  ResId("xnci_flag2",flag2_chr,flag2_chr_size),
  ResId("xnci_flag3",flag3_chr,flag3_chr_size),
  ResId("xnci_flag4",flag4_chr,flag4_chr_size),
  ResId("xnci_flag5",flag5_chr,flag5_chr_size),
  ResId("xnci_flag6",flag6_chr,flag6_chr_size),
  ResId("xnci_flag7",flag7_chr,flag7_chr_size),
  ResId("xnci_p_aqua_skin",aquaskin_chr,aquaskin_chr_size),
  ResId("xnci_p_five_skin",fiveskin_chr,fiveskin_chr_size)
};

const int resnum=13;

ResId transparent_res[]=
{  
  ResId("xnci_iconset1",iconset1_chr,iconset1_chr_size),
  ResId("xnci_iconset2",iconset2_chr,iconset2_chr_size)
};

const int transresnum=2;



//////////////////////////////////////////////////////////////////////////////////

xsharedLoader *pxld;


void load_resources()
{
  int i,x,y;
  for(i=0;i<resnum;i++)
  {
    pxld->load_pixmap(res[i],x,y);
    printf(".");
    fflush(stdout);
  }

  for(i=0;i<transresnum;i++)
  {
    pxld->load_xsprite(transparent_res[i]);
    printf(".");
    fflush(stdout);
  }
}

void show_usage()
{
  printf("\n---==== X Northern Captain shared resource loader v."RES_VERSION"====---\n");
  printf("Loader need for speed-up loading process for X Northern Captain. It "
	 "preloads many X resources needed for xnc into X server and make it shared "
	 "between xnc copies. So it uses X server for storing images, small data etc.\n");
  printf("\nUsage:\txncloader [options]\n");
  printf("Options:\n");
  printf("\t-override - load all xnc resources and override old ones on X server\n");
  printf("\t-write    - load xnc resources to X server only if its not already loaded\n");
  printf("\t-clear    - find and free all xnc resources on X server\n\n");
  exit(1);
}

void set_options(int argc, char** argv)
{
  int i;
  for(i=1;i<argc;i++)
  {
    if(strcmp(argv[i],"-help")==0 || strcmp(argv[i],"--help")==0)
    {
      show_usage();
      continue;
    }
    if(strcmp(argv[i],"-override")==0)
    {
      pxld->set_workmode(SharedOverride);
      continue;
    }
    if(strcmp(argv[i],"-write")==0)
    {
      pxld->set_workmode(SharedWrite);
      continue;
    }
    if(strcmp(argv[i],"-read")==0)
    {
      pxld->set_workmode(SharedRead);
      continue;
    }
    if(strcmp(argv[i],"-nonshared")==0)
    {
      pxld->set_workmode(NonShared);
      continue;
    }
    fprintf(stderr,"\nInvalid option: try -help or --help\n");
    exit(1);
  }
}
    


int main(int argc, char **argv)
{
  pxld=new xsharedLoader(SharedWrite);

  if(argc>1)
    set_options(argc,argv);

  disp = XOpenDisplay(NULL);
  if (!disp) 
    {
      fprintf(stderr, "xncloader:  unable to open display '%s'\n",
              XDisplayName (NULL));
      exit (2);
    }
  pxld->init();
  init_imagengine();

  printf("X Northern Captain resource loader v"VERSION"\n");
  printf("Sharing resources..");
  fflush(stdout);
  load_resources();

  printf("..OK\n");

  pxld->exit_now();
  XCloseDisplay(disp);
  return 0;
}
