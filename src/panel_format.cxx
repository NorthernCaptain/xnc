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
/* $Id: panel_format.cxx,v 1.2 2002/08/05 10:43:00 leo Exp $ */
#include "panel.h"
#include "systemfuncs.h"

#define FMT_FILE_NAME           'F'
#define FMT_FILE_ATTR           'A'
#define FMT_FILE_ATTR_OCT       'O'
#define FMT_FILE_OWNER          'U'
#define FMT_FILE_SIZE           'S'
#define FMT_FILE_TIME           'D'
#define FMT_DELIMITER           '.'
#define FMT_SEPARATOR           '|'

#define FMT_FILE_NAME_S         'f'
#define FMT_FILE_ATTR_S         'a'
#define FMT_FILE_ATTR_OCT_S     'o'
#define FMT_FILE_OWNER_S        'u'
#define FMT_FILE_SIZE_S         's'
#define FMT_FILE_TIME_S         'd'

// This file contains structs and methods for drawing file info on panel

int Lister::file_display_border=1;

//-------------------------------------------------------------------------
// ******************* DisplayFileFormat methods **********************
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Constructor of the struct
//-------------------------------------------------------------------------
Lister::DisplayFileFormat::DisplayFileFormat()
{
    strcpy(format,"F");
    max_interval=1;
    max_filename_len=20;    
}


//-------------------------------------------------------------------------
// Initing intervals while decoding format
//-------------------------------------------------------------------------
void Lister::DisplayFileFormat::init(int max_len, 
				     XFontStruct *ifontstr)
{
    int text_width, from_x, i_len, i, fname_len,
	fname_interval_idx=-1;

    fontstr=ifontstr;

    text_width=XTextWidth(fontstr,"M",1);

    fname_len=max_filename_len*text_width + 2*Lister::file_display_border;

    if(fname_len>=max_len) //Too small display area -> only filename
    {
	max_interval=1;
	intervals[0].init(0,max_len-1,&Lister::fmt_draw_file_name);
	return;
    }

    max_interval=0;
    from_x=0;

    for(i=0;format[i];i++)
    {
	switch(format[i])
	{
	case FMT_FILE_NAME:  //File name
	case FMT_FILE_NAME_S:  //File name
	    i_len=get_interval_len(fname_len, max_len);
	    fname_interval_idx=max_interval;
	    xncdprintf(("display_format: filename %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_name);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_FILE_TIME:
	case FMT_FILE_TIME_S:
	    fname_len=XTextWidth(fontstr,"00-MMM-MMMM/00:00",FILE_TIMESTAMP-1) +
		2*Lister::file_display_border;
	    i_len=get_interval_len(fname_len, max_len);
	    xncdprintf(("display_format: file_time %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_time);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_FILE_ATTR_OCT:
	case FMT_FILE_ATTR_OCT_S:
	    fname_len= XTextWidth(fontstr,"0000",4) + 2*Lister::file_display_border;
	    i_len=get_interval_len(fname_len, max_len);
	    xncdprintf(("display_format: file_attr_oct %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_attr_oct);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_FILE_ATTR:
	case FMT_FILE_ATTR_S:
	    fname_len= XTextWidth(fontstr,"--w--w--w-",10) + 2*Lister::file_display_border;
	    i_len=get_interval_len(fname_len, max_len);
	    xncdprintf(("display_format: file_attr %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_attr);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_FILE_OWNER:
	case FMT_FILE_OWNER_S:
	    fname_len= XTextWidth(fontstr,"w",1)*10 + 2*Lister::file_display_border;
	    i_len=get_interval_len(fname_len, max_len);
	    xncdprintf(("display_format: file_owner %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_owner);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_FILE_SIZE:
	case FMT_FILE_SIZE_S:
	    fname_len= XTextWidth(fontstr,"0.000.000.000",13) + 2*Lister::file_display_border;
	    i_len=get_interval_len(fname_len, max_len);
	    xncdprintf(("display_format: file_size %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_size);
	    max_len-=i_len;
	    from_x+=i_len;
	    break;

	case FMT_DELIMITER:
	    from_x+=Lister::file_display_border;
	    max_len-=Lister::file_display_border;
	    break;

	case FMT_SEPARATOR:
	    i_len=get_interval_len(2*Lister::file_display_border+1, max_len);
	    xncdprintf(("display_format: file_attr_oct %d,[%d/%d]\n",
			from_x, i_len, max_len));
	    intervals[max_interval++].init(from_x, i_len, 
					   &Lister::fmt_draw_file_separator);
	    from_x+=2*Lister::file_display_border+1;
	    max_len-=2*Lister::file_display_border+1;
	    break;
	}
	if(max_len<=0)
	    break;
    }
    if(max_len>0 && fname_interval_idx!=-1) 
	//we have additional free space, let's expand filename
    {
	intervals[fname_interval_idx++].len+=max_len;
	for(i=fname_interval_idx;i<max_interval;i++)
	    intervals[i].from_x+=max_len;
    }
}

//-------------------------------------------------------------------------
// return interval length
//-------------------------------------------------------------------------
int Lister::DisplayFileFormat::get_interval_len(int item_len, int max_len)
{
    if(item_len<max_len)
	return item_len;
    return max_len;
}


//-------------------------------------------------------------------------
// sets the format
//-------------------------------------------------------------------------
void Lister::DisplayFileFormat::set_format(char* fmt)
{
    strcpy(format, fmt);
}


//-------------------------------------------------------------------------
// sets clip area using XSetClipRectangles
//-------------------------------------------------------------------------
void Lister::set_clip_area(int x, int y, DisplayFileFormat::Interval ival)
{
    XRectangle rect;
    rect.x=x+ival.from_x;
    rect.y=y;
    rect.width = ival.len;
    rect.height = main_area.item_h;
    XSetClipRectangles(disp, wgcw, 0, 0, &rect, 1, Unsorted);
}


//-------------------------------------------------------------------------
// *********************** Lister draw format methods *********************
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Draw file name
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_name(int x, int y, int ty, unsigned int color,
				DisplayFileFormat::Interval ival, 
				FList* ol)
{
    set_clip_area(x,y,ival);
    XDrawString(disp, w, wgcw, x + ival.from_x, ty, ol->name, ol->namelen);
}

//-------------------------------------------------------------------------
// Draw file date/time
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_time(int x, int y, int ty, unsigned int color,
				DisplayFileFormat::Interval ival, 
				FList* ol)
{
    set_clip_area(x,y,ival);
    XDrawString(disp, w, wgcw, x + ival.from_x, ty, ol->chr_time, FILE_TIMESTAMP-1);
}

//-------------------------------------------------------------------------
// Draw file owner.group
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_owner(int x, int y, int ty, unsigned int color,
				 DisplayFileFormat::Interval ival, 
				 FList* ol)
{
    char buf[128];
    sprintf(buf,"%s.%s",ol->user, ol->group);
    set_clip_area(x,y,ival);
    XDrawString(disp, w, wgcw, x + ival.from_x, ty, buf, strlen(buf));
}

//-------------------------------------------------------------------------
// Draw file attributes in octal mode
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_attr_oct(int x, int y, int ty, unsigned int color,
				DisplayFileFormat::Interval ival, 
				FList* ol)
{
    char buf[32];
    set_clip_area(x,y,ival);
    sprintf(buf,"%04o",ol->mode & 07777);
    XDrawString(disp, w, wgcw, x + ival.from_x, ty, buf, 4);
}

//-------------------------------------------------------------------------
// Draw file attributes in -rwxr-xr-x mode
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_attr(int x, int y, int ty, unsigned int color,
				DisplayFileFormat::Interval ival, 
				FList* ol)
{
    char buf[32];
    set_clip_area(x,y,ival);
    printperm(buf,ol->mode);
    XDrawString(disp, w, wgcw, x + ival.from_x, ty, buf, 10);
}

//-------------------------------------------------------------------------
// Draw separator
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_separator(int x, int y, int ty, unsigned int color,
				     DisplayFileFormat::Interval ival, 
				     FList* ol)
{
    set_clip_area(x,y,ival);
    XDrawLine(disp, w, wgcw, x + ival.from_x ,
	      y, x + ival.from_x , y + main_area.item_h - 1);
}

//-------------------------------------------------------------------------
// Draw file size
//-------------------------------------------------------------------------
void Lister::fmt_draw_file_size(int x, int y, int ty, unsigned int color,
				DisplayFileFormat::Interval ival, 
				FList* ol)
{
    char buf[32];
    unsigned int len, tlen;
    set_clip_area(x,y,ival);
    dig2ascii(buf,ol->size);
    len=strlen(buf);
    tlen=XTextWidth(fontstr,buf,len);
    XDrawString(disp, w, wgcw, x + ival.from_x + ival.len - tlen -
		fontstr->max_bounds.rbearing - fontstr->min_bounds.lbearing, 
		ty, buf, len);
}

//----------------------------- End of file -------------------------------
