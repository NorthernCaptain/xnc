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
/* $Id: dircacheman.cxx,v 1.1 2002/10/29 16:03:55 leo Exp $ */
#include "globals.h"
#include "dircacheman.h"

//========================================================================

//  --------=======     Directory Cache List methods    =======-----------

//========================================================================

//-------------------------------------------------------------------------
// Create list element
//-------------------------------------------------------------------------
DirCacheManager::DirCacheList::DirCacheList(char* dir_str)
{
    next=0;
    data=new char[strlen(dir_str)+1];
    strcpy(data, dir_str);
}

DirCacheManager::DirCacheList::~DirCacheList()
{
    if(data)
	delete data;
}

//-------------------------------------------------------------------------
// Return non_zero if equal data and input param
//-------------------------------------------------------------------------
int DirCacheManager::DirCacheList::equal(const char* dir_str)
{
    return strcmp(dir_str, data)==0;
}


//========================================================================

//  --------=======   Directory Cache Manager methods   =======-----------

//========================================================================

//-------------------------------------------------------------------------
// Constructor of the manager
//-------------------------------------------------------------------------
DirCacheManager::DirCacheManager()
{
}

DirCacheManager::~DirCacheManager()
{
    clean_all();
}

//-------------------------------------------------------------------------
// Clean all list, delete all elements
//-------------------------------------------------------------------------
void DirCacheManager::clean_all()
{
    DirCacheList *next_element,*element=dir_list.next;
    while(element)
    {
	next_element=element->next;
	delete element;
	element=next_element;
    }
    dir_list.next=0;
}

//-------------------------------------------------------------------------
// Add element to list if it's not exist.
//-------------------------------------------------------------------------
char* DirCacheManager::add_entry(char *dir)
{
    DirCacheList *element=dir_list.next;
    while(element)
    {
	if(element->equal(dir))
	    return element->data;
	element=element->next;
    }
    element=new DirCacheList(dir);
    element->next=dir_list.next;
    dir_list.next=element;
    return element->data;
}

//------------------------------End of file---------------------------------

