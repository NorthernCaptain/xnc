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
/* $Id: fivegeom.cxx,v 1.5 2002/11/10 16:18:46 leo Exp $ */
#include "xh.h"
#include "skin.h"
#include "geomfuncs.h"


//--------------Sprites for LOOKFIVE appearence----------------------//

Sprite     spr_upmenu[]={
                        {&skin,  12, 0, 49, 22,   0, 0},
                        {&skin,  58, 0, 82, 22,  49, 0},
                        {&skin, 293, 0, 82, 22, 140, 0},
                        {&skin, 156, 0, 63, 22, 230, 0},
                        {&skin, 580, 0, 56, 22,  -1, 0}};
Sprite     spr_upmenupress[]={
                        {&skin,  12, 24, 49, 22,   0, 0},
                        {&skin,  58, 24, 82, 22,  49, 0},
                        {&skin, 293, 24, 82, 22, 140, 0},
                        {&skin, 156, 24, 63, 22, 140, 0},
                        {&skin, 580, 24, 56, 22,  -1, 0}};
Sprite     spr_upmenufill={&skin, 285, 0, 10, 22, 0, 0};
Sprite     spr_upmenupiece={&skin, 538, 38, 7, 9, 0, 0};
Sprite     spr_menusw     ={&skin, 0,  0, 12, 22, 0, 0};
Sprite     spr_menuswpress={&skin, 0, 24, 12, 22, 0, 0};

Sprite     spr_dnmenu[]={
                        {&skin,   0, 49, 57, 14,   0, 0},
                        {&skin,  57, 49, 66, 14,  57, 0},
                        {&skin, 123, 49, 60, 14, 123, 0},
                        {&skin, 183, 49, 55, 14, 183, 0},
                        {&skin, 238, 49, 61, 14, 238, 0},
                        {&skin, 299, 49, 64, 14, 299, 0},
                        {&skin, 363, 49, 69, 14, 363, 0},
                        {&skin, 432, 49, 72, 14, 432, 0},
                        {&skin, 504, 49, 52, 14, 504, 0},
                        {&skin, 557, 49, 62, 14,  -1, 0}
                        };

Sprite     spr_dnmenupress[]={
                        {&skin,   0, 65, 57, 14,   0, 0},
                        {&skin,  57, 65, 66, 14,  57, 0},
                        {&skin, 123, 65, 60, 14, 123, 0},
                        {&skin, 183, 65, 55, 14, 183, 0},
                        {&skin, 238, 65, 61, 14, 238, 0},
                        {&skin, 299, 65, 64, 14, 299, 0},
                        {&skin, 363, 65, 69, 14, 363, 0},
                        {&skin, 432, 65, 72, 14, 432, 0},
                        {&skin, 504, 65, 52, 14, 504, 0},
                        {&skin, 557, 65, 62, 14,  -1, 0}
                        };
                        
Sprite     spr_win_coners[]={{&skin, 123, 130, 15, 15, 0, 0},   //Down-right corner
                             {&skin, 139, 130, 15, 15, 0, 0},   //Down-left corner
                             {&skin, 123, 101, 15, 15, 0, 0},   //Up-right corner
                             {&skin, 139, 101, 15, 15, 0, 0}};  //Up-left corner

Sprite     spr_xterm= {&skin, 57,168, 47, 23, 0, 0};

Sprite     spr_dnmenufill={&skin, 553, 49, 6, 14, 0, 0};

Sprite     spr_key_yes[]= {{&skin, 45, 80, 50, 21, 0, 0},
                           {&skin, 45, 80, 50, 21, 0, 0}};
                           
Sprite     spr_key_empty[]={{&skin, 213, 80, 65, 21, 0, 0},
                            {&skin, 213, 80, 65, 21, 0, 0}};

Sprite     spr_bookmark[]={ {&skin, 324, 80, 44, 296, 0, 0},
			    {&skin,   0, 80, 44, 296, 0, 0},
			    {&skin, 279, 80, 44, 296, 0, 0}};

Sprite     spr_lister[]= {  {&skin,  45,151, 39,  13, -85, -20},  // arrow < 
			    {&skin,  86,151, 39,  13, -42, -20},  // arrow >
			    {&skin, 155,101, 11,  21,   1, -25},  // statbar corner
			    {&skin, 155,123, 11,  21,-100, -25},  // statbar corner
			    {&skin,   0,402, 17,  20,   0,   0},  // panel corner
			    {&skin,  18,402, 18,  20,   0,   0},  // panel corner
			    {&skin,  38,402, 17,  20,   0,   0},
			    {&skin,   0,423, 17,  20,   0,   0},
			    {&skin,  18,423, 17,  20,   0,   0},
			    {&skin,  38,423, 17,  20,   0,   0} 
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

RecArea    ra_book_cuts[]={{ 5,148, 15, 15, 0, 0},
			   { 5,173, 15, 15, 0, 0},
			   { 5,201, 15, 15, 0, 0},
			   { 5,226, 15, 15, 0, 0},
			   { 5,252, 15, 15, 0, 0},
			   { 5,278, 15, 15, 0, 0},
			   { 5,305, 15, 15, 0, 0},
			   { 5,331, 15, 15, 0, 0},
			   { 5,357, 15, 15, 0, 0},
			   { 5,382, 15, 15, 0, 0}};
RecArea   ra_book_pages[]={{ 0,144, 44, 22, 0, 0}, // 1
			   { 0,169, 44, 22, 0, 0}, // 2
			   { 0,195, 44, 22, 0, 0}, // 3
			   { 0,222, 44, 22, 0, 0}, // 4
			   { 0,248, 44, 22, 0, 0}, // 5
			   { 0,273, 44, 22, 0, 0}, // 6
			   { 0,302, 44, 22, 0, 0}, // 7
			   { 0,326, 44, 22, 0, 0}, // 8
			   { 0,352, 44, 22, 0, 0}, // 9
			   { 0,378, 44, 22, 0, 0}};

		       
//------------------Look FIVE translations----------------------------//

GEOM_TBL look5_transtbl[]={        //LOOKFIVE definitions
  {GUI_IGNORE,"MENU_UP"   , DONT_CARE, DONT_CARE, DONT_CARE,        20, spr_upmenu, spr_upmenupress, &spr_upmenufill, &spr_upmenupiece},
  {GUI_IGNORE,"FVISUAL"   , DONT_CARE, DONT_CARE, DONT_CARE,        20, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_SW"   , DONT_CARE, DONT_CARE,        11,        20, &spr_menusw, &spr_menuswpress, NULL, NULL},
  {GUI_IGNORE,"MENU_DN"   , DONT_CARE, DONT_CARE, DONT_CARE,        14, spr_dnmenu, spr_dnmenupress, &spr_dnmenufill, NULL},
  {GUI_WIN   ,"COPYWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_copywin, NULL, NULL},
  {GUI_WIN   ,"KILLWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_copywin, NULL, NULL},
  {GUI_WIN   ,"QUERYWIN"  , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_copywin, NULL, NULL},
  {GUI_WIN   ,"DIALWIN"   , DONT_CARE, DONT_CARE, DONT_CARE,       160, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"FTPWIN"    , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"CONFWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"ATTR2WIN"  , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_attr2win, NULL, NULL},
  {GUI_WIN   ,"ATTRWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_attr2win, NULL, NULL},
  {GUI_WIN   ,"NTEXTWIN"  , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"SELWIN"    , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"SIMPWIN"   , DONT_CARE, DONT_CARE, DONT_CARE,        95, spr_win_coners, &ra_simpwin, NULL, NULL},
  {GUI_INFOWIN,"INFOWINK"  , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_infowin, NULL, NULL},
  {GUI_WIN   ,"MSGWIN"    , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_simpwin, NULL, NULL},
  {GUI_WIN   ,"OWNERWIN"  , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"QWIN"      , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_dialwin, NULL, NULL},
  {GUI_WIN   ,"FINDWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_findwin, NULL, NULL},
  {GUI_WIN   ,"OVERWIN"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_findwin, NULL, NULL},
  {GUI_PAGER ,"FTPPAGE"   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, &ra_ftppage, NULL, NULL},
  {GUI_KEY   ,"DIAL_OK"   ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"DIAL_CAN"  ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"CONF_OK"   ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"CONF_CAN"  ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OWNERCHG"  ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OWNERCAN"  ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Q_YES"     ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Q_NEVER"   ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Change"    , DONT_CARE, DONT_CARE,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPOK"     ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPCAN"    ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERYES"   ,      -232,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERCAN"   ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERNO"    ,       -82,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"OVERALL"   ,      -157,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FFIND"     ,      -232,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FQUIT"     ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FSTOP"     ,       -82,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FINDFR"    ,      -157,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"to BG"     , DONT_CARE,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"COPY_OK"   ,        -7,       -29,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"COPY_CAN"  ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"KILLOK"    ,        -7,       -29,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"KILLCAN"   ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"QUERY_OK"  ,        -7,       -29,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"QUERY_CAN" ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SIMP_OK"   ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"MSG_OK"    ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"NTEXT_OK"  ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"NTEXT_CAN" ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SEL_OK"    ,       -80,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"SEL_CAN"   ,        -7,        -5,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"FTPUPD"    ,       -10,       -10,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR2_SET" ,       -17,       -65,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR2_CLE" ,       -17,       -40,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR2_CAN" ,       -17,       -15,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR_OK"   ,       -17,       -65,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"ATTR_CAN"  ,       -17,       -15,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,"Yes"       , DONT_CARE, DONT_CARE,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   ,     "OK"   , DONT_CARE, DONT_CARE,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   , "Update"   , DONT_CARE, DONT_CARE,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_KEY   , "Cancel"   , DONT_CARE, DONT_CARE,        65,        21, spr_key_empty, NULL, NULL, NULL},
  {GUI_BOOKMARK,"BOOKMARK", DONT_CARE, DONT_CARE,        44, DONT_CARE, spr_bookmark , ra_book_cuts, ra_book_pages, NULL},        
  {GUI_CMDLINE,  "CMDLINE", DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, &spr_xterm ,    NULL, NULL, NULL},        
  {GUI_LISTER,   "LISTER1", DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_lister ,    NULL, NULL, NULL},        
  {GUI_LISTER,   "LISTER2", DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_lister ,    NULL, NULL, NULL},        
  {GUI_WIN   ,       ""   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, NULL, NULL, NULL},
  {GUI_INFOWIN,       ""   , DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, spr_win_coners, NULL, NULL, NULL}
                        };




void five_geom_init()
{
  geom_set_transtbl(look5_transtbl,sizeof(look5_transtbl)/S_GEOM_TBL);
}
  


