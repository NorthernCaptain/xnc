#ifndef IVESCLN_H
#define IVESCLN_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlib.h>

#include "ivesdata.h"

class IVES_client
{
  Atom xa_ives;
  Atom xa_ives_data;
  Atom xa_ives_cmd;
  Atom xa_ives_retval;
  Window w;
  void (*func)(int);
  int (*prev_handler)(Display*, XErrorEvent*);
  Atom gen_ives_id();
  void ignore_xerror();
  void restore_xerror();
 public:
  IVES_client(void (*ifunc)(int)=0) { func=ifunc;};
  int Init();
  int is_window_available(Window w);
  int send_data(Window win,IVES_data);
  int check_event(XEvent* ev);
};


#endif
/* ------------ End of file -------------- */

