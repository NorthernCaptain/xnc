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

//This file contains routines for Archive File System (AFS).

#include "vfs.h"
#include "panel.h"
#include "query_windows.h"
#include "infowin.h"
#include "c_externs.h"
#include "bookmark.h"

#define AFS_CACHE_MEM        50000

int    dont_update = 0;                //don't ask for update archives on exiting from it. (Never do it);

// The return value is static, so copy it before using the
// method again!

char *create_dirs(char *rootdir, const char* dirlist)
{
   char str[L_MAXPATH];
   char cont[FLIST_NAME];
   strcpy(str,rootdir);
   do{
           dirlist=get_first_content(dirlist,cont);
           add_path_content(str,cont);
           if(::mkdir(str,0755)==-1 && errno !=EEXIST)
                   return NULL;
       } while(dirlist);
   return rootdir;
}

char  *AFS::get_dir_header()
{
  static char strtmp[2*L_MAXPATH];
   
  sprintf(strtmp, "%s[%s]:%s", sp->name, arcname, curdir);
  return strtmp;
}

int    AFS::can_we_optimize()
{
  if (sp->luarc[0] == (char)-1 && sp->luarc[1] == (char)-1)
    return 0;
  return 1;
}

int    AFS::is_pocket_addition_allowed()
{
  if (sp->lparc[0] == (char)-1 && sp->lparc[1] == (char)-1)
    return 0;
  return 1;
}

int    AFS::is_full_extract_allowed()
{
  if (sp->uarc[0] == (char)-1 && sp->uarc[1] == (char)-1)
    return 0;
  if (sublevel == 0)
    return 1;
  return 0;
}

int    AFS::is_exec_supported()
{
  if (sp->sarc[0] == (char)-1 && sp->sarc[1] == (char)-1)
    return 0;
  return 1;
}

int    AFS::is_copy_supported()
{
  if (sp->sarc[0] == (char)-1 && sp->sarc[1] == (char)-1)
    return 0;
  return 1;
}

int    AFS::is_del_supported()
{
  if (sp->darc[0] == (char)-1 && sp->darc[1] == (char)-1)
    return 0;
  return 1;
}

int    AFS::check_for_full_extract(int self)
{
  int    coun = 0;
  FList *o = dl.next;

  while ((strcmp(o->name, "..") == 0) || (strcmp(o->name, ".") == 0))
    {
      o = o->next;
      if (o == NULL)
        break;
    }
  while (o != NULL)
    {
      coun++;
      o = o->next;
    }
  if (coun == self)
    return 1;
  return 0;
}

void   AFS::delete_vfs_list()
{
  int    r;
  char  *b;
  char  strtmp[4096];

  if (update_on_exit && dont_update != 2 && (sp->pdarc[0] != (char)-1 || sp->pdarc[1] != (char)-1))
    {
      if (dont_update == 0)
        ask_for_update();
      else
        update_ok = 1;
      if (update_ok)
        {
          update_ok = 0;
          create_infowin(sp->name);
          to_infowin(_("Updating..."));
          sp->single_pack_dir_command(strtmp, fullarcname, ".");
// *INDENT-OFF*        
          ::chdir(tmppath);
          r = psystem(strtmp);
          b = strrchr(fullarcname, '/');
          *b = 0;
          ::chdir(fullarcname);
// *INDENT-ON*        

          *b = '/';
          del_infowin();
        }
    }
// *INDENT-OFF*        
  VFS::delete_vfs_list();
  ::unlink(listpath);
// *INDENT-ON*        

/* sprintf(strtmp2,"rm -rf %s",tmppath);
   psystem(strtmp2);
 */
};

int    AFS::remove(FList * cur)
{
  int    r, l;
  char   strtmp[4096];
  char   strtmp2[4096];

  if (is_del_supported() == 0)
    return 255;
  if (strcmp(curdir, ".") == 0)
    {
      if (prefix[0])
        sprintf(strtmp, "%s%s", prefix, cur->name);
      else
        strcpy(strtmp, cur->name);
    }
  else if (prefix[0])
    sprintf(strtmp, "%s%s/%s", prefix, curdir + 2, cur->name);
  else
    sprintf(strtmp, "%s/%s", curdir + 2, cur->name);
  if (cur->mode & S_IFDIR)
    strcat(strtmp, "/*");
  quote_path(strtmp2, strtmp);
  sp->single_delete_command(strtmp, fullarcname, strtmp2);
  r = psystem(strtmp);                //Add error checking here!!!!!!!!!!!!

  if (r != 0 && (cur->mode & S_IFDIR))
    {
      l = strlen(strtmp);
      if (strtmp[l - 1] == '\'')
        strtmp[l - 3] = '\'';
      strtmp[l - 2] = 0;
      r = psystem(strtmp);
    }
  if (r == 0)
    need_reread = 1;
  return r;
}


char  *AFS::get_file_for_execute(FList *ptr)
{
  char  *vfsname=ptr->name;
  char  *b;
  int    r;
  char   strtmp[2*L_MAXPATH];
  char   strtmp2[2*L_MAXPATH];
  char   strtmp3[2*L_MAXPATH];
  static char   filename[L_MAXPATH];
  struct stat dstat;
  FList *o;
  char *curdir=strtmp3;
  char *to_dir;
  strcpy(strtmp3,this->curdir);
  if(o=find_content(vfsname))
  {
          if(o->mode & S_IFLNK)
          {
                  to_dir=o->link->name;
                  while(strncmp(to_dir,"..",2)==0)
                  {
                          upper_path(curdir);
                          to_dir+=3;
                  }
                  strcpy(filename,to_dir);
          }
          else
                  strcpy(filename,vfsname);
  } else
          strcpy(filename,vfsname);
  if (strcmp(curdir, ".") == 0)
    sprintf(strtmp, "%s/%s", tmppath, filename);
  else
    sprintf(strtmp, "%s/%s/%s", tmppath, curdir + 2, filename);
  if (stat(strtmp, &dstat) != -1)
  {
    if(o!=NULL && (o->mode & S_IFLNK))
    {
            create_dirs(tmppath,this->curdir+2);
            return o->link->name;
    }
    else
            return filename;
  }
  create_infowin(sp->name);
  to_infowin(_("Preparing to execution..."));
  if (strcmp(curdir, ".") == 0)
    {
      if (prefix[0])
        sprintf(strtmp, "%s%s", prefix, filename);
      else
        strcpy(strtmp, filename);
    }
  else if (prefix[0])
    sprintf(strtmp, "%s%s/%s", prefix, curdir + 2, filename);
  else
    sprintf(strtmp, "%s/%s", curdir + 2, filename);
    
  quote_path(strtmp2, strtmp);
  sp->single_extract_command(strtmp, fullarcname, strtmp2);
// *INDENT-OFF*        
  if (::chdir(tmppath) == -1)
    if (::mkdir(tmppath, 0700) == -1)
      {
        del_infowin();
        vfs_error(sp->name, _("Can't access exeTMP dir"));
        return NULL;
      }
    else
      ::chdir(tmppath);
   r = psystem(strtmp);                //Add error checking here!!!!!!!!!!!!

  del_infowin();
  b = strrchr(fullarcname, '/');
  *b = 0;
  ::chdir(fullarcname);
  *b = '/';
// *INDENT-ON*        

  if (r)
    {
      vfs_error(sp->name, _("Unpacking FAILED!"));
      return NULL;
    }
  update_on_exit = 1;
  if(o!=NULL && (o->mode & S_IFLNK))
  {
            create_dirs(tmppath,this->curdir+2);
            return o->link->name;
  }
  return filename;
}

int    AFS::full_extract_to_vfs(VFS * vfs)
{
  int    r;
  char  *b;
  FList *o = dl.next;
  char  strtmp[2*L_MAXPATH];
  
  if (vfs->fstype != DFS_TYPE)
    {
      vfs_error("AFS", _("Can't copy to destination VFS"));
      return 255;
    };

  if (dont_ask == 0)
    while (o != NULL)                //First of all check file existance

      {
        if (o->options & S_SELECT)
          {
            if (vfs->is_file_exist(vfs->curdir, o->name))
              {
                init_overwrite_query(_("Copy file"), o->name);
                wait_for_query_done();
                if (ow_all == 0 && ow_file == 0)
                  if (ow_cancel)
                    return ow_cancel;        //if cancel we return as error!!!

                  else
                    o->options &= ~S_SELECT;
              }
          }
        ow_file = 0;
        if (ow_all)
          break;
        o = o->next;
      }
// *INDENT-OFF*        
  ::chdir(vfs->curdir);
  sprintf(strtmp, sp->unpack, fullarcname);
  xncdprintf(("FULL_EXTRACT: run: [%s]\n",strtmp));
  r = psystem(strtmp);
  b = strrchr(fullarcname, '/');
  *b = 0;
  ::chdir(fullarcname);
  *b = '/';
// *INDENT-ON*        

  if (r)
    vfs_error(sp->name, _("Unpacking FAILED!"));
  return r;
}

void construct_full_link(char *from1, char* from2)
{
  char *to_dir;
  to_dir=from2;
  while(strncmp(to_dir,"..",2)==0)
    {
         upper_path(from1);
         to_dir+=3;
    }
    if(from1[strlen(from1)-1]!='/')
        strcat(from1,"/");
    strcat(from1,to_dir);
}


int    AFS::optimized_copy_to_dfs(VFS * vfs)
{
  FList *o = dl.next;
  char   s[100];
  int    need_tmp = 1;
  int    r = 0;
  FILE  *fp;
  char  *b;
  char  strtmp[2*L_MAXPATH];
  char  strtmp2[2*L_MAXPATH];
  
  if (dont_ask == 0)
    while (o != NULL)                //First of all check file existance

      {
        if (o->options & S_SELECT)
          {
            if (vfs->is_file_exist(vfs->curdir, o->name))
              {
                init_overwrite_query(_("Copy file"), o->name);
                wait_for_query_done();
                if (ow_all == 0 && ow_file == 0)
                  if (ow_cancel)
                    return ow_cancel;        //if cancel we return as error!!!

                  else
                    o->options &= ~S_SELECT;
              }
          }
        ow_file = 0;
        if (ow_all)
          break;
        o = o->next;
      }

  o = dl.next;
  sprintf(s, "%s/AFS/.list", syspath);
  fp = fopen(s, "w");
  if (fp == NULL)
    {
      vfs_error(sp->name, _("TMP dir not accessable!"));
      return 255;
    };
  if (strcmp(curdir, ".") == 0)
    need_tmp = 0;
  else
    need_tmp = 1;
  while (o != NULL)
    {
      if (o->options & S_SELECT)
        {
          if (need_tmp == 0)
            {
              if (prefix[0])
                sprintf(strtmp2, "%s%s", prefix, o->name);
              else
                strcpy(strtmp2, o->name);
            }
          else if (prefix[0])
            sprintf(strtmp2, "%s%s/%s", prefix, curdir + 2, o->name);
          else
            sprintf(strtmp2, "%s/%s", curdir + 2, o->name);
          if (o->mode & S_IFDIR)
            strcat(strtmp2, "/*");
          fprintf(fp, "%s\n", strtmp2);
        }
      o = o->next;
    }
  fclose(fp);
  sp->single_listextract_command(strtmp, fullarcname, s);
// *INDENT-OFF*        
  if (need_tmp == 0)
      ::chdir(vfs->curdir);
  else
    {
     if (::chdir(tmppath) == -1)
       if (::mkdir(tmppath, 0700) == -1)
          {
            del_infowin();
            vfs_error(sp->name, _("Can't access execTMP dir"));
            unlink(s);
            return 255;
          }
        else
        ::chdir(tmppath);
    }
// *INDENT-ON*        

  r = psystem(strtmp);                //Add error checking here!!!!!!!!!!!!

  unlink(s);
  b = strrchr(fullarcname, '/');
  if (need_tmp)                        //here we create DFS entry and move unpacked file

    {                                //from tmpdir to destination VFS

      DFS    df;
// *INDENT-OFF*        
      ::chdir(curdir);
// *INDENT-ON*        

      df.set_dir_pointer(strtmp2, 1024);
      o = dl.next;
      while (o != NULL)
        {
          if (o->options & S_SELECT)
            {
              if (df.move(o, vfs))
                r |= 1;
              else
                o->options &= ~S_SELECT;
            }
          o = o->next;
        }
    }
  else if (r == 0)
    {
      o = dl.next;
      while (o != NULL)
        {
          o->options &= ~S_SELECT;
          o = o->next;
        }
    }
  *b = 0;
// *INDENT-OFF*        
  ::chdir(fullarcname);
// *INDENT-ON*        

  *b = '/';
  return r;
}

int    AFS::add_to_vfs_by_list(FList * dd)
{
  FList *o = dd;
  char   s[L_MAXPATH];
  int    r = 0;
  FILE  *fp;
  char   strtmp[2*L_MAXPATH];
  char   strtmp2[2*L_MAXPATH];

  sprintf(s, "%s/AFS/.list", syspath);
  fp = fopen(s, "w");
  if (fp == NULL)
    {
      vfs_error(sp->name, _("TMP dir not accessable!"));
      return 255;
    };
  while (o != NULL)
    {
      if (o->options & S_SELECT)
        {
          sprintf(strtmp2, "%s", o->name);
          if (o->mode & S_IFDIR)
            strcat(strtmp2, "/*");
          fprintf(fp, "%s\n", strtmp2);
        }
      o = o->next;
    }
  fclose(fp);
  sp->single_listpack_command(strtmp, fullarcname, s);
  r = psystem(strtmp);                //Add error checking here!!!!!!!!!!!!

  unlink(s);
  if (r == 0)
    {
      o = dd;
      while (o != NULL)
        {
          o->options &= ~S_SELECT;
          o = o->next;
        }
    }
  need_reread = 1;
  return r;
}

int    AFS::copy(FList * cur, VFS * vfs)
{
  int    r;
  int    need_tmp = 1;
  char  *b;
  char   strtmp[2*L_MAXPATH];
  char   strtmp2[2*L_MAXPATH];
  
  if(sp->sarc[0]==(char)-1 && sp->sarc[1]==(char)-1)
  {
          vfs_error(sp->name,_("Operation not supported in this archive"));
          return 255;
  }
  
  if (vfs->is_copy_supported())
    {
      if (opt && vfs->fstype == DFS_TYPE)
        {
          opt = 0;
          return optimized_copy_to_dfs(vfs);
        }
      if (ow_all == 0 && dont_ask == 0)
        if (vfs->is_file_exist(vfs->curdir, cur->name))
          {
            init_overwrite_query(_("Copy file"), cur->name);
            wait_for_query_done();
            if (ow_all == 0 && ow_file == 0)
              return ow_cancel;        //if cancel we return as error!!!

          }
      ow_file = 0;

      if (vfs->fstype == DFS_TYPE)        //Only AFS->DFS now supported (no AFS->AFS yet)

        {
          //Check for symlinked dir inside archive first.
          if((cur->mode & S_IFLNK) && (cur->mode & S_IFDIR) && cur->link)
          {
                  ::chdir(vfs->curdir);
                  if(::symlink(cur->link->name,cur->name)==-1)
                          {
                                  errno2mes();
                                  ::chdir(fullarcname);
                                  return 255;
                          }
                  ::chdir(fullarcname);
                  return 0;
          }
          
          if (strcmp(curdir, ".") == 0)
            {
              need_tmp = 0;
              if (prefix[0])
                sprintf(strtmp, "%s%s", prefix, cur->name);
              else
                strcpy(strtmp, cur->name);
            }
          else if (prefix[0])
            sprintf(strtmp, "%s%s/%s", prefix, curdir + 2, cur->name);
          else
            sprintf(strtmp, "%s/%s", curdir + 2, cur->name);
          if (cur->mode & S_IFDIR)
            strcat(strtmp, "/*");
          quote_path(strtmp2, strtmp);
          sp->single_extract_command(strtmp, fullarcname, strtmp2);
// *INDENT-OFF*        
          if (need_tmp == 0)
            ::chdir(vfs->curdir);
          else
            {
            if (::chdir(tmppath) == -1)
              if (::mkdir(tmppath, 0700) == -1)
                  {
                    del_infowin();
                    vfs_error(sp->name, _("Can't access execTMP dir"));
                    return 255;
                  }
                else
                  ::chdir(tmppath);
            }
          r = psystem(strtmp);        //Add error checking here!!!!!!!!!!!!

          b = strrchr(fullarcname, '/');
          if (need_tmp && r == 0)        //here we create DFS entry and move unpacked file

            {                        //from tmpdir to destination VFS

              DFS    df;
              ::chdir(curdir);
              df.set_dir_pointer(strtmp2, 1024);
              r = df.move(cur, vfs);
            }
          *b = 0;
          ::chdir(fullarcname);
// *INDENT-ON*        

          *b = '/';
          return r;
        }
    }
  return 255;
}

char  *AFS::get_execute_dir(FList* ptr)
{
  static char strtmp[L_MAXPATH];
   
  if (strcmp(curdir, ".") == 0)
    return tmppath;
  else
    sprintf(strtmp, "%s/%s", tmppath, curdir + 2);
  return strtmp;
}


//Return 0 if init failed.
int    AFS::init_support(char *fname, char* subdir)
{
  struct stat st;
  int idx;
  xncdprintf(("AFS::ini arcfile --> %s <--\n",fname));
  strcpy(arcname, fname);
  update_on_exit = 0;
  sublevel = 0;
  sp = find_support(arcname);
  if (sp == NULL)
    {
      vfs_error("AFS", _("No support for this file"));
      return 0;
    }
  if (dont_construct_path == 0)
    {
// *INDENT-OFF*        
      ::getcwd(fullarcname, 1024);
// *INDENT-ON*        

      strcat(fullarcname, "/");
      strcat(fullarcname, arcname);
    }
  dont_construct_path = 0;
  sprintf(listpath, "%s/AFS/afs.%d.%08d", syspath, getpid(), this);
  sprintf(tmppath, "%s/.%d.%s.%08d", syspath, getpid(), arcname, this);
// *INDENT-OFF*        
  if (::mkdir(tmppath, 0700) == -1)
    {
      if (errno != EEXIST)
        {
          vfs_error("AFS", _("Can't create AFSTMP directory!"));
          return 0;
        }
      else
        update_on_exit = 1;
    }
// *INDENT-ON*        

  need_reread = 1;
  sublevel = 0;
  need_change_vfs = 0;
  strcpy(curdir, ".");
  if(subdir)
  {
    if(*subdir=='/')
      subdir++;
    strcpy(to_dir,subdir);
    /*    idx=0;
    if(subdir[idx]!='.')
      curdir[idx++]='.';
    if(subdir[0]!='/' && idx==0)
      curdir[idx++]='/';
    strcpy(curdir+idx,subdir);
    */
  }
  else
    to_dir[0] = 0;

  if(stat(fullarcname, &st)!=-1)
    return reread();

  return 0;
}

void   AFS::convert_attributes(int &mode, int siz, char *at, char *name)
{
  mode = 0;
  int    i, delta = 1;
  if (sp->dattr == -1)
    {
      mode = 0777;
      if (siz == 0)
        mode |= S_IFDIR;
    }
  else
    {
      if (strlen(at) > 9)
        {
          if (at[0] == 'd')
            mode |= S_IFDIR;
          else if (at[0] == '.' && siz == 0)
            mode |= S_IFDIR;
          if(at[0] == 'l' || check_for_link(name))
                  mode |= S_IFLNK;
        }
      else
        {
          if (siz == 0)
            mode |= S_IFDIR;
          delta = 0;
        };
      for (i = 0; i < 9; i++)
        if (at[i + delta] != '-' && (at[i + delta] == 'r' || at[i + delta] == 'w' || at[i + delta] == 'x' || at[i + delta] == '.'))
          mode |= (1 << (8 - i));
    }
}

int    AFS::get_sublevel(char *file, int mode)
{
  int    l = strlen(file), i, coun = 0;
  for (i = 0; i < l; i++)
    if (file[i] == '/')
      coun++;
  if (file[0] == '/')
    coun--;
  if (file[0] == '.' && file[1] == '/')
    coun--;
  if (file[l - 1] == '/')
    coun--;
  else if (mode & S_IFDIR)
    coun--;
  if (coun < 0)
    coun = 0;
  return coun;
}

char  *AFS::get_lastname(char *file)
{
  int    l = strlen(file);
  if (file[l - 1] == '/')
    {
      file[l - 1] = 0;
      l--;
    };
  char  *b = strrchr(file, '/');
  if (b == NULL)
    return file;
  return b + 1;
}

FList *AFS::find_content(char *n)
{
  FList *o = dl.next;
  while (o != NULL)
    if (strcmp(o->name, n) == 0)
      return o;
    else
      o = o->next;
  return NULL;
}

int    AFS::chdir(char *d)
{
  FList *o;
  if (strcmp(d, "..") == 0)
    if (sublevel == 0)
      {
        need_change_vfs = 1;
        return 0;
      }
  if (o=find_content(d))
    {
      if((o->mode & S_IFLNK) && o->link!=NULL)
              strcpy(to_dir, o->link->name);
      else
              strcpy(to_dir, d);
      return 0;
    };
  return -1;
}

char  *AFS::gen_fullname(char *n)
{
  static char strtmp[L_MAXPATH];
  int    i ;
  i=2;
  if (*n == '/')
    n++;
  if (n[0] == '.' && n[1] == '/')
    i = 0;
  else
    {
      strtmp[0] = '.';
      strtmp[1] = '/';
    };
  while (*n != 0)
    strtmp[i++] = *n++;
  if (strtmp[i - 1] == '/')
    i--;
  strtmp[i++] = 0;
  strtmp[i++] = 0;
  strtmp[i] = 0;
  return strtmp;
}

int    AFS::add_to_vfs(FList * cur)
{
  int    r;
  char   strtmp[2*L_MAXPATH];
  char   strtmp2[2*L_MAXPATH];

  if (cur->mode & S_IFDIR)
    {
      if (sp->parc[0] == (char)-1 && sp->parc[1] == (char)-1)
        return 255;
      else
        sp->single_pack_dir_command(strtmp, fullarcname,
                                    quote_path(strtmp2, cur->name));
    }
  else if (sp->parc[0] == (char)-1 && sp->parc[1] == (char)-1)
    return 255;
  else
    sp->single_pack_file_command(strtmp, fullarcname,
                                 quote_path(strtmp2, cur->name));
  r = psystem(strtmp);
  if (r == 0)
    need_reread = 1;
  return r;
}

int    AFS::is_in_this_dir(char *n)
{
  int    i = 0;
  while (curdir[i] != 0)
    if (curdir[i] != *n)
      return 0;
    else
      {
        i++;
        n++;
      };
  if (*n == '/')
    return 1;
  return 0;
}

struct DTree
{
        char name[256];
        int mode,time;
        DTree *thislevel,*sublevel;
        DTree() { thislevel=sublevel=NULL;};
        void init(char *iname,int imode, int t)
        {
                time=t;
                strcpy(name,iname);
                mode=imode;
                thislevel=sublevel=NULL;
        };
};
#define S_DTree        sizeof(DTree)

static DTree dtree_obj;

void   AFS::init_dir_cache()
{
  if(cache[0])
          deinit_dir_cache();
  for(int i=0;i<AFS_CACHE_ENTRIES;i++)
          cache[i]=NULL;
  cache[0]=new char[AFS_CACHE_MEM];
  cachestart=cacheptr=cache[0];
  dtree_obj.thislevel=dtree_obj.sublevel=NULL;
}

void   AFS::deinit_dir_cache()
{
  int i=0;
  while(cache[i]!=NULL)
  {
          delete cache[i];
          cache[i++] = NULL;
  }
}

void *AFS::cache_alloc(int size)
{
        void *ptr;
        if(cacheptr-cachestart+size>AFS_CACHE_MEM)
        {
                int i=0;
                while(cache[i]!=NULL)
                        i++;
                cache[i]=new char[AFS_CACHE_MEM];
                cachestart=cacheptr=cache[i];
        }
        ptr=(void*)cacheptr;
        cacheptr+=size;
        return ptr;
}

void*   AFS::find_in_cache(char *name)
{
  char  *b, *s;
  int    sl = 0;
  DTree *obj=&dtree_obj, *o;
  

  sl = strlen(name);
  if (name[sl - 1] == '/')
    name[sl - 1] = 0;
  b = name;
  sl = 0;
  if(strlen(name)==0 || strcmp(name,".")==0)
          return NULL;
  s=name;
  do{
          b=strchr(s,'/');
          if(b)
                  *b=0;
          o=obj->sublevel;
          while(o)
          {
                  if(strcmp(o->name,s)==0)
                          break;
                  o=o->thislevel;
          }
          if(o==NULL)
                  break;
          obj=o;
          if(b)
          {
                  s=b+1;
                  *b='/';
          } else s=NULL;
     } while(s);
  return o;
}
     

/* This version of cache used Directory Tree (DTree) list for organizing cache
 * Works much faster and need less memory.
 */
int    AFS::try_to_cache(FILE * fp, char *name, int mode, int t)
{
  char  *b, *s;
  int    sl = 0,l;
  DTree *obj=&dtree_obj, *o;
  

  if(*name==0)
      return 0;
  name += 2;
  if(*name==0)
      return 0;
  if ((mode & S_IFDIR) == 0)
    {
      b = strrchr(name, '/');
      if (b == NULL)
        return 0;                //Nothing to cache
      *b = 0;
      mode&=0777;
    }
  sl = strlen(name);
  if (name[sl - 1] == '/')
    name[sl - 1] = 0;
  b = name;
  sl = 0;
  if(strlen(name)==0 || strcmp(name,".")==0)
          return 0;
  s=name;
  
  do{
          b=strchr(s,'/');
          if(b)
                  *b=0;
          o=obj->sublevel;
          while(o)
          {
                  if(strcmp(o->name,s)==0)
                          break;
                  o=o->thislevel;
          }
          if(o==NULL)
          {
                  o=(DTree*)cache_alloc(S_DTree);
                  if(b)
		      l=b-s+1;
                  else
		      l=strlen(s)+1;
		  //                  o->name=(char*)cache_alloc(l);
                  o->init(s,mode,t);
                  o->thislevel=obj->sublevel;
                  obj->sublevel=o;
                  fprintf(fp, "%d %X %d %d %s\n", sl, mode | S_IFDIR, 0, t, name-2);        //Subdir_level filename attrs size
          }
          obj=o;
          if(b)
          {
                  s=b+1;
                  *b='/';
          } else s=NULL;
          sl++;
    } while(s);
    return 1;
}

static void parse_month(struct tm &tm, char *w)
{
  int i;
  static char *mon[] = { "jan", "feb", "mar", "apr", "may", "jun",
                         "jul", "aug", "sep", "oct", "nov", "dec" };
  char buf[4];

  if (*w >= '0' && *w <= '9') {
    sscanf(w, "%d", &tm.tm_mon);
    tm.tm_mon--;
    return;
  }
  for (i=0; i < 3; i++)
    buf[i] = tolower(w[i]);
  buf[3] = 0;
  for (i=0; i < 12; i++) {
    if (strcmp(mon[i], buf) == 0) {
      tm.tm_mon = i;
      return;
    }
  }
}

static void parse_date(struct tm &tm, char *w)
{
  char buf1[20], buf2[20], buf3[20];
  int i1, i2, i3;

//  printf("parse_date: %s\n", w);
  
  if (sscanf(w, "%[^-]-%[^-]-%s", buf1, buf2, buf3) != 3
      && sscanf(w, "%[^.].%[^.].%s", buf1, buf2, buf3) != 3) {
    // One last try: day and month separated by "."
    if (sscanf(w, "%[^.].%s", buf1, buf2) == 2) {
      if (*buf1 < '0' || *buf1 > '9') {
        parse_month(tm, buf1);
        sscanf(buf2, "%d", &i1);
        tm.tm_mday = i1;
      } else if (*buf2 < '0' || *buf3 > '9') {
        parse_month(tm, buf2);
        sscanf(buf1, "%d", &i1);
        tm.tm_mday = i1;
      } else {
        // Have to guess...
        sscanf(buf1, "%d", &i1);
        sscanf(buf2, "%d", &i2);
        if (i2 > 12) {
          tm.tm_mon = i1 - 1;
          tm.tm_mday = i2;
        } else {
          tm.tm_mon = i2 - 1;
          tm.tm_mday = i1;
        }
      }
    }
  } else {
    // check if there is one entry with letters,
    // if yes, we got the month.
    if (*buf1 < '0' || *buf1 > '9') {
      parse_month(tm, buf1);
      sscanf(buf2, "%d", &i1);
      sscanf(buf3, "%d", &i2);
      if (i2 > 31) {
        tm.tm_year = i2;
        tm.tm_mday = i1;
      } else {
        tm.tm_year = i1;
        tm.tm_mday = i2;
      }
      if (tm.tm_year > 1900)
        tm.tm_year -= 1900;
      return;
    } else if (*buf2 < '0' || *buf2 > '9') {
      parse_month(tm, buf2);
      sscanf(buf1, "%d", &i1);
      sscanf(buf3, "%d", &i2);
      if (i1 > 31) {
        tm.tm_year = i1;
        tm.tm_mday = i2;
      } else {
        tm.tm_year = i2;
        tm.tm_mday = i1;
      }
      if (tm.tm_year > 1900)
        tm.tm_year -= 1900;
      return;
    }
    // OK, so we seem to have an all-numbers format
    // try to guess the order
    sscanf(buf1, "%d", &i1);
    sscanf(buf2, "%d", &i2);
    sscanf(buf3, "%d", &i3);
    if (i1 > 31) {
      // year first, should always be ISO order (I hope)
      tm.tm_year = i1;
      tm.tm_mon = i2 - 1;
      tm.tm_mday = i3;
    } else if (i3 > 31) {
      tm.tm_year = i3;
      // if we don't find evidence against it, assume 
      // european order (i.e. day-month-year)
      if (i2 > 12) {
        tm.tm_mon = i1 - 1;
        tm.tm_mday = i2;
      } else {
        tm.tm_mon = i2 - 1;
        tm.tm_mday = i1;
      }
    }
    if (tm.tm_year > 1900)
      tm.tm_year -= 1900;
  }
}


int    AFS::reread()
{
  FILE  *fp, *fp2;
  int    coun, c, ct = 0, mode, sl, i;
  char   name[512];
  char  *fname;
  char   attrchr[20];
  char  *w;
  char   strtmp[2*L_MAXPATH];
  char   lbuf1[L_MAXPATH];
  char   lbuf2[L_MAXPATH];
  int    fsiz;
  struct tm tm;
  time_t tt;
  time_t t=0;
  Namer *namer=NULL,*n,*n2;
  DTree *dt;
  
  char tmp_fulldir[L_MAXPATH];

  quote_path(tmp_fulldir,fullarcname);
  sprintf(strtmp, sp->list, tmp_fulldir);

  fp2 = fopen(listpath, "w");
  if (fp2 == NULL)
    {
      vfs_error("AFS", _("Can't access TMP dir"));
      return 0;
    }

  signal(SIGCHLD,SIG_DFL);
  fp = popen(strtmp, "r");
  if (fp == NULL)
    {
      fclose(fp2);
      unlink(listpath);
      vfs_error(sp->name, _("No specified executable for this AFS"));
      set_cmdchild_signal();
      return 0;
    }
  create_infowin(sp->name);
  to_infowin(_("Reading Archive..."));
  init_dir_cache();
  sp->skip_header(fp);
  coun = 0;
  attrchr[0] = 0;
  time(&tt);
  memcpy(&tm, localtime(&tt), sizeof(tm));
  while ((w = sp->getword(fp, coun)) != NULL)
    {
      bmark->animate();
      c = coun - 1;
      if (c == sp->dname)
        strcpy(name, w);
      else if (c == sp->dsize)
        sscanf(w, "%d", &fsiz);
      else if (c == sp->dattr)
        strcpy(attrchr, w);
      else if (c == sp->ddate)
        parse_date(tm, w);
      else if (c == sp->dtime) {
        if(strchr(w,':'))  //Is is time format or may be it is Year
        {
                sscanf(w, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
                tm.tm_isdst = -1;
        } else        //Yes, this is year, not time  (Some tar's use such technique)
        {
          sscanf(w, "%d", &tm.tm_year);
          if (tm.tm_year > 1900)
          tm.tm_year -= 1900;
        }
      } else if (c == sp->dmonth)
        parse_month(tm, w);
      else if (c == sp->dyear) {
        sscanf(w, "%d", &tm.tm_year);
        if (tm.tm_year > 1900)
          tm.tm_year -= 1900;
      } else if (c == sp->dday)
        sscanf(w, "%d", &tm.tm_mday);
      
      if (coun == sp->total)
        {
          ct++;
          sp->flush_line();
          i = 0;
          while (name[i] == '/' || name[i] == '.')
            {
              prefix[i] = name[i];
              i++;
            };
          prefix[i] = 0;
          if (i != 0 && prefix[i - 1] == '.')
            prefix[i - 1] = 0;
          convert_attributes(mode, fsiz, attrchr,name);
          t = mktime(&tm);
          if (t == -1)
            t = 0;
          memcpy(&tm, localtime(&tt), sizeof(tm));
          fname = gen_fullname(name);
          
          if(mode & S_IFLNK)
          {
                  n=new Namer;
                  breakdown_link(fname,lbuf1,lbuf2);
                  strcpy(n->name,lbuf2);
                  n2=new Namer(lbuf1,(FList*)n);
                  n2->next=namer;
                  namer=n2;
          } else
          {
                  sl = get_sublevel(name, mode);
                  //Subdir_level filename attrs size
                  if ((mode & S_IFDIR) == 0)
                    fprintf(fp2, "%d %X %d %d %s\n", sl, mode, fsiz, t, fname);
                  try_to_cache(fp2, fname, mode, t);
          }
        }
    }
  pclose(fp);
  set_cmdchild_signal();
  while(namer)
  {
          n=namer;
          n2=(Namer*)n->o;
          strcpy(lbuf2,n->name);
          upper_path(lbuf2);
          construct_full_link(lbuf2,n2->name);
          dt=(DTree*)find_in_cache(lbuf2+2);
          if(dt)
          {
                    sl=get_sublevel(n->name,0);
                    fprintf(fp2, "%d %X %d %d %s -> %s\n", sl, (dt->mode & 0777) | S_IFLNK | S_IFDIR, 0, dt->time, n->name, n2->name);
          }   else
          {
                  sl=get_sublevel(n->name,0);
                  fprintf(fp2, "%d %X %d %d %s -> %s\n", sl, n->val1 | S_IFLNK, 0, t, n->name, n2->name);
          }
          namer=namer->next;
          delete n;
          delete n2;
  }
  
  fclose(fp2);
  need_reread = 0;
  deinit_dir_cache();
  del_infowin();
  return 1;
}

int    AFS::direc(char *dir_ch)
{
  FList *ol;
  int    mode=0;
  FILE  *fp;
  int    coun;
  char   name[1024];
  char   strtmp[1024];
  char   lbuf1[1024];
  char   lbuf2[1024];
  int    fsiz;
  int    t;
  Namer *namer=NULL,*n;
  char *to_dir=this->to_dir;
  int linkcoun=0,l=strlen(to_dir);
  
  xncdprintf(("AFS:direc --> to_dir=[%s] dir=[%s] sublevel=%d\n",to_dir,dir_ch,sublevel));
  while(l>2 && strncmp(to_dir,"..",2)==0)
  {
    sublevel--;
    if(sublevel<0)
    {
      sublevel=0;
      vfs_error("AFS", _("Can't follow directory (outside archive)!"));
      return 255;
    }
    upper_path(curdir);
    to_dir+=3;
  }
  
  if (strcmp(to_dir, "..") == 0)
  {
    if (sublevel == 0)
    {
      need_change_vfs = 1;
      return 0;
    }
    else
    {
      sublevel--;
      upper_path(curdir);
    };
  }
  else if (find_content(to_dir))
  {
    sublevel++;
    add_path_content(curdir, to_dir);
  } else        //If we here then we doing cd not to subdir but to subdir/subdir...
  {
    const char *s=to_dir;
    if(*s)
    {
      do
      {
	s=get_first_content(s,strtmp);
	add_path_content(curdir, strtmp);
	sublevel++;
      }while(s);
    }
  }
  this->to_dir[0] = 0;
  xncdprintf(("AFS:direc **> to_dir=[%s] dir=[%s] sublevel=%d\n",to_dir,curdir,sublevel));
  if (need_reread)
    reread();
  fp = fopen(listpath, "r");
  if (fp == NULL)
  {
    vfs_error("AFS", _("Can't access TMP dir"));
    need_change_vfs = 1;
    return 0;
  }
  VFS::delete_vfs_list();
  while (fgets(strtmp,sizeof(strtmp),fp))
  {
    bmark->animate();
    sscanf(strtmp, "%d %X %d %d %[^\n]\n", &coun, &mode, &fsiz, &t, name);
    if (coun == sublevel && strcmp(name, "./.") != 0)
      if (is_in_this_dir(name))
      {
	if (hide_hidden && name[2] == '.')
	  continue;
	ol = (FList*)new_alloc(S_FList);
	if(mode & S_IFLNK)
	{
	  breakdown_link(name, lbuf1, lbuf2);
	  ol->init(get_lastname(lbuf1), mode, fsiz, getuid(), getgid(), t);
	  if((mode & S_IFDIR)==S_IFDIR)
	  {
	    ol->link=(FList*)new_alloc(S_FList);
	    ol->link->init(lbuf2,ol->mode,0,0,0,t);
	  } else
	  {
	    linkcoun++;
	    n=new Namer(lbuf2,ol);
	    n->next=namer;
	    namer=n;
	  }
	} else
	  ol->init(get_lastname(name), mode, fsiz, getuid(), getgid(), t);
	ol->options &= ~S_SELECT;
	if (mode & S_IFDIR)
	  dl.add(ol);
	else
	  fl.add(ol);
      }
  }
  fclose(fp);

  if(linkcoun)
  {                        //Doing post action linking
    n=namer;
    struct stat st;
    while(n)
    {
      ol=(FList*)new_alloc(S_FList);
      if(flist_find_by_name(fl.next,n->name,&st))
      {
	ol->init(n->name, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);
      }
      else
	if(n->o->mode & S_IFDIR)
	{
	  ol->init(n->name,n->o->mode,0,0,0,n->o->time);
	  st.st_mode=ol->mode;
	  st.st_mtime=ol->time;
	}
	else
	{
	  ol->init(n->name,0755,7,0,0,st.st_mtime);
	  st.st_mode=ol->mode;
	}
      n->o->link=ol;
      n->o->size=ol->size;
      ol=n->o;
      if((ol->mode & S_IFDIR)!=S_IFDIR && (st.st_mode & S_IFDIR)==S_IFDIR)
      {        //We are in wrong list. We must be inside dl but now we in fl.
	ol->mode|=S_IFDIR;
	if(ol->prev)
	  ol->prev->next=ol->next;
	if(ol->next)
	  ol->next->prev=ol->prev;
	ol->next=ol->prev=NULL;
	dl.add(ol);
      } else
	if((ol->mode & S_IFDIR)==S_IFDIR && (st.st_mode & S_IFDIR)!=S_IFDIR)
	{        //We are in wrong list. We must be inside fl but now we in dl.
	  ol->mode&=~S_IFDIR;
	  if(ol->prev)
	    ol->prev->next=ol->next;
	  if(ol->next)
	    ol->next->prev=ol->prev;
	  ol->next=ol->prev=NULL;
	  fl.add(ol);
	}              
      ol->mode=(ol->mode & ~0777) | (st.st_mode & 0777);
      n=n->next;
      delete namer;
      namer=n;
    }
  }
  
  bmark->animate();
  ol = (FList*)new_alloc(S_FList);
  ol->init("..", (0777 | S_IFDIR), 0, getuid(), getgid(), 0);
  simple_flist_add(&dl,ol);
  ol = (FList*)new_alloc(S_FList);
  ol->init(".", (0777 | S_IFDIR), 0, getuid(), getgid(), 0);
  simple_flist_add(&dl,ol);
  
  return 1;
}

void   try_clean_afs_cache()
{
  char   strtmp2[128];
  if (panel->vfs->fstype == AFS_TYPE || panel->panel2->vfs->fstype == AFS_TYPE)
  {
    simple_mes(_("Error"), _("Exit from archive before cleaning cache!"));
    return;
  }
  sprintf(strtmp2, "rm -rf '%s'/.%d*", syspath, getpid());
  if (psystem(strtmp2) == 0)
    simple_mes(_("Report"), _("Archive cache is clean."));
  else
    simple_mes(_("Error"), _("Something wrong with AFS cache!!!"));
}


char* AFS::get_dir_for_dnd()
{
    int l=strlen(fullarcname);
    char *buf=new char[l+256+strlen(curdir)];
    sprintf(buf,"%s:/%s#%s",sp->name,fullarcname,curdir);
    xncdprintf(("AFS_get_dir_for_dnd: [%s]\n",buf));
    return buf;
}


//Create archive for given dir
//return 1 if success
int  AFS::create_archive_from_files(char* from_dir, int one_file_only)
{
  int r;
  char strtmp[L_MAXPATH];
  char buf1[L_MAXPATH];
  strong_quote_path(buf1, from_dir);
  if(one_file_only)
      sp->single_create_command(strtmp, fullarcname, buf1);
  else
      sp->single_create_many(strtmp, fullarcname, buf1);

  xncdprintf(("AFS::create_archive - exec_cmd [%s]\n",strtmp));
  r = psystem(strtmp);
  if(r)
    vfs_error(sp->name, _("Errors during archive creation"));
  return r==0;
}

int  AFS::construct_archive_name(char* to, char* from, int is_dir,
				 const char* ext)
{
    char *ptmp=0;
    
    get_last_content(from, to);
    if(!is_dir)
	ptmp=strrchr(to, '.');
    ptmp= ptmp ? ptmp : to+strlen(to);
    strcpy(ptmp, ext);
    return 1;
}
