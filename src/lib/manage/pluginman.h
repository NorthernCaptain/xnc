#ifndef PLUGINMAN_H
#define PLUGINMAN_H

#include "guiplugin.h"

//Manager all plugins (make load/unload)
class PluginManager
{
  int dummy;
 public:
  PluginManager();
  GuiPlugin* load_gui_plugin(char* iname);
  int        load_look(char* look_name);
  void       unload_look();
};

extern GuiPlugin *guiplugin;
extern PluginManager pluginman;
#endif
/* ------------ End of file -------------- */

