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
#include "lbutton.h"
#include <sys/wait.h>

char *arg[30];
void fork_exec(char *s)
{
 int max=0,i;
 int pid;
 for(i=0;i<30;i++)
  arg[i]=NULL;
 while(*s!=0)
 {
 i=0;
 arg[max]=new char[60];
 while(*s!=0 && *s!=' ')
 if(*s!='"')
   arg[max][i++]=*s++;
 else
 {
  arg[max][i++]=*s++;
  while(*s!='"' && *s!=0)
    arg[max][i++]=*s++;
  if(*s=='"') arg[max][i++]=*s++;
 }
 arg[max][i]=0;
 while(*s==' ' && *s!=0) s++;
 max++;
 }
 pid=fork();
 if(pid==0)
 {
  execvp(arg[0],(char**)arg);
  exit(1);
 }
 else
  for(i=0;i<30;i++)
   if(arg[i]) delete arg[i];
}
 

LaunchButton::LaunchButton(int ix,int iy,int il,int ih,char *iname,char *ilaunch,char *imname)
{
 char str[512];
 x=ix;y=iy;l=il;h=ih;name=iname;launch=ilaunch;
 if(iname) {name=new char[strlen(iname)+1];strcpy(name,iname);};
 if(ilaunch) {launch=new char[strlen(ilaunch)+1];strcpy(launch,ilaunch);};
 if(imname)
 {
 im=LoadPixmap(imname,F_AUTO,(int&)pl,(int&)ph,CMP_COM);
 if(im==0)
 {
 sprintf(str,"%s/.xlaunch/%s",getenv("HOME"),imname);
 im=LoadPixmap(str,F_AUTO,(int&)pl,(int&)ph,CMP_COM);
 if(im==0)
  {
   sprintf(str,"%s/%s",searchpath,imname);
   im=LoadPixmap(str,F_AUTO,(int&)pl,(int&)ph,CMP_COM);
  }
 }
 } else im=0;
 px=(l-pl)/2;
 py=(h-ph)/2;
}

void LaunchButton::init(Window ipar)
{
 parent=ipar;
 if(name)
 {
  tl=strlen(name);
  tw=XTextWidth(fontstr,name,tl);
 }
 else tw=tl=0;
 w=XCreateSimpleWindow(disp,parent,x,y,l,h,0,0,keyscol[1]);
 gcv.background=keyscol[1];
 gcv.font=fontstr->fid;
 gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
 XSelectInput(disp,w,
 ButtonPressMask | ExposureMask | ButtonReleaseMask | LeaveWindowMask |
 EnterWindowMask | OwnerGrabButtonMask);
 tx=(l-tw)/2;
 ty=(h+fontstr->max_bounds.ascent-fontstr->max_bounds.descent)/2;
 th=fontstr->max_bounds.ascent-fontstr->max_bounds.descent;
}

void LaunchButton::expose()
{
   XClearWindow(disp,w);
   XSetForeground(disp,gcw,keyscol[2]);
   XDrawLine(disp,w,gcw,0,0,l,0);
   XDrawLine(disp,w,gcw,0,0,0,h);
   XSetForeground(disp,gcw,keyscol[0]);
   XDrawLine(disp,w,gcw,0,h-1,l,h-1);
   XDrawLine(disp,w,gcw,l-1,0,l-1,h);
   if(im) XCopyArea(disp,im,w,gcw,0,0,pl,ph,px,py);
   if(name)
   {
   if(im==0)
   {
   XSetForeground(disp,gcw,cols[0]);
   XDrawString(disp,w,gcw,tx+1,ty+1,name,tl);
   XSetForeground(disp,gcw,cols[1]);
   XDrawString(disp,w,gcw,tx,ty,name,tl);
   } else
   {
   XSetForeground(disp,gcw,cols[0]);
   XSetBackground(disp,gcw,keyscol[1]);
   XDrawImageString(disp,w,gcw,tx+1,h-th/2+1,name,tl);
   XSetForeground(disp,gcw,cols[1]);
   XDrawString(disp,w,gcw,tx,h-th/2,name,tl);
   }
   }
}

void LaunchButton::press()
{
   XClearWindow(disp,w);
   XSetForeground(disp,gcw,keyscol[0]);
   XDrawLine(disp,w,gcw,0,0,l,0);
   XDrawLine(disp,w,gcw,0,0,0,h);
   XDrawLine(disp,w,gcw,1,1,l-1,1);
   XDrawLine(disp,w,gcw,1,1,1,h-1);
   XSetForeground(disp,gcw,keyscol[2]);
   XDrawLine(disp,w,gcw,0,h-1,l,h-1);
   XDrawLine(disp,w,gcw,l-1,0,l-1,h);
   XDrawLine(disp,w,gcw,1,h-2,l-1,h-2);
   XDrawLine(disp,w,gcw,l-2,1,l-2,h-1);
   if(im) XCopyArea(disp,im,w,gcw,0,0,pl,ph,px+1,py+1);
   if(name)
   {
   if(im==0)
   {
   XSetForeground(disp,gcw,cols[0]);
   XDrawString(disp,w,gcw,tx+2,ty+2,name,tl);
   XSetForeground(disp,gcw,cols[1]);
   XDrawString(disp,w,gcw,tx,ty,name,tl);
   } else {
   XSetForeground(disp,gcw,cols[0]);
   XDrawString(disp,w,gcw,tx+2,h-th/2+2,name,tl);
   XSetForeground(disp,gcw,cols[1]);
   XDrawString(disp,w,gcw,tx,h-th/2,name,tl);
   }
   }
}


void LaunchButton::click()
{
 if(ev.xany.window==w)
 {
  switch(ev.type)
  {
  case Expose: expose();break;
  case ButtonPress:
   if(ev.xbutton.button==Button1)
   {
   press();
   prflg=1;
   }
   break;
  case ButtonRelease:
   if(ev.xbutton.button==Button1 && prflg==1)
   {
   prflg=0;
   expose();
   if(launch) fork_exec(launch);
   }
   break;
  case LeaveNotify:
   if(prflg)
   {
    prflg=0;
    expose();
   }
   break;
  case EnterNotify:
   if(prflg==0 && ev.xcrossing.state==Button1Mask)
   {
    prflg=1;
    press();
   };break; 
  };
 }
}

