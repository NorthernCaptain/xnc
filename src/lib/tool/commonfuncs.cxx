/****************************************************************************
*  Copyright (C) 1998 by Leo Khramov
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

//This file contains common functions for most of all XNC files.

#include "globals.h"
#include "xh.h"
#include "au_sup.h"
#include "commonfuncs.h"
#include "c_externs.h"

//This function checks file and if touch interval more than timeout
//then it return YES, otherwise NO
int is_cache_out_time(char *path, int timeout)
{
 struct stat dstat;
 time_t t;
 if(stat(path, &dstat)==-1)
         return YES;
 time(&t);
 if(t-dstat.st_mtime>timeout)
         return YES;
 return NO;
}

int path_to_fullpath(char *dest, char *src)
{
  int l;
  if(*src!='/')
  {
    getcwd(dest, 1024);
    l=strlen(dest);
    if(dest[l-1]!='/')
        strcat(dest, "/");
    strcat(dest,src);
  } else
    strcpy(dest,src);
  l=strlen(dest);
  if(dest[l-1]!='/')
    {
      dest[l]='/';
      dest[l+1]=0;
    }
  return l+1;
}

int path_cmp(char *dir1, char *dir2)
{
   char d1[L_MAXPATH];
   char d2[L_MAXPATH];
   path_to_fullpath(d1,dir1);
   path_to_fullpath(d2,dir2);
   return strcmp(d1,d2);
}


//Add new content to 'curdir' path

//!!!!!FIX ME here:
//On VMS machine we just add filename not directory content...
//need to detect filename of dir and make correct addition...
void   add_path_content(char *curdir, const char *cont, int vms_host)
{
  if(!vms_host)
  {
      if (curdir[strlen(curdir) - 1] != '/')
        strcat(curdir, "/");
  }
  strcat(curdir, cont);
}

//No comments
void   upper_path(char *curdir, int vms_host)
{
  if(!vms_host)
  {
      char  *bb = strrchr(curdir, '/');
      if(bb)
          *bb = 0;
      if (strlen(curdir) == 0)
        strcpy(curdir, "/");
  } else
  {
      char *bb=strrchr(curdir, ']');
      if(bb)
      {
          char *bb2=bb;

          if(*(bb+1)!=0)
          {
            *(bb+1)=0;
            return;
          }
          
          while(*bb!='.' && *bb!='[' && bb!=curdir)
              bb--;
          if(*bb=='[' || bb==curdir)
              return;    //Return because this is the top directory

          while(*bb2!=0)
            *bb++=*bb2++;
          *bb=0;
      }
  }          
}

//Get first content of dirname
const char*   get_first_content(const char *curdir, char *cont)
{
        const char *b=strchr(curdir,'/');
        if(b)
          {
                  strncpy(cont,curdir,b-curdir);
                  cont[b-curdir]=0;
                  return b+1;
          }
        strcpy(cont,curdir);
        return NULL;
}

//Get last content of dirname
//Added return values even if only one content present
void   get_last_content(char *curdir, char *cont)
{
  char  *bb;
  int    l = strlen(curdir);
  if (l == 1)
    {
      strcpy(cont, curdir);
      return;
    };
  if (curdir[l - 1] != '/')
    bb = strrchr(curdir, '/');
  else
    {
      curdir[l - 1] = 0;
      bb = strrchr(curdir, '/');
      curdir[l - 1] = '/';
    }
  if (bb != 0)
    strcpy(cont, bb + 1);
  else
    strcpy(cont,curdir); //Only one content
  l=strlen(cont);
  if(cont[l-1]=='/')
    cont[l-1]=0;
}

void  get_last_and_rest_content(char* curdir, char* last, char* rest)
{
  char  *bb;
  int    l = strlen(curdir);
  if (l == 1)
    {
      strcpy(last, curdir);
      *rest=0;
      return;
    }
  if (curdir[l - 1] != '/')
    bb = strrchr(curdir, '/');
  else
    {
      curdir[l - 1] = 0;
      bb = strrchr(curdir, '/');
      curdir[l - 1] = '/';
    }
  if (bb != NULL)
  {
    strcpy(last, bb + 1);
    *bb=0;
    strcpy(rest,curdir);
    *bb='/';
  } else
  {
    strcpy(last,curdir);
    *rest=0;
  }
  l=strlen(last);
  if(last[l-1]=='/')
    last[l-1]=0;
}

void str_swap(char *str1, char *str2)
{
 char c;
 while(*str1 && *str2)
 {
   c=*str1;
   *str1++=*str2;
   *str2++=c;
 }
 if(*str2)
  while(*str2)
  {
   c=*str1;
   *str1++=*str2;
   *str2++=c;
  }
 else
  if(*str1)
    while(*str1)
    {
      c=*str1;
      *str1++=*str2;
      *str2++=c;
    }
 *str1=*str2=0;
}

//Parse string 'dddd: eeeee' and give parts 'dddd' and 'eeeee'
char  *getseq(char *str, char *w, int nocase)
{
  char *orig_w=w;
  while ((*str == ' ' || *str == ':') && *str != 0)
    str++;
  while (*str != 0 && *str != '\n' && *str != '\r' && *str != ':')
    if (nocase)
      *w++ = tolower(*str++);
    else
      *w++ = *str++;
  w--;
  while(*w==' ' && w>=orig_w)
      w--;
  w++;
  *w = 0;
  return str;
}

//Parse and return rest of line (call after getseq if you want rest of line)
char  *getrest(char *str,char *w, int nocase)
{
  while(*str==' '|| *str == ':')
    str++;
  while(*str && *str!='\n' && *str!='\r')
    if(nocase)
      *w++ = tolower(*str++);
    else
      *w++=*str++;
  w--;
  while(*w==' ')
      w--;
  w++;
  *w=0;
  return str;
}


//--------------------------------------------------------------------------
//Does name *s match filter *ff (1 - if it does, 0 - doesn't)
//--------------------------------------------------------------------------
int    is_filtered(char *s, char *ff)
{
  char  *f="";
  while (*s != 0)
  {
    f = ff;
    if (*f == '*')
    {
      f++;
      while (*f != 0 && *s != 0)
	if (*s == *f)
	  break;
	else
	  s++;
      if (*f == 0)
	return 1;
    }
    while (*s != 0 && *f != 0 && *f != '*')
      if (*f == '?')
      {
	f++;
	s++;
      }
      else if (*s != *f)
      {
	if (*ff == '*')
	  break;
	else
	  return 0;
      }
      else
      {
	s++;
	f++;
      };
    
    if (*f == '*')
      ff = f;
  }
  if (*f == 0 || *f == '*')
    return 1;
  return 0;
}


//////////////////////////Alarm functions//////////////////////
const int max_alarms=10;

static void (*alarm_funcs[max_alarms])(int);
static int    xnc_active_alarms=0;

void init_xnc_alarm()
{
    for(int i=0;i<max_alarms;i++)
        alarm_funcs[i]=0;
}


static void xnc_alarm(int)
{
  xncdprintf(("xnc_alarm got: active [%d]\n",xnc_active_alarms));
    for(int i=0;i<max_alarms;i++)
        if(alarm_funcs[i])
            alarm_funcs[i](0);
    if(xnc_active_alarms)
    {
        signal(SIGALRM, xnc_alarm);
        alarm(1);
    }
}

static void set_alarm_signal()
{
    alarm(0);
    signal(SIGALRM, xnc_alarm);
    alarm(1);
}

static int  find_xnc_alarm_func(void (*func)(int))
{
    for(int i=0;i<max_alarms;i++)
        if(alarm_funcs[i]==func)
            return i;
    return -1;
} 

int set_xnc_alarm(void (*func)(int))
{
   int n=find_xnc_alarm_func(func);
   if(n==-1)
   {
       for(int i=0;i<max_alarms;i++)
           if(alarm_funcs[i]==NULL)
           {
               alarm_funcs[i]=func;
               if(xnc_active_alarms==0)
                   set_alarm_signal();
               xnc_active_alarms++;
               return 1;
           }
   }
   return 0;
}

void remove_xnc_alarm(void (*func)(int))
{
    int n=find_xnc_alarm_func(func);
    if(n!=-1)
    {
        alarm_funcs[n]=NULL;
        xnc_active_alarms--;
    }
}


//---------------------------------Convertation for dnd------------------------//

int hex_one_digit(char ch1)
{
  char ch=toupper(ch1);
  char *hex="0123456789ABCDEF";
  int  i;
  for(i=0;i<16;i++)
    if(ch==hex[i])
      return i;
  return -1;
}

void convert_one_filename(char* from, char *to)
{
  int l=strlen(from),i,n;
  for(i=0;*from;i++)
  {
    if(*from=='%' && i+2<l)
    {
      n=hex_one_digit(*(from+1))*16+hex_one_digit(*(from+2));
      if(n>=0 && n<256)
      {
	*to++=n;
	from+=3;
	continue;
      }
    }
    if(*from!='\r' && *from!='\n')
      *to++=*from++;
    else
      from++;
  }
  *to=0;
}


//----------------------------------Error functions------------------------------

char   last_info_message[256];
char   last_error_message[256] = "UNKNOWN Error";

char   vfserr[256];                //String with error message from VFS

char   vfshead[80];                //String with vfs header for vfserr

void   vfs_error(char *h, char *s)
{
  strcpy(vfshead, h);
  strcat(vfshead, " Error");
  strcpy(vfserr, s);
  sprintf(last_error_message, "Error: %s", s);
}

void   errno2mes()
{
  sprintf(last_error_message, "Error: %s", strerror(errno));
}

void   set_error(char *err)
{
  strcpy(last_error_message, err);
}


//--------------------------------File functions--------------------------------
char* get_sub_content(char* from, char* sub)
{
  while(*from=='/') from++;
  while(*from && *from!='/')
    *sub++=*from++;
  *sub=0;
  return from;
}


//Get from and break it to real DFS 'realdir' plus 'realfile' and rest if exist
void break_to_real_dirfile(char* from, char* realdir, char* realfile, char* rest)
{
  char str[L_MAXPATH];
  struct stat bstat;
  char *rd=realdir;
  char *fr=from;
  *realdir=0;
  *realfile=0;
  *rest=0;

  while(1)
  {
    strcat(realdir,"/");
    fr=get_sub_content(fr,str);
    strcat(realdir,str);
    if(stat(realdir,&bstat)==-1) //We are in non-real part of path
    {
      rd=strrchr(realdir,'/');
      *(++rd)=0;
      *rest='/';
      strcpy(rest,str);
      return;
    }
    if((bstat.st_mode & S_IFDIR)!=S_IFDIR) //this is normal file
    {
      rd=strrchr(realdir,'/');
      strcpy(realfile,++rd);
      *rd=0;
      strcpy(rest,fr);
      return;
    }
    if(*fr==0)
      return;
  }
}

//-----------------------------Other stuff ----------------------------------//

void str2version(char *str,int *ver)
{
        int version=0,major=0,minor=0;
        sscanf(str,"%d.%d.%d",&version,&major,&minor);
        *ver=version*100+major*10+minor;
}


//------------------------------End of file----------------------------------//


