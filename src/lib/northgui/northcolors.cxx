/****************************************************************************
*  Copyright (C) 1996-2002 by Leo Khramov
*  email:   leo@xnc.dubna.su
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
// $Id: northcolors.cxx,v 1.4 2002/09/24 14:59:01 leo Exp $
#include "globals.h"
#include "colors.h"
#include "xh.h"
#include "configman.h"

unsigned long keyscol[MAX_BG_COLORS];
unsigned long cols[MAX_FG_COLORS];
unsigned long skincol[256];

//-------------------------------------------------------------------------
// Generate lighter/darker color than given in XColor
//-------------------------------------------------------------------------
unsigned long gencol(XColor & col, int per)
{
  int    r = 0, g = 0, b = 0, pr;
  XColor cl = col;
  r = (int)col.red;
  b = (int)col.blue;
  g = (int)col.green;
  pr = 100 + per;
  r = r * pr / 100;
  g = g * pr / 100;
  b = b * pr / 100;
  if (r > 65535)
    r = 65535;
  if (g > 65535)
    g = 65535;
  if (b > 65535)
    b = 65535;
  cl.red = r;
  cl.green = g;
  cl.blue = b;
  cl.flags = DoGreen | DoRed | DoBlue;
  if (im_AllocColor(disp, defcmp, &cl) != 0)
    return cl.pixel;
  else
    fprintf(stderr, "Error in gencol\n");
  return 0;
}

//-------------------------------------------------------------------------
// Generate colors for bgcols (background colors)
//-------------------------------------------------------------------------
void   generate_bg_colors(char *bgcolor)
{
  if (XParseColor(disp, defcmp, bgcolor, &color) == 0 ||
      im_AllocColor(disp, defcmp, &color) == 0)
    keyscol[1] = WhitePixel(disp, scr);
  else
    keyscol[1] = color.pixel;

  keyscol[0] = gencol(color, -40);
  keyscol[2] = gencol(color,  30);
  keyscol[3] = gencol(color,  10);
  keyscol[4] = gencol(color, -10);
}

//-------------------------------------------------------------------------
// Generate cursor colors for menus, etc
//-------------------------------------------------------------------------
void   generate_cursor_colors(char *bgcolor)
{
    XColor color;
    if (XParseColor(disp, defcmp, bgcolor, &color) == 0 ||
	im_AllocColor(disp, defcmp, &color) == 0)
	cursor_color = WhitePixel(disp, scr);
    else
	cursor_color = color.pixel;
    
    light_cursor_color = gencol(color, 20);
    dark_cursor_color  = gencol(color,-30);
}


unsigned long allocate_color_by_name(char *colname)
{
    XColor color;
    if (XParseColor(disp, defcmp, colname, &color) == 0 ||
	im_AllocColor(disp, defcmp, &color) == 0)
	return WhitePixel(disp, scr);
    else
	return color.pixel;
}

//-------------------------------------------------------------------------
// Load colors from give config manager and generate depend colors
//-------------------------------------------------------------------------
ConfigDefault  color_schema_res[]=
    {
	/* 0 */	{ "northgui.color.background",        "#d5d2de"},
	/* 1 */	{ "northgui.color.shadow",            "#000000"},
	/* 2 */	{ "northgui.color.text",              "#000000"},
	/* 3 */	{ "northgui.color.text_warn",         "#ff0000"},
	/* 4 */	{ "northgui.color.text2",             "#333333"},
	/* 5 */	{ "northgui.color.cursor",            "#bbb4f0"},
	/* 6 */	{ "xnc.panel_color.info",             "#111100"},
	/* 7 */	{ "xnc.panel_color.selected_file",    "#ffff00"},
	/* 8 */	{ "xnc.panel_color.normal_file",      "#000000"},
	/* 9 */	{ "xnc.panel_color.directory_file",   "#1312f8"},
	/*10 */	{ "xnc.panel_color.execution_file",   "#00ff34"},
	/*11 */	{ "xnc.panel_color.link_file",        "#00fff2"},
	/*12 */	{ "xnc.panel_color.afs_file",         "#fb5056"},
	/*13 */	{ "xnc.panel_color.image_file",       "#ff1111"},
	/*14 */ { "xnc.panel_color.extension_file",   "#003399"}
    };
	
const int max_color_res= sizeof(color_schema_res)/sizeof(ConfigDefault);

void   load_color_schema(ConfigManager* cman)
{
    int i;
    char *value;
    for(i=0;i<max_color_res;i++)
    {
	value=cman->get_value(color_schema_res[i].name,
			      color_schema_res[i].default_value);
	switch(i)
	{
	case 0:
	    generate_bg_colors(value);
	    break;
	case 1:
	    shadow_color=allocate_color_by_name(value);
	    break;
	case 2:
	    window_text_color=allocate_color_by_name(value);
	    break;
	case 3:
	    window_warn_color=allocate_color_by_name(value);
	    break;
	case 4:
	    window_text2_color=allocate_color_by_name(value);
	    break;
	case 5:
	    generate_cursor_colors(value);
	    selectpix=cursor_color;
	    break;
	case 6:
	    panel_info_color=allocate_color_by_name(value);
	    break;
	case 7:
	    f_select_color=allocate_color_by_name(value);
	    break;
	case 8:
	    f_normal_color=allocate_color_by_name(value);
	    break;
	case 9:
	    f_dir_color=allocate_color_by_name(value);
	    break;
	case 10:
	    f_exec_color=allocate_color_by_name(value);
	    break;
	case 11:
	    f_link_color=allocate_color_by_name(value);
	    break;
	case 12:
	    f_afs_color=allocate_color_by_name(value);
	    break;
	case 13:
	    f_image_color=allocate_color_by_name(value);
	    break;
	case 14:
	    f_ext_color=allocate_color_by_name(value);
	    break;
	}
    }
}

////////////////////////////////End of file/////////////////////////////////////////




