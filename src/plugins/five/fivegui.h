#ifndef FIVEGUI_H
#define FIVEGUI_H
#include "xh.h"
#include "gui.h"
#include "pager.h"
#include "infowin.h"
#include "panel.h"
#include "ftpvisuals.h"
#include "bookmark.h"
#include "exwin.h"

#define SCOL2         skincol[4]        //Dark color for menu text
#define SCOL4         skincol[2]        //Storng Dark shadow
#define SCOL5         skincol[5]        //Dark shadow
#define SCOL8         skincol[8]        //MenuBar background
#define SCOL7         skincol[7]
#define SCOL13        skincol[13]       //Dark yellow fro selected text on items
#define SCOL27        skincol[27]       //White color
#define SCOL21        skincol[24]       //Light yellow for menu and win bg.
#define SCOL25        skincol[22]       //Light bg for panels
#define SCOL16        skincol[16]       //Color for disabled text
#define SCOL17        skincol[17]       //Light shadow
#define SCOL26        skincol[26]       //Light color for 3d look
#define SCOL22        skincol[25]       //Light blue
#define SCOL24        skincol[21]


class FiveText:public Text
{
 public:
  FiveText(int ix,int iy,char *iname,int icol):Text(ix,iy,iname,icol) {};
  virtual void show();
  virtual void expose();
};

class FiveKEY:public KEY
{
 protected:
  virtual void set_focus();
  virtual void unset_focus();
 public:
  FiveKEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)()):
    KEY(ix,iy,il,ih,iname,icol,ifunc) {};
  virtual void press();
  virtual void expose();
};

class FiveWin:public Win
{
 public:
  FiveWin(int ix,int iy,int il,int ih,char *iname,int icol,int hfl=0):Win(ix,iy,il,ih,iname,icol,hfl) {};
  virtual void init(Window w);
  virtual void expose();
};

class FiveInput:public Input
{
 protected:
  virtual void hide_cursor();
  virtual void unset_focus();
 public:
  FiveInput(int ix,int iy,int il,int icol,void (*ifunc)()=NULL):Input(ix,iy,il,icol,ifunc) {};
  virtual void showbuf();
  virtual void init(Window);
  virtual void expose();
};

class FiveSwitch:public Switch
{
 public:
  FiveSwitch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)()=NULL):Switch(ix,iy,il,iname,icol,ifunc) {};
  virtual void init(Window);
  virtual void press();
  virtual void expose();
  virtual void select();
};

class FiveSw_panel:public Sw_panel
{
 public:
  FiveSw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn=1):Sw_panel(ix,iy,il,ihead,mn,imax,icolumn){};
  virtual void expose();
};

class FiveMenuBar:public MenuBar
{
 public:
  FiveMenuBar(int ix,int iy,MenuItem *ii,int maxi):MenuBar(ix,iy,ii,maxi) {};
  virtual void expose();
};

class FiveMenu:public Menu
{
 public:
  FiveMenu(MenuItem *ii,int maxi):Menu(ii,maxi) {};
  virtual void expose();
  virtual void showitem(int);
  virtual void select(int);
  virtual void show();
};

class FivePanel:public Panel
{
 public:
  FivePanel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*)):
    Panel(ix,iy,il,ih,iname,imax,icol,ifunc) {};
  virtual void init(Window);
  virtual void expose();
  virtual void click();
  virtual void shownames();
  virtual void showcurs(int=1);
};

class FivePager:public Pager
{
 protected:
  Sprite  *spr_con1,*spr_con2,*spr_con3,*spr_con4;
  RecArea* ra_tbl;
 public:
  FivePager(int ix,int iy,int il,int ih,int imax):Pager(ix,iy,il,ih,imax) 
    {
      spr_con1=spr_con2=spr_con3=spr_con4=0;
      ra_tbl=0;
    };
  virtual void init(Window);
  virtual void expose();
  virtual void draw_areas();
};

class FiveInfoWin:public InfoWin
{
 public:
  FiveInfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey=0):
    InfoWin(ix,iy,il,ih,iname,icol,ikey) {};
  virtual void expose_clip_area();
  virtual void expose_counter();
  virtual void init(Window);
  virtual void expose();
};

class FiveLister:public Lister
{
 protected:
    Sprite *spr_arrow1,*spr_arrow2;
    Sprite *spr_statcon1,*spr_statcon2;
    Sprite *spr_pancon1,*spr_pancon2,*spr_pancon3;
    Sprite *spr_pancon4,*spr_pancon5,*spr_pancon6;
    virtual void  vlook_recalc_all_data();
    virtual void  vlook_draw_column_separators();
 public:
  FiveLister(int ix,int iy,int il,int ih,int icolumns=2):
    Lister(ix,iy,il,ih,icolumns) 
    {
      spr_arrow1=spr_arrow2=0;
      spr_statcon1=spr_statcon2=0;
      spr_pancon1=spr_pancon2=spr_pancon3=0;
      spr_pancon4=spr_pancon5=spr_pancon6=0;
    };
  virtual void init(Window);
  virtual void expose();
  virtual void calc_statusbar_offsets();
  virtual void showfinfo(FList*,int);
  virtual void showdirname();
  virtual void redraw_statusbar();
  virtual void show_string_info(char*);    // Show given string in status bar
  virtual void header_blink();
  virtual void header_blink_other();
};

class FiveEXWin:public EXWin
{
 public:
  FiveEXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl=0):
    EXWin(ix,iy,il,ih,iname,ifunc,hfl) {};
  virtual void init(Window);
};


class FiveSeparator:public Separator
{
 public:
  FiveSeparator(int ix,int iy,int il,Gui *ww,int ivert=NO):Separator(ix,iy,il,ww,ivert) {};
  virtual void expose();
};

class FiveFtpVisual:public FtpVisual
{
 public:
  FiveFtpVisual(int ix,int iy,int il,int ih):FtpVisual(ix,iy,il,ih) {};
  virtual void init(Window);
  virtual void reconfigure(int ix,int iy,int pl,int ph);
  virtual void show_tumb(int i, int ix);
};

class FiveCmdline:public Cmdline
{
 public:
  FiveCmdline(int ix,int iy,int il,int icol):Cmdline(ix,iy,il,icol) {};
  virtual void init(Window);
  virtual void expose();
};

class FiveBookMark:public BookMark
{
 protected:
  RecArea *ra_cuts,*ra_books;
  Sprite  *back_skin,*select_skin,*normal_skin;
 public:
  FiveBookMark(int il,int ih):BookMark(il,ih) {ra_cuts=ra_books=0;back_skin=0;};
  virtual void init(Window);
  virtual void create_listers(Lister ** l1, Lister ** l2, int ix, int iy, int ny);
  virtual void animate();
  virtual void expose();
  virtual void click();
  virtual void draw_page(int);
  virtual void draw_selected_page(int);
  virtual void blink_book(int n,int blinks);
  virtual void animate_moving(int i);
  virtual void animate_swapping(int);
  virtual void add_toempty_book_by_coord(int,char*,int);
  virtual VFS* get_vfs_by_coord(int iy);
  virtual void show_dir_banner(int );
  int get_pagenum(int x,int y);
  int get_page_y(int);
};

#endif
/* ------------ End of file -------------- */

