/****************************************************************************
*  Copyright (C) 2000 by Leo Khramov
*  email:     leo@xnc.dubna.su
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
 ****************************************************************************/
/* $Id: systemfuncs.cxx,v 1.3 2002/09/26 15:04:26 leo Exp $ */
#include "globals.h"
#include "systemfuncs.h"
#include "au_sup.h"

extern "C" void set_cmdchild_signal();
extern "C" int is_term_on();
extern "C" void tt_printf(char *fmt,...);

char  *mnt[256];
int    mntmax = 0;

int    mainuid, maingid;  //uid and gid of current user

void   load_mnt()
{
  FILE  *fp;
  fprintf(stderr, "Scaning for mounting devices");
#ifdef HAVE_MNTENT_H
  mntent *mntstr;
#ifdef MOUNTED
  if ((fp = setmntent(MOUNTED, "r")) != NULL)
#else
  if ((fp = setmntent(MNTTAB, "r")) != NULL)
#endif
    {
      while ((mntstr = getmntent(fp)) != NULL)
        {
          mnt[mntmax] = new char[strlen(mntstr->mnt_dir) + 1];
          strcpy(mnt[mntmax], mntstr->mnt_dir);
          mntmax++;
          fprintf(stderr, ".");
        }
      endmntent(fp);
      fprintf(stderr, "OK\n");
    }
  else
    fprintf(stderr, ".....failed\n");
#else
  char   str[256];
  char   s1[256], *s2;
  signal(SIGCHLD,SIG_DFL);
  fp = popen("mount", "r");
  if (fp == NULL)
    {
      fprintf(stderr, "...disabled [Porting problems]\n");
      return;
    }
  mntmax = 0;
  while (fgets(str, 255, fp) != NULL)
    {
      s2 = strscan2(str, s1);
      s2 = strscan2(s2, s1);
      strscan2(s2, s1);
      mnt[mntmax] = new char[strlen(s1) + 1];
      strcpy(mnt[mntmax], s1);
      mntmax++;
      fprintf(stderr, ".");
    }
  fprintf(stderr, "OK\n");
  pclose(fp);
  set_cmdchild_signal();
#endif
}

char  *findmntent(char *p)
{
  for (int i = 0; i < mntmax; i++)
    if (strcmp(mnt[i], p) == 0)
      return mnt[i];
  return NULL;
}

void   del_mnt()
{
  for (int i = 0; i < mntmax; i++)
    delete mnt[i];
}



USR    users;
USR    groups;
char   unuser[] = "unknown";

void   init_users()
{
  FILE  *fp;
  USR   *us;
  char   str[140];
  fprintf(stderr, "Scaning users");
#ifndef OLD_STYLE
  while ((pw = getpwent()) != NULL)
    {
      fprintf(stderr, ".");
      us = new USR;
      strcpy(us->name, pw->pw_name);
      us->uid = pw->pw_uid;
      us->gid = pw->pw_gid;
      us->next = users.next;
      users.next = us;
    }
  endpwent();
#else
  signal(SIGCHLD,SIG_DFL);
  fp = (FILE *) popen("users.xnc", "r");
  if (fp)
    {
      while (fgets(str, 140, fp) != NULL)
        {
          us = new USR;
          sscanf(str, "%s %d %d", us->name, &us->uid, &us->gid);
          us->next = users.next;
          users.next = us;
          fprintf(stderr, ".");
        }
      pclose(fp);
      set_cmdchild_signal();
      fprintf(stderr, "OK\n");
    }
  else
    fprintf(stderr, "Can't scan users!!!\n");
#endif
  fprintf(stderr, "Scaning groups");
#ifndef OLD_STYLE
  while ((gr = getgrent()) != NULL)
    {
      fprintf(stderr, ".");
      us = new USR;
      strcpy(us->name, gr->gr_name);
      us->uid = gr->gr_gid;
      us->next = groups.next;
      groups.next = us;
    }
  endgrent();
  fprintf(stderr, "OK\n");
#else
  signal(SIGCHLD,SIG_DFL);
  fp = (FILE *) popen("groups.xnc", "r");
  if (fp)
    {
      while (fgets(str, 140, fp) != NULL)
        {
          us = new USR;
          sscanf(str, "%s %d", us->name, &us->uid);
          us->next = groups.next;
          groups.next = us;
          fprintf(stderr, ".");
        }
      pclose(fp);
      set_cmdchild_signal();
      fprintf(stderr, "OK\n");
    }
  else
    fprintf(stderr, "Can't scan groups!!!\n");
#endif
}

void   deinit_users()
{
  USR   *us;
  while (users.next != NULL)
    {
      us = users.next->next;
      delete users.next;
      users.next = us;
    }
  while (groups.next != NULL)
    {
      us = groups.next->next;
      delete groups.next;
      groups.next = us;
    }
}

char  *finduser(int uid)
{
  USR   *us = users.next;
  while (us != NULL)
    if (us->uid == uid)
      return us->name;
    else
      us = us->next;
  return unuser;
}

int    finduserid_by_name(char *n)
{
  USR   *us = users.next;
  while (us != NULL)
    if (strcmp(us->name, n) == 0)
      return us->uid;
    else
      us = us->next;
  return -1;
}

char  *(*finduserbyuid) (int) = finduser;

char  *findgroup(int uid)
{
  USR   *us = groups.next;
  while (us != NULL)
    if (us->uid == uid)
      return us->name;
    else
      us = us->next;
  return unuser;
}

int    findgroupid_by_name(char *n)
{
  USR   *us = groups.next;
  while (us != NULL)
    if (strcmp(us->name, n) == 0)
      return us->uid;
    else
      us = us->next;
  return -1;
}



char   shdir[1024] = "";
void   senddir(char *d)
{
  static char tmpdir[L_MAXPATH];
  if (strcmp(shdir, d) != 0)
  {
    if(is_term_on())
    {
      quote_path(tmpdir,d);
      tt_printf("cd %s\n", tmpdir);
    }
    else
      chdir(d);
    strcpy(shdir, d);
  }
}

int    is_exec(FList * o)
{
  if (mainuid == maingid && mainuid == 0)        //Check we are root or no

  {
    if ((o->mode & S_IXUSR) != 0 || (o->mode & S_IXGRP) != 0 || (o->mode & S_IXOTH) != 0)
      return 1;
    else
      return 0;
  }
  if (((o->mode & S_IXUSR) != 0 && o->uid == mainuid) ||
      ((o->mode & S_IXGRP) != 0 && o->gid == maingid) ||
      (o->mode & S_IXOTH) != 0)
    return 1;
  return 0;
}

void   printperm(char *str, int mode)
{
  char   perm[] = "rwxrwxrwx";
  int    i = 0, mask = 0400;
  if ((mode & S_IFLNK) == S_IFLNK)
    str[i++] = 'l';
  else if ((mode & S_IFDIR) == S_IFDIR)
    str[i++] = 'd';
  else
    str[i++] = '-';
  while (mask)
  {
    if (mode & mask)
      str[i] = perm[i - 1];
    else
      str[i] = '-';
    mask >>= 1;
    i++;
  };
  str[i] = 0;
}

int   dig2ascii2(char *chr, long_size_t n, int radix)
{
  int    i=0, j, k;
  char   cchr[20];
  if(radix==8)
    *chr++='0';
  do
  {
    k=n%(long_size_t)radix;
    n/=(long_size_t)radix;
    cchr[i++]=k+'0';
  } while(n);
  if(i>1 && k==0)
    i--;
  for (j = 0; j < i; j++)
    chr[j]=cchr[i-j-1];
  chr[j] = 0;
  return j;
}

void   dig2ascii(char *chr, long_size_t n, int radix)
{
  int    i=0, j, k;
  char   cchr[100];
  //  k = dig2ascii2(cchr, n);
  do
  {
    k= n%(long_size_t)radix;
    n/=(long_size_t)radix;
    cchr[i++]=k+'0';
  } while(n);
  if(i>1 && k==0)
    i--;
  k=i;
  for (i = 0, j = 0; i < k; i++, j++)
  {
    if (((k - i) % 3) == 0 && i != 0)
    {
      chr[j] = '.';
      j++;
    };
    chr[j] = cchr[k-i-1];
  }
  chr[j] = 0;
}

void   dig2ascii_r(char *chr, long_size_t n, int total, int radix, char fill)
{
  int    i=0, j, k;
  char   cchr[20];
  memset(chr, fill, total);
  do
  {
    k=n%(long_size_t)radix;
    n/=(long_size_t)radix;
    cchr[i++]=k+'0';
  } while(n);
  if(i>1 && k==0)
    i--;
  if(i>total)  //Numeric greater than total digits in array ;(
  {
    chr[0]=0;
    return;
  }
  for (j = 0; j < i; j++)
    chr[total-i+j]=cchr[i-j-1];
  chr[total-i+j] = 0;
}

char  *findsubstr(char *str, char *sub)
{
  int    n = strlen(str), i, j;
  for (i = 0; i < n - 1; i++)
  {
    j = 0;
    while (str[j + i] == sub[j] && str[j + i] != 0 && sub[j] != 0)
      j++;
    if (sub[j] == 0)
      return str + i;
  }
  return NULL;
}


int    is_exist(char *fname)
{
  struct stat dstat;
  if (stat(fname, &dstat) == -1)
    return 0;
  return 1;
}


void xnc_seekdir(DIR *d, int offset)
{
  while(offset && readdir(d))
    offset--;
}
