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
/* $Id: configman.cxx,v 1.3 2002/09/26 13:34:05 leo Exp $ */
#include "configman.h"
#include "commonfuncs.h"

//========================================================================

//  -----------===========   ConfigItem methods   ============------------

//========================================================================

//-------------------------------------------------------------------------
// Constructor - allocate memory for values, and set name + value
//-------------------------------------------------------------------------
ConfigManager::ConfigItem::ConfigItem(const char *iname, const char *ivalue)
{
  strncpy(name, iname, L_CONF_NAME-1);
  name[L_CONF_NAME-1]=0;
  value=new char[strlen(ivalue)+1];
  strcpy(value, ivalue);
} 

//-------------------------------------------------------------------------
// Destructor - free value memory
//-------------------------------------------------------------------------
ConfigManager::ConfigItem::~ConfigItem()
{
  if(value)
    delete value;
}

//-------------------------------------------------------------------------
// Compare name of item with given fname through matching with wildcards
//-------------------------------------------------------------------------
int ConfigManager::ConfigItem::operator == (const char* fname)
{
  return is_filtered(const_cast<char*> (fname), name);
}

//-------------------------------------------------------------------------
// Set new value to item
//-------------------------------------------------------------------------
ConfigManager::ConfigItem& ConfigManager::ConfigItem::operator = (const char* ivalue)
{
  if(value)
    delete value;
  value=new char[strlen(ivalue)+1];
  strcpy(value, ivalue);
  return *this;
}

//========================================================================

//  --------========   Configuration Manager methods   ========-----------

//========================================================================

//-------------------------------------------------------------------------
// Constructor - creates array of items, and read them from file
//-------------------------------------------------------------------------
ConfigManager::ConfigManager(char *fname, int imax_items)
{
  max_items=imax_items;
  strcpy(config_name, fname);
  items=new ConfigItem* [max_items];
  empty_idx=0;
  load_config_from_file();
}

//-------------------------------------------------------------------------
// Destructor - frees all allocated memory 
//-------------------------------------------------------------------------
ConfigManager::~ConfigManager()
{
  int i;
  for(i=0;i<empty_idx;i++)
    delete items[i];
  delete items;
}



//-------------------------------------------------------------------------
//Load config from file. Return number of items read (-1 if no file)
//-------------------------------------------------------------------------
int  ConfigManager::load_config_from_file()
{
  FILE*        fp;
  ConfigItem*  pitem;
  char         buf[L_MAXPATH];

  xncdprintf(("ConfigManager::load_config from file %s\n", config_name));
  fp=fopen(config_name, "r");
  if(fp==0)
  {
    xncdprintf(("ConfigManager::load_config error: %s", strerror(errno)));
    return -1;
  }
  while(fgets(buf, L_MAXPATH, fp))
  {
    pitem=parse_string_to_config_item(buf);
    if(!pitem)
      continue;
    items[empty_idx++]=pitem;
    if(empty_idx>=max_items)
    {
      xncdprintf(("ConfigManager WARNING: Max items [%d] in config [%s] exceeded!\n", 
		  max_items, config_name));
    }
  }
  fclose(fp);
  xncdprintf(("ConfigManager::load_config - %d items loaded\n",empty_idx));
  return empty_idx;
}

//-------------------------------------------------------------------------
//Merge read values with taken from file. Values from file overwrite
//old ones. Return number of records read, -1 on error
//-------------------------------------------------------------------------
int  ConfigManager::merge_file(const char* fname)
{
  FILE*        fp;
  ConfigItem*  pitem;
  char         buf[L_MAXPATH];
  int          coun=0;

  xncdprintf(("ConfigManager::merge_config from file %s\n", fname));
  fp=fopen(fname, "r");
  if(fp==0)
  {
    xncdprintf(("ConfigManager::load_config error: %s", strerror(errno)));
    return -1;
  }
  while(fgets(buf, L_MAXPATH, fp))
  {
    pitem=parse_string_to_config_item(buf);
    if(!pitem)
      continue;
    coun++;
    update_item(pitem);
  }
  fclose(fp);
  xncdprintf(("ConfigManager::load_config - %d items loaded\n",empty_idx));
  return coun;
}

//-------------------------------------------------------------------------
//Save items to config file, return 1 on success
//-------------------------------------------------------------------------
int ConfigManager::save_config(const char* header_string)
{
  FILE* fp;
  int   i;

  fp=fopen(config_name, "w");
  if(!fp)
  {
    xncdprintf(("ConfigManager::save_config - error %s", strerror(errno)));
    return 0;
  }
  fprintf(fp, "# ====================================================================\n"
	      "# This file was generated automatically by ConfigManager from XNC.\n"
	      "# %s\n"
	      "# ====================================================================\n",
	  header_string);
  for(i=0;i<empty_idx;i++)
    fprintf(fp,"%-30s: %s\n", items[i]->name, items[i]->value);
  fprintf(fp, "# ====================== End of file =================================\n");
  fclose(fp);
  return 1;
}




//-------------------------------------------------------------------------
//Parse string "name: value" -> fill ConfigItem structure
//-------------------------------------------------------------------------
ConfigManager::ConfigItem*  ConfigManager::parse_string_to_config_item(char* str)
{
  char   name[L_MAXPATH];
  char   value[L_MAXPATH];
  char   *ptr;

  if(str[0]==0 || str[0]=='#' || str[0]=='!' ||
     str[0]=='\n' || str[0]=='\r') //Skip this line
    return 0;

  ptr=getseq(str, name, 1); //get name of item always in lower case (case insensitive)
  if(name[0]==0)            //Not a valid string, skipping it...
    return 0;
  getrest(ptr, value, 0);  //get values in case sensitive way
  if(value[0]==0)          //Not a valid string (values can't be empty), skipping it...
    return 0;
  xncdprintf(("ConfigManager::parse_string [%s]->[%s]\n", name, value));
  return new ConfigManager::ConfigItem(name,value);
}




//-------------------------------------------------------------------------
//Find item by name and return index in items array.
//Return index on succes or empty_idx value on fail.
//-------------------------------------------------------------------------
int  ConfigManager::find_item_idx(const char* name)
{
  int i;
  for(i=0;i<empty_idx;i++)
    if(*(items[i]) == name)
      break;
  return i;
}

//-------------------------------------------------------------------------
//Find item by name and return value. Use matching with wildcards
//-------------------------------------------------------------------------
char* ConfigManager::get_value(char* name, char* default_value)
{
  int i=find_item_idx(name);
  if(i<empty_idx)
    return items[i]->value;
  return default_value;
}

//-------------------------------------------------------------------------
//Calls get_value and converts result to int
//-------------------------------------------------------------------------
int ConfigManager::get_value_int(char* name, char* default_value)
{
  char* ptr=get_value(name, default_value);
  if(!ptr)
    return 0;
  return atoi(ptr);
}

//-------------------------------------------------------------------------
//Find ietm and change it to given, delete old one
//Return 1 if updated (changed), 0 if new item added
//-------------------------------------------------------------------------
int ConfigManager::update_item(ConfigManager::ConfigItem* pitem)
{
  int i=find_item_idx(pitem->name);
  if(i<empty_idx)
  {
    delete items[i];
    items[i]=pitem;
    xncdprintf(("ConfigMan::update_item [%s]=[%s]\n", items[i]->name,
		items[i]->value));
    return 1;
  }
  items[empty_idx++]=pitem;
  xncdprintf(("ConfigMan::update_item new [%s]=[%s]\n", items[i]->name,
	      items[i]->value));
  return 0;
}

//-------------------------------------------------------------------------
//Find and update value by given name
//Return 0 if new item was created, 1 if updated
//-------------------------------------------------------------------------
int ConfigManager::update_value(const char* name, const char* ivalue)
{
  int i=find_item_idx(name);
  if(i<empty_idx)
  {
    *(items[i])=ivalue;
    xncdprintf(("ConfigMan::update_value [%s]=[%s]\n", items[i]->name,
		items[i]->value));
    return 1;
  }
  xncdprintf(("ConfigMan::update_value new [%s]=[%s]\n", name,
	      ivalue));
  items[empty_idx++]=new ConfigManager::ConfigItem(name, 
						   ivalue);
  return 0;
}

//-------------------------------------------------------------------------
//Find and update value by given name
//Return 0 if new item was created, 1 if updated
//-------------------------------------------------------------------------
int ConfigManager::update_value(const char* name, int ivalue)
{
  char buf[128];
  sprintf(buf, "%d", ivalue);
  return update_value(name, buf);
}

//============================== End of file ==================================

