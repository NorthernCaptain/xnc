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
#include "xh.h"
#include "colors.h"
#include "configman.h"
#include "image.h"
char ver[10]="v.1.0.0";
char geom[30];

int win_resize=0;
int margc;
int centerx,centery;
char** margv;

//////////////////////////////////////////////////////////////////////////

char bgchr[50]="white",
     fgchr[50]="black",
     fontchr[100]="-adobe-helvetica-bold-r-*-*-16-*-*-*-*-*-*-*",
     lfontchr[100]="fixed",
     vfontchr[100]="fixed",
     fixfontchr[100]="fixed",
     kbgchr[100]="light blue",
     *dispchr=NULL,
     selectchr[100]="#4000f0",
     syspath[2048]=".",
     geometrychr[100]="0x1000+0+0",
     viewgeom[100]="400x400+20+40";

char bgchr1[50];
char bgchr2[50];
char bgchr3[50];
char bgchr4[50];
char bgchr5[50];
char bgchr6[50];

char rfile[80];


X_PAR resources[]=
{
  "leoprogs.background","LeoProgs.Background",bgchr,
  "leoprogs.foreground","LeoProgs.Foreground",fgchr,
  "leoprogs.keys.background","LeoProgs.Keys.Background",kbgchr,
  "leoprogs.font","LeoProgs.Font",fontchr,
  "leoprogs.selectedcolor","LeoProgs.SelectedColor",selectchr,
  "leoprogs.font.fixed","LeoProgs.Font.Fixed",fixfontchr,
  "xnc.sysfiles.path","Xnc.SysFiles.Path",syspath,
  "leoprogs.viewer.font","LeoProgs.Viewer.Font",vfontchr,
  "xnc.viewer.geometry","Xnc.Viewer.Geometry",viewgeom
};

int resn=9;


X_PAR colors[]=
{
  "leoprogs.syscolor1","LeoProgs.SysColor1",bgchr1,
  "leoprogs.syscolor2","LeoProgs.SysColor2",bgchr2,
  "leoprogs.syscolor3","LeoProgs.SysColor3",bgchr3,
  "leoprogs.syscolor4","LeoProgs.SysColor4",bgchr4,
  "leoprogs.syscolor5","LeoProgs.SysColor5",bgchr5,
  "leoprogs.syscolor6","LeoProgs.SysColor6",bgchr6,
};

int coln=6;

XFontStruct *fontstr;
XFontStruct *lfontstr;
XFontStruct *vfontstr;
XFontStruct *fixfontstr;
unsigned long bgpix,fgpix,selectpix;

XWMHints         *pxwmh;
XSizeHints       *pxsh;
XClassHint       *pch;
XTextProperty    wname;
XTextProperty    iname;
Display          *disp;
Window           Main;
Window           Key;
GC               gc;
XGCValues        gcv;
XEvent           ev;
XrmDatabase      rdb;
XrmValue         rmv;
XColor           color;
Colormap         defcmp;
int              done=0;
int              scr;
int              Mainl,Mainh;

int              viewx,viewy;
uint             viewl,viewh;


//Pixmap picon;
//Pixmap piconmask;
char *icname="IVES";
int layout=1;

ConfigManager *iniman;

void InitXClient(int argc,char** argv,int wx,int wy,int wl,int wh,char
 *winname)
{
 int i,ppx,ppy;
 uint ppl,pph;
 char *rmt;
 XSetWindowAttributes xswa;
 fprintf(stderr,"XClient...\n");
 for(i=1;i<argc;i++)
  if(strcmp("-display",argv[i])==0 || strcmp("-d",argv[i])==0) dispchr=argv[i+1];
 if((disp=XOpenDisplay(dispchr))==NULL)
 {
  fprintf(stderr,"%s: connection to X server failed on display %s.....\n",argv[0],dispchr);
  exit(1);
 }
 scr=DefaultScreen(disp);
 XrmInitialize();
 fprintf(stderr,"Loading resources....");
 strcpy(rfile,".setup.ini");
 iniman= new ConfigManager(rfile);
 rdb=XrmGetFileDatabase(rfile);
 if(rdb!=NULL)
  for(i=0;i<resn;i++)
  {
//   fprintf(stderr,"Value[%d] before: %s\n",i, resources[i].pstr);
   if(XrmGetResource(rdb,resources[i].name,resources[i].cname,&rmt,&rmv))
    strcpy(resources[i].pstr,rmv.addr);
   else fprintf(stderr,"%s: Error getting resource: %s\n",argv[0],resources[i].name);
//   fprintf(stderr,"Read value for %s - %s\n", resources[i].name, rmv.addr);
//   fprintf(stderr,"Value[%d] after: %s\n",i, resources[i].pstr);
  }          
 else fprintf(stderr,"%s: Can't open resource file.\n",argv[0]);
 defcmp=DefaultColormap(disp,scr);

 load_color_schema(iniman);
 selectpix=cursor_color;

 if((fontstr=XLoadQueryFont(disp,resources[3].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],resources[3].pstr);
   exit(1);
  }

 if((fixfontstr=XLoadQueryFont(disp,resources[5].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],resources[5].pstr);
   exit(1);
  }
 if((vfontstr=XLoadQueryFont(disp,resources[7].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],resources[7].pstr);
   exit(1);
  }


 fprintf(stderr,"OK\n");
 generate_bg_colors(resources[2].pstr);
 if((pxsh=XAllocSizeHints())==NULL)
 {
  fprintf(stderr,"%s: Error allocating size hints\n",argv[0]);
  exit(1);
 }
 

 pxsh->flags=(PPosition | PSize | PMinSize | PMaxSize);
 pxsh->height=wh;
 pxsh->width=wl;
 pxsh->min_width=pxsh->width;
 pxsh->min_height=pxsh->height;
 pxsh->max_width=pxsh->width;
 pxsh->max_height=pxsh->height;
 pxsh->x=wx;
 pxsh->y=wy;
 Mainl=wl;Mainh=wh;
 sprintf(geom,"%dx%d+%d+%d",pxsh->width,pxsh->height,pxsh->x,pxsh->y);
 i=XGeometry(disp,scr,geom,geom,BDWIDTH,fontstr->max_bounds.width,
             fontstr->max_bounds.ascent+fontstr->max_bounds.descent,
             1,1,&(pxsh->x),&(pxsh->y),&(pxsh->width),&(pxsh->height));
 if(i & (XValue | YValue)) pxsh->flags|=USPosition;
 if(i & (WidthValue | HeightValue)) pxsh->flags|=USSize;
 Main=XCreateSimpleWindow(disp,DefaultRootWindow(disp),
                          pxsh->x,pxsh->y,pxsh->width,pxsh->height,
                          BDWIDTH,fgpix,keyscol[1]);
 centerx=pxsh->width/2;
 centery=pxsh->height/2;
 if((pch=XAllocClassHint())==NULL)
 {
   fprintf(stderr,"%s: Error allocating ClassHint\n",argv[0]);
   exit(1);
 }
 pch->res_name="leoprogs";
 pch->res_class="LeoProgs";
 if(winname==NULL) winname=argv[0];
 if(XStringListToTextProperty(&winname,1,&wname)==0)
 {
  fprintf(stderr,"%s: Error creating TextProperty\n",argv[0]);
  exit(1);
 }       
 if(XStringListToTextProperty(&icname,1,&iname)==0)
 {
  fprintf(stderr,"%s: Error creating TextProperty\n",argv[0]);
  exit(1);
 }       
 if((pxwmh=XAllocWMHints())==NULL)
 {
  fprintf(stderr,"%s: Error allocating WMHints\n",argv[0]);
  exit(0);
 }
// picon=XCreateBitmapFromData(disp,Main,(char*)xnc_bits,xnc_width,xnc_height);
// piconmask=XCreateBitmapFromData(disp,DefaultRootWindow(disp),(char*)xncm_bits,xnc_width,xnc_height);
 pxwmh->flags=(InputHint|StateHint|IconMaskHint);
 pxwmh->input=True;
 pxwmh->initial_state=NormalState;
// pxwmh->icon_pixmap=picon;
// pxwmh->icon_mask=piconmask;
 XSetWMProperties(disp,Main,&wname,&iname,argv,argc,pxsh,pxwmh,pch);
 
 gcv.font=fontstr->fid;
 gcv.foreground=fgpix;
 gcv.background=bgpix;
 gc=XCreateGC(disp,Main,(GCFont | GCForeground | GCBackground),&gcv);
 xswa.colormap=defcmp;
 xswa.bit_gravity=CenterGravity;
 XChangeWindowAttributes(disp,Main,(CWColormap | CWBitGravity),&xswa);
 XSelectInput(disp,Main,ExposureMask | FocusChangeMask);
 
 XMapWindow(disp,Main);
}

void DeinitXClient()
{
// XFreePixmap(disp,picon);
// XFreePixmap(disp,piconmask);
 delall_el();
 delall_exp();
// XFreeGC(disp,gc);
// XUnloadFont(disp,fontstr->fid);
// XUnloadFont(disp,lfontstr->fid);
 XDestroyWindow(disp,Main);
 XCloseDisplay(disp);
 disp=NULL;
 exit(0);
}


Window create_win(char *winname,int wx,int wy,int wl,int wh,int mask)
{
 int i;
 Window W;
 XSetWindowAttributes xswa;
 pxsh->flags=(PPosition | PSize);
 if(win_resize==0)
 pxsh->flags|=( PMinSize | PMaxSize);
 win_resize=0;
 pxsh->height=wh;
 pxsh->width=wl;
 pxsh->min_width=pxsh->width;
 pxsh->min_height=pxsh->height;
 pxsh->max_width=pxsh->width;
 pxsh->max_height=pxsh->height;
 pxsh->x=wx;
 pxsh->y=wy;
 sprintf(geom,"%dx%d+%d+%d",pxsh->width,pxsh->height,pxsh->x,pxsh->y);
 i=XGeometry(disp,scr,geom,geom,BDWIDTH,fontstr->max_bounds.width,
             fontstr->max_bounds.ascent+fontstr->max_bounds.descent,
             1,1,&(pxsh->x),&(pxsh->y),&(pxsh->width),&(pxsh->height));
// if(i & (XValue | YValue)) pxsh->flags|=USPosition;
 if(i & (WidthValue | HeightValue)) pxsh->flags|=USSize;
 W=XCreateSimpleWindow(disp,DefaultRootWindow(disp),
                          pxsh->x,pxsh->y,pxsh->width,pxsh->height,
                          BDWIDTH,fgpix,keyscol[1]);
 pch->res_name="ives";
 pch->res_class="IVES";
 if(XStringListToTextProperty(&winname,1,&wname)==0)
 {
  fprintf(stderr,"Error creating TextProperty\n");
  exit(1);
 }       
 pxwmh->flags=(InputHint|StateHint);
 pxwmh->input=True;
 pxwmh->initial_state=NormalState;
 XSetWMProperties(disp,W,&wname,&iname,margv,margc,pxsh,pxwmh,pch);
 xswa.colormap=defcmp;
 xswa.bit_gravity=CenterGravity;
 XChangeWindowAttributes(disp,W,(CWColormap | CWBitGravity),&xswa);
 XSelectInput(disp,W,mask);
 
 return W;
}

//Dummy funcs
char *finduser(int)
{
    return "unknown";
}

char *findgroup(int)
{
    return "unknown";
}


