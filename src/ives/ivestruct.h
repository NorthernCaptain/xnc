#ifndef IVESTRUCT
#define IVESTRUCT
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlib.h>
struct IVESid
{
   char name[5];
   int  ivespid[50][2];
   int ready;
   int type;
   int autopid;
   int retval;
   int maxent;
   Window focus_to;
};
#endif
/* ------------ End of file -------------- */

