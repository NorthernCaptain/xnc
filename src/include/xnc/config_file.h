#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H 1

#include "configman.h"

class Lister;

extern ConfigManager*      cfgman;
extern ConfigManager*      iniman;
extern char                home_files_dir[];
extern int                 use_prompt;
extern int                 option_bits;
extern int                 xnc_ver,
                           xnc_orig_ver;          //Original XNC version
extern int                 init_starts, 
                           xnc_starts;
extern int                 wm_y_correction;
extern char                magic_file_location[];
extern int                 iconset_number;
extern int                 branch_depth;         //Scan depth for branch view
extern char                guiplugin_name[];
extern char                html_browser[];

extern void                create_cfgman(char* dir, char* fname);
extern void                load_cfg(Lister* l1, Lister* l2);
extern void                save_cfg(Lister * l1, Lister * l2);
extern void                load_gui_plugin();
extern void                browse_documentation(); //in config_doc.cxx.in

#endif



