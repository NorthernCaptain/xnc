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
/* $Id: panel_vlook.cxx,v 1.2 2002/08/05 10:43:00 leo Exp $ */
#include "panel.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                          HeaderSize class methods
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Calculating panel header sizes
//----------------------------------------------------------------------------
void Lister::HeaderSize::recalc_data(PageOrientation orientation,
				     int ix, int iy, int il, int ih)
{

    switch(orientation)
    {
    case OnePage:
	x=ix+Lister::header_inner_delta; 
	y=iy+Lister::header_inner_delta;
	l=il-Lister::header_inner_delta*2;
	h=ih-Lister::header_inner_delta;
	active_from_x=x;
	active_to_x=x+l-1;
	break;

    case RightPage:
	x=ix, y=iy;
	l=il, h=ih;
	active_from_x=x+l-l * Lister::header_active_percent/100 - 1;
	active_to_x=x+l-1;
	break;

    case LeftPage:
	x=ix, y=iy;
	l=il, h=ih;
	active_from_x=x;
	active_to_x=x+l * Lister::header_active_percent/100 - 1;
	break;
    }
    menu_button_from_x=active_from_x;
    menu_button_to_x=menu_button_from_x + Lister::header_menu_button_width;

    xncdprintf(("Header params: %d,%d %dx%d [%d,%d]\n",
		x,y,l,h,active_from_x, active_to_x));
}

//----------------------------------------------------------------------------
// Return PanelArea name detected  from given coords for header
//----------------------------------------------------------------------------
PanelArea Lister::HeaderSize::get_area_by_xy(int ix, int iy)
{
    if(ix>=x && ix<x+l && iy>=y && iy<y+h) //areaHeader
    {
	if(ix>=active_from_x && ix<=active_to_x && 
	   iy>=y && iy<y+h) //active area
	{
	    if(ix>=menu_button_from_x && ix<=menu_button_to_x 
	       && iy>=y && iy<y+h) //menu button area
	    {
		return areaHeaderMenuButton;
	    }
	    return areaHeaderActive;
	}
	return areaHeaderInactive;
	
    }
    return areaNone;
}
//----------------------------------------------------------------------------
// Is position inside header?
//----------------------------------------------------------------------------
int Lister::HeaderSize::is_inside(int ix, int iy)
{
    return (ix>=x && ix<x+l && iy>=y && iy<y+h);
} 

//----------------------------------------------------------------------------
// Is position inside active header part?
//----------------------------------------------------------------------------
int Lister::HeaderSize::is_inside_active(int ix, int iy)
{
    return (ix>=active_from_x && ix<=active_to_x && iy>=y && iy<y+h);
} 

//----------------------------------------------------------------------------
// Is position inside active menu button?
//----------------------------------------------------------------------------
int Lister::HeaderSize::is_inside_menu_button(int ix, int iy)
{
    return (ix>=menu_button_from_x && ix<=menu_button_to_x 
	    && iy>=y && iy<y+h);
} 

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                         MainSize class methods
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void Lister::MainSize::recalc_data(int ix, int iy, int il, int ih, int column_len)
{
    x=ix;
    y=iy;
    h=ih;
    l=il;
    active_from_x=x+Lister::main_area_delta_x;
    active_from_y=y+Lister::main_area_delta_y*2;
    active_l=l-2*Lister::main_area_delta_x;
    active_h=h-3*Lister::main_area_delta_y;
    active_to_x=active_from_x+active_l-1;
    active_to_y=active_from_y+active_h-1;
    item_h=text_h + Lister::main_area_item_border*2;
    item_l=column_len - Lister::main_area_column_border*2; 
}

PanelArea Lister::MainSize::get_area_by_xy(int ix, int iy)
{
    if(ix>=x && ix<x+l && iy>=y && iy<y+h) //areaMain*
    {
	if(ix>=active_from_x && ix<=active_to_x && 
	   iy>=active_from_y && iy<=active_to_y) //active area
	{
	    return areaMainActive;
	}
	return areaMain;
    }
    return areaNone;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                         StatusBarSize class methods
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void Lister::StatusBarSize::recalc_data(int ix, int iy, int il, int fixh, int fixy)
{
    x=ix;
    y=iy-Lister::status_area_height;
    h=Lister::status_area_height;
    l=il;
    active_from_x=x+Lister::status_area_delta_x;
    active_l=l - Lister::status_area_delta_x*2 - Lister::status_area_button_l*2;
    active_to_x=active_from_x+active_l-1;
    active_from_y=y;
    active_h=h - Lister::status_area_delta_y;
    active_to_y=active_from_y + active_h - 1;
    active_text_y=active_from_y + (active_h - fixh + 1)/2 + fixy;
    Lister::status_area_button_h=active_h;
    if(Lister::status_area_button_l==0)
	Lister::status_area_button_l=active_h;
    left_button_x=active_to_x + 1;
    right_button_x=left_button_x + Lister::status_area_button_l;
    buttons_y=active_from_y;
}

PanelArea Lister::StatusBarSize::get_area_by_xy(int ix, int iy)
{
    if(ix>=x && ix<x+l && iy>=y && iy<y+h) //areaMain*
    {
	if(ix>=left_button_x && ix<left_button_x+Lister::status_area_button_l)
	    return areaStatusBarButtonLeft;
	if(ix>=right_button_x && ix<right_button_x+Lister::status_area_button_l)
	    return areaStatusBarButtonRight;

	return areaStatusBar;
    }
    return areaNone;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//                         Lister class drawing methods
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int Lister::header_draw_diagonal_offset=5;
int Lister::header_height=28; //Header height in pixels
int Lister::header_active_percent=55; //Percent of length in active
int Lister::header_menu_button_width=24;
int Lister::header_inner_delta=5; //Inner delta x for OnePage panels

int Lister::main_area_delta_x=2; //Delta x from vertical edges of area
int Lister::main_area_delta_y=2; //Delta y ...
int Lister::main_area_column_border=3; //border around each column
int Lister::main_area_item_border=1; //border around text of the item
int Lister::main_area_icons_width=20; //max width of icons to display

int Lister::status_area_height=30; //Height of status bar
int Lister::status_area_delta_x=4;
int Lister::status_area_delta_y=4;
int Lister::status_area_button_l=0;
int Lister::status_area_button_h=24;
int Lister::status_border_around_font=5;

int Lister::header_menu_button_border=5;
int Lister::header_border_around_font=5;

//Draw head of the panel according to its orientation.
//Don't display any text on it, just draw it.
void Lister::vlook_draw_head()
{
    int head_max_y=head.y+head.h-1;
    int head_max_x=head.x+head.l-1;

    unsigned int menu_len;
    int menu_y;

    switch(orientation)
    {
    case LeftPage:
    case RightPage:
	XSetForeground(disp, gcw, light_bg_color);
	//draw bottom lines, left and right from head
	XDrawLine(disp, w, gcw, 
		  head.x, head_max_y, 
		  head.active_from_x, head_max_y);
	XDrawLine(disp, w, gcw, 
		  head.active_to_x, head_max_y, 
		  head_max_x, head_max_y);
	//draw light part of the head
	XDrawLine(disp, w, gcw, 
		  head.active_from_x,head_max_y,
		  head.active_from_x,head.y+Lister::header_draw_diagonal_offset);
	//draw left diagonal
	XDrawLine(disp, w, gcw, 
		  head.active_from_x,head.y+Lister::header_draw_diagonal_offset,
		  head.active_from_x+Lister::header_draw_diagonal_offset,head.y);
	//draw upper head horizontal line
	XDrawLine(disp, w, gcw, 
		  head.active_from_x+Lister::header_draw_diagonal_offset,head.y,
		  head.active_to_x-Lister::header_draw_diagonal_offset,head.y);

	XSetForeground(disp, gcw, dark_bg_color);
	//right diagonal (dark color)
	XDrawLine(disp, w, gcw, 
		  head.active_to_x-Lister::header_draw_diagonal_offset,head.y,
		  head.active_to_x,head.y+Lister::header_draw_diagonal_offset);
	//right vertical side of head
	XDrawLine(disp, w, gcw, 
		  head.active_to_x,head.y+Lister::header_draw_diagonal_offset,
		  head.active_to_x,head_max_y-1);

	//draw menu button
	menu_len=head.menu_button_to_x - head.menu_button_from_x - 
	    Lister::header_menu_button_border*2;
	menu_y=head.y + head.h/2 - menu_len/2;

	XSetForeground(disp, gcw, light_bg_color);

	XDrawArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2, menu_y, 
		 menu_len, menu_len, 45*64, -180*64);

	XFillArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2 + 3, menu_y + 3, 
		 menu_len-6, menu_len-6, 0, 360*64);


	XSetForeground(disp, gcw, dark_bg_color);
	XDrawArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2, menu_y, 
		 menu_len, menu_len, 45*64, 180*64);
	/*
	XDrawLine(disp, w, gcw,
		  head.menu_button_from_x+Lister::header_menu_button_border*2, menu_y,
		  head.menu_button_to_x, menu_y); 

	XDrawLine(disp, w, gcw,
		  head.menu_button_from_x+Lister::header_menu_button_border*2, menu_y,
		  head.menu_button_from_x+Lister::header_menu_button_border*2, menu_y+menu_len);
	
	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw,
		  head.menu_button_from_x+Lister::header_menu_button_border*2, menu_y+menu_len,
		  head.menu_button_to_x, menu_y+menu_len); 

	XDrawLine(disp, w, gcw,
		  head.menu_button_to_x, menu_y,
		  head.menu_button_to_x, menu_y+menu_len); 
	*/
	//draw inactive head
	if(orientation==LeftPage)
	{
	    XSetForeground(disp, gcw, light_bg_color);
	    XDrawLine(disp, w, gcw, 
		      head.active_to_x-2, head.y+2,
		      head_max_x-Lister::header_draw_diagonal_offset-2, head.y+2);
	    XSetForeground(disp, gcw, dark_bg_color);
	    XDrawLine(disp, w, gcw, 
		      head_max_x-Lister::header_draw_diagonal_offset-2, head.y+2,
		      head_max_x-2, head.y+2+Lister::header_draw_diagonal_offset);
	    XDrawLine(disp, w, gcw, 
		      head_max_x-2, head.y+2+Lister::header_draw_diagonal_offset,
		      head_max_x-2, head_max_y - 1);
	} else
	{
	    XSetForeground(disp, gcw, light_bg_color);
	    XDrawLine(disp, w, gcw, 
		      head.active_from_x+2,head.y+2,
		      head.x+Lister::header_draw_diagonal_offset+2, head.y+2);
	    XDrawLine(disp, w, gcw, 
		      head.x+Lister::header_draw_diagonal_offset+2, head.y+2,
		      head.x+2, head.y+2+Lister::header_draw_diagonal_offset);
	    XDrawLine(disp, w, gcw, 
		      head.x+2, head.y+2+Lister::header_draw_diagonal_offset,
		      head.x+2, head_max_y-1);

	}
	break;

    case OnePage:
    default:
	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw, 
		  head.x, head.y, head.active_to_x, head.y);

	XDrawLine(disp, w, gcw, 
		  head.x, head.y, head.x, head.y+head.h-1);

	XSetForeground(disp, gcw, dark_bg_color);
	XDrawLine(disp, w, gcw, 
		  head.x, head.y+head.h-1, head.active_to_x, head.y+head.h-1);

	XDrawLine(disp, w, gcw, 
		  head.active_to_x, head.y, 
		  head.active_to_x, head.y+head.h-1);

	menu_len=head.menu_button_to_x - head.menu_button_from_x - 
	    Lister::header_menu_button_border*2;
	menu_y=head.y + head.h/2 - menu_len/2;

	XSetForeground(disp, gcw, light_bg_color);

	XDrawArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2, menu_y, 
		 menu_len, menu_len, 45*64, -180*64);

	XFillArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2 + 3, menu_y + 3, 
		 menu_len-6, menu_len-6, 0, 360*64);


	XSetForeground(disp, gcw, dark_bg_color);
	XDrawArc(disp, w, gcw,
		 head.menu_button_from_x + head.h/2 - menu_len/2, menu_y, 
		 menu_len, menu_len, 45*64, 180*64);

	break;
    }
}

//Draw frame inside the panel window by corners
void Lister::vlook_draw_frame()
{
    switch(orientation)
    {
    case LeftPage:
    case RightPage:
 	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw, 
		  0,head.y+head.h,
		  0,h-1);

	XSetForeground(disp, gcw, dark_bg_color);
	XDrawLine(disp, w, gcw, 
		  l-1,head.y+head.h,
		  l-1,h-1);
	XDrawLine(disp, w, gcw, 
		  0,h-1,
		  l-1,h-1);
	//draw internal border for active main area
	XDrawLine(disp, w, gcw,
		  main_area.active_from_x,main_area.active_from_y-Lister::main_area_delta_y,
		  main_area.active_to_x,main_area.active_from_y-Lister::main_area_delta_y);
	XDrawLine(disp, w, gcw,
		  main_area.active_from_x,main_area.active_from_y-Lister::main_area_delta_y,
		  main_area.active_from_x,h-1-Lister::main_area_delta_y);

 	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw,
		  main_area.active_from_x,h-1-Lister::main_area_delta_y,
		  main_area.active_to_x,h-1-Lister::main_area_delta_y);
	XDrawLine(disp, w, gcw,
		  main_area.active_to_x,main_area.active_from_y-Lister::main_area_delta_y,
		  main_area.active_to_x,h-1-Lister::main_area_delta_y);
	break;

    case OnePage:
    default:
 	XSetForeground(disp, gcw, light_bg_color);
	XDrawLine(disp, w, gcw, 
		  0,0,
		  0,h-1);
	XDrawLine(disp, w, gcw, 
		  0,0,
		  l-1,0);

	XDrawLine(disp, w, gcw,
		  Lister::main_area_delta_x,h-1-Lister::main_area_delta_y,
		  l-1-Lister::main_area_delta_x,h-1-Lister::main_area_delta_y);
	XDrawLine(disp, w, gcw,
		  l-1-Lister::main_area_delta_x,Lister::main_area_delta_y,
		  l-1-Lister::main_area_delta_x,h-1-Lister::main_area_delta_y);

	XSetForeground(disp, gcw, dark_bg_color);
	XDrawLine(disp, w, gcw, 
		  l-1,0,
		  l-1,h-1);
	XDrawLine(disp, w, gcw, 
		  0,h-1,
		  l-1,h-1);

	XDrawLine(disp, w, gcw,
		  Lister::main_area_delta_x,Lister::main_area_delta_y,
		  l-1-Lister::main_area_delta_x,Lister::main_area_delta_y);
	XDrawLine(disp, w, gcw,
		  Lister::main_area_delta_x,Lister::main_area_delta_y,
		  Lister::main_area_delta_x,h-1-Lister::main_area_delta_y);
	break;
    }

}

//----------------------------------------------------------------------------
// Draw header string, cuts string length to header length
//----------------------------------------------------------------------------
void Lister::vlook_display_header_string(char* str, int str_len,
					 unsigned long text_color)
{
    int display_len=str_len, delta_len=0;
    unsigned int free_pixs=head.active_to_x - head.menu_button_to_x - 
	Lister::header_inner_delta*2;
    int free_len=free_pixs/fixl;
    int text_x=head.menu_button_to_x+Lister::header_inner_delta;
    int text_y=head.y+(head.h-fixh+1)/2+fixy;

    if(orientation!=OnePage)
	text_y+=Lister::main_area_delta_y;

    if(free_len < str_len)
    {
	display_len=free_len;
	delta_len=str_len-free_len;
    }
    XClearArea(disp,w,text_x, text_y-fixy,free_pixs+1,fixh+1,0);
    if(shadow)
    {
	XSetForeground(disp, gcw, shadow_color);
	XDrawString(disp, w, gcw, text_x+1, text_y+1, str+delta_len, display_len);
    }
    XSetForeground(disp, gcw, text_color);
    XDrawString(disp, w, gcw, text_x, text_y, str+delta_len, display_len);
}

//----------------------------------------------------------------------------
// Draw header string, cuts string length to header length
//----------------------------------------------------------------------------
void Lister::vlook_display_header_inactive_string(char* str, int str_len,
						  unsigned long text_color)
{
    int display_len=str_len, delta_len=0;

    unsigned int free_pixs;
    int free_len;
    int text_x;
    int text_y;
    
    switch(orientation)
    {
    case LeftPage:
	free_pixs=head.l - head.active_to_x - 
	    Lister::header_inner_delta*4;
	free_len=free_pixs/fixl;
	text_x=head.active_to_x+Lister::header_inner_delta*2;
	text_y=head.y+head.h/2-fixh/2+fixy+1;
	break;
	
    case RightPage:
	free_pixs=head.active_from_x - head.x - 
	    Lister::header_inner_delta*4;
	free_len=free_pixs/fixl;
	text_x=head.x+Lister::header_inner_delta*2;
	text_y=head.y+head.h/2-fixh/2+fixy+1;
	break;

    default:
	return;
    }

    if(orientation!=OnePage)
	text_y+=Lister::main_area_delta_y;

    if(free_len < str_len)
    {
	display_len=free_len;
	delta_len=str_len-free_len;
    }
    XClearArea(disp,w,text_x, text_y-fixy,free_pixs,fixh,0);
    XSetForeground(disp, gcw, text_color);
    XDrawString(disp, w, gcw, text_x, text_y, str+delta_len, display_len);
}

//----------------------------------------------------------------------------
// Draw column separators on panel if columns > 1
//----------------------------------------------------------------------------
void Lister::vlook_draw_column_separators()
{
    int i,px;

    if (col > 1)
    {
	for (i = 0, px = pixels_per_column+main_area.active_from_x; 
	     i < col - 1; 
	     i++, px += pixels_per_column)
	{
	    XSetForeground(disp, gcw, light_bg_color);
	    XDrawLine(disp, w, gcw, px - 1, 
		      main_area.active_from_y + Lister::main_area_column_border, 
		      px - 1, main_area.active_from_y + main_area.active_h - 
		      Lister::main_area_column_border*2);
	    XSetForeground(disp, gcw, dark_bg_color);
	    XDrawLine(disp, w, gcw, px, 
		      main_area.active_from_y + Lister::main_area_column_border, 
		      px, main_area.active_from_y + main_area.active_h - 
		      Lister::main_area_column_border*2);
	}
    }
}

//----------------------------------------------------------------------------
// Draw two buttons on status bar < >
//----------------------------------------------------------------------------
void Lister::vlook_draw_status_buttons()
{
    urect(w, gcw, 
	  status_area.left_button_x, status_area.buttons_y,
	  Lister::status_area_button_l, Lister::status_area_button_h);
    urect(w, gcw, 
	  status_area.right_button_x, status_area.buttons_y,
	  Lister::status_area_button_l, Lister::status_area_button_h);
    
    if(shadow)
    {
	XSetForeground(disp, gcw, shadow_color);
	XDrawString(disp, w, gcw, 
		    status_area.left_button_x +
		    (Lister::status_area_button_l - fixl)/2 +1,
		    status_area.active_text_y+1, "<", 1);
	XDrawString(disp, w, gcw, 
		    status_area.right_button_x +
		    (Lister::status_area_button_l - fixl)/2 +1,
		    status_area.active_text_y+1, ">", 1);
    }
    XSetForeground(disp, gcw, statusbar_color);
	XDrawString(disp, w, gcw, 
		    status_area.left_button_x +
		    (Lister::status_area_button_l - fixl)/2,
		    status_area.active_text_y, "<", 1);
	XDrawString(disp, w, gcw, 
		    status_area.right_button_x +
		    (Lister::status_area_button_l - fixl)/2,
		    status_area.active_text_y, ">", 1);
}


//----------------------------------------------------------------------------
// Recalculate all nessesary data for proper drawing
//----------------------------------------------------------------------------
void  Lister::vlook_recalc_all_data()
{
    orientation=PageOrientation(lay);
    Lister::header_height= fixh + Lister::header_border_around_font*2 -
	(orientation==OnePage ? 0 : Lister::main_area_delta_y);
    Lister::header_menu_button_width=Lister::header_height;
    head.recalc_data(orientation,0,0,l,Lister::header_height);

    Lister::status_area_height= fixh + Lister::status_border_around_font*2;

    status_area.recalc_data(0,h-1,l,fixh,fixy);

    calc_statusbar_offsets();

    main_area.recalc_data(0,head.h+head.y,
			  l,h-head.h-head.y-status_area.h,
			  pixels_per_column);
}

