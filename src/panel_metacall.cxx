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
/* $Id: panel_metacall.cxx,v 1.9 2003/12/25 12:43:18 leo Exp $ */
#include "panel.h"
#include "pluginman.h"

//==================== File for metacall Lister methods ===================

// ListerPtrCaller - static callers for context menu building
static ListerPtrCaller   copy_caller(&panel, &Lister::metacall_copy);
static ListerPtrCaller   move_caller(&panel, &Lister::metacall_move);
static ListerPtrCaller   rename_caller(&panel, &Lister::metacall_rename);
static ListerPtrCaller   delete_caller(&panel, &Lister::metacall_delete);
static ListerPtrCaller   execute_caller(&panel, &Lister::metacall_execute);
static ListerPtrCaller   view_caller(&panel, &Lister::metacall_view);
static ListerPtrCaller   edit_caller(&panel, &Lister::metacall_edit);
static ListerPtrCaller   chmod_caller(&panel, &Lister::metacall_change_attributes);
static ListerPtrCaller   chown_caller(&panel, &Lister::metacall_change_owner);
static ListerPtrCaller   mkdir_caller(&panel, &Lister::metacall_make_dir);
static ListerPtrCaller   manpage_caller(&panel, &Lister::metacall_man_page);
static ListerPtrCallerChar create_tar_gz(&panel, &Lister::pack_files, "test-meta.tar.gz"); 
static ListerPtrCallerChar create_zip(&panel, &Lister::pack_files, "test-meta.zip"); 

//-------------------------------------------------------------------------
// Build and show context menu!
//-------------------------------------------------------------------------
void Lister::show_context_menu(int ix, int iy)
{
    max_context_idx=0;

    if(selfiles)
	build_context_menu_for_selected_files();
    else
	build_context_menu_for_current_file();

    if(context_menu)
	context_menu->set_menu(context_items, max_context_idx);
    else
	context_menu=guiplugin->new_Menu(context_items, max_context_idx);
    context_menu->setpos(ix-3,iy-3);
    context_menu->init(w);
    context_menu->show();
}

void Lister::build_context_menu_for_current_file()
{
    VFS* other_vfs=panel2->vfs;
    int is_dir=(cur->mode & S_IFDIR)==S_IFDIR;
    int is_archive=0;
    static char tmpstr1[L_MAXPATH];
    static char tmpstr2[L_MAXPATH];

    context_items[max_context_idx++].init(_noop("Man page"), 
					  menu_text_color_idx,
					  0, 0, &manpage_caller, help_icon,
					  menu_function, AMan);

    context_items[max_context_idx++].init(_noop("Make directory "),
					  menu_text_color_idx,
					  0, 0, &mkdir_caller, mkdir_icon,
					  menu_function, AMakeDir);

    AFS::construct_archive_name(create_tar_gz.get_data(), 
				selfiles ? vfs->get_dir_header() : cur->name, 
				selfiles ? 1 : (cur->file_type==file_is_dir),
				".tar.gz");
    sprintf(tmpstr1, _("Create archive %s"), create_tar_gz.get_data());
    context_items[max_context_idx++].init(tmpstr1,
					  menu_text_color_idx,
					  0, 0, &create_tar_gz, pack_icon,
					  menu_function, ACreateArchive);

    AFS::construct_archive_name(create_zip.get_data(), 
				selfiles ? vfs->get_dir_header() : cur->name,
				selfiles ? 1 : (cur->file_type==file_is_dir),
				".zip");
    sprintf(tmpstr2, _("Create archive %s "), create_zip.get_data());
    context_items[max_context_idx++].init(tmpstr2,
					  menu_text_color_idx,
					  0, 0, &create_zip, pack_icon,
					  menu_function, ACreateArchive);

    if(is_dir)
	context_items[max_context_idx++].init(_noop("Go to subdirectory "),
					      menu_text_color_idx,
					      0, 0, &execute_caller, exec_icon,
					      menu_function, AExecute);
    else
	if(is_exec(cur))
	{
	    if(vfs->is_exec_supported())
		context_items[max_context_idx++].init(_noop("Execute "), 
						      menu_text_color_idx,
						      0, 0, &execute_caller, 
						      exec_icon, menu_function,
						      AExecute);
	} else
	{
	    if(cur->options & S_EXTEND)
	    {
		if((is_archive=is_it_afs_file(cur->name))!=0)
		    context_items[max_context_idx++].init(_noop("Browse into archive "), 
							  menu_text_color_idx,
							  0, 0, &execute_caller,
							  exec_icon, menu_function,
							  AExecute);
		else
		    context_items[max_context_idx++].init(_noop("Execute association "), 
							  menu_text_color_idx,
							  0, 0, &execute_caller,
							  exec_icon, menu_function,
							  AExecute);
	    } else
		context_items[max_context_idx++].init(_noop("Execute with ... "), 
						      menu_text_color_idx,
						      0, 0, &execute_caller,
						      exec_icon, menu_function,
						      AExecute);

	}   

    if(other_vfs->is_copy_supported())
	context_items[max_context_idx++].init(_noop("Copy file(s)"), menu_text_color_idx,
					      0, 0, &copy_caller, copy_icon,
					      menu_function, AFCopy);
    if(other_vfs->is_move_supported())
    {
	context_items[max_context_idx++].init(_noop("Move/Rename file(s)"), menu_text_color_idx,
					      0, 0, &move_caller, move_icon,
					      menu_function, AFMove);
	if(!selfiles)
	    context_items[max_context_idx++].init(_noop("Rename file"), menu_text_color_idx,
					      0, 0, &rename_caller, no_icon,
					      menu_function, ARenameFile);
    }
    if(vfs->is_del_supported())
	context_items[max_context_idx++].init(_noop("Delete file(s)"), menu_text_color_idx,
					      0, 0, &delete_caller, delete_icon,
					      menu_function, AFDelete);
    if(!is_dir && !is_archive && vfs->is_exec_supported())
    {
	context_items[max_context_idx++].init(_noop("View file"), menu_text_color_idx,
					      0, 0, &view_caller, view_icon,
					      menu_function, AFView);
	if(vfs->fstype==DFS_TYPE)
	    context_items[max_context_idx++].init(_noop("Edit file"), menu_text_color_idx,
						  0, 0, &edit_caller, edit_icon,
						  menu_function, AFEdit);
    }

    if(vfs->is_chmod_supported())
	context_items[max_context_idx++].init(_noop("Change file(s) attributes"), menu_text_color_idx,
					      0, 0, &chmod_caller, attr_icon,
					      menu_function, AAttributes);
    if(vfs->is_chmod_supported())
	context_items[max_context_idx++].init(_noop("Change file(s) owner"), menu_text_color_idx,
					      0, 0, &chown_caller, owner_icon,
					      menu_function, AChown);
}

void Lister::build_context_menu_for_selected_files()
{
    build_context_menu_for_current_file();
}
//============================ Meta calls ================================


//-------------------------------------------------------------------------
// Calls copy file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_copy(MetaCallData)
{
    xncdprintf(("Lister::metacall_copy\n"));
    copy();
    return 1;
}

//-------------------------------------------------------------------------
// Calls move file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_move(MetaCallData)
{
    xncdprintf(("Lister::metacall_move\n"));
    move();
    return 1;
}

//-------------------------------------------------------------------------
// Calls rename file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_rename(MetaCallData)
{
    xncdprintf(("Lister::metacall_rename\n"));
    rename();
    return 1;
}

//-------------------------------------------------------------------------
// Calls delete file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_delete(MetaCallData)
{
    xncdprintf(("Lister::metacall_delete\n"));
    del();
    return 1;
}

//-------------------------------------------------------------------------
// Calls activate file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_execute(MetaCallData)
{
    xncdprintf(("Lister::metacall_execute\n"));
    mouse=1;
    activate();
    mouse=0;
    return 1;
}

//-------------------------------------------------------------------------
// Calls view file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_view(MetaCallData)
{
    xncdprintf(("Lister::metacall_view\n"));
    view(1);
    return 1;
}

//-------------------------------------------------------------------------
// Calls edit file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_edit(MetaCallData)
{
    xncdprintf(("Lister::metacall_edit\n"));
    edit();
    return 1;
}

//-------------------------------------------------------------------------
// Calls change_attributes file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_change_attributes(MetaCallData)
{
    xncdprintf(("Lister::metacall_change_attributes\n"));
    change_attributes();
    return 1;
}

//-------------------------------------------------------------------------
// Calls change owner file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_change_owner(MetaCallData)
{
    xncdprintf(("Lister::metacall_change_owner\n"));
    init_chown_dialog(cur);
    return 1;
}

//-------------------------------------------------------------------------
// Calls man page file method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_man_page(MetaCallData)
{
    xncdprintf(("Lister::metacall_man_page\n"));
    newtextfile(_("Man page"), this, FMANPAGE);
    return 1;
}

//-------------------------------------------------------------------------
// Calls make dir method
// return 1 on success
//-------------------------------------------------------------------------
int  Lister::metacall_make_dir(MetaCallData)
{
    xncdprintf(("Lister::metacall_make_dir\n"));
    newtextfile(_("Make directory"), this, FMKDIR);
    return 1;
}

// ---------------------------  End of file ------------------------------
