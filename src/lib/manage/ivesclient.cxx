#include "xh.h"
#include "ivesclient.h"
#include "globals.h"

static int dummy_x_error_handler(Display*, XErrorEvent*)
{
  return 0;
}

Atom IVES_client::gen_ives_id()
{
  int i;
  struct utsname uts;
  char atom_name[256];
  uname(&uts);
  sprintf(atom_name,"IVES_%s_%d",uts.nodename,getuid());
  for(i=0;i<strlen(atom_name);i++)
    if(!((atom_name[i]>='0' && atom_name[i]<='9') ||
	(atom_name[i]>='A' && atom_name[i]<='Z') ||
	(atom_name[i]>='a' && atom_name[i]<='z')))
      atom_name[i]='_';
  return XInternAtom(disp,atom_name,False);
}

int IVES_client::Init()
{
  Window *ww;
  int *ii;
  char atom_name[256];
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_remain;
  xa_ives=gen_ives_id();
  ignore_xerror();
  if(XGetWindowProperty(disp,DefaultRootWindow(disp),xa_ives,0,1,False,XA_WINDOW,
			&actual_type,&actual_format,&nitems,&bytes_remain,
			(unsigned char**)&ww)==Success && nitems>0)
    {
      w=ww[0];
      XFree(ww);
      restore_xerror();
      if(!is_window_available(w))
	return 0;
      ignore_xerror();
      if(XGetWindowProperty(disp,w,xa_ives,0,1,False,XA_INTEGER,
			    &actual_type,&actual_format,&nitems,&bytes_remain,
			    (unsigned char**)&ii)==Success && nitems>0)
	{
	  XFree(ii);
	  sprintf(atom_name,"IVES_DATA_%d",getpid());
	  xa_ives_data=XInternAtom(disp,atom_name,False);   //We will put data to ives in this property
	  xa_ives_cmd=XInternAtom(disp,"IVES_CMD",False);
	  xa_ives_retval=XInternAtom(disp,"IVES_RETVAL",False);
	  restore_xerror();
	  return 1;
	}
    }
  restore_xerror();
  return 0;
}


int IVES_client::send_data(Window win, IVES_data data)
{
  XEvent event;
  if(!is_window_available(w))
    return 0;
  event.type=ClientMessage;
  event.xclient.message_type=xa_ives_cmd;
  event.xclient.format=32;
  event.xclient.window=w;
  event.xclient.data.l[0]=(long)DefaultRootWindow(disp);  //Window where we store data in property
  event.xclient.data.l[1]=(long)getpid();     //process id where we store data
  event.xclient.data.l[2]=(long)data.retw;    //Window id for returning result
  event.xclient.data.l[3]=(long)(data.type | (data.retval<<16));    //Type of action and retval
  event.xclient.data.l[4]=(long)data.focus_to;//Window that receives focus after action
  XChangeProperty(disp,win,xa_ives_data,XA_STRING,8,PropModeReplace,
		  (unsigned char*)data.data, strlen(data.data)+1);
  XSendEvent(disp,w,False,NoEventMask,&event);
  XFlush(disp);
  return 1;
}

void IVES_client::ignore_xerror()
{
  prev_handler=XSetErrorHandler(dummy_x_error_handler);
}

void IVES_client::restore_xerror()
{
  XSetErrorHandler(prev_handler);
}

int IVES_client::is_window_available(Window w)
{
  XWindowAttributes xwa;
  prev_handler=XSetErrorHandler(dummy_x_error_handler);
  if(XGetWindowAttributes(disp,w,&xwa))
    {
      XSetErrorHandler(prev_handler);
      return 1;
    }
  XSetErrorHandler(prev_handler);
  return 0;
}

int IVES_client::check_event(XEvent *xev)
{
  if(xev->type==ClientMessage && xev->xclient.message_type==xa_ives_retval)
    {
      if(func)
	func((int)xev->xclient.data.l[0]);
      return 1;
    }
  return 0;
}






