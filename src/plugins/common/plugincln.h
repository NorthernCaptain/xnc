#ifndef PLUGINCLN_H
#define PLUGINCLN_H
#include "plugin.h"

extern "C" void fatal_plugin_error(char *error);

#define InitPluginFunc(plugin_func)   void* (*init_plugin)(void*)=(void* (*)(void*))plugin_func;

#endif
/* ------------ End of file -------------- */

