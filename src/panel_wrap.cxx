/****************************************************************************
*  Copyright (C) 1996-2003 by Leo Khramov
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
/* $Id: panel_wrap.cxx,v 1.2 2003/01/30 16:12:21 leo Exp $ */
#include "panel.h"
#include "bookmark.h"
#include "query_windows.h"
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Fill and show list of remounting devices
//-------------------------------------------------------------------------
void Lister::show_remount_device_list()
{
    int i;
    for (i = 0; i < mntmax; i++)
	panlist[i] = mnt[i];
    newpanel(_("Remount Device ?"), mntmax, panmnt);
}


//-------------------------------------------------------------------------
// Change attributes of the file or files
//-------------------------------------------------------------------------
void Lister::change_attributes()
{
    if(selfiles==0) 
	::attrib(cur,FSETATTR); 
    else 
	attrib2();
}

//-------------------------------------------------------------------------
// Invert selection of files
//-------------------------------------------------------------------------
void Lister::invert_mask_selection()
{
    FList *ko;
    ko = dl.next->next;
    while (ko != NULL)
    {
	selected(ko);
	ko = ko->next;
    };
    showfilelist();
}

//-------------------------------------------------------------------------
// Add/remove selection of files by mask
//-------------------------------------------------------------------------
void Lister::add_selection_by_mask()
{
    selectmask(this, FSELBYMASK);
}

void Lister::del_selection_by_mask()
{
    selectmask(this, FDESELMASK);
}

//-------------------------------------------------------------------------
// Add/remove selection of files by extension
//-------------------------------------------------------------------------
void Lister::add_selection_by_ext()
{
    select_file_mask(YES);
}

void Lister::del_selection_by_ext()
{
    select_file_mask(NO);
}


//-------------------------------------------------------------------------
// Add new entry to bookmarm if possible
//-------------------------------------------------------------------------
void Lister::add_to_bookmark()
{
    bmark->add_toempty_book(vfs->get_info_for_bookmark(), vfs->fstype);
}


//-------------------------------------------------------------------------
// Add new entry to bookmarm if possible
//-------------------------------------------------------------------------
void Lister::switch_hidden_files_showing()
{
    hide_hidden ^=1;
    reread_all();
}

//-------------------------------------------------------------------------
// Switch panel into brief display mode
//-------------------------------------------------------------------------
void Lister::switch_to_brief()
{
  set_display_mode(BriefMode);
  reread();
  subm->switches(0);
}

//-------------------------------------------------------------------------
// Switch panel into brief display mode
//-------------------------------------------------------------------------
void Lister::switch_to_full()
{
  set_display_mode(FullMode);
  reread();
  subm->switches(1);
}

//-------------------------------------------------------------------------
// Switch panel into brief display mode
//-------------------------------------------------------------------------
void Lister::switch_to_custom()
{
  set_display_mode(CustomMode);
  reread();
  subm->switches(2);
}

//-------------------------------------------------------------------------
// Switch panel into one column
//-------------------------------------------------------------------------
void Lister::switch_to_one_column()
{
  repanel(1);
  XClearWindow(disp, w);
  expose();
  subm->switches(3);
}

//-------------------------------------------------------------------------
// Switch panel into two columns
//-------------------------------------------------------------------------
void Lister::switch_to_two_columns()
{
  repanel(2);
  XClearWindow(disp, w);
  expose();
  subm->switches(4);
}

//-------------------------------------------------------------------------
// Switch panel into three columns
//-------------------------------------------------------------------------
void Lister::switch_to_three_columns()
{
  repanel(3);
  XClearWindow(disp, w);
  expose();
  subm->switches(5);
}


//-------------------------------------------------------------------------
// Switch panel into four columns
//-------------------------------------------------------------------------
void Lister::switch_to_four_columns()
{
  repanel(4);
  XClearWindow(disp, w);
  expose();
  subm->switches(6);
}

//-------------------------------------------------------------------------
// Switch panel sort method by name
//-------------------------------------------------------------------------
void Lister::sort_by_name()
{
    sortbyname();
    subm->switches(7);
}

//-------------------------------------------------------------------------
// Switch panel sort method by ext
//-------------------------------------------------------------------------
void Lister::sort_by_ext()
{
    sortbyext();
    subm->switches(8);
}

//-------------------------------------------------------------------------
// Switch panel sort method by size
//-------------------------------------------------------------------------
void Lister::sort_by_size()
{
    sortbysize();
    subm->switches(9);
}

//-------------------------------------------------------------------------
// Switch panel sort method by time
//-------------------------------------------------------------------------
void Lister::sort_by_time()
{
    sortbytime();
    subm->switches(10);
}

//-------------------------------------------------------------------------
// Switch panel sort method by unsort
//-------------------------------------------------------------------------
void Lister::sort_by_unsort()
{
    sortbyunsort();
    subm->switches(11);
}

//-------------------------------------------------------------------------
// Switch panel reverse sort order
//-------------------------------------------------------------------------
void Lister::switch_reverse()
{
    rev_order^=1;
    reread();
    subm->switches(12);
}

//-------------------------------------------------------------------------
// Open window for cd command
//-------------------------------------------------------------------------
void Lister::show_cd_window()
{
    newtextfile(_("Change directory"), this, FCDDIR, 1, _("Directory:"));
}
