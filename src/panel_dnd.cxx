/****************************************************************************
*  Copyright (C) 2002 by Leo Khramov
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
/* $Id: panel_dnd.cxx,v 1.2 2002/08/05 10:43:00 leo Exp $ */
#include "panel.h"
#include "bookmark.h"

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//                  Drag and Drop (XDnd v3) methods for panel
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//Processing internal drops on panel
//-------------------------------------------------------------------------
int Lister::internal_dnd_drop(int mx, int my, DndAction action)
{
    FList *o;
    char fname[L_MAXPATH];
    VFS   *ivfs;
    PanelArea area;
    xncdprintf(("Detected internal drop operation\n"));
    if(panel==this)
    {
	xncdprintf(("We are dragger and dropper, all in one\n"));
	area=head.get_area_by_xy(mx,my);
	if(area!=areaNone)
	{
	    if(area==areaHeaderInactive)
	    {
		xncdprintf(("User dropped on opposite header\n"));
		if(action==dnd_move)
		    move();
		else
		    copy();
	    }
	    xncdprintf(("Nothing to do here - user mistake\n"));
	} else
	{
	    if (vfs->fstype != DFS_TYPE)
	    {
		XBell(disp, 0);
		return 0;
	    }
	    
	    o=find_entry_by_coord(mx,my);
	    xncdprintf(("Dropped on entry: 0x%x at %d,%d\n",o, mx, my));
	    if(o && strcmp(o->name,"..") && (o->mode & S_IFDIR)==S_IFDIR)
	    {
		sprintf(fname, "%s/%s", curdir, o->name);
		ivfs = define_vfs("DFS", fname);
		if(action==dnd_move)
		    move(ivfs);
		else
		    copy(ivfs);
	    }	      
	}
	
    } else
    {
	xncdprintf(("Dragger is opposite panel\n"));
	o=find_entry_by_coord(mx,my);
	xncdprintf(("Dropped on entry: 0x%x at %d,%d\n",o, mx, my));
	if(o && strcmp(o->name,"..") && (o->mode & S_IFDIR)==S_IFDIR)
	{
	    sprintf(fname, "%s/%s", curdir, o->name);
	    ivfs = define_vfs("DFS", fname);
	    if(action==dnd_move)
		move(ivfs);
	    else
		copy(ivfs);
	} else	      
	    if(action==dnd_move)
		panel->move();
	    else
		panel->copy();
    }
    return 1;
}

//-------------------------------------------------------------------------
//We are dropped from internal bookmark
//-------------------------------------------------------------------------
void  Lister::bookmark_dropper(int cy, DndAction action)
{
    int wx,wy;
    VFS* ivfs = bmark->get_vfs_by_coord(cy);
    int startx=default_dnd_man->get_drag_startx();
    int starty=default_dnd_man->get_drag_starty();
    default_dnd_man->global2win_coords(w,startx,starty,wx,wy);
    if (ivfs)
    {
	if (action==dnd_move)
	    move(ivfs);
	else
	    copy(ivfs);
    }
    else 
	if (wy > 25)
	{
	    xncdprintf(("Calling try_to_add_to_bookmark\n"));
	    try_to_add_to_bookmark(cy);
	}
	else
	{
	    xncdprintf(("Calling add_toempty_book_by_coord\n"));
	    bmark->add_toempty_book_by_coord(cy, 
					     vfs->get_info_for_bookmark(), 
					     vfs->fstype);
	}
}

//-------------------------------------------------------------------------
//Begin and do the dragging itself
//-------------------------------------------------------------------------
int   Lister::start_dragging()
{
  Atom  atext=XInternAtom(disp,"text/uri-list",False);
  char* ddir=vfs->get_dir_for_dnd();
  int nums=(selfiles ? selfiles : 1);
  if(ddir==0)  //Does vfs supports dnd exporting?
  {
      xncdprintf(("VFS does not support dnd export\n"));
      return 0;
  }

  DndData *dat=new DndData((strlen(ddir)+256)*nums,atext);
  if(!selfiles)
    dat->add_file_entry(ddir,cur->name);
  else
  {
    FList *o=dl.next;
    while(o)
    {
      if(o->options & S_SELECT)
	dat->add_file_entry(ddir,o->name);
      o=o->next;
    }
  }
  xncdprintf(("Prepared DndData buffer: [%s]\n",dat->data));
  
  dnd_drag_now(dat);

  delete dat;
  delete ddir;
  return 1;
}


//-------------------------------------------------------------------------
// Function for processing drops from DnD motions
//-------------------------------------------------------------------------
int  Lister::dnd_drop(DndData* dd, int x, int y, DndAction action)
{
    Lister *slavepan=panel2;
    int i=0,mx,my;
    FList *o;
    char fname[L_MAXPATH];
    
    default_dnd_man->global2win_coords(w,x,y,mx,my);
    
    if(default_dnd_man->is_dragging()) //This is internal drag_n_drop
    {
	delete dd;
	return internal_dnd_drop(mx,my,action);
    }
    
    if(dd->break_to_filelist())
    {
	VFS_Info* vfsi;
	dd->show_filelist();
	vfsi=get_vfs_info_for_file(dd->file_list[0]);
	o=find_entry_by_coord(mx,my);
	if(o && strcmp(o->name,"..") && (o->mode & S_IFDIR)==S_IFDIR)
	{
	    slavepan=panel2;
	    vfs->chdir(o->name);
	    vfs->getcwd(curdir,1020);
	    direc(curdir);
	    xncdprintf(("Drop on entry: [%s] coords %d,%d\n",o->name,mx,my));
	}
	else
	{
	    if(head.get_area_by_xy(mx,my)==areaHeaderInactive)
		slavepan=this;
	    xncdprintf(("Drop on whole panel\n"));
	}

	slavepan->switch_to_me();
	slavepan->push_n_switch(vfsi->vfstype,vfsi->full_path);
	slavepan->clean_selection();
	for(i=0;i<dd->file_listlen;i++)
	{
	    get_last_content(dd->file_list[i],fname);
	    xncdprintf(("file->list: [%s]\n",fname));
	    o=slavepan->find_file_entry(fname);
	    if(o)
		slavepan->select_file(o);
	}
	get_last_content(dd->file_list[0],fname);
	xncdprintf(("DROP: Stepping to name %s\n",fname));
	if(action==dnd_move)
	    slavepan->move();
	else
	    slavepan->copy();
	slavepan->step_to_name(fname);
	delete vfsi;
    }
    delete dd;
    xncdprintf(("*********End dnd_drop**********\n"));
    return 1;
}

