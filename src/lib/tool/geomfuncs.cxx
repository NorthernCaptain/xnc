/****************************************************************************
*  Copyright (C) 1998-99 by Leo Khramov
*  email:     leo@xnc.dubna.su
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
#include "xh.h"
#include "gui.h"
#include "skin.h"
#include "geomfuncs.h"

#define        DEBUG_GEOM        0

int mlook=LOOKFIVE;        //MultiLook variable. Can be LOOKFOUR, LOOKFIVE or LOOKMOTIF.
Skin        skin;                //Skin for new look


GEOM_TBL xnc_transtbl[]={        //LOOKFOUR definitions
  {GUI_IGNORE,"MENU_UP", DONT_CARE, DONT_CARE, DONT_CARE, 20, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"FVISUAL", DONT_CARE, DONT_CARE, DONT_CARE, 20, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_SW", DONT_CARE, DONT_CARE,        15, 20, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_DN", DONT_CARE, DONT_CARE, DONT_CARE, 20, NULL, NULL, NULL, NULL},
  {GUI_KEY   , ""      , DONT_CARE, DONT_CARE, DONT_CARE, 21, NULL, NULL, NULL, NULL}
                        };

static GEOM_TBL *current_tbl=xnc_transtbl;
static int      max_tbl_entries=sizeof(xnc_transtbl)/S_GEOM_TBL;

//--------------Motif look translation table------------------------//

GEOM_TBL mlook_transtbl[]={        //LOOKMOTIF definitions
  {GUI_IGNORE,"MENU_UP", DONT_CARE, DONT_CARE, DONT_CARE, 28, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"FVISUAL", DONT_CARE, DONT_CARE, DONT_CARE, 28, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_SW", DONT_CARE, DONT_CARE, DONT_CARE, 28, NULL, NULL, NULL, NULL},
  {GUI_IGNORE,"MENU_DN", DONT_CARE, DONT_CARE, DONT_CARE, 28, NULL, NULL, NULL, NULL},
  {GUI_KEY,"", DONT_CARE, DONT_CARE, DONT_CARE,       34, NULL, NULL, NULL, NULL}
                        };

//-----------------Translation routines------------------------//
#ifdef OLD_GUITRANS
GEOM_TBL*  geom_get_transtbl(int *maxentries)
{
        switch(mlook)
        {
                case LOOKMOTIF:
                        *maxentries=sizeof(mlook_transtbl)/S_GEOM_TBL;
                        return mlook_transtbl;
                case LOOKFOUR:
                        *maxentries=sizeof(xnc_transtbl)/S_GEOM_TBL;
                        return xnc_transtbl;
                case LOOKFIVE:
                        *maxentries=sizeof(look5_transtbl)/S_GEOM_TBL;
                        return look5_transtbl;
        }
}
#else

GEOM_TBL*  geom_get_transtbl(int *maxentries)
{
  *maxentries=max_tbl_entries;
  return current_tbl;
}

#endif

void geom_set_transtbl(GEOM_TBL* toset,int max_entries)
{
  max_tbl_entries=max_entries;
  current_tbl=toset;
}

void geom_translate_by_guitype(int guitype, int *ix, int *iy, int *il, int *ih)
{
        int maxentries=0,i;
        *ix=DONT_CARE;
        *iy=DONT_CARE;
        *il=DONT_CARE;
        *ih=DONT_CARE;
        GEOM_TBL *tbl=geom_get_transtbl(&maxentries);
#if DEBUG_GEOM
        fprintf(stderr,"GEOM Guitype %d\n",guitype);
#endif
        for(i=0;i<maxentries;i++)
        {
           if((tbl[i].guitype!=GUI_IGNORE && tbl[i].guitype!=guitype) || tbl[i].iname[0]!=0)
                   continue;

                        *ix=tbl[i].x;
                        *iy=tbl[i].y;
                        *il=tbl[i].l;
                        *ih=tbl[i].h;
#if DEBUG_GEOM
                        fprintf(stderr,"GEOM Found %d for GUI %d\n",i, guitype);
#endif
                        return;
        }
}

GEOM_TBL* geom_get_data_by_iname(int guitype, char *iname)
{
        int maxentries=0,i;
        GEOM_TBL *tbl=geom_get_transtbl(&maxentries);
        for(i=0;i<maxentries;i++)
        {
           if(tbl[i].guitype!=GUI_IGNORE && tbl[i].guitype!=guitype)
                   continue;
           if((tbl[i].iname[0]!=0 && strcmp(tbl[i].iname,iname)==0) ||
               (tbl[i].iname[0]==0 && tbl[i].guitype==guitype))
                {
#if DEBUG_GEOM
                        fprintf(stderr,"GEOM_TBL Found %d for GUI %d\n",i, guitype);
#endif
                        return &tbl[i];
                }
        }
        return NULL;
}

void geom_translate_by_iname(int guitype,char *iname, int *ix, int *iy, int *il, int *ih)
{
        int maxentries=0,i;
        *ix=DONT_CARE;
        *iy=DONT_CARE;
        *il=DONT_CARE;
        *ih=DONT_CARE;
        GEOM_TBL *tbl=geom_get_transtbl(&maxentries);
#if DEBUG_GEOM
        fprintf(stderr,"GEOM Guitype %d\n",guitype);
#endif
        for(i=0;i<maxentries;i++)
        {
           if(tbl[i].guitype!=GUI_IGNORE && tbl[i].guitype!=guitype)
                   continue;
           if((tbl[i].iname[0]!=0 && strcmp(tbl[i].iname,iname)==0) ||
               (tbl[i].iname[0]==0 && tbl[i].guitype==guitype))
                {
                        *ix=tbl[i].x;
                        *iy=tbl[i].y;
                        *il=tbl[i].l;
                        *ih=tbl[i].h;
#if DEBUG_GEOM
                        fprintf(stderr,"GEOM Found %d for GUI %d\n",i, guitype);
#endif
                        return;
                }
        }
}

//---------------------------------------End of file-------------------------------------------//
