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
/* $Id: mouse_actions.cxx,v 1.3 2002/11/12 16:08:23 leo Exp $ */

#include "xh.h"
#include "mouse_actions.h"
#include "config_file.h"

const unsigned int mouse_state_mask= ControlMask | ShiftMask | Button1Mask | Button2Mask | Button3Mask;
const unsigned int mouse_button_mask= Button1 | Button2 | Button3 | Button4 | Button5;

MouseActionManager *default_mouse_action_man;

//-------------------------------------------------------------------------
// Data for translating XEvent MouseMotion events to MouseState
//-------------------------------------------------------------------------
MouseItem MouseActionManager::mouse_motion_items[]=
    {
	{ mButton1Motion,      "button_1_motion",              Button1Mask,                0, OneClick},
	{ mButton2Motion,      "button_2_motion",              Button2Mask,                0, OneClick},
	{ mButton3Motion,      "button_3_motion",              Button3Mask,                0, OneClick},
	{ mButton1ShiftMotion, "shift+button_1_motion",        ShiftMask    | Button1Mask, 0, OneClick},
	{ mButton2ShiftMotion, "shift+button_2_motion",        ShiftMask    | Button2Mask, 0, OneClick},
	{ mButton3ShiftMotion, "shift+button_3_motion",        ShiftMask    | Button3Mask, 0, OneClick},
	{ mButton1CtrlMotion,  "ctrl+button_1_motion",         ControlMask  | Button1Mask, 0, OneClick},
	{ mButton2CtrlMotion,  "ctrl+button_2_motion",         ControlMask  | Button2Mask, 0, OneClick},
	{ mButton3CtrlMotion,  "ctrl+button_3_motion",         ControlMask  | Button3Mask, 0, OneClick},
	{ mButtonNone,         "",                                                      0, 0, OneClick}
    };

//-------------------------------------------------------------------------
// Data for translating XEvent ButtonPress events to MouseState
//-------------------------------------------------------------------------
MouseItem MouseActionManager::mouse_press_items[]=
    {
	{ mButton1,      "button_1_pressed",                         0, Button1, OneClick},
	{ mButton1Double,"button_1_double_click",                    0, Button1, DoubleClick},
	{ mButton2,      "button_2_pressed",                         0, Button2, OneClick},
	{ mButton3,      "button_3_pressed",                         0, Button3, OneClick},
	{ mButton4,      "button_4_pressed",                         0, Button4, OneClick},
	{ mButton5,      "button_5_pressed",                         0, Button5, OneClick},
	{ mButton1Shift, "shift+button_1_pressed",           ShiftMask, Button1, OneClick},
	{ mButton2Shift, "shift+button_2_pressed",           ShiftMask, Button2, OneClick},
	{ mButton3Shift, "shift+button_3_pressed",           ShiftMask, Button3, OneClick},
	{ mButton4Shift, "shift+button_4_pressed",           ShiftMask, Button4, OneClick},
	{ mButton5Shift, "shift+button_5_pressed",           ShiftMask, Button5, OneClick},
	{ mButton1Ctrl,  "ctrl+button_1_pressed",          ControlMask, Button1, OneClick},
	{ mButton2Ctrl,  "ctrl+button_2_pressed",          ControlMask, Button2, OneClick},
	{ mButton3Ctrl,  "crtl+button_3_pressed",          ControlMask, Button3, OneClick},
	{ mButton4Ctrl,  "ctrl+button_4_pressed",          ControlMask, Button4, OneClick},
	{ mButton5Ctrl,  "ctrl+button_5_pressed",          ControlMask, Button5, OneClick},
	{ mButtonNone,   "",                                         0,       0, OneClick}
    };


//-------------------------------------------------------------------------
// Action name and actions
//-------------------------------------------------------------------------
ActionNames MouseActionManager::action_names[]=
    {
	{AMouseSetCurrent,  "AMouseSetCurrent"},
	{AExecute,          "AExecute"},
	{AInsert,           "ASelectCurrent"},
	{AContextMenu,      "AShowContextMenu"},
	{AMoveLeft,         "AMoveLeft"},
	{AMoveRight,        "AMoveRight"},
	{AMoveUp,           "AMoveUp"},
	{AMoveDown,         "AMoveDown"},
	{AMovePageUp,       "AMovePageUp"},
	{AMovePageDn,       "AMovePageDown"},
	{AMoveHome,         "AMoveHome"},
	{AMoveEnd,          "AMoveEnd"},
	{AFDelete,          "AFileDelete"},
	{AFCopy,            "AFileCopy"},
	{AFMove,            "AFileMoveRename"},
	{AFEdit,            "AFileEdit"},
	{AFView,            "AFileView"},
	{AAttributes,       "AFileChangeAttributes"},
	{AChown,            "AFileChangeOwner"},
	{ASelectFileMask,   "ASelectFilesByExtension"},
	{ADeselectFileMask, "ADeselectFilesByExtension"},
	{AInodeInfo,        "AShowInodeInfoWindow"},
	{AQuickFInfo,       "AShowQuickFileInfo"},
	{AEmpty,            "AEmptyAction"}
    };

//-------------------------------------------------------------------------
// Default actions with MouseState links
//-------------------------------------------------------------------------
MouseActions MouseActionManager::default_actions[]=
    {
       	{ mButton1,             AMouseSetCurrent },
	{ mButton1Double,       AExecute },
	{ mButton2,             AContextMenu },
	{ mButton3,             AInsert }, //select current file
	{ mButton1Ctrl,         AInsert }, //select current file
	{ mButton1CtrlMotion,   AInsert },
	{ mButton3Motion,       AInsert },
	{ mButton4,             AMoveLeft },
	{ mButton5,             AMoveRight },
	{ mButtonNone,          AEmpty }
    };


//-------------------------------------------------------------------------
// Constructor of MouseActionManager
//-------------------------------------------------------------------------
MouseActionManager::MouseActionManager()
{
    int i=0,j;
    for(j=0;mouse_press_items[j].mstate!=mButtonNone;j++,i++)
	mstate_name_list[i]=mouse_press_items[j].mname;
    for(j=0;mouse_motion_items[j].mstate!=mButtonNone;j++,i++)
	mstate_name_list[i]=mouse_motion_items[j].mname;

    for(max_actions=0;action_names[max_actions].action!=AEmpty;max_actions++)
	maction_name_list[max_actions]=action_names[max_actions].action_name;
    
    maction_name_list[max_actions]=action_names[max_actions].action_name;
    max_actions++;

    set_default_actions();
}


//-------------------------------------------------------------------------
// Set default actions to real_actions for proper working 
//-------------------------------------------------------------------------
void MouseActionManager::set_default_actions()
{
    int i=0;

    clear_real_actions();
    while(default_actions[i].mstate!=mButtonNone)
    {
	real_actions[i]=default_actions[i];
	i++;
    }
}

//-------------------------------------------------------------------------
// Clear array of real_actions
//-------------------------------------------------------------------------
void MouseActionManager::clear_real_actions()
{
    int i;
    for(i=0;i<max_mouse_active_states;i++)
    {
	real_actions[i].mstate=mButtonNone;
	real_actions[i].action=AEmpty;
    }
}
  
//-------------------------------------------------------------------------
// Main caller - receives MouseData event and translate it to Action
// Writes Action -> data.action, if no action define for this
// event -> writes AEmpty.
//-------------------------------------------------------------------------
int  MouseActionManager::get_action_by_mouse_data(MouseData& data)
{
    MouseState mstate;

    switch(data.xev->type)
    {
    case ButtonPress:
    case ButtonRelease:
	mstate=get_mouse_press_state_by_x_state(data.xev->xbutton.button,
						data.xev->xbutton.state,
						data.click_type);
	break;

    case MotionNotify:
	mstate=get_mouse_motion_state_by_x_state(data.xev->xmotion.state);
	break;
	
    default:
	data.action=AEmpty;
	return AEmpty;
    }
    data.action=get_action_by_mstate(mstate);
    return data.action;
}

//-------------------------------------------------------------------------
// MousePress X Event -> MouseState
//-------------------------------------------------------------------------
MouseState MouseActionManager::get_mouse_press_state_by_x_state(unsigned int button,
								unsigned int state,
								MouseClickType click)
{
    int i=0;

    state  &= (ControlMask | ShiftMask);
    button &= mouse_button_mask;

    while(mouse_press_items[i].mstate!=mButtonNone)
    {
	if(mouse_press_items[i].x_state == state &&
	   mouse_press_items[i].x_button == button &&
	   mouse_press_items[i].click==click)
	    return mouse_press_items[i].mstate;
	i++;
    }
    return mButtonNone;
}

//-------------------------------------------------------------------------
// MouseMotion X Event -> MouseState
//-------------------------------------------------------------------------
MouseState MouseActionManager::get_mouse_motion_state_by_x_state(unsigned int state)
{
    int i=0;

    state  &= mouse_state_mask;

    while(mouse_motion_items[i].mstate!=mButtonNone)
    {
	if(mouse_motion_items[i].x_state == state)
	    return mouse_motion_items[i].mstate;
	i++;
    }
    return mButtonNone;
}

//-------------------------------------------------------------------------
// MouseState by name
//-------------------------------------------------------------------------
MouseState MouseActionManager::get_mstate_by_name(char* name)
{
    int i;
    for(i=0;mouse_press_items[i].mstate!=mButtonNone;i++)
	if(strcmp(mouse_press_items[i].mname, name)==0)
	    return mouse_press_items[i].mstate;
    for(i=0;mouse_motion_items[i].mstate!=mButtonNone;i++)
	if(strcmp(mouse_motion_items[i].mname, name)==0)
	    return mouse_motion_items[i].mstate;
    return mButtonNone;
}

//-------------------------------------------------------------------------
// MouseState name by mstate
//-------------------------------------------------------------------------
char* MouseActionManager::get_mstate_name_by_state(MouseState mstate)
{
    int i;
    for(i=0;mouse_press_items[i].mstate!=mButtonNone;i++)
	if(mouse_press_items[i].mstate==mstate)
	    return mouse_press_items[i].mname;
    for(i=0;mouse_motion_items[i].mstate!=mButtonNone;i++)
	if(mouse_motion_items[i].mstate==mstate)
	    return mouse_motion_items[i].mname;
    return 0;
}

//-------------------------------------------------------------------------
// Action by name
//-------------------------------------------------------------------------
int MouseActionManager::get_action_by_name(char* name)
{
    int i;
    for(i=0;action_names[i].action!=AEmpty;i++)
	if(strcmp(action_names[i].action_name,name)==0)
	    return action_names[i].action;
    return AEmpty;
}

//-------------------------------------------------------------------------
// Action name by action
//-------------------------------------------------------------------------
char* MouseActionManager::get_action_name_by_action(int action)
{
    int i;
    for(i=0;action_names[i].action!=AEmpty;i++)
	if(action_names[i].action==action)
	    return action_names[i].action_name;
    return 0;
}

//-------------------------------------------------------------------------
// MouseState -> Action
//-------------------------------------------------------------------------
int MouseActionManager::get_action_by_mstate(MouseState mstate)
{
    int i=0;
    while(real_actions[i].mstate!=mButtonNone &&
	  real_actions[i].mstate!=mstate)
	i++;
    return real_actions[i].action;
}

//-------------------------------------------------------------------------
// Open file with actions and load it to real_actions
//-------------------------------------------------------------------------
int MouseActionManager::load_actions_from_file()
{
    char fname[L_MAXPATH];
    char read_str[L_MAXPATH];
    char token[L_MAXPATH];
    char *step;
    FILE *fp;
    MouseState mstate;
    int counter=0;

    get_actions_file_name(fname);
    fp=fopen(fname,"r");
    if(!fp)
	return 0;

    clear_real_actions();
    while(fgets(read_str, L_MAXPATH-1, fp))
    {
	if(counter>=max_mouse_active_states)
	    break;

	if(read_str[0]=='#')
	    continue; //skipping comments

	step=getseq(read_str,token,1);
	mstate=get_mstate_by_name(token);
	if(mstate==mButtonNone) //not a button state -> skipping
	    continue;

	getseq(step,token,0);
	real_actions[counter].action=get_action_by_name(token);
	real_actions[counter].mstate=mstate;
	counter++;
    }

    fclose(fp);
    if(counter<1)
    {
	set_default_actions();
	return 0;
    }
    return 1;
}

//-------------------------------------------------------------------------
// Save real_actions to file
//-------------------------------------------------------------------------
int MouseActionManager::save_actions_to_file()
{
    char fname[L_MAXPATH];
    FILE *fp;
    int i;

    get_actions_file_name(fname);
    fp=fopen(fname,"w");
    if(!fp)
	return 0;

    fprintf(fp,
	    "# Mouse actions config file for X Northern Captain\n"
	    "# This file is generated automatically by xnc\n");

    for(i=0;real_actions[i].mstate!=mButtonNone;i++)
    {
	fprintf(fp,"%-30s: %s\n",
		get_mstate_name_by_state(real_actions[i].mstate),
		get_action_name_by_action(real_actions[i].action));
    }
    
    fprintf(fp,"# ===================== End of file ==================\n");

    fclose(fp);
    return 1;
}

//-------------------------------------------------------------------------
// Return name of file to write to/read from actions
//-------------------------------------------------------------------------
void MouseActionManager::get_actions_file_name(char* name_to_return)
{
    sprintf(name_to_return, "%s/xnc.mouse", home_files_dir);
}

//-------------------------------------------------------------------------
// Return index in action_names array identified by mouse state name
//-------------------------------------------------------------------------
int  MouseActionManager::get_action_names_idx_by_mstate_name(char* mname)
{
    int i,action=get_action_by_mstate(get_mstate_by_name(mname));
    for(i=0;action_names[i].action!=action;i++);
    return i;
}

//-------------------------------------------------------------------------
// Search and update real_actions by mstate
// if no such state in real_actions the add it.
//-------------------------------------------------------------------------
int  MouseActionManager::update_action_by_mstate_name(char* mstate_name, 
						      char* action_name)
{
    MouseState mstate=get_mstate_by_name(mstate_name);
    int i,action=get_action_by_name(action_name);
    for(i=0;real_actions[i].mstate!=mButtonNone;i++)
	if(real_actions[i].mstate==mstate)
	{
	    real_actions[i].action=action;
	    return 1;
	}
    real_actions[i].mstate=mstate;
    real_actions[i].action=action;
    return 1;
}

//--------------------------------- End of file -----------------------------------
