#ifndef IVESDATA_H
#define IVESDATA_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xlib.h>

struct IVES_data
{
  Window retw;
  Window focus_to;
  int    retval;
  int    type;
  char*  data;
  IVES_data()
  {
    retw=focus_to=0;
    retval=0;
    type=0;
    data=0;
  };

  IVES_data(int itype, char* idata)
  {
    retw=focus_to=0;
    retval=0;
    type=itype;
    data=idata;
  };

  IVES_data(Window iretw,int itype, Window ifocus,char* idata, int iretval=0)
  {
    retw=iretw;
    focus_to=ifocus;
    type=itype;
    data=idata;
    retval=iretval;
  };  
};

#endif
/* ------------ End of file -------------- */

