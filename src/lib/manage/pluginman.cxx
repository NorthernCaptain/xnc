/****************************************************************************
*  Copyright (C) 1996-99 by Leo Khramov
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
/* $Id: pluginman.cxx,v 1.2 2002/08/05 10:43:10 leo Exp $ */
#include "pluginman.h"
#include "xncdirs.h"

#ifdef USE_SHARED_LOADING

/////////////////////////////////////////////////////////////////////////////
// This part of code taken from SWI_Prolog pl-load.c. Thanks to Jan Wielemaker
		 /*******************************
		 *     DLOPEN() AND FRIENDS	*
		 *******************************/

#ifndef SO_EXT
#define SO_EXT "so"
#endif

#ifdef HAVE_DLOPEN			/* sysvr4, elf binaries */

#include <dlfcn.h>

#endif /*HAVE_DLOPEN*/

#ifdef HAVE_SHL_LOAD			/* HPUX */

#include <dl.h>
#define dlopen(path, flags) shl_load((path), (flags), 0L)
#define dlclose(handle)	    shl_unload((shl_t)(handle))
#define dlerror()	    strerror(errno)

void *
dlsym(void* vhandle, const char *name)
{ 
  void *value;
  shl_t handle=(shl_t)vhandle; 

  if ( shl_findsym(&handle, name, TYPE_PROCEDURE, &value) < 0 )
    return 0;

  return value;
}

#define RTLD_LAZY	BIND_DEFERRED
#ifdef BIND_IMMEDIATE
#define RTLD_NOW	BIND_IMMEDIATE
#endif

#endif

#if defined(HAVE_DLOPEN) || defined(HAVE_SHL_LOAD) || defined(EMULATE_DLOPEN)

#ifndef RTLD_GLOBAL			/* solaris defines this */
#define RTLD_GLOBAL 0
#endif
#ifndef RTLD_NOW			/* implicit on some versions */
#define RTLD_NOW 0
#endif
#ifndef RTLD_LAZY			/* freeBSD doesn't have this? */
#define RTLD_LAZY 0
#endif

#endif

#endif /* USE_SHARED_LOADING */
////////////////////////////////////////////////////////////////////////////


//Print error messages to console
void print_error(const char* error)
{
  fprintf(stderr,"Error: %s\n",error);
}

//This function needed becouse on a.out binaries we need _func_symbol
//instead of func_symbol on ELFs
void* internal_dlsym(void* handle,const char* name)
{
  char  name2[L_MAXPATH];
  void  *ret;
  ret=dlsym(handle,name);
  if(ret==0)
  {
    sprintf(name2,"_%s",name);
    ret=dlsym(handle,name2);
  }
  return ret;
}

GuiPlugin *guiplugin;
PluginManager pluginman;

PluginManager::PluginManager()
{
  baseguiplugin=guiplugin=new GuiPlugin("internal","0.1.0");
}

GuiPlugin* PluginManager::load_gui_plugin(char* plugin_name)
{
#ifdef USE_SHARED_LOADING
  char libname[L_MAXPATH];
  void *plugin_handle;
  const char *error;
  GuiPlugin *pobj;
  PlugInitData *pdata;

  sprintf(libname,"libxnc%s.%s",plugin_name,SO_EXT);

  plugin_handle=dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
  sprintf(libname,"%s/libxnc%s.%s",XNCLIBS,plugin_name,SO_EXT);

  if(!plugin_handle)
    plugin_handle=dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
      
  if (!plugin_handle) {
    print_error(dlerror());
    return 0;
  }      
   
  GuiPlugin* (*new_obj)(void*);
   
  new_obj=(GuiPlugin* (*)(void*))internal_dlsym(plugin_handle,"c_plugin_init");
  if (new_obj == 0)  
  {
    error=dlerror();
    print_error(error);
    dlclose(plugin_handle);
    return 0;
  }

  pdata=new PlugInitData("xnc",libname,GUI_Plugin);
  pobj = new_obj(pdata);
  delete pdata;

  if(pobj==0)
  {
    print_error("Plugin return zero pointer!");
    dlclose(plugin_handle);
    return 0;
  }
  pobj->set_handle(plugin_handle);
  fprintf(stderr,"Plugin %s v.%s registered\n",pobj->Name(),pobj->Version());
  return pobj;
#else
  fprintf(stderr,"\nWARNING: Program compiled WITHOUT shared object loading "
                 "interface\nWARNING: Plugins support is disabled\n\n");
  return 0;
#endif
}

int PluginManager::load_look(char* look_name)
{
  unload_look();
  baseguiplugin=guiplugin=load_gui_plugin(look_name);
  if(guiplugin==0)
  {
    baseguiplugin=guiplugin=new GuiPlugin("internal","0.1.0");
    return 0;
  }
  return 1; 
}


void PluginManager::unload_look()
{
#ifdef USE_SHARED_LOADING
  void* handle;
  if(guiplugin)
  {
    handle=guiplugin->get_handle();
    if(handle)
      dlclose(handle);
    delete guiplugin;
    baseguiplugin=guiplugin=0;
  }
#endif
}



