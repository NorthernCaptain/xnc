/****************************************************************************
*  Copyright (C) 1996-98 by Leo Khramov
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

//This file contains routines for FList class.
//This is base strcture used for storing directory listings inside XNC.

#include "xh.h"
#include "gui.h"
#include "flist.h"
#include "systemfuncs.h"
#include "au_sup.h"
#include "colors.h"

char* is_image(char* name, int len);

int        sort_order=1;        //If 1 then sorting in alphabetical order else in reverse order

char  *getext(char *str)
{
  int    l = strlen(str) - 1;
  if (l == 1 && str[l] == str[l - 1] && str[l] == '.')
    return NULL;
  while (l > 0 && str[l] != '.')
    l--;
  if (l > 0)
    return str + l;
  return NULL;
}


//*******************************************************************//
//Compare functions used for sorting dir listing in different ways.
//These functions Return the same values as 'strcmp' does.

//Compare two files by extensions.
//If extensions equal then compare by names.
int    extcmp(char *str1, int aa1, int aa2, char *str2, int aa3, int aa4)
{
  char  *d1, *d2;
  int    k;
  if(*str2==0)
    return 1;
  d1 = getext(str1);
  d2 = getext(str2);
  if (d1 == NULL)
  {
    if (d2 == NULL)
      return sort_order ? strcmp(str1, str2) : -strcmp(str1, str2);
    else
      return sort_order ? -1 : 1;
  }
  else if (d2 == NULL)
    return sort_order ? 1 : -1;
  k = sort_order ? strcmp(d1, d2) : -strcmp(d1, d2);
  if (k == 0)
  {
    *d1 = 0;
    *d2 = 0;
    k = extcmp(str1, aa1, aa2, str2, aa3, aa4);
    *d1 = '.';
    *d2 = '.';
    return k;
  }
  return k;
}

//Name compare i.e -> strcmp.
int    mystrcmp(char *str1, int aa1, int aa2, char *str2, int aa3, int aa4)
{
  if(*str2==0)
    return 1;
  return sort_order ? strcmp(str1, str2) : -strcmp(str1, str2);
}

//First compare file sizes,
//if equal then name compare.
int    sizecmp(char *str1, int aa1, int aa2, char *str2, int aa3, int aa4)
{
  if (str1[0] == 0)
    return -1;
  if (str2[0] == 0)
    return 1;
  else if (str1[0] == '.' && str1[1] == 0)
  {
    if (str2[0] == '.' && str2[1] == 0)
      return 0;
    return sort_order ? -1 : 1;
  }
  else if (str1[0] == '.' && str1[1] == '.' && str1[2] == 0)
  {
    if (str2[0] == '.' && str2[1] == 0)
      return sort_order ? 1 : -1;
    if (str2[0] == '.' && str2[1] == '.' && str2[2] == 0)
      return 0;
    return sort_order ? -1 : 1;
  }
  else if (str2[0] == '.' && str2[1] == 0)
  {
    if (str1[0] == '.' && str1[1] == 0)
      return 0;
    return sort_order ? 1 : -1;
  }
  else if (str2[0] == '.' && str2[1] == '.' && str2[2] == 0)
    return sort_order ? 1 : -1;
  if (aa1 == aa3)
    return sort_order ? strcmp(str1, str2) : -strcmp(str1, str2);
  return aa1 < aa3 ? (sort_order ? -1 : 1) : (sort_order ? 1 : -1);
}

//Compare last access time of files.
int    timecmp(char *str1, int aa1, int aa2, char *str2, int aa3, int aa4)
{
  if (str1[0] == 0)
    return -1;
  if (str2[0] == 0)
    return 1;
  else if (str1[0] == '.' && str1[1] == 0)
  {
    if (str2[0] == '.' && str2[1] == 0)
      return 0;
    return sort_order ? -1 : 1;
  }
  else if (str1[0] == '.' && str1[1] == '.' && str1[2] == 0)
  {
    if (str2[0] == '.' && str2[1] == 0)
      return 1;
    if (str2[0] == '.' && str2[1] == '.' && str2[2] == 0)
      return 0;
    return sort_order ? -1 : 1;
  }
  else if (str2[0] == '.' && str2[1] == 0)
  {
    if (str1[0] == '.' && str1[1] == 0)
      return 0;
    return sort_order ? 1 : -1;
  }
  else if (str2[0] == '.' && str2[1] == '.' && str2[2] == 0)
    return sort_order ? 1 : -1;
  if (aa2 == aa4)
    return sort_order ? strcmp(str1, str2) : -strcmp(str1, str2);
  return aa2 > aa4 ? (sort_order ? -1 : 1) : (sort_order ? 1 : -1);
}

//Unsorted - no comparision.
int    unsortcmp(char *str1, int aa1, int aa2, char *str2, int aa3, int aa4)
{
  if(*str2==0)
    return 1;
  return sort_order ? 1 : -1;
}




//Pointer to compare functions. Store current compare function.
int    (*compare) (char *, int, int, char *, int, int) = extcmp;



//**********************************************************************//


void find_user_and_group(char *touser, int uid, char* togroup, int gid)
{
  static int rem_uid=-1, rem_gid=-1;
  static char *rem_user=NULL, *rem_group=NULL;
  
  if(rem_uid!=uid)
  {
    rem_user=finduser(uid);
    rem_uid=uid;
  }
  strncpy(touser, rem_user, L_USER);
  touser[L_USER-1]=0;
  if(rem_gid!=gid)
  {
    rem_group=findgroup(gid);
    rem_gid=gid;
  }
  strncpy(togroup, rem_group, L_GROUP);
  togroup[L_GROUP-1]=0;
}

//------------------------------------------------------------------------------

//FList class functions. FList is a two way sorted list which stores dir listing.
//Constructor and initialization functions.
// *INDENT-OFF*        
FList::FList(char *iname, int imode, int is, int iuid, int igid, time_t t, 
	     struct stat* ist)
  // *INDENT-ON*        
{
  init(iname,imode,is,iuid,igid,t,ist);
};

void   FList::init(char *iname, int imode, int is, int iuid, int igid, 
		   time_t t, struct stat *ist)
{
    struct tm *ttm;
    next = prev = link = NULL;
    gid = igid;
    uid = iuid;
    options=0;
    time = t;
    ttm = localtime(&t);
    strftime(chr_time, FILE_TIMESTAMP, "%d-%b-%Y/%H:%M", ttm);
    strcpy(name, iname);
    mode = imode;
    size = is;
    if(ist)
	st=*ist;
    else
    {
	memset(&st, 0, sizeof(st));
	st.st_mode=mode;
	st.st_size=size;
	st.st_uid=uid;
	st.st_gid=gid;
	st.st_mtime=time;
    }
    magic[0]='\0';
    namelen=strlen(name);
    dir_name=0;
    
    find_user_and_group(user, uid, group, gid);
    
    set_file_type();
}

void   FList::set_file_type()
{

    do {
	if((mode & S_IFLNK)==S_IFLNK)
	{
	    file_type=file_is_symlink;
	    file_color=f_link_color;
	    icon_type=f_link_icon;
	    break;
	}
	
	if((mode & S_IFSOCK)==S_IFSOCK)
	{
	    file_type=file_is_file;
	    file_color=f_normal_color;
	    icon_type=f_normal_icon;
	    break;
	}

	if((mode & S_IFBLK)==S_IFBLK)
	{
	    file_type=file_is_file;
	    file_color=f_normal_color;
	    icon_type=f_normal_icon;
	    break;
	}

	if((mode & S_IFDIR)==S_IFDIR)
	{
	    file_type=file_is_dir;
	    file_color=f_dir_color;
	    if(name[0]==name[1] && name[2]==0 && name[0]=='.')
		icon_type=f_updir_icon;
	    else
		icon_type=f_dir_icon;
	    break;
	}
	
	if (is_it_afs_file(name))
	{
	    options |= S_EXTEND;
	    file_type=file_is_archive;
	    file_color=f_afs_color;
	    icon_type=f_afs_icon;
	    break;
	}
	
	if(is_exec(this))
	{
	    file_type=file_is_execatable;
	    file_color=f_exec_color;
	    icon_type=f_exec_icon;
	    break;
	}
	
	if(is_image(name, namelen))
	{
	    if (ext_find(name) != -1)
		options |= S_EXTEND;
	    xncdprintf(("IMAGE: %s has %s extension command\n",
			name, (options & S_EXTEND) ? "  " : "NO"));
	    file_type=file_is_image;
	    file_color=f_image_color;
	    icon_type=f_image_icon;
	    break;
	}

	if (ext_find(name) != -1)
	{
	    options |= S_EXTEND;
	    file_type=file_is_association;
	    file_color=f_ext_color;
	    icon_type=f_ext_icon;
	    break;
	}
	
	file_type=file_is_file;
	file_color=f_normal_color;
	icon_type=f_normal_icon;
    } while(0);

}

//Add new Flist element to the list using current compare function for sorting
void   FList::add(FList * it)
{
  FList *cur = this;
  while (compare(it->name, it->size, it->time, cur->name, cur->size, cur->time) > 0)
  {
    if (cur->next)
      cur = cur->next;
    else
    {
      cur->next = it;
      it->prev = cur;
      break;
    };
  };
  if (compare(it->name, it->size, it->time, cur->name, cur->size, cur->time) <= 0)
  {
    it->next = cur;
    it->prev = cur->prev;
    if (cur->prev)
      cur->prev->next = it;
    cur->prev = it;
  }
}

void   FList::set_user_group_names(char *fromuser, char *fromgroup)
{
  strncpy(user, fromuser, L_USER);
  user[L_USER-1]=0;
  strncpy(group, fromgroup, L_GROUP);
  group[L_GROUP-1]=0;
}

void   FList::out()
{
}

void   FList::add_magic(char* mag)
{
  strncpy(magic, mag, L_MAGIC);
  magic[L_MAGIC-1]=0;
}

FList* flist_find_by_name(FList *list, char *name, struct stat *st)
{
  while(list)
  {
    if(strcmp(list->name,name)==0)
    {
      if(st)
      {
	st->st_mode=list->mode;
	st->st_size=list->size;
	st->st_mtime=list->time;
	st->st_uid=list->uid;
	st->st_gid=list->gid;
      }
      return list;
    }
    list=list->next;
  }
  return NULL;
}

//---------------------------------------------------------------------------------
char* is_image(char* name, int len)
{
    static char *imgs[]={ 
	"gif",
	"jpg",
	"png",
	"tif",
	"xpm",
	"bmp",
	"lbm",
	"pcx",
	"pnm",
	"tga",
	"GIF",
	"JPG",
	"PNG",
	"TIF",
	"XPM",
	"BMP",
	"LBM",
	"PCX",
	"PNM",
	"TGA"
    };
	
    int max_imgs=20;
    int i;

    if(len<5) //Assume that we have name.ext and ext is 3 symbols, name at least 1
	return 0;

    name+=(len-3);
    for(i=0;i<max_imgs;i++)
	if(strcmp(name,imgs[i])==0)
	    return imgs[i];
    return 0;
}


//---------------------------------------------------------------------------------

//Break string 'something1 -> something2' to two strings 'something1' and 'something2'
//return 1 if success and 0 otherwise
int breakdown_link(char *from, char *to1, char *to2)
{
  char *linkid[3]={" -> ", " == ", " link to "};
  char *s;
  int i;
  for(i=0;i<3;i++)
  {
    s=strstr(from, linkid[i]);
    if(s)
    {
      memcpy(to1,from,s-from);
      to1[s-from]=0;
      strcpy(to2,s+strlen(linkid[i]));
      return 1;
    }
  }
  strcpy(to1,from);
  return 0;
}

int check_for_link(char *from)
{
  char *linkid[3]={" -> ", " == ", " link to "};
  char *s;
  int i;
  for(i=0;i<3;i++)
  {
    s=strstr(from, linkid[i]);
    if(s)
    {
      return 1;
    }
  }
  return 0;
}

void   simple_flist_add(FList * base, FList * it)
{
  it->next = base->next;
  if (base->next)
    base->next->prev = it;
  it->prev = base;
  base->next = it;
}



//----------------------------End of file----------------------//


