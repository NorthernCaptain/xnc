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
/* $Id: iconsets.cxx,v 1.7 2002/10/18 14:53:43 leo Exp $ */
#include "iconsets.h"
#include "config_file.h"

#include "iconset1mask.xbm"
#include "iconset2mask.xbm"

extern char iconset2_chr[];
extern int  iconset2_chr_size;

extern char iconset1_chr[];
extern int  iconset1_chr_size;

void load_iconsets()
{
    default_iconman->load_iconset(menu_iconset, "menu_items", 
				  ResId("xnci_iconset1", iconset1_chr, iconset1_chr_size),
				  ResId("iconset1mask", iconset1mask_bits, 
					iconset1mask_width, iconset1mask_height),
				  0,0, 16,16, 19);
    default_iconman->load_iconset(file_type_iconset, "file_types", 
				  ResId("xnci_iconset2", iconset2_chr, iconset2_chr_size),
				  ResId("iconset2mask", iconset2mask_bits, 
					iconset2mask_width, iconset2mask_height),
				  0,16, 16,16, 8);

}

void set_current_iconset_style()
{
    default_iconman->change_iconset_style(file_type_iconset, 
					  0, 16*iconset_number);
    default_iconman->change_iconset_style(menu_iconset, 
					  0, 16*iconset_number);
}
