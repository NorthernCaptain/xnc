#ifndef ICONSETS_H
#define ICONSETS_H

#include "iconman.h"

enum IconsetType
    {
	file_type_iconset=0,
	menu_iconset=1,
	garbage_iconset=2
    };

enum MenuTypeIconset
    {
	no_icon=-1,
	copy_icon=0,
	move_icon=1,
	delete_icon=2,
	mkdir_icon=3,
	edit_icon=4,
	view_icon=5,
	ftp_icon=6,
	print_icon=7,
	exec_icon=8,
	disk_icon=9,
	proc_icon=10,
	find_icon=11,
	owner_icon=12,
	attr_icon=13,
	config_icon=14,
	help_icon=15,
	menu_icon=16,
	remount_icon=17,
	pack_icon=18
    };

enum FileTypeIconset
    {
	f_normal_icon=5,
	f_dir_icon=3,
	f_link_icon=4,
	f_afs_icon=0,
	f_image_icon=6,
	f_exec_icon=2,
	f_ext_icon=1,
	f_updir_icon=7
    };

extern void load_iconsets();
extern void set_current_iconset_style();

#endif

