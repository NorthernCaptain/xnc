#include "xh.h"
#include "globals.h"
#include "ivesversion.h"
#include "ivesserver.h"


Atom IVES_server::gen_ives_id()
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

void IVES_server::Init(Window win, void(*ifunc)(IVES_data))
{
  int vers=INT_VERSION;
  w=win;
  func=ifunc;
  xa_ives=gen_ives_id();
  XChangeProperty(disp,DefaultRootWindow(disp),xa_ives,XA_WINDOW,32,PropModeReplace,
		  (unsigned char*)&w, 1);
  XChangeProperty(disp,w,xa_ives,XA_INTEGER,32,PropModeReplace,
		  (unsigned char*)&vers, 1);
  xa_ives_cmd=XInternAtom(disp,"IVES_CMD",False);
  xa_ives_retval=XInternAtom(disp,"IVES_RETVAL",False);
}

int IVES_server::check_event(XEvent *xev)
{
  if(xev->type==ClientMessage && xev->xclient.message_type==xa_ives_cmd)
    {
      return extract_data(xev->xclient);
    }
  return 0;
}


int IVES_server::extract_data(XClientMessageEvent& xc)
{
  Atom actual_type;
  int actual_format;
  unsigned long nitems, bytes_remain;

  char *data_chr;
  char atom_name[32];
  Window fromw=(Window)xc.data.l[0];
  Atom   xa_fromp;

  sprintf(atom_name,"IVES_DATA_%d",(int)xc.data.l[1]);
  xa_fromp=XInternAtom(disp,atom_name,False);
  if(XGetWindowProperty(disp,fromw,xa_fromp,0L,1024L,True,XA_STRING,
			&actual_type,&actual_format,&nitems,&bytes_remain,
			(unsigned char**)&data_chr)==Success && nitems>0)
    {
      data.data=new char[nitems];
      strncpy(data.data,data_chr,nitems);
      XFree(data_chr);
      data.retw=(Window)xc.data.l[2];
      data.type=(int)(xc.data.l[3] & 0xffff);
      data.retval=(int)((xc.data.l[3] & 0x7fff0000)>>16);
      data.focus_to=(Window)xc.data.l[4];
      if(func)
	func(data);
      return 1;
    }  
  return 0;
}


void IVES_server::send_retval(Window retw, int retval)
{
  XEvent event;
  event.type=ClientMessage;
  event.xclient.message_type=xa_ives_retval;
  event.xclient.format=32;
  event.xclient.window=retw;
  event.xclient.data.l[0]=(long)retval;  //Return value
  XSendEvent(disp,retw,False,NoEventMask,&event);
}

