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
/* $Id: vfs.cxx,v 1.3 2002/10/14 06:09:52 leo Exp $ */
#include "vfs.h"
#include "panel.h"

DFS    dfstmp;
AFS    afstmp;
VFS    defvfs; //These VFSes use for bookmark operations and non-default copying

int    afstmp_defined = 0;

//--------------------------------------------------------------------------
//Detect d (DFS/FTP/AFS):path string and define appropriate VFS
//--------------------------------------------------------------------------
VFS   *define_vfs(char *d)
{
  char  *dd = d;
  int    i;
  for (i = 0; i < 5; i++)
    if (d[i] == 0)
      break;
    else if (d[i] == ':')
      break;
  if (d[i] == ':')
  {
    dd = d + i + 1;
    if (str_ncasecmp(d, "dfs", 3) != 0)
    {
      if (str_ncasecmp(d,"ftp",3) == 0 && panel->panel2->vfs->fstype==FTP_TYPE)
      {
	if(path_cmp(dd, panel->panel2->vfs->curdir)!=0)
	{
	  panel->panel2->vfs->options|=VFSDIFF_BIT;
	  strcpy(panel->panel2->vfs->dest, dd);
	}
	return panel->panel2->vfs;
      }
      if (str_ncasecmp(d, "afs", 3) != 0 && str_ncasecmp(d,"arc",3)!=0)
      {
	vfs_error("VFS", "Wrong VFS type!");
	return NULL;
      }
      else if (panel->panel2->vfs->fstype != AFS_TYPE)
      {
	if (afstmp_defined)
	  return &afstmp;
	vfs_error("VFS", "Second panel must be inside Archive!");
	return NULL;
      }
      else if (afstmp_defined)
	return &afstmp;
      else
      {
	if (panel->panel2->vfs->curdir[0] != '/')
	  panel->panel2->vfs->make_fullpath();
	return panel->panel2->vfs;
      }
    }
  }
  dfstmp.curdir = dd;
  if (dfstmp.curdir[0] != '/')
    dfstmp.make_fullpath();
  return &dfstmp;
}

//--------------------------------------------------------------------------
//Same as previous, but parts are separate: type=(DFS/FTP/AFS) d=path
//--------------------------------------------------------------------------
VFS   *define_vfs(char *type, char *d)
{
  char  *b;
  afstmp_defined = 0;
  if (str_casecmp(type, "dfs") == 0)
  {
    dfstmp.curdir = d;
    if (dfstmp.curdir[0] != '/')
      dfstmp.make_fullpath();
    return &dfstmp;
  }
  else if ((str_casecmp(type, "arc") == 0) || (str_casecmp(type,"afs")==0))
  {
    strcpy(afstmp.fullarcname, d);
    if(d[0]=='/')
      afstmp.dont_construct_path = 1;
    else
      afstmp.dont_construct_path = 0;
    b = strrchr(d, '/');
    afstmp.init_support((b==0 ? d : b + 1));
    afstmp_defined = 1;
    return &afstmp;
  }
  defvfs.curdir = d;
  if (defvfs.curdir[0] != '/')
    defvfs.make_fullpath();
  return &defvfs;
}


