#ifndef CONFIGMAN_H
#define CONFIGMAN_H

#include "globals.h"

const int L_CONF_NAME     = 80;       //Length of resource name
const int MAX_CONF_ITEMS  = 200;      //Maximum items per config

struct ConfigDefault
{
  char*  name;
  char*  default_value;
};

class ConfigManager
{
  struct ConfigItem
  {
    char        name[L_CONF_NAME];
    char*       value;
    ConfigItem()
    {
      name[0]=0;
      value=0;
    };
    ConfigItem(const char* iname, const char* ivalue);
    ~ConfigItem();

    int  operator == (const char* fname);
    ConfigItem& operator = (const char* ivalue);
  };
 
  char          config_name[L_MAXPATH];
 
  ConfigItem**  items;
  int           max_items;
  int           empty_idx;

  int           load_config_from_file();   //return number of items read (-1 if no file)
  ConfigItem*   parse_string_to_config_item(char* string);
  int           find_item_idx(const char* name);
  int           update_item(ConfigManager::ConfigItem* pitem);

 public:
  ConfigManager(char* fname, int imax_items=MAX_CONF_ITEMS);
  ~ConfigManager();
  
  int           merge_file(const char* fname);
  int           save_config(const char* header_string);
  char*         get_value(char* name, char* default_value=0);
  int           get_value_int(char* name, char* default_value=0);
  int           update_value(const char* name, const char* ivalue);
  int           update_value(const char* name, int ivalue);
};

#endif

//============================== End of file ==================================



