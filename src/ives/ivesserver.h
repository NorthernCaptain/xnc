#ifndef IVESSERV_H
#define IVESSERV_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlib.h>

#include "ivesdata.h"

class IVES_server
{
  Atom xa_ives;
  Atom xa_ives_cmd;
  Atom xa_ives_retval;
  Window w;
  void (*func)(IVES_data);
  IVES_data data;
  Atom gen_ives_id();
 public:
  IVES_server() {func=0;};
  void Init(Window win,void (*ifunc)(IVES_data));
  int  check_event(XEvent *xev);
  int  extract_data(XClientMessageEvent& xc);
  void send_retval(Window retw, int retval);
};


#endif
/* ------------ End of file -------------- */

