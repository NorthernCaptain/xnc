#ifndef INTERNALS_H
#define INTERNALS_H
#include "xh.h"

extern char*  flags_chr[];
extern int    flags_sizes[];
extern Pixmap dnd1pix;
extern Pixmap dnd1mpix;
extern Cursor cdnd1, cdnd2;
extern int    dot_bar;

extern void   create_dnd_cursors();
extern void   show_mes(char *head,char *mes);
extern void   hide_mes();
extern void   load_pixmaps();
extern void   load_skins();
extern void   show_intro();
extern void   hide_intro();
extern void   x_free_pixmaps();
extern void   win_nodecor(Window W);
extern void   free_panel_pixmaps();
extern void   show_dot();
extern Pixmap get_rootwindow_pixmap();

extern int    internals_set_fds(int fd, fd_set* fds);
extern void   internals_check_fds(fd_set* fds);
extern int    flush_expose(Window w);
extern void   DndSetData(int Type, unsigned char *Data, unsigned long Size);
extern void   init_dnd();
extern void   show_about_dialog();
extern void   show_switch_term_button();
extern void   hide_switch_term_button();

extern Atom DndProtocol, DndSelection;
extern Atom Old_DndProtocol, Old_DndSelection;

class IconKey;

extern IconKey* term_key;
#endif

/* ------------ End of file -------------- */

