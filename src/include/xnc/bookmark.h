#ifndef BOOKMARK_H
#define BOOKMARK_H
#include "panel.h"
#include "qhelp.h"
#include "configman.h"

extern unsigned int recycle_l,recycle_h;
extern Pixmap pixrecycle[];

class BookMark: public Gui
{
 public:
  Window rw;
  int             x,y,prflg,tx,ty,hflg,dflg;
  uint            l,h,tl;
  ulong           col;
  void            (*escfunc)();
  Lister          *lst1,*lst2;
  GC              gcm,gcrw;
  int             page_h,last_used,maxbooks,maxy,flagl,flagh,flag_anim,mapped;
  int             cur_flag,flag_coun,max_anims;
  char            *books[9];
  int             act_books[9];
  char            fs[9][4];
  Pixmap          flagpix[10],a1pix,a2pix;
  GC              rgc;
  int             ty2,pagenum;
  QuickHelp       qh;
  int             rwx,rwy,leftrx,rightrx,percent;
  int             listx,listy,listny;

  BookMark(int il,int ih);
  ~BookMark(); 

  virtual void    create_listers(Lister **l1,Lister **l2, int ix, int iy, int ny);
  virtual void    rebuild_listers();
  virtual void    reconfigure(int ix,int iy,int il,int ih);
  virtual void    draw_page(int);
  virtual void    draw_selected_page(int);
  virtual void    blink_book(int n,int blinks);
  virtual void    animate();
  virtual VFS*    get_vfs_by_coord(int iy);
  virtual void    animate_moving(int i);
  virtual void    animate_swapping(int);
  virtual void    add_toempty_book_by_coord(int,char*,int);
  virtual void    show_dir_banner(int );
  virtual void    hide_dir_banner();
  virtual void    init(Window ipar);
  virtual void    expose();
  virtual void    click();
  virtual void    show();
  virtual void    hide();
  virtual void    rw_expose();
  virtual int     dnd_drop(DndData* dd, int x, int y, DndAction action);
  virtual int     dnd_position(XEvent*, int globx, int globy);
  virtual int     start_dragging(int n);

  void            clear_and_expose();
  void            set_escapefunc(void (*v)()) {escfunc=v;};
  void            load_flags(int);
  void            del_book(int i);
  int             add_book(int, char*,int);
  void            add_toempty_book(char*,int);
  void            swap_books(int );
  void            switch_books(int );
  void            only_switch_to_exist_book(int i);
  void            save_books(ConfigManager*);
  void            set_recycle_image(int i);
  void            free_pix();
  int             load_pixmap(int);
  int             is_in_bookmark(char *s);
  void            set_panels(Lister *p1,Lister* p2) {lst1=p1;lst2=p2;};
};         

extern   int detect_fs(char *s);
extern   char  *fschr[];
extern   BookMark *bmark;

extern   Pixmap       recyclepix;
extern   Pixmap       recyclemask;
extern   unsigned int recycle_l, recycle_h;
extern   Pixmap       pixrecycle[];
extern   Pixmap       maskrecycle[];
extern   unsigned int l_recycle[], h_recycle[];
extern   int          cur_recycle;
extern   Cursor       rw_cur;

#endif
/* ------------ End of file -------------- */

