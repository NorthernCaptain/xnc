/****************************************************************************
*  Copyright (C) 2001 by Leo Khramov
*  email:     leo@xnc.dubna.su
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/

#include "xdndclass.h"

//Bitmaps and Pixmaps
#include "dnd1.xbm"
#include "dnd1m.xbm"
#include "dnd2.xbm"
#include "dnd2m.xbm"

DndManager* default_dnd_man=0;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/////////////////////////////////DndObject////////////////////////////////
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//This method calls when window receives XDndEnter
int DndObject::dnd_enter(XEvent*, int x, int y)
{
  xncdprintf(("Abstract call to DndObject::dnd_enter(XEvent*, %d, %d)\n",x,y));
  return 1;
}

//This method calls when window receives XDndLeave
int DndObject::dnd_leave(XEvent*, int x, int y)
{
  xncdprintf(("Abstract call to DndObject::dnd_leave(XEvent*, %d, %d)\n",x,y));
  return 1;
}

//This method calls when window receives XDndPosition
int DndObject::dnd_position(XEvent*, int x, int y)
{
  xncdprintf(("Abstract call to DndObject::dnd_position(XEvent*, %d, %d)\n",x,y));
  return 1;
}

//This method calls when window receives XDndDrop
//you MUST DELETE DndData object after use!
int DndObject::dnd_drop(DndData* d, int x, int y,DndAction action)
{
  xncdprintf(("Abstract call to DndObject::dnd_drop('%s', %d, %d, %d)\n",
	      d->data,x,y,action));
  delete d;
  return 1;
}

//You call this method when you want to begin dragging process
int DndObject::dnd_drag_now(DndData* dat)
{
  
  return dndman->dragging(this,dat);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//////////////////////////////////DndData/////////////////////////////////
//------------------------------------------------------------------------
//------------------------------------------------------------------------

int  DndData::calc_entries(char* dat, char sym)
{
  int i=0;
  while(*dat)
  {
    if(*dat==sym)
      i++;
    dat++;
  }
  return i;
}

void DndData::add_file_entry(char *dir, char* fname)
{
  while(*dir)
  {
    data[dat_idx++]=*dir++;
  }
  if(*(dir-1)!='/')
    data[dat_idx++]='/';
  while(*fname)
  {
    data[dat_idx++]=*fname++;
  }
  data[dat_idx++]='\n';
  data[dat_idx]=0;
  file_listlen++;
}

void DndData::free_file_list()
{
  int i;
  if(!file_list)
    return;
  for(i=0;i<file_listlen;i++)
    if(file_list[i])
      delete file_list[i];
  delete file_list;
  file_list=0;
}

int DndData::break_to_filelist()
{
  int len,i=0,j;
  free_file_list();
  len=calc_entries(data,'\n')+1;
  file_list=new char* [len];
  file_listlen=0;
  while(data[i])
  {
    j=0;
    file_list[file_listlen]=new char[L_MAXPATH];
    while(data[i] && data[i]!='\n' && data[i]!='\r')
      file_list[file_listlen][j++]=data[i++];
    file_list[file_listlen][j]=0;
    while(data[i]=='\n' || data[i]=='\r')
      i++;
    file_listlen++;
  }
  return file_listlen;
}

void DndData::show_filelist()
{
  int i;
  for(i=0;i<file_listlen;i++)
  {
    xncdprintf(("Filelist[%d]: [%s]\n",i,file_list[i]));
  }
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//////////////////////////////////DndWin//////////////////////////////////
//------------------------------------------------------------------------
//------------------------------------------------------------------------

DndWin::DndWin(DndManager* iman,Window iw, DndObject* o, int ix, int iy, uint il, uint ih)
{
  int i;

  obj=o;
  w=iw;
  man=iman;
  rx=ix;ry=iy;
  l=il;h=ih;
  typelist=new Atom[MAX_TYPES_PER_ITEM];
  for(i=0;i<MAX_TYPES_PER_ITEM;i++)
    typelist[i]=0;
}

DndWin::DndWin(DndManager* iman,Window iw, int ix, int iy, uint il, uint ih)
{
  int i;

  obj=0;
  w=iw;
  man=iman;
  rx=ix;ry=iy;
  l=il;h=ih;
  typelist=new Atom[MAX_TYPES_PER_ITEM];
  for(i=0;i<MAX_TYPES_PER_ITEM;i++)
    typelist[i]=0;
}

void DndWin::add_type(char* name)
{
  int i;
  Atom a=XInternAtom(man->display(),name,False);
  for(i=0;typelist[i];i++)
    if(typelist[i]==a)
      return;
  typelist[i]=a;
  xncdprintf(("Added to DndWin:\n"));
  man->debug_atom(a);
  man->add_in_type(a);
}

int DndWin::check_type(Atom t)
{
  int i;
  xncdprintf(("DndWin check_type:\n"));
  man->debug_atom(t);
  for(i=0;typelist[i];i++)
    if(typelist[i]==t)
      return 1;
  return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
///////////////////////////////DndManager/////////////////////////////////
//------------------------------------------------------------------------
//------------------------------------------------------------------------


//Manager initialization, building all nessesary data
DndManager::DndManager(Display* d,Window irootw)
{
  int i;

  disp=d;
  rootw=irootw;
  winar=new DndWin* [MAX_WINS];
  our_in_typelist=new Atom[MAX_TYPES];
  out_typelist=new Atom[MAX_TYPES];
  drag_status=dnd_none;

  for(i=0;i<MAX_WINS;i++)
    winar[i]=0;

  for(i=0;i<MAX_TYPES;i++)
    our_in_typelist[i]=out_typelist[i]=0;

  version=XDND_VERSION;
  typelist=0;
  curtype=-1;
  dnd_data=0;
  curaction=0;
  curwin=0;
  drag_drop_time_out=1;
  xevent_processor=0;
  drag_object=0;

  XdndAware =             XInternAtom (disp, "XdndAware", False);
  XdndSelection =         XInternAtom (disp, "XdndSelection", False);
  XdndEnter =             XInternAtom (disp, "XdndEnter", False);
  XdndLeave =             XInternAtom (disp, "XdndLeave", False);
  XdndPosition =          XInternAtom (disp, "XdndPosition", False);
  XdndDrop =              XInternAtom (disp, "XdndDrop", False);
  XdndFinished =          XInternAtom (disp, "XdndFinished", False);
  XdndStatus =            XInternAtom (disp, "XdndStatus", False);
  XdndActionCopy =        XInternAtom (disp, "XdndActionCopy", False);
  XdndActionMove =        XInternAtom (disp, "XdndActionMove", False);
  XdndActionLink =        XInternAtom (disp, "XdndActionLink", False);
  XdndActionAsk =         XInternAtom (disp, "XdndActionAsk", False);
  XdndActionPrivate =     XInternAtom (disp, "XdndActionPrivate", False);
  XdndTypeList =          XInternAtom (disp, "XdndTypeList", False);
  XdndActionList =        XInternAtom (disp, "XdndActionList", False);
  XdndActionDescription = XInternAtom (disp, "XdndActionDescription", False);  
  dnd_data_atom =         XInternAtom (disp, "dnd_data_atom", False);  

  make_dnd_aware();

  create_dnd_cursors();

  xncdprintf(("DndManager initialized. DndEnter is 0x%x\n",XdndEnter));
  //  add_in_type("text/plain");
  //add_in_type("text/uri-list");
}

//Creating cursors for displaing to the user while dragging
void DndManager::create_dnd_cursors()
{
  Pixmap dnd1pix,dnd1mpix;
  XColor xc1, xc2;
  xc1.pixel = BlackPixel(disp,DefaultScreen(disp));
  xc1.red = xc1.green = xc1.blue = 0l;
  xc1.flags = DoRed | DoBlue | DoGreen;
  xc2.pixel = WhitePixel(disp,DefaultScreen(disp));
  xc2.red = 65535l;
  xc2.green = 65535l;
  xc2.blue = 65535l;
  xc2.flags = DoRed | DoBlue | DoGreen;
  dnd1pix = XCreateBitmapFromData(disp, rootw, (char *)dnd1_bits, dnd1_width, dnd1_height);
  dnd1mpix = XCreateBitmapFromData(disp, rootw, (char *)dnd1m_bits, dnd1m_width, dnd1m_height);
  curs_one = XCreatePixmapCursor(disp, dnd1pix, dnd1mpix, &xc1, &xc2, 11, 8);
  XFreePixmap(disp, dnd1pix);
  XFreePixmap(disp, dnd1mpix);
  dnd1pix = XCreateBitmapFromData(disp, rootw, (char *)dnd2_bits, dnd2_width, dnd2_height);
  dnd1mpix = XCreateBitmapFromData(disp, rootw, (char *)dnd2m_bits, dnd2m_width, dnd2m_height);
  curs_many = XCreatePixmapCursor(disp, dnd1pix, dnd1mpix, &xc1, &xc2, 12, 13);
  XFreePixmap(disp, dnd1pix);
  XFreePixmap(disp, dnd1mpix);
}

//Setting our main window to dnd_aware state (if we don't do this -> no DND at all)
void DndManager::make_dnd_aware()
{
  XWindowAttributes attr;
  XGetWindowAttributes(disp,rootw,&attr);
  xncdprintf(("Root window input mask: %x %x\n",attr.your_event_mask, 
	      attr.all_event_masks));
  XSelectInput(disp,rootw,attr.your_event_mask | EnterWindowMask 
	       | LeaveWindowMask | StructureNotifyMask);
  XChangeProperty (disp, rootw, XdndAware, XA_ATOM, 32, PropModeReplace,
		   (unsigned char *) &version, 1);
  xncdprintf(("Setting XDndAware property to 0x%x\n",rootw));
}

//Cleaning our garbage at destroy
DndManager::~DndManager()
{
  int i;
  for(i=0;i<MAX_WINS;i++)
    if(winar[i])
      delete winar[i];
  delete winar;
  winar=0;
  if(typelist)
    delete typelist;
  if(dnd_data)
    delete dnd_data;
  delete out_typelist;
}



//-----------------------------------------------------------------------------
// **************  Internal DndManager lists and indexes *********************
//                  Methods that provide them.
//-----------------------------------------------------------------------------


int DndManager::find_win_idx(Window w)
{
  int i;
  for(i=0;i<MAX_WINS;i++)
    if(winar[i] && winar[i]->w==w)
      return i;
  return -1;
}

///global DndManager::find_dnd_win(rx,ry,action)
/// Find best dnd window, that can receive our drop
/// Think that top window on stacking order is at the end of our list

DndWin* DndManager::find_dnd_win(int rx, int ry, Atom action)
{
  int i;
  Atom a=None;
  DndWin* last=0;
  if(curtype!=-1)
    a=our_in_typelist[curtype];
  for(i=0;winar[i] && i<MAX_WINS;i++)
    if(rx>=winar[i]->rx && ry>=winar[i]->ry && 
       winar[i]->rx+winar[i]->l>rx && winar[i]->ry+winar[i]->h>ry)
    {
      xncdprintf(("find_dnd_win: found entry %d\n",i));
      if(winar[i]->check_type(a))
      {
	xncdprintf(("check_type - ok\n"));
	last=winar[i];
      }
       else
       {
	 xncdprintf(("check_type - fail\n"));
	 last=0;
       }
    }
  return last;
}

int DndManager::new_idx()
{
  int i;
  for(i=0;i<MAX_WINS;i++)
    if(winar[i]==0)
      return i;
  return -1;
}

DndWin* DndManager::add_dnd_win(Window w)
{
  int idx;
  int x,y;
  uint l,h;
  if((idx=find_win_idx(w))!=-1)
    return winar[idx];
  idx=new_idx();
  trace_win_geom(w,x,y,l,h);
  winar[idx]=new DndWin(this,w,x,y,l,h);
  return winar[idx];
}

DndWin* DndManager::add_dnd_object(DndObject* o, Window w)
{
  int idx;
  int x,y;
  uint l,h;
  if((idx=find_win_idx(w))!=-1)
    return winar[idx];
  idx=new_idx();
  trace_win_geom(w,x,y,l,h);
  winar[idx]=new DndWin(this,w,o,x,y,l,h);
  o->dnd_init(winar[idx],this);
  return winar[idx];
}

//Deleting object from list of registers dropable windows
void DndManager::del_dnd_win(Window w)
{
  int i,j;
  for(i=0;i<MAX_WINS;i++)
    if(winar[i] && winar[i]->w==w)
    {
      delete winar[i];
      for(j=i+1;j<MAX_WINS;j++)
	winar[j-1]=winar[j];
      winar[MAX_WINS-1]=0;
      xncdprintf(("DndObject deleted from DndManager list\n"));
      return;
    }
}

//Moving object to the top of the list
int DndManager::move_to_top(Window w)
{
  int i;
  DndWin* tmp;
  if((i=find_win_idx(w))==-1)
    return 0;
  tmp=winar[i];
  for(;i<MAX_WINS-1;i++)
  {
    winar[i]=winar[i+1];
    if(winar[i]==0)
      break;
  }
  winar[i]=tmp;
  return 1;
}


//-----------------------------------------------------------------------------
// ***************  Working with windows geometry *****************************
//-----------------------------------------------------------------------------
//Translate window coords into coords on root window
void DndManager::trace_win_geom(Window w, int& x, int& y, uint& l, uint& h)
{
  Window screenw=DefaultRootWindow(disp);
  Window subw;
  uint bw,dep;
  XGetGeometry(disp,w,&subw,&x,&y,&l,&h,&bw,&dep);
  XTranslateCoordinates(disp,w,screenw,0,0,&x,&y,&subw);
  xncdprintf(("Traced coords for 0x%x: %d,%d %dx%d\n",w,x,y,l,h));
}

//Translate global coords to window coords 
void DndManager::global2win_coords(Window w, int fromx, int fromy, int& x, int& y)
{
  Window screenw=DefaultRootWindow(disp);
  Window subw;
  XTranslateCoordinates(disp,screenw,w,fromx,fromy,&x,&y,&subw);
}


void DndManager::update_geoms()
{
  int i;
  for(i=0;i<MAX_WINS;i++)
    if(winar[i])
      trace_win_geom(winar[i]->w,winar[i]->rx, winar[i]->ry, 
		     winar[i]->l, winar[i]->h);
}

//-----------------------------------------------------------------------------
// **************** Dnd Event processing methods (drops) **********************
//-----------------------------------------------------------------------------

//Look on event and process it, if event is for us -> return 1
//If event is not processed then return 0
int DndManager::process_event(XEvent* ev)
{
  switch(ev->type)
  {
  case ClientMessage:
    //XdndEnter received
    if(XDND_MESTYPE(ev)==XdndEnter)
    {
      xncdprintf(("XdndEnter for 0x%x: VER[%d] SRCWIN[0x%x] 3TYPES[%d]\n",ev->xany.window,
		  XDND_ENTER_VERSION(ev),XDND_ENTER_SOURCE_WIN(ev),XDND_ENTER_THREE_TYPES(ev)));
      process_XdndEnter(ev,XDND_ENTER_SOURCE_WIN(ev),XDND_ENTER_VERSION(ev));
      return 1;
    } 

    //XdndPosition received
    if(XDND_MESTYPE(ev)==XdndPosition)
    {
      xncdprintf(("XdndPosition:\n"));
      process_XdndPosition(ev,XDND_POSITION_SOURCE_WIN(ev));
      return 1;
    }
    //XdndPosition received
    if(XDND_MESTYPE(ev)==XdndLeave)
    {
      xncdprintf(("XdndLeave: cleanups\n"));
      process_XdndLeave(ev);
      return 1;
    }
    if(XDND_MESTYPE(ev)==XdndDrop)
    {
      xncdprintf(("XdndDrop: retreiving the data\n"));
      process_XdndDrop(ev);
      return 1;
    }
    break;
    
  case SelectionNotify:
    if(ev->xany.window==rootw && ev->xselection.property==dnd_data_atom)
    {
      data_ready_get_it();
    }
    break;
  }
  return 0;
}


//Processing XDndEnter message
int DndManager::process_XdndEnter(XEvent* ev, Window srcw, int vers)
{
  if(vers>version)
  {
    xncdprintf(("Source has newer version that we, reject XdndEnter\n"));
    return 0;
  }
  srcwin=srcw;
  if(XDND_ENTER_THREE_TYPES(ev))
  {
    xncdprintf(("Source provide three types only\n"));
    build_threetype_list(ev,srcw);
  } else
  {
    xncdprintf(("Source provide more than three types\n"));
    build_bigtype_list(ev,srcw);
  }
  if((curtype=check_type_list(ev))==-1)
    return 0;

  return 1;
}


//Processing XDndPosition message
int DndManager::process_XdndPosition(XEvent* ev, Window srcw)
{
  DndWin* pwin;
  int rx=XDND_POSITION_ROOT_X(ev);
  int ry=XDND_POSITION_ROOT_Y(ev);

  curaction=(Atom)XDND_POSITION_ACTION(ev);
  xncdprintf(("SrcWin[0x%x] X=%d, Y=%d, Action:\n",srcw,rx,ry));
  debug_atom((Atom)XDND_POSITION_ACTION(ev));
  if(srcw!=srcwin)
  {
    xncdprintf(("Wrong source window, rejecting\n"));
    return 0;
  }
  pwin=find_dnd_win(rx,ry,curaction);
  if(pwin!=curwin)
  {
    if(curwin)
      curwin->obj->dnd_leave(ev,rx,ry);
    curwin=pwin;
    if(curwin)
      curwin->obj->dnd_enter(ev,rx,ry);
  }
  if(curwin)
  {
    curx=rx;
    cury=ry;
    curwin->obj->dnd_position(ev,rx,ry);
    send_status(1,curaction,1);
  }
  else 
    send_status(0);
  return 1;
}


//Processing XdndLeave message - doing clean-ups
int DndManager::process_XdndLeave(XEvent* ev)
{
  if(dnd_data)
    delete dnd_data;
  dnd_data=0;
  curtype=-1;
  if(curwin)
    curwin->obj->dnd_leave(ev,curx,cury);
  curwin=0;
  return 1;
}

//Processing XdndDrop message - get data and kick object
int DndManager::process_XdndDrop(XEvent* ev)
{
  if(!try_convert_drag_data(ev,srcwin))
  {
    send_finished();
    return 0;
  }
  if(curwin==0)
  {
    send_finished();
    return 0;
  }
    
  return 1;
}

//-----------------------------------------------------------------------------
// ********* Working with data given to us from dragger ***********************
//-----------------------------------------------------------------------------

int DndManager::try_convert_drag_data(XEvent* ev,Window srcw)
{
  Window w;
  if (curtype==-1)
    return 0;
  if (!(w = XGetSelectionOwner(disp, XdndSelection))) 
  {
    xncdprintf(("XGetSelectionOwner failed"));
    return 0;
  }
  /*  if(w!=srcwin)
  {
    xncdprintf(("Source not an owner of XdndSelection [0x%x/0x%x], rejecting\n",w,srcwin));
    return 0;
    } */
  XConvertSelection(disp, XdndSelection, our_in_typelist[curtype],
		     dnd_data_atom, rootw, CurrentTime);
  return 1;
}

int DndManager::data_ready_get_it()
{
  DndData *dd;
  Atom type, *a;
  int format, i;
  unsigned long count, remaining;
  unsigned char *data = NULL;

  XGetWindowProperty (disp, rootw, dnd_data_atom,
		      0, 0x8000000L, True, our_in_typelist[curtype],
		      &type, &format, &count, &remaining, &data);

  if (type != our_in_typelist[curtype] || format != 8 || count == 0 || !data) {
    if (data)
      XFree (data);
    xncdprintf(("data_ready_get_it failed: format=%d, count=%d, type:", format,count));
    debug_atom(type);
    if(curwin)
      send_finished();
    return 0;
  }
  
  if(dnd_data)
    delete dnd_data;
  dnd_data=new char[count+1];
  strncpy(dnd_data,(char*)data,count);
  dnd_data[count]=0;
  xncdprintf(("Got the following data: '%s'\n",dnd_data));
  if(curwin)
  {
    dd=new DndData((char*)data,count,type);
    curwin->obj->dnd_drop(dd,curx,cury,atom_action_to_dnd_action(curaction));
    send_finished();
  }
  XFree(data);
  return 1;
}

//-----------------------------------------------------------------------------
// Translates atom actions to DndAction
//-----------------------------------------------------------------------------
DndAction DndManager::atom_action_to_dnd_action(Atom act)
{
    if(act==XdndActionCopy)
	return dnd_copy;
    if(act==XdndActionMove)
	return dnd_move;
    if(act==XdndActionLink)
	return dnd_link;
    return dnd_private;
}

//-----------------------------------------------------------------------------
// *********************  Getting type list from dragger **********************
//-----------------------------------------------------------------------------

//Building list of types from xclient data structure
int DndManager::build_threetype_list(XEvent* ev,Window)
{
  int i;
  if(typelist)
    delete typelist;
  typelist = new Atom [XDND_THREE + 1];
  for (i = 0; i < XDND_THREE; i++)
    typelist[i] = XDND_ENTER_TYPE (ev, i);
  typelist[XDND_THREE] = 0;	/* although typelist[1] or typelist[2] may also be set to nill */
  return XDND_THREE;
}

//Building list of types from xclient data structure
int DndManager::build_bigtype_list(XEvent* ev,Window srcw)
{
  Atom type, *a;
  int format, i;
  unsigned long count, remaining;
  unsigned char *data = NULL;

  if(typelist)
    delete typelist;

  typelist = 0;

  XGetWindowProperty (disp, srcw, XdndTypeList,
		      0, 0x8000000L, False, XA_ATOM,
		      &type, &format, &count, &remaining, &data);

  if (type != XA_ATOM || format != 32 || count == 0 || !data) {
    if (data)
      XFree (data);
    xncdprintf(("XGetWindowProperty failed [XdndTypeList = %ld]", XdndTypeList));
    return 0;
  }
  typelist = new Atom [count + 1];
  a = (Atom *) data;
  for (i = 0; i < count; i++)
  {
    typelist[i] = a[i];
    debug_atom(a[i]);
  }
  typelist[count] = 0;

  XFree (data);
  
  xncdprintf(("XdndTypeList: Received %d types\n",count));

  return count;
}


//Checking typelist for type we need
int DndManager::check_type_list(XEvent* ev)
{
  int i,j;
  if(!typelist || !our_in_typelist)
    return -1;
  for(i=0;typelist[i];i++)
    for(j=0;our_in_typelist[j];j++)
      if(typelist[i]==our_in_typelist[j])
      {
	xncdprintf(("Found entry in our_in_typelist:\n"));
	debug_atom(our_in_typelist[j]);
	return j;
      }
  return -1;
}


//-----------------------------------------------------------------------------
// ************************    Sending methods   ******************************
//-----------------------------------------------------------------------------

int DndManager::send_finished()
{
  XEvent xevent;
  
  memset (&xevent, 0, sizeof (xevent));
  
  xevent.xany.type = ClientMessage;
  xevent.xany.display = disp;
  xevent.xclient.window = srcwin;
  xevent.xclient.message_type = XdndFinished;
  xevent.xclient.format = 32;
  xevent.xclient.data.l[0]=rootw;

  send_event(xevent);
  process_XdndLeave(0);
  return 1;
}

int DndManager::send_status(int will_accept,Atom action,int want_position, int x, int y, 
			    uint l, uint h)
{
  XEvent xevent;
  
  memset (&xevent, 0, sizeof (xevent));
  
  xevent.xany.type = ClientMessage;
  xevent.xany.display = disp;
  xevent.xclient.window = srcwin;
  xevent.xclient.message_type = XdndStatus;
  xevent.xclient.format = 32;

  XDND_STATUS_TARGET_WIN (&xevent) = rootw;
  XDND_STATUS_WILL_ACCEPT_SET (&xevent, will_accept);
  if (will_accept)
    XDND_STATUS_WANT_POSITION_SET (&xevent, want_position);
  if (want_position)
    XDND_STATUS_RECT_SET (&xevent, x, y, l, h);
  XDND_STATUS_ACTION (&xevent) = action;
  send_event(xevent);
  return 1;
}

int DndManager::send_enter(DndWin* owin, Window to_send)
{
    XEvent xevent;
    int n, i;
    n = atom_array_length(out_typelist);

    memset (&xevent, 0, sizeof (xevent));

    xevent.xany.type = ClientMessage;
    xevent.xany.display = disp;
    xevent.xclient.window = to_send;
    xevent.xclient.message_type = XdndEnter;
    xevent.xclient.format = 32;

    XDND_ENTER_SOURCE_WIN (&xevent) = owin->w;
    XDND_ENTER_THREE_TYPES_SET (&xevent, n > XDND_THREE);
    XDND_ENTER_VERSION_SET (&xevent, version);
    for (i = 0; i < n && i < XDND_THREE; i++)
	XDND_ENTER_TYPE (&xevent, i) = out_typelist[i];
    send_event(to_send, xevent);
    return 1;
}

int DndManager::send_leave(DndWin* o, Window to_send)
{
    XEvent xevent;
    memset (&xevent, 0, sizeof (xevent));

    xevent.xany.type = ClientMessage;
    xevent.xany.display = disp;
    xevent.xclient.window = to_send;
    xevent.xclient.message_type = XdndLeave;
    xevent.xclient.format = 32;

    XDND_LEAVE_SOURCE_WIN (&xevent) = o->w;

    return 1;
}

int DndManager::send_position(DndWin *owin, Window to_send,
			      Atom action, 
			      int x, int y, 
			      unsigned long cur_time)
{
    XEvent xevent;

    memset (&xevent, 0, sizeof (xevent));

    xevent.xany.type = ClientMessage;
    xevent.xany.display = disp;
    xevent.xclient.window = to_send;
    xevent.xclient.message_type = XdndPosition;
    xevent.xclient.format = 32;

    XDND_POSITION_SOURCE_WIN (&xevent) = owin->w;
    XDND_POSITION_ROOT_SET (&xevent, x, y);
    if (version_in>=1)
	XDND_POSITION_TIME (&xevent) = cur_time;
    if (version_in>=2)
	XDND_POSITION_ACTION (&xevent) = action;

    send_event(to_send, xevent);
    return 1;
}

int DndManager::send_selection(XSelectionRequestEvent *request, 
				char *data, 
				int length)
{
    XEvent xevent;
    xncdprintf(("requestor = 0x%x, property = %ld, length = %d\n", 
		request->requestor,
		request->property,length));

    XChangeProperty (disp, request->requestor, request->property,
		     request->target, 8, PropModeReplace, 
		     (unsigned char*)data, 
		     length);
    xevent.xselection.type = SelectionNotify;
    xevent.xselection.property = request->property;
    xevent.xselection.display = request->display;
    xevent.xselection.requestor = request->requestor;
    xevent.xselection.selection = request->selection;
    xevent.xselection.target = request->target;
    xevent.xselection.time = request->time;
    send_event(request->requestor, xevent);
    return 1;
}

int DndManager::send_drop(DndWin *owin, Window to_send, unsigned long time)
{
    XEvent xevent;

    memset (&xevent, 0, sizeof (xevent));

    xevent.xany.type = ClientMessage;
    xevent.xany.display = disp;
    xevent.xclient.window = to_send;
    xevent.xclient.message_type = XdndDrop;
    xevent.xclient.format = 32;

    XDND_DROP_SOURCE_WIN (&xevent) = owin->w;
    if (version_in>=1)
	XDND_DROP_TIME (&xevent) = time;

    send_event(to_send, xevent);
    return 1;
}


void DndManager::send_event(XEvent& ev)
{
  XSendEvent(disp,srcwin,False,0L,&ev);
}

void DndManager::send_event(Window to_win, XEvent& ev)
{
  XSendEvent(disp,to_win,False,0L,&ev);
}

int DndManager::set_selection_owner(DndWin *owin)
{
    if (!XSetSelectionOwner (disp, XdndSelection, owin->w, CurrentTime)) 
    {
	xncdprintf(("set_selection_owner(): XSetSelectionOwner failed\n"));
	return 0;
    }
    return 1;
}

//-----------------------------------------------------------------------------
// **************************    Asking methods   ****************************
//-----------------------------------------------------------------------------


//Checking is given Window from out list of registered dropping windows
//If so return 1, else 0 - need this for detecting internal drags
int DndManager::is_our_own_window(Window w)
{
    return 0; //Currently disabled
}

//-----------------------------------------------------------------------------
// Checking window for Dnd awareness, if yes then get 
// Dnd protocol version -> version_in
//-----------------------------------------------------------------------------
int DndManager::is_dnd_aware(DndWin* dndw, Window window)
{
    Atom actual;
    int format;
    unsigned long count, remaining;
    unsigned char *data = 0;
    Atom *types, *t;
    int result = 1;

    version_in = 0;
    XGetWindowProperty (disp, window, XdndAware,
			0, 0x8000000L, False, XA_ATOM,
			&actual, &format,
			&count, &remaining, &data);
    
    if (actual != XA_ATOM || format != 32 || count == 0 || !data) {
      xncdprintf(("XGetWindowProperty failed in is_dnd_aware - XdndAware = %ld\n", XdndAware));
      if (data)
	XFree (data);
      return 0;
    }
    types = (Atom *) data;
    if (types[0] < 3) {
      if (data)
	XFree (data);
      return 0;
    }
    version_in = XDND_VERSION < types[0] ? XDND_VERSION : types[0];	/* minimum */
    xncdprintf(("Using XDND version %d\n", version));
    if (count > 1) {
      result = 0;
      for (t = dndw->typelist; *t; t++) 
      {
	int j;
	for (j = 1; j < count; j++) 
	{
	  if (types[j] == *t) 
	  {
	    result = 1;
	    break;
	  }
	}
	if (result)
	  break;
      }
    }
    XFree (data);
    return result;
}

//-----------------------------------------------------------------------------
// ************************     Dragging   ********************************
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Do the whole dragging process, sending events to dropper, receiving answer
// processing all of that, calling other helpers
//-----------------------------------------------------------------------------
int DndManager::dragging(DndObject* o,DndData* dat)
{
  XEvent xevent;
  int    result,dummy;
  Window dummyw;
  unsigned int udummy;

  drag_want_position=0;
  drag_ready_to_drop=0;
  drag_dnd_aware=0;
  drag_dropper_toplevel=None;
  drag_last_dropper_toplevel=None;
  drag_dropper_window=None;
  drag_over_window=None;
  drag_last_window=None;
  out_typelist[0]=dat->type;
  out_typelist[1]=0;
  drag_status=dnd_dragging;
  drag_object=o;

  XQueryPointer(disp, DefaultRootWindow(disp), &dummyw, &dummyw, 
		&drag_startx, &drag_starty, &dummy, &dummy, &udummy);

  xncdprintf(("Starting drag from object %x\n",o));
  if (XGrabPointer (disp, rootw, 
		    False,
		    ButtonMotionMask | 
		    PointerMotionMask | 
		    ButtonPressMask | 
		    ButtonReleaseMask,
		    GrabModeAsync, 
		    GrabModeAsync, 
		    None,
		    (dat->file_listlen > 1 ? curs_many : curs_one), 
		    CurrentTime) != GrabSuccess)
  {
    xncdprintf(("Unable to grab pointer"));
    return 0;
  }

  memset(&xevent, 0, sizeof(xevent));

  while (xevent.xany.type != ButtonRelease) 
  {
    XAllowEvents (disp, SyncPointer, CurrentTime);
    XNextEvent (disp, &xevent);
    switch (xevent.type) 
    {
    case Expose:
	if(xevent_processor)
	    xevent_processor(&xevent);
	break;
    case EnterNotify:
	if(xevent_processor)
	    xevent_processor(&xevent);
      /* this event is not actually reported, so we find out by ourselves from motion events */
	break;
    case LeaveNotify:
	if(xevent_processor)
	    xevent_processor(&xevent);
      /* this event is not actually reported, so we find out by ourselves from motion events */
	break;
    case ButtonRelease:
      /* done, but must send a leave event */
	break;
    case MotionNotify:
	drag_process_motion_notify(o,xevent);
	break;
    case ClientMessage:
	drag_process_client_message(xevent);
	break;
    case SelectionRequest:
	drag_process_selection_request(dat, xevent);
	break;
      
    }
  }

  result = drag_do_actual_drop(o,dat,xevent);

  XUngrabPointer(disp,CurrentTime);
  drag_status=dnd_none;
  drag_object=0;
  xncdprintf(("======DRAGGING FINISHED=======\n"));
  return result;
}


//-------------------------------------------------------------------------
// Finishing dragging - do actual drop and waiting until reply
// (XDndFinished - is the final event)
//-------------------------------------------------------------------------
int DndManager::drag_do_actual_drop(DndObject *o, DndData *dat, XEvent& xevent)
{
    Time time;
    if(drag_ready_to_drop) 
    {
	xncdprintf(("DRAG-> ready_to_drop - sending XdndDrop\n"));
	time = xevent.xbutton.time;
	if (drag_internal) 
	{
	    /* we are dealing with our own widget, 
	       no need to send drop events, just put the data straight */
	} else 
	{
	    set_selection_owner(o->get_dndwin());
	    send_drop(o->get_dndwin(), drag_dropper_toplevel, time);
	}
	
	if (!drag_internal)
	    for (;;) 
	    {
		XAllowEvents (disp, SyncPointer, CurrentTime);
		XNextEvent(disp, &xevent);
		if (xevent.type == ClientMessage && 
		    xevent.xclient.message_type == XdndFinished) 
		{
		    xncdprintf(("DRAG-> XdndFinished, source correct - "
				"exiting event loop, action=%ld\n", 
				drag_supported_action));
		    return 1;
		  
		} else 
		    if (xevent.type == Expose) 
		    {
			if(xevent_processor)
			    xevent_processor(&xevent);
		    } else 
			if (xevent.type == MotionNotify) 
			{
			    if (xevent.xmotion.time > time + 
				(drag_drop_time_out ? drag_drop_time_out * 1000 : 10000)) 
			    {	/* allow a ten second timeout as default */
				xncdprintf(("DRAG-> timeout - exiting event loop\n"));
				return 0;
			    }
			} else 
			    if (xevent.type == SelectionRequest && 
				xevent.xselectionrequest.selection == XdndSelection) 
			    {
				drag_process_selection_request(dat,xevent);
				/* don't wait for a XdndFinished event */
				if (version_in<2)
				    return 1;
			    } else
				if(xevent_processor)
				    xevent_processor(&xevent);
	    }
    } else 
    {
	xncdprintf(("DRAG-> not ready_to_drop - ungrabbing pointer\n"));
    }

    return 0;
}


//-------------------------------------------------------------------------
// Dragging - receiving MotionNotify event with mouse positions
// Let's process them...
//-------------------------------------------------------------------------
void DndManager::drag_process_motion_notify(DndObject* o, XEvent& xevent)
{
    XEvent xevent_temp;
    int internal_dropable = 0;

    drag_dnd_aware = 0;
    drag_internal = 0;
    memcpy (&xevent_temp, &xevent, sizeof (xevent));

    xncdprintf(("MotionNotify for window 0x%x\n",xevent.xmotion.window));
    if(!drag_trace_dropper_window(o,xevent))
	return;
    
    /* drag_last_window is just for debug purposes */
    xncdprintf(("====>DRAG dropper_window/over_window is 0x%x/0x%x\n",
		drag_dropper_window,drag_over_window));
    
    if (drag_last_window != xevent.xmotion.subwindow) 
    {
	xncdprintf(("Window crossing to 0x%x\n", xevent.xmotion.subwindow));
	xncdprintf(("Current window is 0x%x\n", drag_over_window));
	xncdprintf(("drag_last_window = 0x%x, xmotion.subwindow = 0x%x\n", 
		    drag_last_window, xevent.xmotion.subwindow));
	xncdprintf(("drag_dropper_toplevel = 0x%x, drag_last_dropper_toplevel.subwindow = 0x%x\n", 
		    drag_dropper_toplevel, drag_last_dropper_toplevel));
    }
    //Need to check for internal drop here, leave for future....
    
    internal_dropable = 1;
    
    if ((drag_dropper_toplevel != drag_last_dropper_toplevel ||
	 drag_last_window != xevent.xmotion.subwindow) && internal_dropable &&
	drag_dnd_aware)
    {
	drag_leave_current_window(o);
	drag_enter_to_new_window(o,xevent,xevent_temp);      
    } else
    { //Moving in the same window
	drag_moving_and_sending(o,xevent);
    }
    
    drag_last_window = xevent.xmotion.subwindow;     
}

//-------------------------------------------------------------------------
// During dragging we send Position message to dropper and receive
// client_message with Status reply...
//-------------------------------------------------------------------------
void DndManager::drag_process_client_message(XEvent& xevent)
{
    xncdprintf(("Drag - ClientMessage recieved\n"));
    if (xevent.xclient.message_type == XdndStatus && !drag_internal) 
    {
	xncdprintf(("Drag - XdndStatus recieved\n"));
	if (drag_status == dnd_entered) 
	{
	    xncdprintf(("Drag - XdndStatus stage correct, dropper window correct\n"));
	    drag_want_position = XDND_STATUS_WANT_POSITION (&xevent);
	    drag_ready_to_drop = XDND_STATUS_WILL_ACCEPT (&xevent);
	    drag_rectangle.x = XDND_STATUS_RECT_X (&xevent);
	    drag_rectangle.y = XDND_STATUS_RECT_Y (&xevent);
	    drag_rectangle.width = XDND_STATUS_RECT_WIDTH (&xevent);
	    drag_rectangle.height = XDND_STATUS_RECT_HEIGHT (&xevent);
	    drag_supported_action = XdndActionCopy;
	    if(version_in>=2)
		drag_supported_action = XDND_STATUS_ACTION (&xevent);
	    xncdprintf(("DRAG->return action=%ld, ready=%d\n", 
			drag_supported_action, 
			drag_ready_to_drop));
	    /* if not ready, keep sending positions, this check is repeated 
	       above for internal widgets */
	    if (!drag_ready_to_drop) 
	    {
		drag_want_position = 1;
		drag_rectangle.width = drag_rectangle.height = 0;
	    }
	    xncdprintf(("Drag - rectangle = (x=%d, y=%d,w=%d, h=%d), want_position=%d\n", 
			drag_rectangle.x, drag_rectangle.y,
			drag_rectangle.width, drag_rectangle.height, 
			drag_want_position));
	}
	else 
	{
	    xncdprintf(("DRAG!! XdndStatus stage incorrect = %d", drag_status));
	}
    } else
	if(xevent_processor)
	    xevent_processor(&xevent);
}


//-------------------------------------------------------------------------
// Sending data to requestor through XSelection
//-------------------------------------------------------------------------
void DndManager::drag_process_selection_request(DndData *dat, XEvent& xevent)
{
    /* the target widget MAY request data, so wait for SelectionRequest */
    xncdprintf(("Drag - SelectionRequest - getting widget data\n"));

    //    (*dnd->widget_get_data) (dnd, from, &data, &length, xevent.xselectionrequest.target);
    xncdprintf(("Drag - sending selection to 0x%x\n",
		xevent.xselectionrequest.requestor));
    send_selection(&xevent.xselectionrequest, dat->data, dat->dat_idx+1);
}



//-------------------------------------------------------------------------
// On each MotionNotify event we send Position message to dropper
//-------------------------------------------------------------------------
void DndManager::drag_moving_and_sending(DndObject *o, XEvent& xevent)
{
    int x,y;
    
    /* got here, so we are just moving `inside' the same window */
    if (drag_status == dnd_entered)
    {
	if((xevent.xmotion.state & ShiftMask)==ShiftMask)
	    drag_supported_action = XdndActionMove;
	else
	    drag_supported_action = XdndActionCopy;
	xncdprintf(("Got motion at right stage\n"));
	x = xevent.xmotion.x_root;
	y = xevent.xmotion.y_root;
	if (drag_want_position || drag_outside_rectangle(x, y)) 
	{
	    xncdprintf(("Want position and outside rectangle\n"));
	    if (drag_internal) 
	    {
		xncdprintf(("Our own widget\n"));
		/* if not ready, keep sending positions, this check is repeated below for XdndStatus from external widgets */
		if (!drag_ready_to_drop) 
		{
		    drag_want_position = 1;
		    drag_rectangle.width = drag_rectangle.height = 0;
		}
	    } else 
	    {
		xncdprintf(("Not our own widget - sending XdndPosition to 0x%x, action %ld\n", 
			    drag_dropper_toplevel, 
			    drag_supported_action));
		send_position(o->get_dndwin(), drag_dropper_toplevel, 
			      drag_supported_action, 
			      x, y, xevent.xmotion.time);
	    }
	} else 
	    if (drag_want_position) 
	    {
		xncdprintf(("We are inside rectangle\n"));
	    } else 
	    {
		xncdprintf(("Opponent doesn't want position\n"));
	    }
    }
}

//-------------------------------------------------------------------------
// We are crossing window edge -> entering to the new window,
// send Enter to it.
//-------------------------------------------------------------------------
void DndManager::drag_enter_to_new_window(DndObject* o, XEvent& xevent,
					  XEvent& xevent_temp)
{
    /* entering window we are currently over */
    drag_over_window = xevent.xmotion.subwindow;
    xncdprintf(("Enetering to window 0x%x\n",drag_over_window));
    if (drag_dnd_aware) 
    {
	drag_status=dnd_entered;
	
	if(is_our_own_window(drag_over_window))
	{
	    drag_internal=1;
	    //Internal object do optimization
	} else
	{
	    xncdprintf(("Not our widget - sending XdndEnter to 0x%x\n", drag_over_window));
	    if (drag_dropper_toplevel != drag_last_dropper_toplevel)
		send_enter(o->get_dndwin(), drag_dropper_toplevel);
	}
	drag_want_position = 1;
	drag_ready_to_drop = 0;
	drag_rectangle.width = drag_rectangle.height = 0;
	drag_dropper_window = drag_over_window;
	/* we want an additional motion event in case the pointer enters and then stops */
	XSendEvent (disp, o->get_dndwin()->w, 0, ButtonMotionMask, &xevent_temp);
	XSync (disp, 0);
    }
    drag_last_dropper_toplevel = drag_dropper_toplevel;
    /* we are now officially in a new window */
}

//-------------------------------------------------------------------------
// We are crossing window edge -> leaving current window, send Leave to it
//-------------------------------------------------------------------------
void DndManager::drag_leave_current_window(DndObject* o)
{
    /* leaving window we were over */
    xncdprintf(("Leaving the window 0x%x\n",drag_over_window));
    if(drag_over_window)
    { 
	if (drag_status == dnd_entered) 
	{
	    xncdprintf(("Got leave at right stage for 0x%x\n",drag_over_window));
	    drag_status = dnd_dragging;
	    
	    if (is_our_own_window(drag_over_window)) 
	    {
		drag_internal=1;
		xncdprintf(("Leaving our own widget\n"));
		// Need to call XDndLeave internally
	    } else 
	    {
		xncdprintf(("Not our widget - sending XdndLeave\n"));
		if (drag_dropper_toplevel != drag_last_dropper_toplevel)
		    send_leave(o->get_dndwin(),drag_last_dropper_toplevel);
	    }
	    //Make cleanup here
	    drag_dropper_window=None;
	    drag_internal=0;
	} else 
	{
	    xncdprintf(("Got leave at wrong stage - ignoring\n"));
	} 
    }
}

//-------------------------------------------------------------------------
// We are moving, so we need to trace windows stack and find proper
// window to drop with DndAware.
//-------------------------------------------------------------------------
int DndManager::drag_trace_dropper_window(DndObject *obj, XEvent& xevent)
{
    Window root_return, child_return;
    int x_temp, y_temp;
#ifdef DEBUG_XNC
    int x_return, y_return;
    unsigned int width_return, height_return, depth_return, border_return;
#endif
    unsigned int mask_return;
    xncdprintf(("--------trace from root window---------\n"));
    xevent.xmotion.subwindow = DefaultRootWindow(disp);
    while (XQueryPointer (disp, xevent.xmotion.subwindow, 
			  &root_return, &child_return,
			  &x_temp, &y_temp, &xevent.xmotion.x,
			  &xevent.xmotion.y, &mask_return)) 
    {
#ifdef DEBUG_XNC
	XGetGeometry(disp,xevent.xmotion.subwindow,&root_return,
		     &x_return, &y_return,
		     &width_return, &height_return,
		     &border_return, &depth_return);
#endif
	xncdprintf(("Tracing pointer to window 0x%x at %d,%d [%dx%d]\n",
		    xevent.xmotion.subwindow,
		    xevent.xmotion.x,
		    xevent.xmotion.y,
		    width_return,
		    height_return));
	if (!drag_dnd_aware) 
	{
	    if ((drag_dnd_aware = is_dnd_aware(obj->get_dndwin(), xevent.xmotion.subwindow))) 
	    {
		drag_dropper_toplevel = xevent.xmotion.subwindow;
		xevent.xmotion.x_root = x_temp;
		xevent.xmotion.y_root = y_temp;
		xncdprintf(("Window 0x%x is supported XDND version %d\n",
			    drag_dropper_toplevel,version_in));
	    }
	}
	if (!child_return)
	    return 1;
	xevent.xmotion.subwindow = child_return;
    }
    return 0;
}

//-------------------------------------------------------------------------
// Just check that we are inside rectangle
//-------------------------------------------------------------------------
int  DndManager::drag_outside_rectangle(int x, int y)
{
    return (x < drag_rectangle.x || 
	    y < drag_rectangle.y || 
	    x >= drag_rectangle.x + drag_rectangle.width || 
	    y >= drag_rectangle.y + drag_rectangle.height);
}

//-----------------------------------------------------------------------------
// **********************       Debugging and other     **********************
//-----------------------------------------------------------------------------

void DndManager::debug_atom(Atom a)
{
#ifdef DEBUG_XNC
  if(!a)
    return;
  char *str=XGetAtomName(disp,a);
  xncdprintf(("Atom: '%s' - %d - 0x%x\n",str,a,a));
  XFree(str);
#endif
}

int DndManager::atom_array_length(Atom* ar)
{
    int i;
    for(i=0;ar[i];i++);
    return i;
}

//-----------------------------------------------------------------------------
//          Adding type to in_type
//-----------------------------------------------------------------------------

//Adding type for our_in_types that we will accept
void DndManager::add_in_type(Atom intype)
{
  int i;
  for(i=0;our_in_typelist[i];i++)
    if(our_in_typelist[i]==intype)
      return;
  our_in_typelist[i]=intype;
  xncdprintf(("Atom added to our_in_typelist:\n"));
  debug_atom(intype);
}

void DndManager::add_in_type(char* name)
{
  Atom a=XInternAtom(disp,name,False);
  add_in_type(a);
}

