/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
*  email:   leo@pop.convey.ru
*  Fido:      2:5030/627.15
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

char ver[10]="v.1.0.1";
char geom[30];
char *searchpath="/usr/local/X11/include/X11/pixmaps";

int margc;
int centerx,centery;
char** margv;
int win_inp=1;
int ScreenL,ScreenH,vcenterx;

//////////////////////////////////////////////////////////////////////////

char *bgchr="white",
     *fgchr="black",
     *fontchr="fixed",
     *lfontchr="fixed",
     *vfontchr="fixed",
     *fixfontchr="fixed",
     *kbgchr="light blue",
     *dispchr=NULL,
     *selectchr="#4000f0",
     *syspath=".",
     *geometrychr="0x1000+0+0",
     *viewgeom="30x30+20+40",
     *padsizechr="1x20";

char rfile[80];
uint padl,padh;
uint butl,buth;

X_PAR resources[]=
{
  "leoprogs.background","LeoProgs.Background",&bgchr,
  "leoprogs.foreground","LeoProgs.Foreground",&fgchr,
  "leoprogs.keys.background","LeoProgs.Keys.Background",&kbgchr,
  "leoprogs.font","LeoProgs.Font",&fontchr,
  "xnlaunch.geometry","XnLaunch.Geometry",&viewgeom,
  "xnlaunch.pad.size","XnLaunch.Pad.Size",&padsizechr,
  "xnlaunch.pixmaps.searchpath","XnLaunch.Pixmaps.SearchPath",&searchpath
};

int resn=sizeof(resources)/sizeof(X_PAR);

X_PAR colors[]=
{
  "leoprogs.syscolor1","LeoProgs.SysColor1",&bgchr,
  "leoprogs.syscolor2","LeoProgs.SysColor2",&bgchr,
  "leoprogs.syscolor3","LeoProgs.SysColor3",&bgchr
};

int coln=sizeof(colors)/sizeof(X_PAR);

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
char iicname[20]="XNLaunch";
char *icname=iicname;
int layout=1;

 
void InitXClient(int argc,char** argv,int wx,int wy,uint wl,uint wh,char
 *winname)
{
 struct passwd *pwent;
 int i,ppx,ppy;
 uint ppl,pph;
 char *rmt;
 XSetWindowAttributes xswa;
 XWindowAttributes xwinattr;
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
 strcpy(rfile,getenv("HOME"));
 strcat(rfile,"/.xlaunch/.xnlaunch.ini");
 rdb=XrmGetFileDatabase(rfile);
 if(rdb!=NULL)
  for(i=0;i<resn;i++)
  {
   fprintf(stderr,".");
   if(XrmGetResource(rdb,resources[i].name,resources[i].cname,&rmt,&rmv))
    *resources[i].pstr=rmv.addr;
   else fprintf(stderr,"%s: Error getting resource: %s\n",argv[0],resources[i].name);
  }          
 else fprintf(stderr,"%s: Can't open resource file.\n",argv[0]);
 XParseGeometry(geometrychr,&ppx,&ppy,&ppl,&pph);
 if(ppl>0)
 {
  wx=ppx;wy=ppy;wl=ppl;wh=pph;
  if(wl<640) wl=640;
  if(wh<300) wh=300;
 }
 XParseGeometry(padsizechr,&i,&i,&padl,&padh);
 XParseGeometry(viewgeom,&wx,&wy,&wl,&wh);
 butl=wl;buth=wh;
 wl*=padl;wh=wh*padh+16;
 defcmp=DefaultColormap(disp,scr);

 if(XParseColor(disp,defcmp,*resources[0].pstr,&color)==0 || 
    XAllocColor(disp,defcmp,&color)==0)
    bgpix=WhitePixel(disp,scr);
 else bgpix=color.pixel;
 if(XParseColor(disp,defcmp,*resources[4].pstr,&color)==0 || 
    XAllocColor(disp,defcmp,&color)==0)
    selectpix=BlackPixel(disp,scr);
 else selectpix=color.pixel;
 if(XParseColor(disp,defcmp,*resources[1].pstr,&color)==0 || 
    XAllocColor(disp,defcmp,&color)==0)
    fgpix=BlackPixel(disp,scr);
 else fgpix=color.pixel;

  for(i=0;i<coln;i++)
   if(XrmGetResource(rdb,colors[i].name,colors[i].cname,&rmt,&rmv))
    {
     fprintf(stderr,".");
     *colors[i].pstr=rmv.addr;
     if(XParseColor(disp,defcmp,*colors[i].pstr,&color)==0 || 
       XAllocColor(disp,defcmp,&color)==0)
        cols[i]=BlackPixel(disp,scr);
        else cols[i]=color.pixel;
     }

 if((fontstr=XLoadQueryFont(disp,*resources[3].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],fontchr);
   exit(1);
  }



 fprintf(stderr,"OK\n");
 generate_bg_colors(*resources[2].pstr);
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
 i=XGeometry(disp,scr,geom,geom,0,fontstr->max_bounds.width,
             fontstr->max_bounds.ascent+fontstr->max_bounds.descent,
             1,1,&(pxsh->x),&(pxsh->y),&(pxsh->width),&(pxsh->height));
 if(i & (XValue | YValue)) pxsh->flags|=USPosition;
 if(i & (WidthValue | HeightValue)) pxsh->flags|=USSize;
 Main=XCreateSimpleWindow(disp,DefaultRootWindow(disp),
                          pxsh->x,pxsh->y,pxsh->width,pxsh->height,
                          0,fgpix,keyscol[1]);
 centerx=pxsh->width/2;
 centery=pxsh->height/2;
 if((pch=XAllocClassHint())==NULL)
 {
   fprintf(stderr,"%s: Error allocating ClassHint\n",argv[0]);
   exit(1);
 }
 pch->res_name="xnlaunch";
 pch->res_class="XNLAUNCH";
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
 pxwmh->initial_state=IconicState;
// pxwmh->icon_pixmap=picon;
// pxwmh->icon_mask=piconmask;
 XSetWMProperties(disp,Main,&wname,&iname,argv,argc,pxsh,pxwmh,pch);
 
 gcv.font=fontstr->fid;
 gcv.foreground=fgpix;
 gcv.background=bgpix;
 gc=XCreateGC(disp,Main,(GCFont | GCForeground | GCBackground),&gcv);
 xswa.colormap=defcmp;
 xswa.bit_gravity=CenterGravity;
 xswa.override_redirect=True;
 XChangeWindowAttributes(disp,Main,(CWColormap | CWBitGravity | CWOverrideRedirect),&xswa);
 XSelectInput(disp,Main,ExposureMask | FocusChangeMask |
 StructureNotifyMask | ButtonPressMask | Button3MotionMask);
 XGetWindowAttributes(disp,DefaultRootWindow(disp),&xwinattr);
 ScreenL=xwinattr.width;
 ScreenH=xwinattr.height;
 
 XMapWindow(disp,Main);
}

void DeinitXClient()
{
 delall_el();
 delall_exp();
 XFreeGC(disp,gc);
 XUnloadFont(disp,fontstr->fid);
 XDestroyWindow(disp,Main);
 XCloseDisplay(disp);
 exit(0);
}


Window create_win(char *winname,int wx,int wy,int wl,int wh,int mask)
{
 int i;
 Window W;
 XSetWindowAttributes xswa;
 pxsh->flags=(PPosition | PSize | PMinSize | PMaxSize);
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
                          0,fgpix,keyscol[1]);
 pch->res_name="xnlaunch";
 pch->res_class="XNLAUNCH";
 if(XStringListToTextProperty(&winname,1,&wname)==0)
 {
  fprintf(stderr,"Error creating TextProperty\n");
  exit(1);
 }       
 pxwmh->flags=(InputHint|StateHint);
 pxwmh->input=win_inp;win_inp=1;
 pxwmh->initial_state=NormalState;
 XSetWMProperties(disp,W,&wname,&iname,margv,margc,pxsh,pxwmh,pch);
 xswa.colormap=defcmp;
 xswa.bit_gravity=CenterGravity;
 XChangeWindowAttributes(disp,W,(CWColormap | CWBitGravity),&xswa);
 XSelectInput(disp,W,mask);
 
 return W;
}

void win_nodecor(Window W)
{
 XSetWindowAttributes xswa;
 xswa.override_redirect=True;
 XChangeWindowAttributes(disp,W,CWOverrideRedirect,&xswa);
}
 
