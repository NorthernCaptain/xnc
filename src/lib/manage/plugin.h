#ifndef PLUGIN_H
#define PLUGIN_H

//Common interface for xnc plugins
#include "config.h"
#include "globals.h"

enum PluginType{UNK_Plugin,GUI_Plugin};

const int L_VERSION=10;
const int L_LOADER= 20;
const int L_LIBNAME=40;
const int L_NAME=   20;

//Basic Class for plugins support
class PluginObj
{
 private:
   void * handle;
   PluginType type;
   char version[L_VERSION];
   char name[L_NAME];
 public:
   PluginObj(PluginType pt,char* iname,char *ver) 
     { 
       handle=0;
       type=pt;
       strncpy(version,ver,L_VERSION);
       strncpy(name,iname,L_NAME);
     };
   void  set_handle(void* h) {handle=h;};
   void  *get_handle() {return handle;};
   const char* Name() { return name; };
   const char* Version() { return version;};
   virtual int post_install() {return 1;};
};   

//Initial data passing to plugin through this struct
struct PlugInitData
{
   char loader[L_LOADER];    //program loaded and called plugin
   char libname[L_LIBNAME];
   PluginType type;
   PlugInitData(char* iloader,char* ilib,PluginType itype)
     {
	strncpy(loader,iloader,L_LOADER);
	strncpy(libname,ilib,L_LIBNAME);
        type=itype;
     };
};

#endif
/* ------------ End of file -------------- */

