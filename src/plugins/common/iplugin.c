#include <stdio.h>

extern void* (*init_plugin)(void*);

void fatal_plugin_error(char *error)
{
   fprintf(stderr,"Fatal plugin error: %s\n",error);
   exit(1);
}

void* c_plugin_init(void* data)
{
   if(init_plugin)
     return init_plugin(data);
   else
     fatal_plugin_error("init_plugin not initialized");
   return NULL;
}

