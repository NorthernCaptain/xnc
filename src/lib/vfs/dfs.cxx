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
/* $Id: dfs.cxx,v 1.10 2003/11/27 15:36:43 leo Exp $ */
//This file contains routines for Disk File System (DFS). This is default FS
//which uses on normal mounted partitions

#include "vfs.h"
#include "panel.h"
#include "query_windows.h"
#include "infowin.h"
#include "inodewin.h"
#include "c_externs.h"
#include "bookmark.h"
#include "systemfuncs.h"
#include "config_file.h"

int    symlink_as_is = 1;        //Copy symlink as is (symlink as symlink, not a copy of file it points to) 


//Internal routine for copying single file (FList*) to directory (vfs->curdir).
//This routine will be called only for DFS->DFS copying.
//Note: Need add multiple reread if error while reading occurs....
static int internal_copy_file(FList * o, DFS * dfs, dev_t dev, ino_t inode)
{
  char pwds[L_MAXPATH];
  char   *buf;
  struct stat st;
  int    siz, fp1, fp2, wsiz;
  int    bsize = o->size;
  long   total = 0;
  int    l = strlen(dfs->curdir), do_add = 1;

  ::getcwd(pwds,L_MAXPATH);
  if (bsize == 0)
    bsize = L_MAXPATH;
  if (bsize > 31072)
    bsize = 31072;
  if (dfs->curdir[l - 1] != '/')
    {
      if(stat(dfs->curdir, &st)==-1 ||
         (!S_ISDIR(st.st_mode)))
                do_add = 0;
    }
  if (do_add)
    add_path_content(dfs->curdir, o->name);

  // 'Symlink as is' copy part.
  if (symlink_as_is)
    {
      if(lstat(o->name, &st)!=-1 &&
         S_ISLNK(st.st_mode))
        {
          buf = new char[2*L_MAXPATH];
          if ((siz = readlink(o->name, buf, 2*L_MAXPATH)) == -1)
            {
              errno2mes();
              if (do_add)
                upper_path(dfs->curdir);
              delete buf;
              return 0;
            }
          buf[siz] = 0;
          if (symlink(buf, dfs->curdir) == -1)
            {
              if (errno == EEXIST)        //Symlink exist: superseed!
                {
                  if (unlink(dfs->curdir) == -1)        //Unlink it and..

                    {
                      errno2mes();
                      if (do_add)
                        upper_path(dfs->curdir);
                      delete buf;
                      return 0;
                    }
                  if (symlink(buf, dfs->curdir) == -1)        //Link again with new pointer

                    {
                      errno2mes();
                      if (do_add)
                        upper_path(dfs->curdir);
                      delete buf;
                      return 0;
                    }
                }
            }
          if (do_add)
            upper_path(dfs->curdir);
          delete buf;
          return 1;
        }
    }

  //If we are here then we are copying file, not symlink or dir.
  if (stat(dfs->curdir, &st) != -1 && st.st_dev == dev && st.st_ino == inode)
    {
      //Waw! Trying to copy to itself... BAD :)
      if (do_add)
        upper_path(dfs->curdir);
      set_error(_("Error: trying copy to itself"));
      return 0;
    }
  fp2 = creat(dfs->curdir, o->mode | O_EXCL);
  if (fp2 == -1 && errno == EEXIST)
    fp2 = open(dfs->curdir, O_WRONLY | O_TRUNC);
  fp1 = open(o->name,O_RDONLY);
  if (fp2 != -1 && fp1 != -1)
    {
      buf = new char[bsize];
      while ((siz = read(fp1, buf, bsize)) > 0)
        {
          wsiz = write(fp2, buf, siz);
          if (wsiz != siz)
            {
              set_error(_("Error: can't write data to destination"));
              close(fp1);
              close(fp2);
              unlink(dfs->curdir);
              if (do_add)
                upper_path(dfs->curdir);
              delete buf;
              return 0;
            }
          total += wsiz;
          disable_reread=1;
          while (XPending(disp))
            {
              XNextEvent(disp, &ev);
              process_x_event(&ev);
              XFlush(disp);
            }
          disable_reread=0;
        }
      close(fp1);
      close(fp2);
      delete buf;
      if (siz == -1)
        {
          errno2mes();
          unlink(dfs->curdir);
          if (do_add)
            upper_path(dfs->curdir);
          return 0;
        };
      if (do_add)
        upper_path(dfs->curdir);
      if (total != o->size)
        {
          set_error(_("Error: operation incomplete!"));
          return 0;
        }
      return 1;
    }
  errno2mes();
  if(fp1!=-1)
    close(fp1);
  if(fp2!=-1)
    close(fp2);
  unlink(dfs->curdir);
  if (do_add)
    upper_path(dfs->curdir);
  return 0;
}

static int internal_move_file(FList * o, DFS * dfs, dev_t dev, ino_t inode)
{
  struct stat st;
  int    l = strlen(dfs->curdir), do_add = 1, ret;
/*
  disable_reread=1;
  while (XPending(disp))        //Look for XEvent, if yes then processing it and after that go to move..

    {
      XNextEvent(disp, &ev);
      process_x_event(&ev);
      XFlush(disp);
    }
  disable_reread=0;
*/
  if (S_ISLNK(o->mode))                //First of all looking for a symlink
    {
      char buf[L_MAXPATH];
      if((ret=readlink(o->name,buf,L_MAXPATH))==-1)
      {
          errno2mes();
          return 0;
      }
      buf[ret]=0;
      if(stat(dfs->curdir, &st)!=-1 && (st.st_mode & S_IFDIR))
      {
              add_path_content(dfs->curdir, o->name);
              if(symlink(buf,dfs->curdir)==-1)
              {
                      upper_path(dfs->curdir);
                      errno2mes();
                      return 0;
              }
              upper_path(dfs->curdir);
              if(unlink(o->name)==-1)
              {
                      errno2mes();
                      return 0;
              }
      } else
      {
        ret = rename(o->name, dfs->curdir);
        upper_path(dfs->curdir);
        if (ret == -1)
           errno2mes();
        return ret == -1 ? 0 : 1;
      }         
      return 1;
   }

  if (S_ISDIR(o->mode)) //We are moving dir so only try rename it, if fails then return
    {
      add_path_content(dfs->curdir, o->name);
      xncdprintf(("MOVE: %s\n",dfs->curdir));
      if (stat(dfs->curdir, &st) != -1)
	  if(st.st_dev == dev && st.st_ino == inode)
      {
	  upper_path(dfs->curdir);
	  set_error(_("Error: trying copy to itself"));
	  return 0;
      }
      ret = rename(o->name, dfs->curdir);
      upper_path(dfs->curdir);
      if (ret == -1)
        errno2mes();
      return ret == -1 ? 0 : 1;
    }
  if (dfs->curdir[l - 1] != '/')
    {
      if(stat(dfs->curdir, &st)==-1 ||
         S_ISDIR(st.st_mode) == 0)
	 do_add = 0;
    }
  if (do_add)
    add_path_content(dfs->curdir, o->name);
  if ((ret = rename(o->name, dfs->curdir)) == -1 && errno != EXDEV)        //If error and not DIFFERENT_DEVICES then return error

    {
      errno2mes();
      if (do_add)
        upper_path(dfs->curdir);
      return 0;
    }
  if (do_add)
    upper_path(dfs->curdir);
  if (ret != -1)                //No errors, return success!

    return 1;
  ret = internal_copy_file(o, dfs, dev, inode);
  if (ret)
    {
      ret = unlink(o->name);
      if (ret == -1)
        errno2mes();
      return ret == -1 ? 0 : 1;
    }
  return ret;
}

int internal_symlink(FList* o, DFS* dfs)
{
    char from[L_MAXPATH];
    struct stat st;
    int    l = strlen(dfs->curdir), do_add = 1, ret=0;

    if (dfs->curdir[l - 1] != '/')
    {
	if(stat(dfs->curdir, &st)==-1 ||
	   (!S_ISDIR(st.st_mode)))
	    do_add = 0;
    }
    if (do_add)
	add_path_content(dfs->curdir, o->name);

    ::getcwd(from, L_MAXPATH-1);
    add_path_content(from, o->name);

    if(stat(dfs->curdir, &st)!=-1)
	ret=::unlink(dfs->curdir);
    if(ret==-1)
    {
	errno2mes();
	return 0;
    }

    if(symlink_as_is)
    {
	xncdprintf(("SYMLINK: %s --> %s\n", from, dfs->curdir));
	ret=::symlink(from, dfs->curdir);
    }
    else
    {
	xncdprintf(("LINK: %s --> %s\n", from, dfs->curdir));
	ret=::link(from, dfs->curdir);
    }

    if(ret==-1)
	errno2mes();

    if(do_add)
	upper_path(dfs->curdir);

    return ret==-1 ? 0 : 1;
}

///////////////////////  DFS class methods /////////////////////////////


char  *DFS::get_dir_header()
{
  return curdir;
}

int    DFS::make_fullpath()
{
  char   tstr[L_MAXPATH];
  int    l;

// *INDENT-OFF*        
  ::getcwd(tstr, sizeof(tstr));
// *INDENT-ON*        

  l = strlen(tstr);
  if (tstr[l - 1] != '/')
    strcat(tstr, "/");
  strcat(tstr, curdir);
  strcpy(curdir, tstr);
  return 0;
}

int    DFS::direc(char *dir_ch)
{
  FList *ol;
  DIR   *d = NULL;
  struct dirent *dr;
  struct stat st;
  int fsiz;
  int dirf;
  int mmode;
  FList *o;
  int l;
  char lbuf[FLIST_NAME];

  d = opendir(dir_ch);
  if (d == NULL)
    return 0;

  delete_vfs_list();

  while ((dr = readdir(d)) != NULL)
    {
      if ((strcmp(dr->d_name, ".") == 0) || (strcmp(dr->d_name, "..") == 0))
        continue;
      if (hide_hidden && dr->d_name[0] == '.')
        continue;
      lstat(dr->d_name, &st);
      if(S_ISLNK(st.st_mode))
      {
             stat(dr->d_name,&st);
             st.st_mode|=S_IFLNK;
      }
      ol =(FList*) new_alloc(S_FList);
      ol->init(dr->d_name, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
      if(st.st_mode & S_IFLNK)
      {
                     l= ::readlink(ol->name,lbuf,FLIST_NAME);
                     if(l>0)
                     {
                             lbuf[l]=0;
                             stat(ol->name,&st);
                             o =(FList*) new_alloc(S_FList);
                             o->init(lbuf, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
                             ol->link=o;
                     } else
                     {
                             o = (FList*)new_alloc(S_FList);
                             sprintf(lbuf,"Readlink: %s", strerror(errno));
                             o->init(lbuf, 0, 0, 0, 0, st.st_mtime);
                             ol->link=o;
                     }
      }

      if (st.st_mode & S_IFDIR)
        dl.add(ol);
      else
          fl.add(ol);
    }
  closedir(d);

  stat("..", &st);
  bmark->animate();
  ol = (FList*)new_alloc(S_FList);
  ol->init("..", st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
  simple_flist_add(&dl, ol);

  ol = (FList*)new_alloc(S_FList);
  ol->init(".", S_IFDIR, 0, st.st_uid, st.st_gid, st.st_mtime);
  simple_flist_add(&dl, ol);

/*
   ol=dl.next;
   while(ol!=next)
   {
     xncdprintf(("DFS::direc() Dir: %s\n",ol->name));
     ol=ol->next;
   }
   xncdprintf(("-----------------------------------\n"));
 */

  magic_scanner();
  return 1;
}

int DFS::magic_scan()
{
 char tmppath[L_MAXPATH];
 FList *o=dl.next->next;
 if(magic_need_scan)
     return 0;
 options|=MAGIC_WORK;
 while(o)
 {
     process_X();
     if((options & MAGIC_WORK)==0)
     {
         xncdprintf(("DFS::magic_scan() Magic break work\n"));
         return 0;
     }
     if(o->magic[0]==0 || o->magic[0]==' ')
     {
         if(strcmp(o->name, ".."))
         {
           sprintf(tmppath, "%s/%s", curdir, o->name);
           o->add_magic(magic_process(tmppath, &o->st));
         }
     }
     o=o->next;
 }
 options&=~MAGIC_WORK;
 return 1;
}

//Look for file in dir and answer 1 if it is exist
int    DFS::is_file_exist(char *dir, char *file)
{
  char strtmp[L_MAXPATH];
  struct stat dstat;
   
  if (stat(dir, &dstat) == -1)
    return 0;
  if (strlen(dir)+strlen(file)+1 > L_MAXPATH)
    return 0;
  if (S_ISDIR(dstat.st_mode))
    {
      sprintf(strtmp, "%s/%s", dir, file);
      if (lstat(strtmp, &dstat) == -1)
        return 0;
    }
  return 1;
}


int    DFS::copy(FList * cur, VFS * vfs)
{
  int    r;
  struct stat dstat;
#ifndef __FreeBSD__
  char   mstr[L_MAXPATH];
#else
  char   buf1[L_MAXPATH];
  char   buf2[L_MAXPATH];
#endif
  FList otmp;

  if (vfs->is_copy_supported())
    {
      if (vfs->fstype == DFS_TYPE)        //DFS->DFS copy part
        {
          if (!ow_all && !dont_ask &&
              vfs->is_file_exist(vfs->curdir, cur->name))
              {
                init_overwrite_query(_("Copy file"), cur->name);
                wait_for_query_done();
                if (!ow_all && !ow_file)
                  return ow_cancel;        //if cancel we return as error!!!

              }
          ow_file = 0;

	  if(cur->get_dir())
	      chdir(cur->get_dir());

          stat(cur->name, &dstat);
          bmark->animate();
          if (S_ISDIR(dstat.st_mode) && !((cur->mode & S_IFLNK) && symlink_as_is))
            {
#ifdef __FreeBSD__
		quote_first_path(buf1, cur->name);
		quote_first_path(buf2, vfs->curdir);
		if (symlink_as_is)
		    return psystem2("cp", "-rsf", buf1, buf2, NULL);
		else
		    return psystem2("cp", "-rf", buf1, buf2, NULL);
#else
              strcpy(mstr, curdir);
              otmp = *cur;
              //Doing recursive copying file by file.
              r = copydir(&otmp, vfs, dstat.st_mode, mstr);
// *INDENT-OFF*        
              ::chdir(curdir);
// *INDENT-ON*        

              return r;
#endif
            }
          else
            {
              return internal_copy_file(cur, (DFS *)vfs, dstat.st_dev, dstat.st_ino) ? 0 : 255;
            }
        }                        //End of DFS->DFS copy part

      else if (vfs->fstype == AFS_TYPE || vfs->fstype==FTP_TYPE)                //DFS->AFS copy part

        {
       // We check existance here
          if (opt)
            {
              opt = 0;
              return vfs->add_to_vfs_by_list(dl.next);
            };
          if (!ow_all && !dont_ask)
            if (vfs->is_file_exist(vfs->curdir, cur->name))
              {
                init_overwrite_query(_("Copy file"), cur->name);
                wait_for_query_done();
                if (!ow_all && !ow_file)
                  return ow_cancel;        //if cancel we return as error!!!

              }
          ow_file = 0;

          return vfs->add_to_vfs(cur);
        }
    }
  return 255;
}

int    DFS::remove(FList * cur)
{
  int    ret;
  char buf1[L_MAXPATH];

  if(cur->get_dir())
      chdir(cur->get_dir());

  if ((cur->mode & S_IFDIR) == S_IFDIR && (cur->mode & S_IFLNK) != S_IFLNK)
    {
	quote_first_path(buf1, cur->name);
	xncdprintf(("DFS::remove: call external: [rm -rf %s]\n", buf1));
	ret = psystem2("rm", "-rf", buf1, NULL);
	if (ret)
	    set_error(_("Error: RM call failed (Unknown reason)"));
	return ret;
    }
  ret = unlink(cur->name);
  if (ret == -1)
    errno2mes();
  return ret == -1 ? 255 : 0;
}

int    DFS::move(FList * cur, VFS * vfs)
{
  struct stat st;
  int    r = 0;
  char   mstr[L_MAXPATH];
  FList  otmp;

  if (vfs->is_move_supported())
    {
      if (vfs->fstype == DFS_TYPE)        //Moving DFS->DFS
        {
          if (!ow_all && !dont_ask)
            if (vfs->is_file_exist(vfs->curdir, cur->name))
              {
                init_overwrite_query(_("Move file"), cur->name);
                wait_for_query_done();
                if (!ow_all && !ow_file)
                  return ow_cancel;        //if cancel we return as error!!!
              }
          ow_file = 0;

	  if(cur->get_dir())
	      chdir(cur->get_dir());

          lstat(cur->name, &st);
          cur->options=(cur->options & (S_SELECT | S_EXTEND));
          cur->mode=st.st_mode;
          
          bmark->animate();
          if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode))
            {
              strcpy(mstr, curdir);
              otmp = *cur;
              if (internal_move_file(cur, (DFS *)vfs,st.st_dev,st.st_ino)==0)
                 //Use system 'mv' for moving inside single FS.
//              if ((r = psystem2("mv", "-f", cur->name, vfs->curdir, NULL)) != 0)
                {
                  //Doing recursive moving file by file.
                  r = movedir(&otmp, vfs, st.st_mode, mstr);
// *INDENT-OFF*        
                  ::chdir(curdir);
// *INDENT-ON*        

                }
              return r;
            }
          else
            return internal_move_file(cur,(DFS *)vfs,st.st_dev,st.st_ino) ? 0 : 255;
//            return psystem2("mv", "-f", cur->name, vfs->curdir, NULL);
        }
    }
  return 255;
}

int    DFS::copydir(FList * cur, VFS * vfs, mode_t mode, char *mstr)
{
  int    dcoun = 0;
  int    do_add = 0;
  DIR   *d;
  struct dirent *dr;
  struct stat dstat;

  to_infowin(cur->name);
  if (stat(vfs->curdir, &dstat) != -1 && S_ISDIR(dstat.st_mode))
    do_add = 1;
  if (do_add)
    add_path_content(vfs->curdir, cur->name);
  if (vfs->mkdir(vfs->curdir, mode | 0700) == -1)        //Create remote dir
    {
       if(errno==EEXIST)
         {
         if(!(stat(vfs->curdir, &dstat)!=-1 && S_ISDIR(dstat.st_mode)))
           {
             errno2mes();
             if(do_add)
               upper_path(vfs->curdir);
             return 255;
           }
         } else 
           {
             errno2mes();
             if (do_add)
               upper_path(vfs->curdir);
             return 255;
           }
    }
  add_path_content(mstr, cur->name);        //If dir created cd to original

  d = opendir(mstr);
  if (d == NULL)
    {
      errno2mes();
      upper_path(mstr);
      if (do_add)
        upper_path(vfs->curdir);
      return 255;
    }

  while ((dr = readdir(d)) != NULL)        //Copy files only not subdirs
    {
      dcoun++;
      if (strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0)        //Don't even think about copying this..
	{
	  // *INDENT-OFF*        
          if (::chdir(mstr) == -1)
            {
              errno2mes();
              upper_path(mstr);
              if (do_add)
                upper_path(vfs->curdir);
              closedir(d);
              return 255;
            }
        // *INDENT-ON*        

	  lstat(dr->d_name, &dstat);
	  if(S_ISLNK(dstat.st_mode) && !symlink_as_is)
	    stat(dr->d_name,&dstat);
	  if (S_ISDIR(dstat.st_mode) == 0)
	    {
	      strcpy(cur->name, dr->d_name);
	      cur->mode = dstat.st_mode;
	      cur->size = dstat.st_size;
	      bmark->animate();
	      to_infowin(cur->name);
	      if (copy(cur, vfs))
		{
		  closedir(d);
		  upper_path(mstr);
		  if (do_add)
		    upper_path(vfs->curdir);
		  return 255;
		}
	    }
	  else
	    {
	      closedir(d);
	      bmark->animate();
	      strcpy(cur->name, dr->d_name);
	      if (copydir(cur, vfs, dstat.st_mode, mstr))
		{
		  upper_path(mstr);
		  if (do_add)
		    upper_path(vfs->curdir);
		  return 255;
		}
	      else
		{
		  // *INDENT-OFF*        
		  if (::chdir(mstr) == -1)
		    {
		      errno2mes();
		      upper_path(mstr);
		      if (do_add)
			upper_path(vfs->curdir);
		      return 255;
		    };
		  // *INDENT-ON*        
		  d = opendir(mstr);
		  if (d == NULL)
		    {
		      errno2mes();
		      upper_path(mstr);
		      if (do_add)
			upper_path(vfs->curdir);
		      return 255;
		    };
		  xnc_seekdir(d, dcoun);
		}
	    }
	  bmark->animate();
	}
    }

  closedir(d);
  upper_path(mstr);
  if (do_add)
    upper_path(vfs->curdir);
// *INDENT-OFF*        
  if (::chdir(mstr) == -1)
    {
      errno2mes();
      return 255;
    }
// *INDENT-ON*        


  return 0;
}


char*  DFS::magic_file_process(FList *fo)
{
    char *tmps=magic_process(fo->name, &fo->st);
    fo->add_magic(tmps);
    return tmps;
}

int    DFS::movedir(FList * cur, VFS * vfs, mode_t mode, char *mstr)
{
  int    do_add = 0;
  DIR   *d;
  struct dirent *dr;
  struct stat dstat;

  to_infowin(cur->name);
  if (stat(vfs->curdir, &dstat) != -1)
  {
      if(dstat.st_dev == cur->st.st_dev && 
	 dstat.st_ino == cur->st.st_ino)
      {
	  set_error(_("Error: trying copy to itself"));
	  return 255;
      }
      if(S_ISDIR(dstat.st_mode))
      {
	  do_add = 1;
	  add_path_content(vfs->curdir, cur->name);
      }
  }

  if (vfs->mkdir(vfs->curdir, mode | 0200) == -1)        //Create remote dir with write perm for us.
    {
       if(errno==EEXIST)
         {
         if(!(stat(vfs->curdir, &dstat)!=-1 && S_ISDIR(dstat.st_mode)))
           {
             errno2mes();
             if(do_add)
               upper_path(vfs->curdir);
             return 255;
           }
         } else 
           {
             errno2mes();
             if (do_add)
               upper_path(vfs->curdir);
             return 255;
           }
    }
  add_path_content(mstr, cur->name);        //If dir created cd to original

  d = opendir(mstr);
  if (d == NULL)
    {
      errno2mes();
      upper_path(mstr);
      if (do_add)
        upper_path(vfs->curdir);
      return 255;
    }
// *INDENT-OFF*        
  if (::chdir(mstr) == -1)
    {
      errno2mes();
      upper_path(mstr);
      if (do_add)
        upper_path(vfs->curdir);
      closedir(d);
      return 255;
    }
// *INDENT-ON*        

  while ((dr = readdir(d)) != NULL)        //Move files only not subdirs

    {
      lstat(dr->d_name, &dstat);
      if (S_ISDIR(dstat.st_mode) == 0 || S_ISLNK(dstat.st_mode))
        {
          strcpy(cur->name, dr->d_name);
          cur->mode = dstat.st_mode;
          cur->size = dstat.st_size;
          bmark->animate();
          to_infowin(cur->name);
          if (move(cur, vfs))
            {
              closedir(d);
              upper_path(mstr);
              if (do_add)
                upper_path(vfs->curdir);
              return 255;
            }
        }
    }
  rewinddir(d);
  while ((dr = readdir(d)) != NULL)        //Ok, now start moving subdirs...
    {
      if (strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0)        //Don't even think about moving this..
        {
          stat(dr->d_name, &dstat);
          if (S_ISDIR(dstat.st_mode))
            {                        //from here
              closedir(d);
              bmark->animate();
              strcpy(cur->name, dr->d_name);
              if (movedir(cur, vfs, dstat.st_mode, mstr))
                {
                  upper_path(mstr);
                  if (do_add)
                    upper_path(vfs->curdir);
                  return 255;
                }
              else
                {
// *INDENT-OFF*        
                 if (::chdir(mstr) == -1)
                    {
                      errno2mes();
                      upper_path(mstr);
                      if (do_add)
                        upper_path(vfs->curdir);
                      return 255;
                    }
// *INDENT-ON*        

                  d = opendir(mstr);
                  if (d == NULL)
                    {
                      errno2mes();
                      upper_path(mstr);
                      if (do_add)
                        upper_path(vfs->curdir);
                      return 255;
                    }
                }                //to here

              bmark->animate();
            }
        }
    }
  closedir(d);
  get_last_content(mstr, cur->name);
  upper_path(mstr);
  if (do_add)
    upper_path(vfs->curdir);
// *INDENT-OFF*        
  if (::chdir(mstr) == -1)
    {
      errno2mes();
      return 255;
    }
  if (::rmdir(cur->name) == -1           //Delete empty dir 
      && ::unlink(cur->name) == -1)      // Or link...
    {
      errno2mes();
      return 255;
    }
// *INDENT-ON*        

  return 0;
}


//-------------------------------------------------------------------------
// Make symbolic link for cur file to given vfs
//-------------------------------------------------------------------------
int    DFS::symlink(FList * cur, VFS * vfs)
{
    int    r;
    struct stat dstat;
    FList otmp;
    
    if (vfs->is_symlink_supported())
    {
	if (!ow_all && !dont_ask &&
	    vfs->is_file_exist(vfs->curdir, cur->name))
	{
	    init_overwrite_query(_("Symlink file"), cur->name);
	    wait_for_query_done();
	    if (!ow_all && !ow_file)
		return ow_cancel;        //if cancel we return as error!!!
	    
	}
	ow_file = 0;
	
 	if(cur->get_dir())
 	    chdir(cur->get_dir());
	
	r=internal_symlink(cur, (DFS*) vfs);
 	bmark->animate();

// 	stat(cur->name, &dstat);
// 	strcpy(mstr, curdir);
// 	otmp = *cur;
// 	//Doing recursive copying file by file.
// 	r = copydir(&otmp, vfs, dstat.st_mode, mstr);

// 	// *INDENT-OFF*        
 	::chdir(curdir);
// 	// *INDENT-ON*        
	
 	return r ? 0 : 255;
    }
    else
	vfs_error(vfs->vfs_prefix,_("Operation not supported on this VFS"));
    return 255;
}


char* DFS::get_dir_for_dnd()
{
  int l=strlen(curdir)+10;
  char *buf=new char[l];
  strcpy(buf,"file:");
  strcpy(buf+5,curdir);
  return buf;
}

//-------------------------------------------------------------------------
// Scan dirs in branch view and create file list
//-------------------------------------------------------------------------
int    DFS::branch_direc(char *dir_ch, DirCacheManager* dir_cache_man)
{
    int ret=0;
    FList *ol;
    struct stat st;
    char  branch_dir[L_MAXPATH];
    int   counter=0;
    char* cached_dir;

    xncdprintf(("BRANCH_DIREC: for dir %s\n", dir_ch));

    strcpy(branch_dir, dir_ch);

    delete_vfs_list();

    dir_cache_man->clean_all();

    cached_dir=dir_cache_man->add_entry(branch_dir);
    ret=read_one_dir_files(cached_dir);
    
    while(counter < branch_depth)
    {

	ol=dl.next; //get all subdirs here
	dl.next=0;
	counter++;
	while(ol)
	{
	    bmark->animate();
	    strcpy(branch_dir, ol->get_dir());
	    add_path_content(branch_dir, ol->name);
	    cached_dir=dir_cache_man->add_entry(branch_dir);
	    chdir(branch_dir);
	    xncdprintf(("BRANCH_DIREC: scan dir: %s\n",branch_dir));
	    ret=read_one_dir_files(cached_dir);
	    ol=ol->next;
	}
    }

    chdir(dir_ch);
    stat("..", &st);
    bmark->animate();
    ol = (FList*)new_alloc(S_FList);
    ol->init("..", st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
    simple_flist_add(&dl, ol);
  
    ol = (FList*)new_alloc(S_FList);
    ol->init(".", S_IFDIR, 0, st.st_uid, st.st_gid, st.st_mtime);
    simple_flist_add(&dl, ol);
  
/*
  ol=dl.next;
   while(ol!=next)
   {
   xncdprintf(("DFS::direc() Dir: %s\n",ol->name));
   ol=ol->next;
   }
   xncdprintf(("-----------------------------------\n"));
 */

    magic_scanner();
    return ret;
}

//-------------------------------------------------------------------------
// Scan dirs in branch view and create file list
//-------------------------------------------------------------------------
int   DFS::read_one_dir_files(char* dir_ch)
{
    FList *ol;
    DIR   *d = NULL;
    struct dirent *dr;
    struct stat st;
    int fsiz;
    int dirf;
    int mmode;
    FList *o;
    int l;
    char lbuf[FLIST_NAME];
    d = opendir(dir_ch);
    if (d == NULL)
	return 0;
    
    while ((dr = readdir(d)) != NULL)
    {
	if ((strcmp(dr->d_name, ".") == 0) || (strcmp(dr->d_name, "..") == 0))
	    continue;
	if (hide_hidden && dr->d_name[0] == '.')
	    continue;
	lstat(dr->d_name, &st);

	if(S_ISLNK(st.st_mode))
	{
	    stat(dr->d_name,&st);
	    st.st_mode|=S_IFLNK;
	}
	ol =(FList*) new_alloc(S_FList);
	ol->init(dr->d_name, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
	ol->set_dir(dir_ch);
	if(st.st_mode & S_IFLNK)
	{
	    l= ::readlink(ol->name,lbuf,FLIST_NAME);
	    if(l>0)
	    {
		lbuf[l]=0;
		stat(ol->name,&st);
		o =(FList*) new_alloc(S_FList);
		o->init(lbuf, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime, &st);
		ol->link=o;
	    } else
	    {
		o = (FList*)new_alloc(S_FList);
		sprintf(lbuf,"Readlink: %s", strerror(errno));
		o->init(lbuf, 0, 0, 0, 0, st.st_mtime);
		ol->link=o;
	    }
	    o->set_dir(dir_ch);
	}
	
	if (st.st_mode & S_IFDIR)
	    dl.add(ol);
	else
	    fl.add(ol);
    }
    closedir(d);

    return 1;
}


char  *DFS::get_execute_dir(FList *ptr)
{
    if(!ptr)
	return curdir;
    if(!ptr->get_dir())
	return curdir;
    return ptr->get_dir();
}
