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
#include <pwd.h>
#include "colors.h"
#include "configman.h"
#include "ivesgray.xbm"
#include "ivesmask.xbm"

char geom[30];

int margc;
int centerx,centery;
char** margv;
int win_inp=1;
int win_resize=0;
int ScreenL,ScreenH;
int return_focus=1;
extern int shadow;
extern char cfindbuf[];        //in editor.cxx

extern int str_casecmp(char*,char*);        //defined in gui.cxx

char home_files_dir[1024];

extern int ivesini_chr_size;
extern char ivesini_chr[];

void make_home_file_dir()
{
 char str[256];
 int fp;
 sprintf(str,"%s/.xnc/ives.ini",getenv("HOME"));
 fp=open(str,O_RDONLY);
 if(fp==-1)
 {
  fprintf(stderr,"\nWARNING: User configuration files not found - using system ones...\n");
  fp=open(str,O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if(fp==-1)
   fprintf(stderr,"\nWARNING: Can't reate system default configuration files ...\n");
  else
   {
           write(fp,ivesini_chr,ivesini_chr_size);
           close(fp);
   }
 } else close(fp);
 strcpy(home_files_dir,getenv("HOME"));
 strcat(home_files_dir,"/.xnc");
}
   
//////////////////////////////////////////////////////////////////////////

char bgchr[100]="white",
     fgchr[100]="black",
     fontchr[100]="fixed",
     lfontchr[100]="fixed",
     vfontchr[100]="fixed",
     fixfontchr[100]="fixed",
     kbgchr[100]="#9cb0c4",
     *dispchr=NULL,
     selectchr[100]="#4000f0",
     editname[100]="vi",
     viewname[100]="vi",
     syspath[100]=".",
     geometrychr[100]="0x1000+0+0",
     viewgeom[100]="400x400+20+40",
     ret_foc[100]="yes",
     shadchr[100]="yes";

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
  "xnc.viewer.geometry","Xnc.Viewer.Geometry",viewgeom,
  "leoprogs.focus.return","LeoProgs.Focus.Return",ret_foc,
  "leoprogs.text.shadow","LeoProgs.Text.Shadow",shadchr
};

int resn=11;

char bgchr1[100];
char bgchr2[100];
char bgchr3[100];
char bgchr4[100];
char bgchr5[100];
char bgchr6[100];

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
XFontStruct *mfixfontstr;
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
char iicname[20]="IVES";
char *icname=iicname;
int layout=1;

char ch_user[40];

char *scanstr(char *src,char *dest)
{
 while(*src!=':')
  *dest++=*src++;
 *dest=0;
 return src;
}

void find_user(int fuid)
{
 FILE *fp;
 char str[140];
 int uid;
 if((fp=popen("users.xnc","r"))!=NULL)
 {
  while(fgets(str,139,fp))
  {
   sscanf(str,"%s %d",ch_user,&uid);
   if(uid==fuid) break;
  }
  pclose(fp);
 }
  if(uid!=fuid)
   sprintf(ch_user,"%d",fuid);
}
 
Pixmap picon;
Pixmap piconmask;
extern int load_etags(); //in editor.cxx

ConfigManager *iniman;

void InitXClient(int argc,char** argv,int wx,int wy,int wl,int wh,char
 *winname)
{
 int i,ppx,ppy;
 uint ppl,pph;
 char *rmt;
 XSetWindowAttributes xswa;
 XWindowAttributes xwinattr;
 fprintf(stderr,"XClient...\n");
 make_home_file_dir();
 for(i=1;i<argc;i++)
  if(strcmp("-display",argv[i])==0 || strcmp("-d",argv[i])==0) dispchr=argv[i+1];
 if((disp=XOpenDisplay(dispchr))==NULL)
 {
  fprintf(stderr,"%s: connection to X server failed on display %s.....\n",argv[0],dispchr);
  exit(1);
 }
#ifndef OLD_STYLE
 struct passwd *pwent;
 pwent=getpwuid(getuid());
 strcpy(ch_user,pwent->pw_name);
 endpwent();
#else
 find_user(getuid());
#endif
 strcat(winname,"  User: ");
 strcat(winname,ch_user);
 strcat(icname," : ");
 strcat(icname,ch_user);
 scr=DefaultScreen(disp);
 XrmInitialize();
 fprintf(stderr,"Loading resources....");
 strcpy(rfile,home_files_dir);
 strcat(rfile,"/ives.ini");
 strcpy(cfindbuf, home_files_dir);
 strcat(cfindbuf, "/TAGS");
 
 iniman= new ConfigManager(rfile);

 load_etags();
 rdb=XrmGetFileDatabase(rfile);
 if(rdb!=NULL)
  for(i=0;i<resn;i++)
  {
   fprintf(stderr,".");
   if(XrmGetResource(rdb,resources[i].name,resources[i].cname,&rmt,&rmv))
    strcpy(resources[i].pstr,rmv.addr);
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
 XParseGeometry(viewgeom,&viewx,&viewy,&viewl,&viewh);
 defcmp=DefaultColormap(disp,scr);
 if(str_casecmp(ret_foc,"yes")==0) return_focus=1; else return_focus=0;
 if(str_casecmp(shadchr,"no")==0) shadow=0; else shadow=1;

 load_color_schema(iniman);
 selectpix=cursor_color;

 if((fontstr=XLoadQueryFont(disp,resources[3].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],fontchr);
   exit(1);
  }

 if((fixfontstr=XLoadQueryFont(disp,resources[5].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],lfontchr);
   exit(1);
  }
 if((vfontstr=XLoadQueryFont(disp,resources[7].pstr))==NULL)
  {
   fprintf(stderr,"%s: Error loading font %s\n",argv[0],lfontchr);
   exit(1);
  }

 fprintf(stderr,"OK\n");

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
 pch->res_name="ives";
 pch->res_class="IVES";
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
 picon=XCreateBitmapFromData(disp,Main,(char*)ivesgray_bits,ivesgray_width,ivesgray_height);
 piconmask=XCreateBitmapFromData(disp,DefaultRootWindow(disp),(char*)ivesmask_bits,ivesmask_width,ivesmask_height);
 pxwmh->flags=(InputHint|StateHint|IconPixmapHint|IconMaskHint);
 pxwmh->input=True;
 pxwmh->initial_state=IconicState;
 pxwmh->icon_pixmap=picon;
 pxwmh->icon_mask=piconmask;
 XSetWMProperties(disp,Main,&wname,&iname,argv,argc,pxsh,pxwmh,pch);
 
 gcv.font=fontstr->fid;
 gcv.foreground=fgpix;
 gcv.background=bgpix;
 gc=XCreateGC(disp,Main,(GCFont | GCForeground | GCBackground),&gcv);
 xswa.colormap=defcmp;
 xswa.bit_gravity=CenterGravity;
 XChangeWindowAttributes(disp,Main,(CWColormap | CWBitGravity),&xswa);
 XSelectInput(disp,Main,ExposureMask | FocusChangeMask | StructureNotifyMask);
 XGetWindowAttributes(disp,DefaultRootWindow(disp),&xwinattr);
 ScreenL=xwinattr.width;
 ScreenH=xwinattr.height;
 if(strcmp(syspath,".")==0 || strcmp(syspath, "auto")==0) 
          strcpy(syspath,home_files_dir);
 
 XMapWindow(disp,Main);
}

void DeinitXClient()
{
// XFreePixmap(disp,picon);
// XFreePixmap(disp,piconmask);
 delall_el();
 delall_exp();
 XFreeGC(disp,gc);
 XUnloadFont(disp,fontstr->fid);
// XUnloadFont(disp,lfontstr->fid);
 XDestroyWindow(disp,Main);
 XCloseDisplay(disp);
 exit(0);
}


Window create_win(char *winname,int wx,int wy,int wl,int wh,int mask)
{
 int i;
 Window W;
 XSetWindowAttributes xswa;
 pxsh->flags=(PPosition | PSize);
 if(win_resize==0)
  pxsh->flags|=(PMinSize | PMaxSize);
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
                          0,fgpix,keyscol[1]);
 pch->res_name="ives";
 pch->res_class="IVES";
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

Window create_win(char *winname,int wx,int wy,int wl,int wh,int mask,int)
{
 return create_win(winname,wx,wy,wl,wh,mask);
}

void win_nodecor(Window W)
{
 XSetWindowAttributes xswa;
 xswa.override_redirect=True;
 XChangeWindowAttributes(disp,W,CWOverrideRedirect,&xswa);
}
 
//Dummy funcs
char  *finduser(int uid)
{
    return "unknown";
}

char  *findgroup(int uid)
{
    return "unknown";
}

