#ifndef GUI_MENU_H
#define GUI_MENU_H
#include "gui_base.h"
#include "metacaller.h"
#include "iconsets.h"
#include "x_actions.h"

class Menu;

enum MenuState { 
    menu_none,
    menu_function,      //Menu function
    menu_sw_set,    //Menu switch state
    menu_sw_unset,  //Menu switch state
    menu_ch_set,    //Menu checkbox
    menu_ch_unset,
    menu_menu       //Menu with submenu
};

//Struct Menu Item that defines one menu entry
struct MenuItem
{
    MenuState   menu_state;
    char       *name;
    ulong       col;
    void      (*func)();
    Menu       *o;
    MetaCaller *metacall;
    MenuTypeIconset icon_type;
    int         action;  
    void init(char *iname, ulong icol, void (*ifunc)(), 
	      Menu *iobj, 
	      MetaCaller* mcall, 
	      MenuTypeIconset icon=no_icon,
	      MenuState mstate=menu_function,
	      int iaction=AEmpty)
    {
	name=iname;
	o=iobj;
	func=ifunc;
	metacall=mcall;
	col=icol;
	icon_type=icon;
	menu_state=mstate;
	action=iaction;
    };
};

class MenuBar; //We need this class here for notifing such objects for drawing extra stuff

// This class provide menu window with items in it
// Uses MenuItem for displaying menu entry and activate calls on choose
class Menu:public Gui
{
 protected:
  int     *dx;
  int     *tl;
  int     *dl;
  MenuItem *items;
  Pixmap  xpm;
  char**  names;
  char**  action_names;
  int*    action_len;
  int*    action_len_x;
  //  List *oldel;

  static int  max_icon_width;
  static int  window_border;
  static unsigned int mitemh;
  static int  icon_delta_x;
  static unsigned int action_space;
  static int  action_delta_x;

  void  grab_now();
  void  ungrab_now();
  void  calculate_xy_by_parent();
  void  execute();
  int   construct_name(int idx, const char* name);
  void  delete_names();
  void  new_names();
  char* item_name(int idx) { return names[idx];};
  char* action_name(int idx) { return action_names[idx];};

 public:
  int max,ty,cur,prflg,shflg,actfl;
  MenuBar *menu_to_notify;
  int     right_justified;
  
  Menu(MenuItem *ii,int maxi) :Gui()
    {
	tl=0;
	names=0;
	guitype=GUI_MENU;
	right_justified=NO;
	menu_to_notify=NULL;
	set_menu(ii, maxi);
    };
  virtual ~Menu() 
      {
	  delete_names();
      };

  virtual void set_menu(MenuItem *ii, int maxi);
  virtual void init(Window);
  virtual void expose();
  virtual void click();
  virtual void show();
  virtual void hide();
  virtual void showitem(int);
  virtual void select(int);
  void setpos(int ix,int iy) {x=ix;y=iy;};
  int  is_shown() {return shflg;};
  int  is_active() {return actfl;};
  void switches(int num=-1);
  void set(int i) {cur=i;switches();};
};


//Class provides horizontal menu bar with buttons
//each button can have action or Menu
//Class uses MenuItem for displaing and activating items 
class MenuBar:public Gui
{
 protected:
  int *dx;
  int *tl;
  int *dl;
  MenuItem *items;
  int max,ty,cur,prflg,enx,stx,deltal;
  Sprite *skin;
  Sprite *skinpr;
  Pixmap fillpix;
  unsigned int fill_l, fill_h;
  Sprite *pskin;
  int last_idx;
  int highlight_idx;

  void     calculate_sizes();

 public:
  
  int menuh;
  
  MenuBar(int ix,int iy,MenuItem *ii,int maxi):Gui() 
    {x=ix;y=iy;max=maxi;items=ii;dx=new int[max];tl=new int[max];dl=new int[max];
        guitype=GUI_MENUBAR;deltal=0;skin=NULL;skinpr=pskin=NULL;last_idx=-1;};
  virtual ~MenuBar() {delete dx;delete tl;delete dl;if(disp) XFreeGC(disp,gcw);};
  virtual void init(Window);
  virtual void expose();
  virtual void click();
  virtual int  geometry_by_iname();
  void showitem(int);
  void reconfigure(int ix,int iy,int pl,int ph);
  void pressitem(int);
  Menu* getmenu() {return items[cur].o;};
  void substract_from_l(int dd) {deltal=dd;};
  void notify(int code);
};

class Separator:public Gui
{
  public:
   int vertical;
   Gui *win;
   Separator(int ix,int iy,int il,Gui *ww,int ivert=NO) : Gui() {x=ix;y=iy;l=il;win=ww;guitype=GUI_SEP;vertical=ivert;};
   virtual void init(Window) {w=win->w;gcw=win->gcw;};
   virtual void show() {}; 
   virtual void hide() {};
   virtual void click();
   virtual void expose();
   virtual int  is_selfdraw() { return 0;}; //Redraw object by itself (own window)
};


extern MenuBar *current_menu_bar;

#endif
