/* $Id: panel.h,v 1.24 2003/12/25 13:27:30 leo Exp $ */
#ifndef PANEL_H
#define PANEL_H
#include "base_cap.h"
#include "scroll.h"
#include "mouse_actions.h"
#include "cmdline.h"

extern int mystrcmp(char*,int,int,char*,int,int);

#define STL_SIZE_NCHAR   13
#define STL_ATTR_NCHAR    4
#define STL_TIME_NCHAR   17
#define STL_OWNER_NCHAR  16


#define STL_SIZE_PTR     stl_arr[0]
#define STL_ATTR_PTR     stl_arr[1]
#define STL_TIME_PTR     stl_arr[2]
#define STL_NAME_PTR     stl_arr[3]
#define STL_OWNER_PTR    stl_arr[4]

#define STL_MAX_MEM      5

//StatusLine height
#define STL_H            22

#define STL_SPACE        7
#define STL_SPACE2       (STL_SPACE/2)
struct StatusLinePos
{
    int x;    //Offset from start in horizontal
    int y;    //Offset from start in vetical
    int tx;   //Offset for text
    int ty;   //Offset for text
    int tlen; //Max length of text
    int len;  //Length of area in pixels
    int enabled; //If 1 then display this entry
};

enum PageOrientation { OnePage=0, LeftPage=1, RightPage=2 };

enum PanelDisplayMode { FullMode=0, BriefMode=1, CustomMode=2 };

enum PanelArea { 
    areaNone=0, 
    areaHeaderActive=1,
    areaHeaderInactive=2,
    areaHeaderMenuButton=3,
    areaMain=4,
    areaMainActive=5,
    areaStatusBar=6,
    areaStatusBarButtonLeft=7,
    areaStatusBarButtonRight=8,
    areaStatusBarAttribute=9
};


class Lister;

struct PanelAction
{
    int       action;
    int       (Lister::*method)(MouseData*);
};

const int max_context_menu_items=30;
const int L_DISPLAY_FORMAT=20;
const int MAX_INTERVALS=20;
const int MAX_FORMATS=3;

class Lister:public Gui,public BaseCaptain
{
 protected:

    PageOrientation orientation;

    struct HeaderSize
    {
	int x,y,l,h;
	int active_from_x, active_to_x;
	int menu_button_from_x, menu_button_to_x;
	HeaderSize() { x=y=l=h=0; active_from_x=active_to_x=0;};
	virtual void recalc_data(PageOrientation, int, int, int, int);
	
	virtual PanelArea get_area_by_xy(int ix, int iy);
	virtual int  is_inside(int ix, int iy);
	virtual int  is_inside_active(int ix, int iy);
	virtual int  is_inside_menu_button(int ix, int iy);
    };
    friend struct HeaderSize;


    struct MainSize
    {
	int x,y,l,h;
	int active_from_x, active_from_y;
	int active_to_x,active_to_y;
	int active_l, active_h;
	int text_h,text_y;
	int item_h,item_l;
	MainSize() 
	{
	    x=y=l=h=0;
	    active_from_x=active_from_y=0;
	    active_to_x=active_to_y=0;
	    active_l=active_h=0;
	    text_h=text_y=0;
	};
	virtual void recalc_data(int ix, int iy, int il, int ih, int pixs_col);
	virtual PanelArea get_area_by_xy(int ix, int iy);
    };
    friend struct MainSize;

    struct StatusBarSize
    {
	int x,y,l,h;
	int active_from_x,active_to_x,
	    active_l, active_h,
	    active_from_y, active_to_y,
	    active_text_y;
	int left_button_x, right_button_x,
	    buttons_y;
	StatusBarSize() {x=y=l=h=0;};
	void recalc_data(int ix, int iy, int il, int fixh, int fixy);
	virtual PanelArea get_area_by_xy(int ix, int iy);
    };
    friend struct StatusBarSize;


    static PanelAction  mouse_actions[];

    struct DisplayFileFormat
    {
	struct Interval
	{
	    int         from_x, len;
	    void        (Lister::*draw_method)(int x, int y, int ty, unsigned int color,
					       DisplayFileFormat::Interval interval, 
					       FList* item);
	    void        init(int fx, int ln, 
			     void (Lister::*method)(int x, int y, int ty, unsigned int color,
						    DisplayFileFormat::Interval interval, 
						    FList* item)
			     )
	    {
		from_x=fx;
		len=ln;
		draw_method=method;
	    };
	};

	char          format[L_DISPLAY_FORMAT];
	Interval      intervals[MAX_INTERVALS];
	int           max_interval;
	int           max_filename_len;
	XFontStruct   *fontstr;
	
	DisplayFileFormat();
	void          set_max_filename_len(int max) { max_filename_len=max;};
	void          set_format(char* fmt);
	void          init(int maxlen, XFontStruct *ifontstr);
	int           get_interval_len(int, int);
    };
    friend struct DisplayFileFormat;

	      //swap_panels();
    static int    header_draw_diagonal_offset;
    static int    header_height; //Header height in pixels
    static int    header_active_percent; //Percent of length in active
    static int    header_menu_button_width;
    static int    header_menu_button_border;
    static int    header_inner_delta;
    static int    header_border_around_font;
    static int    main_area_delta_x; //Delta x from vertical edges of area
    static int    main_area_delta_y; //Delta y ...
    static int    main_area_column_border; //border around each column
    static int    main_area_item_border; //border around text of the item
    static int    main_area_icons_width; //max width of icons to display
    static int    status_area_height; //Height of status bar
    static int    status_area_delta_x;
    static int    status_area_delta_y;
    static int    status_area_button_l;
    static int    status_area_button_h;
    static int    status_border_around_font;
    static int    file_display_border; //border beetwen parts of field


    DisplayFileFormat  display_format[MAX_FORMATS];
    PanelDisplayMode   display_mode;

    GC            wgcw;
    GC            xpmgc;
    XRectangle    rect,rect2;
    
    HeaderSize    head;
    MainSize      main_area;
    StatusBarSize status_area;

    StatusLinePos stl_arr[STL_MAX_MEM];

    unsigned int  pixels_per_column;

    Menu          *context_menu;
    MenuItem      context_items[max_context_menu_items];
    int           max_context_idx;
    
    void          set_clip_area(int x, int y, DisplayFileFormat::Interval ival);
    int           internal_dnd_drop(int mx, int my, DndAction action);

    virtual void  vlook_draw_head();
    virtual void  vlook_draw_frame();
    virtual void  vlook_display_header_string(char* str, int str_len,
					      unsigned long text_color);
    virtual void  vlook_display_header_inactive_string(char* str, int str_len,
						       unsigned long text_color);
    virtual void  vlook_draw_column_separators();
    virtual void  vlook_draw_status_buttons();
    virtual void  vlook_recalc_all_data();

    virtual int   get_xy_of_item_by_number(int n, int& to_x, 
					   int& to_y_text,
					   int& to_y);
    virtual int   get_item_number_by_xy(int x, int y, int& k); 
    
    void          process_mouse_press_event(XEvent& ev);
    void          process_mouse_release_event(XEvent& ev);
    void          process_mouse_press_on_status_bar(XEvent& ev);
    void          process_mouse_motion_event(XEvent& ev);

    int           call_method_by_action_for_mouse(MouseData data);
    void          build_context_menu_for_current_file();
    void          build_context_menu_for_selected_files();

    virtual void  showdirname();              // Show directory name in the header
    virtual void  showitem(FList*,int);       // Show one file item
    virtual void  showempty(int);             // Show one empty item
    virtual void  show_ff_item(int,int);      // Show fast find item
    virtual void  show_string_info(char*);    // Show given string in status bar

    void          show_remount_device_list();
    void          show_context_menu(int ix, int iy);
    void          change_attributes();

    int           set_cursor_by_xy_no_redraw(int x, int y);
    int           set_cursor_by_xy_with_redraw(int x, int y);

    //Mouse actions
    int           mouse_action_execute(MouseData* pdata);
    int           mouse_action_set_current(MouseData* pdata);
    int           mouse_action_left(MouseData* pdata);
    int           mouse_action_right(MouseData* pdata);
    int           mouse_action_up(MouseData* pdata);
    int           mouse_action_down(MouseData* pdata);
    int           mouse_action_page_up(MouseData* pdata);
    int           mouse_action_page_down(MouseData* pdata);
    int           mouse_action_select(MouseData* pdata);
    int           mouse_action_home(MouseData* pdata);
    int           mouse_action_end(MouseData* pdata);
    int           mouse_action_delete(MouseData* pdata);
    int           mouse_action_copy(MouseData* pdata);
    int           mouse_action_move(MouseData* pdata);
    int           mouse_action_edit(MouseData* pdata);
    int           mouse_action_view(MouseData* pdata);
    int           mouse_action_chown(MouseData* pdata);
    int           mouse_action_change_attributes(MouseData* pdata);
    int           mouse_action_select_by_ext(MouseData* pdata);
    int           mouse_action_deselect_by_ext(MouseData* pdata);
    int           mouse_action_inode_info(MouseData* pdata);
    int           mouse_action_quick_file_info(MouseData* pdata);
    int           mouse_action_context_menu(MouseData* pdata);

    //File display formatters
    void          fmt_draw_file_name(int, int, int, unsigned int,
				     DisplayFileFormat::Interval, FList*);
    void          fmt_draw_file_time(int, int, int, unsigned int,
				     DisplayFileFormat::Interval, FList*);
    void          fmt_draw_file_owner(int, int, int, unsigned int,
				      DisplayFileFormat::Interval, FList*);
    void          fmt_draw_file_size(int x, int y, int ty, unsigned int color,
				     DisplayFileFormat::Interval ival, 
				     FList* ol);
    void          fmt_draw_file_attr_oct(int, int, int, unsigned int,
					 DisplayFileFormat::Interval, FList*);
    void          fmt_draw_file_attr(int, int, int, unsigned int,
				     DisplayFileFormat::Interval, FList*);
    void          fmt_draw_file_separator(int, int, int, unsigned int,
					  DisplayFileFormat::Interval, FList*);
    void          clean_expose();
    void          get_geometry(int& ix, int& iy,
			       unsigned int& l, unsigned int& h);
    void          run_with_extension_menu();
    void          focus_in();
    void          focus_out();

 public:

    Lister *panel2;
    Cmdline *cmdl;
    int bclk,lmx,lmy,lowfl,side,mouse;
    int prflg,lastn,fixl,fixh,fixy,fattr1;
    int attdx;
    Time dbtime;
    Menu *subm;
    void (*viewfunc)(char*);
    void (*viewonlyfunc)(char*);
    void (*editorfunc)(char*);
    ScrollBar *scr;
    
    Lister(int ix,int iy,int il,int ih,int icolumns=2);

    void         repanel() { repanel(col);};
    void         repanel(int);
    void         delall_l(FList*);
    void         cmdl_insert_name(char *name);
    void         reconfigure(int ix,int iy,int il,int ih);
    void         reconfigure_without_expose(int ix,int iy,int il,int ih);
    void         setcmdl(Cmdline* cmd) {cmdl=cmd;};
    void         funcs(ManFuncs);
    void         setpanel(Lister* pp) {panel2=pp;};
    void         setviewer(void (*vw)(char*),void (*vow)(char*)) {viewfunc=vw;viewonlyfunc=vow;};
    void         seteditor(void (*vw)(char*)) {editorfunc=vw;};
    void         setsubmenu(Menu* sbm) {subm=sbm;};
    void         activate();
    ~Lister() {XFreeGC(disp,gcw);XFreeGC(disp,wgcw);XFreeGC(disp,xpmgc);delall_l(&dl);delete maskchr;};
    virtual void init(Window);
    virtual void click();
    virtual void expose();
    virtual void show();
    virtual void scrollup(Gui*);
    virtual void scrolldown(Gui*);
    virtual void calc_statusbar_offsets();
    
    void         view(int);
    void         edit();
    void         copy();
    void         move();
    void         make_link();
    void         rename();
    void         pack();
    void         copy(VFS*);
    void         move(VFS*);
    void         del();
    void         remount();
    void         mkdir();
    void         attrib();
    void         raise_terminal_if_need();
    void         lpr();
  
    int          moved(char*);
    void         fedit();
    void         ext_exec(char*);
    void         menu_exec(char*);
    void         wide_exec();
    void         ps_info();
    void         try_to_add_to_bookmark(int cy);
    void         fast_find(XEvent*);
    void         create_ftp_link();
    void         bg_view(char *name);
    void         bg_edit(char *name);
    virtual void header_blink();
    virtual void header_blink_other();
    void         insert_right_dir();
    void         insert_left_dir();
    void         switch_to_me();
    FList*       find_entry_by_coord(int,int);
    int          start_dragging();  //return 1 on success
    virtual void redraw_statusbar();
    
    virtual int  am_i_visible();
    virtual void showdir();
    virtual void fserror(char*,FList*);
    virtual void total_expose();
    virtual void total_reread();
    virtual void flush_screen();
    virtual int  is_panel_vfs(VFS*);
    virtual void set_recycle_state(int);     // Visualization of recycle operation  
    virtual void create_infowin(char *head); // Create progress bar window
    virtual void del_infowin();              // Delete progress window
    virtual void to_infowin(char *str);      // Message to show in window
    virtual void infowin_set_coun(int);      // Set progress length
    virtual void infowin_update_coun(int);   // Update progress bar
    virtual void show_item(FList*);          // Show one item from list
    virtual void showfinfo(FList*, int);     // Show file info to status bar
    virtual void expose_panel();
    virtual BaseCaptain* other();            // Return other(second) panel pointer
    virtual VFS* other_vfs();                // Return other(second) panel current vfs
    virtual int  dnd_drop(DndData*, int x, int y, DndAction action);
    virtual void do_simple_dialog(char*, ManFuncs, int show_text=1);
    void         bookmark_dropper(int cy, DndAction action);

    //Meta calls for common operations
    int          metacall_copy(MetaCallData);
    int          metacall_move(MetaCallData);
    int          metacall_rename(MetaCallData);
    int          metacall_delete(MetaCallData);
    int          metacall_execute(MetaCallData);
    int          metacall_view(MetaCallData);
    int          metacall_edit(MetaCallData);
    int          metacall_change_attributes(MetaCallData);
    int          metacall_change_owner(MetaCallData);
    int          metacall_man_page(MetaCallData);
    int          metacall_make_dir(MetaCallData);
    
    int          process_special_action_in_term(XEvent* ev);
    char*        get_list_format(int idx) { return display_format[idx].format;};
    void         set_list_format(int idx, char *fmt)
	{
	    display_format[idx].set_format(fmt);
	};

    PanelDisplayMode get_display_mode() { return display_mode;};
    void             set_display_mode(PanelDisplayMode mod) { display_mode=mod;};
    int              get_display_max_file_len(PanelDisplayMode mod)
	{
	    return display_format[mod].max_filename_len;
	};
    void             set_display_max_file_len(PanelDisplayMode mod, int len)
	{
	    display_format[mod].set_max_filename_len(len);
	};
    void         switch_branch_view();
    void         swap_panels();
    void         set_opposite_to_current();
    void         invert_mask_selection();
    void         add_selection_by_mask();
    void         del_selection_by_mask();
    void         add_selection_by_ext();
    void         del_selection_by_ext();
    void         add_to_bookmark();
    void         reread_all() { total_reread();};
    void         switch_hidden_files_showing();

    void         switch_to_brief();
    void         switch_to_full();
    void         switch_to_custom();
    void         switch_to_one_column();
    void         switch_to_two_columns();
    void         switch_to_three_columns();
    void         switch_to_four_columns();
    void         sort_by_name();
    void         sort_by_ext();
    void         sort_by_size();
    void         sort_by_time();
    void         sort_by_unsort();
    void         switch_reverse();

    void         show_cd_window();
};

extern Lister *panel;


extern void   save_cfg(Lister *,Lister *);
extern void   load_cfg(Lister *,Lister *);
extern void   qview();
extern void   qv_update(FList*);
extern void   init_qview();
extern void   qv_signal();

extern void   toggle_inodewin();
extern void   show_inodeinfo(FList *o);

extern  int   hide_hidden;

extern int    symlink_as_is;  //Copy symlink as is (symlink as symlink, not a copy of file it points to) 
extern char*  panlist[];
extern char*  panlist2[];
extern int    psmax;
extern FList  otmp;
extern int    dont_update;
extern int    just_at_startup;

extern void   simple_flist_add(FList *base, FList* it);

#include "commonfuncs.h"

//////////////////////////////////////////////////End of file/////////////////////////////////////////////
#endif

/* ------------ End of file -------------- */

