#ifndef XDNDCLASS
#define XDNDCLASS

#include "globals.h"     //Common headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

//////////////////////////Defines//////////////////////////
#define XDND_VERSION 3


#define XDND_MESTYPE(e)                 ((e)->xclient.message_type)

/* XdndEnter */
#define XDND_THREE 3
#define XDND_ENTER_SOURCE_WIN(e)	((e)->xclient.data.l[0])
#define XDND_ENTER_THREE_TYPES(e)	(((e)->xclient.data.l[1] & 0x1UL) == 0)
#define XDND_ENTER_THREE_TYPES_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_ENTER_VERSION(e)		((e)->xclient.data.l[1] >> 24)
#define XDND_ENTER_VERSION_SET(e,v)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~(0xFF << 24)) | ((v) << 24)
#define XDND_ENTER_TYPE(e,i)		((e)->xclient.data.l[2 + i])	/* i => (0, 1, 2) */

/* XdndPosition */
#define XDND_POSITION_SOURCE_WIN(e)	((e)->xclient.data.l[0])
#define XDND_POSITION_ROOT_X(e)		((e)->xclient.data.l[2] >> 16)
#define XDND_POSITION_ROOT_Y(e)		((e)->xclient.data.l[2] & 0xFFFFUL)
#define XDND_POSITION_ROOT_SET(e,x,y)	(e)->xclient.data.l[2]  = ((x) << 16) | ((y) & 0xFFFFUL)
#define XDND_POSITION_TIME(e)		((e)->xclient.data.l[3])
#define XDND_POSITION_ACTION(e)		((e)->xclient.data.l[4])

/* XdndStatus */
#define XDND_STATUS_TARGET_WIN(e)	((e)->xclient.data.l[0])
#define XDND_STATUS_WILL_ACCEPT(e)	((e)->xclient.data.l[1] & 0x1L)
#define XDND_STATUS_WILL_ACCEPT_SET(e,b) (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_STATUS_WANT_POSITION(e)	((e)->xclient.data.l[1] & 0x2UL)
#define XDND_STATUS_WANT_POSITION_SET(e,b) (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x2UL) | (((b) == 0) ? 0 : 0x2UL)
#define XDND_STATUS_RECT_X(e)		((e)->xclient.data.l[2] >> 16)
#define XDND_STATUS_RECT_Y(e)		((e)->xclient.data.l[2] & 0xFFFFL)
#define XDND_STATUS_RECT_WIDTH(e)	((e)->xclient.data.l[3] >> 16)
#define XDND_STATUS_RECT_HEIGHT(e)	((e)->xclient.data.l[3] & 0xFFFFL)
#define XDND_STATUS_RECT_SET(e,x,y,w,h)	{(e)->xclient.data.l[2] = ((x) << 16) | ((y) & 0xFFFFUL); (e)->xclient.data.l[3] = ((w) << 16) | ((h) & 0xFFFFUL); }
#define XDND_STATUS_ACTION(e)		((e)->xclient.data.l[4])

/* XdndLeave */
#define XDND_LEAVE_SOURCE_WIN(e)	((e)->xclient.data.l[0])

/* XdndDrop */
#define XDND_DROP_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_DROP_TIME(e)		((e)->xclient.data.l[2])

/* XdndFinished */
#define XDND_FINISHED_TARGET_WIN(e)	((e)->xclient.data.l[0])


enum DndStatus 
    { 
	dnd_none=0, 
	dnd_entered=1, 
	dnd_dragging=2 
    };

enum DndAction
    {
	dnd_copy=0,
	dnd_move=1,
	dnd_link=2,
	dnd_private=3
    };

///////////////////////Classes//////////////////////////////////////

const int MAX_WINS=            100; //Maximum number or dnd windows.
const int MAX_TYPES=            20; //Maximum number of different types supported by app
const int MAX_TYPES_PER_ITEM=   10; //Maximum number of different types for each app window

class DndManager;
class DndObject;

struct DndData
{
  char*  data;
  int    len;
  Atom   type;
  char** file_list;
  int    file_listlen;
  int    dat_idx;
  DndData(char* idata, int ilen,Atom itype)
  {
    len=ilen;
    data=new char[len+1];
    strncpy(data,idata,len);
    data[len]=0;
    type=itype;
    file_list=0;
    file_listlen=0;
    dat_idx=len;
  };
  DndData(int ilen,Atom itype)
  {
    len=ilen;
    data=new char[len+1];
    data[0]=0;
    type=itype;
    file_list=0;
    file_listlen=0;
    dat_idx=0;
  };
  ~DndData() { delete data; free_file_list();};
  int  break_to_filelist();
  void free_file_list();
  int  calc_entries(char* dat, char sym);
  void show_filelist();
  void add_file_entry(char* dir, char* f);
};


struct DndWin
{
  Window        w;
  DndManager*   man;
  DndObject*    obj;
  int           rx,ry;
  uint          l,h;
  Atom*         typelist;

  DndWin(DndManager* iman,Window iw, DndObject *o, int ix, int iy, uint il, uint ih);
  DndWin(DndManager* iman,Window iw, int ix, int iy, uint il, uint ih);
  ~DndWin() {delete typelist;};
  
  void add_type(char*);
  int  check_type(Atom);
  
};

class DndObject
{
 protected:
  DndWin*       dndwin;
  DndManager*   dndman;

 public:
  DndObject() { dndwin=0;dndman=0;};

  DndWin*       get_dndwin() { return dndwin;};

  virtual void  dnd_init(DndWin* iwin, DndManager* iman) { dndwin=iwin; dndman=iman;};

  virtual int   dnd_enter(XEvent*, int x, int y);
  virtual int   dnd_leave(XEvent*, int x, int y);
  virtual int   dnd_position(XEvent*, int newx, int newy);
  virtual int   dnd_drop(DndData*, int x, int y, DndAction action);
  virtual int   dnd_drag_now(DndData*);
};

class DndManager
{
 protected:
  int             version;


  Display*        disp;
  Window          rootw;

  Window          srcwin;    //source window that provides the data
  int             curtype;   //current input type of data 
  int             curx, cury;
  Atom            curaction;
  DndWin*         curwin;

  DndWin**        winar;

  Atom*           typelist;
  Atom*           our_in_typelist;
  Atom*           out_typelist;
  
  //Atoms for working with given names
  Atom            XdndAware;
  Atom            XdndSelection;
  Atom            XdndEnter;
  Atom            XdndLeave;
  Atom            XdndPosition;
  Atom            XdndDrop;
  Atom            XdndFinished;
  Atom            XdndStatus;
  Atom            XdndActionCopy;
  Atom            XdndActionMove;
  Atom            XdndActionLink;
  Atom            XdndActionAsk;
  Atom            XdndActionPrivate;
  Atom            XdndTypeList;
  Atom            XdndActionList;
  Atom            XdndActionDescription;
  
  Atom            dnd_data_atom;
  char*           dnd_data;
  int             version_in;
  Cursor          curs_one,curs_many;

  //Dragging vars
  DndStatus       drag_status;
  Window          drag_over_window;
  Window          drag_last_window;
  Window          drag_last_dropper_toplevel;
  Window          drag_dropper_toplevel;
  Window          drag_dropper_window;
  XRectangle      drag_rectangle;
  Atom            drag_supported_action;
  int             drag_dnd_aware;
  int             drag_want_position;
  int             drag_ready_to_drop;
  int             drag_internal;
  int             drag_drop_time_out;
  int             drag_startx;
  int             drag_starty;
  DndObject      *drag_object;
  int             (*xevent_processor)(XEvent*);


  void            make_dnd_aware();
  int             find_win_idx(Window);
  DndWin*         find_dnd_win(int rx, int ry, Atom action);
  int             new_idx();
  void            trace_win_geom(Window w, int& x, int& y, uint& l, uint& h);

  //Processing drop events
  int             process_XdndEnter(XEvent* ev, Window srcw, int version);
  int             process_XdndPosition(XEvent* ev, Window srcw);
  int             process_XdndLeave(XEvent*);
  int             process_XdndDrop(XEvent* ev);
  int             build_bigtype_list(XEvent* ev,Window srcw);
  int             build_threetype_list(XEvent* ev,Window srcw);
  int             check_type_list(XEvent* ev);
  int             data_ready_get_it();
  int             try_convert_drag_data(XEvent* ev,Window srcw);
  DndAction       atom_action_to_dnd_action(Atom act);

  // Sending events
  int             send_status(int will_accept,Atom action=None,
			      int want_position=0, int x=0, int y=0, 
			      uint l=0, uint h=0);
  int             send_finished();
  int             send_enter(DndWin* dndwin, Window to_send);
  int             send_leave(DndWin* o, Window to_send);
  int             send_position(DndWin *owin, Window to_send,
				Atom action, 
				int x, int y, 
				unsigned long cur_time);
  int             send_selection(XSelectionRequestEvent * request, 
				 char *data, 
				 int length);
  int             send_drop(DndWin *owin, Window to_send, unsigned long time);

  void            send_event(XEvent& ev);
  void            send_event(Window w,XEvent& ev);
  int             set_selection_owner(DndWin *owin);

  DndWin*         add_dnd_win(Window w);

  int             is_dnd_aware(DndWin* dndwin, Window window);
  void            create_dnd_cursors();
  int             is_our_own_window(Window);
  int             atom_array_length(Atom* ar);

  //Helpers for dragging
  int             drag_trace_dropper_window(DndObject* obj,XEvent& xevent);
  void            drag_leave_current_window(DndObject* o);
  void            drag_enter_to_new_window(DndObject* o, XEvent& xevent,
					   XEvent& xevent_temp);
  void            drag_moving_and_sending(DndObject *o, XEvent& xevent);
  void            drag_process_client_message(XEvent& xevent);
  void            drag_process_motion_notify(DndObject* o, XEvent& xevent);
  void            drag_process_selection_request(DndData *dat, XEvent& xevent);
  int             drag_do_actual_drop(DndObject *o, DndData *dat, XEvent& xevent);
  int             drag_outside_rectangle(int x, int y);

 public:
    DndManager(Display*,Window);
    ~DndManager();
    
    DndWin*         add_dnd_object(DndObject* o, Window w);
    void            del_dnd_win(Window w);
    int             process_event(XEvent* e);
    int             set_external_xevent_processor(int (*xproc)(XEvent*)) 
	{ xevent_processor=xproc; return 1;};
    void            add_in_type(Atom intype);
    void            add_in_type(char* intypename);
    void            debug_atom(Atom);
    void            update_geoms();
    int             move_to_top(Window w);
    int             dragging(DndObject* o,DndData* d);
    void            global2win_coords(Window w, int fromx, int fromy, int& x, int& y);
    Display*        display() { return disp;};
    int             is_dragging() { return drag_status!=dnd_none;};
    DndObject*      get_drag_object() { return drag_object;};
    int             get_drag_startx() { return drag_startx;};
    int             get_drag_starty() { return drag_starty;};
};

extern DndManager* default_dnd_man;

#endif
/* ------------ End of file -------------- */

