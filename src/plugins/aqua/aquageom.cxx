/****************************************************************************
*  Copyright (C) 2000 by Leo Khramov
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
/* $Id: aquageom.cxx,v 1.5 2002/11/10 16:18:44 leo Exp $ */
#include "xh.h"
#include "skin.h"
#include "geomfuncs.h"


//--------------Sprites for AQUALOOK appearence----------------------//

// Upper MenuBar
Sprite     spr_upmenu[]={
                        {&skin,   1,302,  38, 20,   0, 0},
                        {&skin,  40,302,  79, 20,  44, 0},
                        {&skin, 290,302,  76, 20, 131, 0},
                        {&skin, 127,302,  59, 20, 215, 0},
                        {&skin, 240,302,  43, 20,  -1, 0}};
Sprite     spr_upmenupress[]={
                        {&skin,   1,323,  38, 20,   0, 0},
                        {&skin,  40,323,  79, 20,  44, 0},
                        {&skin, 290,323,  76, 20, 131, 0},
                        {&skin, 127,323,  59, 20, 215, 0},
                        {&skin, 240,323,  43, 20,  -1, 0}};

Sprite     spr_upmenufill={&skin, 0, 302, 2, 20, 0, 0};


//Menu switches
Sprite     spr_pullmenu[]={{&skin,  0,303,  2,  6,  0,  0},
			   {&skin,  0,323,  2,  6,  0,  0},
			   {&skin,137,  0, 10, 10,  5,  7}};

//Bottom MenuBar
Sprite     spr_dnmenu[]={
                        {&skin,   0,368, 44, 16,   0, 0},
                        {&skin,  44,368, 51, 16,  44, 0},
                        {&skin,  95,368, 47, 16,  95, 0},
                        {&skin, 142,368, 43, 16, 142, 0},
                        {&skin, 185,368, 49, 16, 185, 0},
                        {&skin, 234,368, 50, 16, 234, 0},
                        {&skin, 284,368, 51, 16, 284, 0},
                        {&skin, 335,368, 55, 16, 335, 0},
                        {&skin, 393,368, 45, 16, 393, 0},
                        {&skin, 449,368, 55, 16,  -1, 0}
                        };

Sprite     spr_dnmenupress[]={
                        {&skin,   0,384, 44, 16,   0, 0},
                        {&skin,  44,384, 51, 16,  44, 0},
                        {&skin,  95,384, 47, 16,  95, 0},
                        {&skin, 142,384, 43, 16, 142, 0},
                        {&skin, 185,384, 49, 16, 185, 0},
                        {&skin, 234,384, 50, 16, 234, 0},
                        {&skin, 284,384, 51, 16, 284, 0},
                        {&skin, 335,384, 55, 16, 335, 0},
                        {&skin, 393,384, 45, 16, 393, 0},
                        {&skin, 449,384, 55, 16,  -1, 0}
                        };
                        
Sprite     spr_dnmenufill={&skin, 0, 368, 1, 16, 0, 0};

//Scroll bar
Sprite     spr_scroll[]={{&skin,  0,171, 10,  6,  0,  0},
			   {&skin,  0,199, 10,  5,  0,  0},
			   {&skin,  0,178, 10, 15,  0,  0}};

//Window images
Sprite     spr_win_coners[]={{&skin, 200,   0,  5,  5, 0, 0},   //Window background
                             {&skin,  81,  49, 13, 14, 4, 3},   //Close win button
                             {&skin,  96,  49, 13, 14, 4, 3},   //Close button pressed
                             {&skin,   0, 302,  2, 20, 0, 0}};  //Title background


//Switch images
Sprite     spr_switch[]= { {&skin,200, 0,  5,  5, 0, 0},  //Switch background
                           {&skin, 80, 0, 18, 20, 0, 0},  //Selected image
                           {&skin, 80,22, 18, 20, 0, 0}}; //Normal image


Sprite     spr_xterm= {&skin, 57,168, 47, 23, 0, 0};

Sprite     spr_key_yes[]= {{&skin,  0, 66, 70, 25, 0, 0},
                           {&skin,  0, 92, 70, 25, 0, 0},
                           {&skin,  0, 92, 70, 25, 0, 0}};

Sprite     spr_bg=         {&skin, 200,  0,  5,   5, 0, 0};    //window bg

//Menu Switcher
Sprite     spr_menusw     ={&skin,112, 0, 12, 20, 0, 0};
Sprite     spr_menuswpress={&skin,136, 0, 12, 20, 0, 0};

//Empty key images                           
Sprite     spr_key_empty[]={{&skin,   0, 66, 70, 25, 0, 0},   //Unfocused
                            {&skin,   0, 92, 70, 25, 0, 0},   //Focused
                            {&skin,   0, 92, 70, 25, 0, 0}};  //Pressed

Sprite     spr_key_long[]= {{&skin,  71, 66, 80, 25, 0, 0},   //Unfocused
                            {&skin,  71, 92, 80, 25, 0, 0},   //Focused
                            {&skin,  71, 92, 80, 25, 0, 0}};  //Pressed

Sprite     spr_bookmark[]={ {&skin, 152,  0, 42, 296, 0, 0},     //back skin
			    {&skin, 195,  0, 42, 296, 0, 0},     //normal skin
			    {&skin, 238,  0, 42, 296, 0, 0},     //selected skin
                            {&skin, 200,  0,  5,   5, 0, 0}};    //window bg

Sprite     spr_lister[]= {  {&skin,   0,269, 23,  22, -50, -25},  // arrow < 
			    {&skin,  25,269, 23,  22, -25, -25},  // arrow >
			    {&skin, 127,187,  3,  23,   0, -25},  // statbar corner
			    {&skin, 133,187, 13,  23, -64, -25},  // statbar corner
			    {&skin,  63,187, 22,  23,   0,   0},  // panel corner
			    {&skin,  93,219, 25,  23,   0,   0},  // panel corner
			    {&skin,   0,219, 23,  23,   0,   0},
			    {&skin,  61,219, 25,  23,   0,   0},
			    {&skin,  94,187, 23,  23,   0,   0},
			    {&skin,  23,219, 22,  23,   0,   0},
			    {&skin, 200,  0,  5,   5,   0,   0}
                         };
//------------Window Areas for Key rounding-------------------//

RecArea    ra_copywin={-80,-55, 75, 46, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};
RecArea    ra_dialwin={-150,-35, 145, 26, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};
RecArea    ra_attr2win={-100,-100, 95, 91, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};
RecArea    ra_ntextwin[]={
                          {  35,-32, 75, 23, LEFT_SIDE | TOP_SIDE | RIGHT_SIDE,0},
                          {-110,-32, 75, 23, LEFT_SIDE | TOP_SIDE | RIGHT_SIDE | LAST_RECAREA,0}
                         };

RecArea    ra_selwin[]={
                          {  25,-32, 75, 23, LEFT_SIDE | TOP_SIDE | RIGHT_SIDE,0},
                          {-100,-32, 75, 23, LEFT_SIDE | TOP_SIDE | RIGHT_SIDE | LAST_RECAREA,0}
                         };

RecArea    ra_simpwin={-80,-35, 75, 26, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};
RecArea    ra_infowin={-80,-31, 75, 22, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};
RecArea    ra_findwin={-308,-35, 303, 26, LEFT_SIDE | TOP_SIDE | LAST_RECAREA,0};

int ftppage_pages[]={1,-1};

RecArea    ra_ftppage={-84,-37, 80, 32, LEFT_SIDE | TOP_SIDE | RIGHT_SIDE | BOTTOM_SIDE | LAST_RECAREA,ftppage_pages};

RecArea    ra_book_cuts[]={{ 5,148, 15, 15, 0, 0},  //Not in use for aqua
			   { 5,173, 15, 15, 0, 0},
			   { 5,201, 15, 15, 0, 0},
			   { 5,226, 15, 15, 0, 0},
			   { 5,252, 15, 15, 0, 0},
			   { 5,278, 15, 15, 0, 0},
			   { 5,305, 15, 15, 0, 0},
			   { 5,331, 15, 15, 0, 0},
			   { 5,357, 15, 15, 0, 0},
			   { 5,382, 15, 15, 0, 0}};

RecArea   ra_book_pages[]={{ 0, 46, 41, 25, 0, 0}, // 1
			   { 0, 74, 41, 25, 0, 0}, // 2
			   { 0,102, 41, 25, 0, 0}, // 3
			   { 0,130, 41, 25, 0, 0}, // 4
			   { 0,158, 41, 25, 0, 0}, // 5
			   { 0,186, 41, 25, 0, 0}, // 6
			   { 0,214, 41, 25, 0, 0}, // 7
			   { 0,242, 41, 25, 0, 0}, // 8
			   { 0,270, 41, 25, 0, 0}, // 9
			   { 0,378, 41, 25, 0, 0}};

		       
//------------------Look AQUA translations----------------------------//

GEOM_TBL aqualook_transtbl[]={        //definitions
  {GUI_IGNORE,"MENU_UP", DONT_CARE, DONT_CARE, DONT_CARE, 20, spr_upmenu, spr_upmenupress, &spr_upmenufill,  NULL},
  {GUI_IGNORE,"FVISUAL", DONT_CARE, DONT_CARE, DONT_CARE, 20, &spr_bg, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_DN", DONT_CARE, DONT_CARE, DONT_CARE, 16, spr_dnmenu, spr_dnmenupress, &spr_dnmenufill, NULL},
  {GUI_KEY   ,"ATTR2_SET" ,       -35, DONT_CARE,        80,        25, spr_key_long, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR2_CLE" ,       -35, DONT_CARE,        80,        25, spr_key_long, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR2_CAN" ,       -35, DONT_CARE,        80,        25, spr_key_long, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR_OK"   ,       -35, DONT_CARE,        80,        25, spr_key_long, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR_CAN"  ,       -35, DONT_CARE,        80,        25, spr_key_long, NULL, NULL, NULL},
  {GUI_KEY   ,""          , DONT_CARE, DONT_CARE,        70, 25, spr_key_empty, NULL, NULL, NULL},

  {GUI_PAGER ,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, &spr_bg, NULL, NULL, NULL},
  {GUI_WIN   ,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, NULL, NULL, NULL},
  {GUI_INFOWIN,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, NULL, NULL, NULL},
  {GUI_SWITCH,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_switch, NULL, NULL, NULL},
  {GUI_MENU  ,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_pullmenu, NULL, NULL, NULL},
  {GUI_SCROLL,""       , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_scroll,  NULL, NULL, NULL},
  {GUI_LISTER,"LISTER1", DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_lister , NULL, NULL, NULL},
  {GUI_LISTER,"LISTER2", DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_lister , NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_SW", DONT_CARE, DONT_CARE,        12,        20, &spr_menusw, &spr_menuswpress, NULL, NULL},










  {GUI_KEY   ,"DIAL_OK"   ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"DIAL_CAN"  ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"CONF_OK"   ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"CONF_CAN"  ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OWNERCHG"  ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OWNERCAN"  ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Q_YES"     ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Q_NEVER"   ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Change"    , DONT_CARE, DONT_CARE,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPOK"     ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPCAN"    ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERYES"   ,      -232,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERCAN"   ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERNO"    ,       -82,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERALL"   ,      -157,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FFIND"     ,      -232,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FQUIT"     ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FSTOP"     ,       -82,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FINDFR"    ,      -157,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"to BG"     , DONT_CARE,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"COPY_OK"   ,        -7,       -29,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"COPY_CAN"  ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"KILLOK"    ,        -7,       -29,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"KILLCAN"   ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"QUERY_OK"  ,        -7,       -29,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"QUERY_CAN" ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SIMP_OK"   ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"MSG_OK"    ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"NTEXT_OK"  ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"NTEXT_CAN" ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SEL_OK"    ,       -80,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SEL_CAN"   ,        -7,        -5,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPUPD"    ,       -10,       -10,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Yes"       , DONT_CARE, DONT_CARE,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,     "OK"   , DONT_CARE, DONT_CARE,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   , "Cancel"   , DONT_CARE, DONT_CARE,        70,        25, spr_key_empty, NULL, NULL, NULL},
  {GUI_BOOKMARK,"BOOKMARK", DONT_CARE, DONT_CARE,        42, DONT_CARE, spr_bookmark , ra_book_pages, ra_book_pages, NULL}        
};



void aqua_geom_init()
{
  geom_set_transtbl(aqualook_transtbl,sizeof(aqualook_transtbl)/S_GEOM_TBL);
}
  


