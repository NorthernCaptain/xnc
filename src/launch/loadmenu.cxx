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

char str1[512];
LaunchButton *buts[100];
int butmax=0;
extern int vcenterx;
int not_empty_or_comments(char* s)
{
 if(*s=='#') return 0;
 if(*s=='\n') return 0;
 if(*s==0) return 0;
 return 1;
}

char *find_chars(char *str,char *s)
{
 char *s0;
 while(*str!=0)
 {
  s0=s;
  while(*s0!=0)
   if(*str==*s0) return str;
   else s0++;
  str++;
 }
 return NULL;
}

void cp_chars(char *dest,char *src,char end)
{
 while(*src!=end && *src!=0 && *src!='\n') *dest++=*src++;
 *dest=0;
}

void gen_pos(int& x,int& y,int max)
{
 if(padh>padl)
 {
  if(padl==1)
  {
   x=0;
   y=max;
  }
  else{
  x=max/padh;
  y=max%padh;
  }
 } else
 {
  if(padh==1)
  {
   x=max;
   y=0;
  } else
  {
  x=max%padl;
  y=max/padl;
  }
  }
  x*=butl;
  y=y*buth+16;
}

void loadmenu(char *menuname)
{
 FILE* fp;
 char *s1;
 char s2[100];
 char s3[100];
 char s4[100];
 int i,x,y;
 sprintf(str1,"%s/.xlaunch/%s",getenv("HOME"),menuname);
 fp=fopen(str1,"r");
 if(fp==NULL)
 {
  fprintf(stderr,"Error: Can't find menu file '%s'!\n",str1);
  exit(1);
 }
 while(fgets(str1,500,fp)!=NULL)
 {
  if(not_empty_or_comments(str1))
  {
   s1=find_chars(str1,"\":");
   if(s1==NULL) fprintf(stderr,"Warning: Wrong format! [%s]\n",str1);
   else
   {
    if(*s1!=':')
    {
     cp_chars(s2,s1+1,'\"');
     s1=find_chars(s1+1,":");
     s1++;
    } else
    {s2[0]=0;s1++;};
     while(*s1==' ') s1++;
     i=0;
     while(*s1!=0 && *s1!=':' && *s1!='\n' && *s1!=' ') s3[i++]=*s1++;
     s3[i]=0;
     s1=find_chars(s1,":");
     s1++;
     while(*s1==' ') s1++;
     cp_chars(s4,s1,':');
     gen_pos(x,y,butmax);
     if(*s2=='\0') buts[butmax]=new LaunchButton(x,y,butl,buth,NULL,s4,s3);
     else
     if(*s3=='\0') buts[butmax]=new LaunchButton(x,y,butl,buth,s2,s4,NULL);
     else buts[butmax]=new LaunchButton(x,y,butl,buth,s2,s4,s3);
     buts[butmax]->init(Main);
     buts[butmax]->show();
     butmax++;
   }
  }
 }
 fclose(fp);
 x+=butl;y+=buth;
 XResizeWindow(disp,Main,(uint)x,(uint)y);
 Mainl=x;Mainh=y;vcenterx=Mainl/2;
 XFlush(disp);
} 
