#ifndef COLORS_H
#define COLORS_H

#include "xheaders.h"

const int MAX_BG_COLORS =    5;
const int MAX_FG_COLORS =  256;

extern unsigned long keyscol[];
extern unsigned long cols[];
extern unsigned long skincol[];

class ConfigManager;

unsigned long gencol(XColor & col, int per);
void          generate_bg_colors(char *bgcolor);
void          generate_cursor_colors(char *bgcolor);
void          load_color_schema(ConfigManager*);

#define dark_bg_color         keyscol[0]
#define normal_bg_color       keyscol[1]
#define light_bg_color        keyscol[2]
#define lighter_bg_color      keyscol[3]
#define darker_bg_color       keyscol[4]

#define cursor_color          cols[50]
#define light_cursor_color    cols[51]
#define dark_cursor_color     cols[52]

#define panel_info_color      cols[3]
#define statusbar_color       panel_info_color
#define shadow_color          cols[0]
#define menu_text_color_idx   1
#define menu_text_color       cols[1]
#define cmdline_bg_color      lighter_bg_color
#define window_text_color     cols[1]
#define window_warn_color     cols[2]
#define window_text2_color    cols[5]

#define f_dir_color           cols[7]
#define f_exec_color          cols[8]
#define f_normal_color        cols[6]
#define f_select_color        cols[13]
#define f_ext_color           cols[9]
#define f_link_color          cols[10]
#define f_afs_color           cols[11]
#define f_image_color         cols[12]

/*
Colors:

   0    -    shadow color
   1    -    gui text color 1
   2    -    gui text color 2
        -    gui text warning color
   3    -    panel info color
   4    -    cursor color
   
   5    -    selected file
   6    -    normal file
   7    -    directory
   8    -    execution
   9    -    extension
  10    -    link
  11    -    afs
  12    -    image

*/


#endif


