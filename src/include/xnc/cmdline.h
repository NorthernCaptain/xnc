#ifndef CMDLINE_H
#define CMDLINE_H

#include "gui.h"
#include "metacaller.h"

const int max_cmd_history=15;

struct StoreBuffer
{
    char      store[L_MAXPATH];
    char      store_cp, store_base;
    StoreBuffer() {store[0]=0; store_cp=store_base=0;};
};

class Cmdline:public Gui
{
 protected:
    int       ty,bbp,bp;
    uint      tl,tw;
    int       delta_x, text_x, cmd_text_x, text_border;
    Pixmap    history_pixmap;
    StoreBuffer store_buf;
    void      delete_one_symbol();
    void      history_up_line();
    void      history_down_line();
    void      move_cursor_right();
    void      move_cursor_left();
    void      add_string_to_backup(char* str);
    void      load_history();
    void      show_history_menu();

 public:
    uint      bl;
    int       col,hflg,cp,ll,el,visl,base;
    char      name[L_MAXPATH];
    char      buf[L_MAXPATH];
    char      bak[max_cmd_history][L_MAXPATH+1];
    GuiLocale gl;
    MenuItem  history_menu_items[max_cmd_history];
    CmdlineCaller history_callers[max_cmd_history];
    Menu      *history_menu;

    Cmdline(int ix,int iy,int il,int icol):Gui()
	{
	    x=ix;y=iy;l=il;
	    col=icol;
	    name[0]=0;
	    tl=0;
	    cp=0;bl=0;
	    buf[0]=0;
	    el=L_MAXPATH-1;
	    guitype=GUI_CMDLINE;
	    visl=base=0;
	    history_menu=0;
	};
    ~Cmdline() {if(disp) XFreeGC(disp,gcw);};
    void setpath(char*);
    void press(int);
    void showbuf();
    int  flush();
    void insert(char);
    void reconfigure(int ix,int iy,int il,int ih);
    void backup();
    int  look_for_cd();
    virtual void init(Window);
    virtual void show();
    virtual void hide();
    virtual void click();
    virtual void expose();
    void save_input(StoreBuffer* pbuf=0);
    void restore_input(StoreBuffer* pbuf=0);
    virtual int  dnd_drop(DndData*, int x, int y, DndAction action);
    int  menu_choose_history(int idx, MetaCallData);
};

#endif
