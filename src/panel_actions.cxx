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
/* $Id: panel_actions.cxx,v 1.4 2003/01/31 16:30:37 leo Exp $ */
#include "panel.h"
#include "mouse_actions.h"
#include "key.h"
#include "x_actions.h"
#include "au_sup.h"
#include "config_file.h"

const Time DBLCLICK = 350;

int    dnd_startx = -10; 
int    dnd_starty = -10;
int    dnd_start_valid=0;
int    dnd_enabled = 0;

PanelAction Lister::mouse_actions[]=
    {
	{AMouseSetCurrent,   &Lister::mouse_action_set_current},
	{AExecute,           &Lister::mouse_action_execute},
	{AMoveLeft,          &Lister::mouse_action_left},
	{AMoveRight,         &Lister::mouse_action_right},
	{AInsert,            &Lister::mouse_action_select},
	{AMoveDown,          &Lister::mouse_action_down},
	{AMoveUp,            &Lister::mouse_action_up},
	{AMovePageDn,        &Lister::mouse_action_page_down},
	{AMovePageUp,        &Lister::mouse_action_page_up},
	{AMoveHome,          &Lister::mouse_action_home},
	{AMoveEnd,           &Lister::mouse_action_end},
	{AFDelete,           &Lister::mouse_action_delete},
	{AFCopy,             &Lister::mouse_action_copy},
	{AFMove,             &Lister::mouse_action_move},
	{AAttributes,        &Lister::mouse_action_change_attributes},
	{AFEdit,             &Lister::mouse_action_edit},
	{AFView,             &Lister::mouse_action_view},
	{AChown,             &Lister::mouse_action_chown},
	{ASelectFileMask,    &Lister::mouse_action_select_by_ext},
	{ADeselectFileMask,  &Lister::mouse_action_deselect_by_ext},
	{AInodeInfo,         &Lister::mouse_action_inode_info},
	{AQuickFInfo,        &Lister::mouse_action_quick_file_info},
	{AContextMenu,       &Lister::mouse_action_context_menu},
	{AEmpty, 0}
    };


//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// common caller for mouse_action* methods
//-------------------------------------------------------------------------
int Lister::call_method_by_action_for_mouse(MouseData data)
{
    int i;
    default_mouse_action_man->get_action_by_mouse_data(data);
    xncdprintf(("Lister::call_method_by_action_for_mouse - action %d\n",data.action));
    i=0;
    while(mouse_actions[i].action!=AEmpty)
    {
	if(mouse_actions[i].action == data.action)
	    return (this->*(mouse_actions[i]).method)(&data);
	i++;
    }
    return 0;
}


//-------------------------------------------------------------------------
// Mouse button was pressed - processing this event
//-------------------------------------------------------------------------
void Lister::process_mouse_press_event(XEvent& ev)
{
    PanelArea area;
    int i,k;

    dnd_startx = ev.xbutton.x;
    dnd_starty = ev.xbutton.y;
    dnd_start_valid = 1;
    clear_ff();

    area=head.get_area_by_xy(ev.xbutton.x, ev.xbutton.y);

    panel=this;

    //Hard coded processing for header clicks
    if (area != areaNone) //we are inside header
    {
	if (area == areaHeaderInactive) //Slave tab on our panel
	    panel = panel2;
	else 
	    if (area == areaHeaderMenuButton) //Menu button on our panel
	    {
		if (subm)
		{
		    subm->setpos(ev.xbutton.x - 2, ev.xbutton.y - 2);
		    subm->init(w);
		    subm->show();
		};
	    }
    }
    
    guiSetInputFocus(disp, panel->w, RevertToNone, CurrentTime);

    //Main area - use mouse_actions
    if (main_area.get_area_by_xy(ev.xbutton.x, ev.xbutton.y) ==
	areaMainActive)
    {
	if (cur == NULL)
	    return;

	if (curn == -1)
	    curn = lcurn;

	call_method_by_action_for_mouse(MouseData(ev));
    }
    else 
    { //Status bar specific features.
	process_mouse_press_on_status_bar(ev);
    }
    prflg = 1;
}


//-------------------------------------------------------------------------
// Got mouse motion event - processing it
//-------------------------------------------------------------------------
void  Lister::process_mouse_motion_event(XEvent& ev)
{
    int cx, cy, rx, ry, action;
    Window rw, cw;
    unsigned kmask;
    MouseData data(ev);

    if((ev.xmotion.state & (Button1Mask | Button2Mask | Button3Mask))==0)
	return;

    action=default_mouse_action_man->get_action_by_mouse_data(data);

    XQueryPointer(disp, Main, &rw, &cw, &rx, &ry, &cx, &cy, &kmask);

    cx-=x; cy-=y;

    //    xncdprintf(("process_mouse_motion_event: cx=%d, cy=%d\n",cx,cy));

    if(action == AEmpty)
    {
	if (dnd_start_valid && (abs(dnd_starty - cy) > 10 || 
				abs(dnd_startx - cx) > 10) &&
	    (ev.xmotion.state & Button1Mask))
	{
	    xncdprintf(("It's time to start dragging\n"));
	    start_dragging();
	}
    } else
	if (main_area.get_area_by_xy(cx,cy) == areaMainActive)
	    call_method_by_action_for_mouse(ev);
}


//-------------------------------------------------------------------------
// Got mouse button release event - processing it
//-------------------------------------------------------------------------
void Lister::process_mouse_release_event(XEvent& ev)
{
    Time delta=ev.xbutton.time - dbtime;
    dnd_start_valid=0;
    if (bclk) //detecting double click
    {
	xncdprintf(("Release button 2 time at: %d\n",ev.xbutton.time));
	prflg = 0;
	bclk = 0;
	if (delta <= DBLCLICK &&
	    abs(ev.xbutton.x - lmx) <= 10 && abs(ev.xbutton.y - lmy) <= 10 &&
	    main_area.get_area_by_xy(ev.xbutton.x, ev.xbutton.y) ==
	    areaMainActive)
	    call_method_by_action_for_mouse(MouseData(ev,DoubleClick));
	else
	    prflg = 1;
    }
    if (prflg)
    {
	xncdprintf(("Release button 1 time at: %d\n",ev.xbutton.time));
	dbtime = ev.xbutton.time;
	lmx = ev.xbutton.x;
	lmy = ev.xbutton.y;
	prflg = 0;
	bclk = 1;
    }
}

//-------------------------------------------------------------------------
// Got event on status bar - let's detect and process it
//-------------------------------------------------------------------------
void Lister::process_mouse_press_on_status_bar(XEvent& ev)
{
    PanelArea area;
    area=status_area.get_area_by_xy(ev.xbutton.x, ev.xbutton.y);
    if (curn == -1)
	curn = lcurn;
    switch(area)
    {
    case areaStatusBarButtonLeft:
	left();
	break;
	
    case areaStatusBarButtonRight:
	right();
	break;
	
    case areaStatusBar:
	if(option_bits & STATUS_ATTR)
	{
	    if (ev.xbutton.state & ShiftMask)
		init_chown_dialog(cur);
	    else 
		if(selfiles==0) 
		    ::attrib(cur,FSETATTR); 
		else 
		    attrib2();
	}
	break;
    }
}

//========================================================================


//       --------===========   Mouse actions   ===========-----------


//========================================================================

//-------------------------------------------------------------------------
// Move up one row
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_up(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_left called with %x\n",pdata));
    go_upline();
    return 1;
}

//-------------------------------------------------------------------------
// Move down one row
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_down(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_left called with %x\n",pdata));
    go_downline();
    return 1;
}

//-------------------------------------------------------------------------
// Move down one row
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_page_down(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_left called with %x\n",pdata));
    page_down();
    return 1;
}

//-------------------------------------------------------------------------
// Move down one row
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_page_up(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_left called with %x\n",pdata));
    page_up();
    return 1;
}

//-------------------------------------------------------------------------
// Move left one column
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_left(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_left called with %x\n",pdata));
    left();
    return 1;
}

//-------------------------------------------------------------------------
// Move right one column
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_right(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    right();
    return 1;
}

//-------------------------------------------------------------------------
// Doing selection/deslection of item
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_select(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_select called with %x\n",pdata));
    if(!pdata)
	return 0;

    if(set_cursor_by_xy_no_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y) ||
       pdata->xev->type!=MotionNotify)
    {
	selected(cur);
	showitem(cur, curn);
    }
    return 1;
}

//-------------------------------------------------------------------------
// Doing execute on mouse event
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_execute(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action execute called with %x\n",pdata));
    if(!pdata)
	return 0;

    set_cursor_by_xy_no_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    showitem(cur, curn);
    mouse = 1;
    activate();
    mouse = 0;
    return 1;
}

//-------------------------------------------------------------------------
// Set cursor to current position from event
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_set_current(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_set_current called with %x\n",pdata));
    if(!pdata)
	return 0;

    set_cursor_by_xy_no_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    showitem(cur, curn);
    return 1;
}

//-------------------------------------------------------------------------
// Move home
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_home(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    go_home();
    return 1;
}

//-------------------------------------------------------------------------
// Move end
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_end(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    go_end();
    return 1;
}

//-------------------------------------------------------------------------
// Delete current file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_delete(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    del();
    return 1;
}

//-------------------------------------------------------------------------
// Copy current file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_copy(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    copy();
    return 1;
}

//-------------------------------------------------------------------------
// Move current file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_move(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    move();
    return 1;
}

//-------------------------------------------------------------------------
// Change file attributes
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_change_attributes(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    change_attributes();
    return 1;
}

//-------------------------------------------------------------------------
// Edit file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_edit(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    edit();
    return 1;
}

//-------------------------------------------------------------------------
// View file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_view(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    view(1);
    return 1;
}

//-------------------------------------------------------------------------
// Change file owner
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_chown(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    init_chown_dialog(cur);
    return 1;
}

//-------------------------------------------------------------------------
// Select files by extension of current one
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_select_by_ext(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    select_file_mask(YES);
    return 1;
}

//-------------------------------------------------------------------------
// Deselect files by extension of current one
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_deselect_by_ext(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    select_file_mask(NO);
    return 1;
}

//-------------------------------------------------------------------------
// Toggle Inode window
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_inode_info(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    toggle_inodewin();
    return 1;
}

//-------------------------------------------------------------------------
// Show quick file info
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_quick_file_info(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    do_quick_file();
    return 1;
}

//-------------------------------------------------------------------------
// Show context menu for current file
// return 1 on success
//-------------------------------------------------------------------------
int Lister::mouse_action_context_menu(MouseData* pdata)
{
    xncdprintf(("Lister::mouse_action_right called with %x\n",pdata));
    set_cursor_by_xy_with_redraw(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    show_context_menu(pdata->xev->xbutton.x, pdata->xev->xbutton.y);
    return 1;
}


//------------------------------ End of file -----------------------------------

