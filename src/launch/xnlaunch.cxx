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
#include "image.h"
#include "lbutton.h"
#include "baseguiplugin.h"

char *vers="v1.0.2";

char home_files_dir[1024];

extern void prect(Window w,GC& gcw,int x,int y,int l,int h);
extern void urect(Window w,GC& gcw,int x,int y,int l,int h);
extern void loadmenu(char *menuname);

LaunchButton *but1;
int mx,my;

void MainExpose()
{
 GC gcw;
 XRaiseWindow(disp,Main);
 gcw=XCreateGC(disp,Main,0,NULL);
 XSetFont(disp,gcw,fontstr->fid);
 urect(Main,gcw,0,0,Mainl-1,15);
 XSetForeground(disp,gcw,cols[2]);
 XDrawString(disp,Main,gcw,vcenterx-XTextWidth(fontstr,"Go",2)/2,
   (15+fontstr->max_bounds.ascent-fontstr->max_bounds.descent)/2,"Go",2);
 XFreeGC(disp,gcw);
}

void sigchild(int)
{
 signal(SIGCHLD,sigchild);
 wait(NULL);
}

void XEVENT()       
{
 Gui *o;
 int fiflg=1;
  XNextEvent(disp,&ev);
  if(ev.xany.window!=Main)
  {
   if(ev.type==Expose)
   {
   if(ev.xexpose.count==0)
   {
    o=findexp(ev.xany.window);
    if(o) o->click();
   }
   }
   else {
   o=find(ev.xany.window);
   if(o) o->click();
        }
  }
 else
 switch(ev.type)
 {
  case Expose:
   if(ev.xexpose.count==0)
      MainExpose();
      break;
  case ButtonPress:
    if(ev.xbutton.button==Button3)
    {
     mx=ev.xbutton.x;
     my=ev.xbutton.y;
    }
    else
       XRaiseWindow(disp,Main);XFlush(disp);
    break;
  case MotionNotify:
    XMoveWindow(disp,Main,ev.xmotion.x_root-mx,ev.xmotion.y_root-my);
    XFlush(disp);
    break;
  case MapNotify:
     break;
   case UnmapNotify:
     break;
  };
/*   else
   if(ev.type==FocusIn || (ev.type==Expose && fiflg==1)) 
   { if(last) XSetInputFocus(disp,last->w,RevertToNone,CurrentTime);fiflg=0;}
*/}

int pid;
int main(int argc,char** argv)
{
 char name[40]="XnLaunch v1.0";
 fprintf(stderr,"Initialisation:\n");
 margc=argc;margv=argv; 
 InitXClient(argc,argv,900,50,70,290,name);
 init_basegui_plugin();
 vcenterx=Mainl/2;
 pid=fork();
 if(pid==-1)
 {
  fprintf(stderr,"Can't run XnLaunch\n");
  exit(1);
 }
 if(pid!=0)
 {
  fprintf(stderr,"Running in background\n");
  disp=NULL;
  exit(0);
 } else
 {
 init_imagengine();
 signal(SIGCHLD,sigchild);
 loadmenu(".launch.menu");
 fprintf(stderr,"X Northern Launch Pad %s\nCopyright 1997 Leonid V. Khramov aka Leo <leo@pop.convey.ru>\n",vers);
 while(!done)  XEVENT();
 fprintf(stderr,"Exiting from XnLaunch....");
 fprintf(stderr,"OK\n");
 DeinitXClient();
 }
 return 0;
}

Switch* guiplugin_new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)())
{
  return new Switch(ix,iy,il,iname,icol,ifunc);
}
