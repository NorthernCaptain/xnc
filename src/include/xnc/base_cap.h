/* $Id: base_cap.h,v 1.9 2003/11/27 15:36:43 leo Exp $ */
#ifndef BASECAP_H // -*- c++ -*-
#define BASECAP_H

#include "vfs.h"
#include "query_windows.h"

//Panel masks - these definitions will with 'masks' variable
#define PANEL_CD_DONE        1
#define BRANCH_VIEW_MODE     2

extern int    disable_reread;        //If 1 reread disables in ***::expose;

//Base Captain - class storing all functions with vfses, MUST NOT use any graphic output,
//only calls to virtual methods for it
class BaseCaptain
{
 protected:
  FList    *last_ff;
  char     ff_buf[128];
  uint     items_per_column;        //Number of items in one column
  uint     items_per_page;          //Number of items in one page (items_per_column*columns)
  DirCacheManager  dir_cache_man;   //Directory manager for branch view mode

  int      is_branch_view_mode() { return masks & BRANCH_VIEW_MODE;};
  void     set_cursor_to_nearest(int pos);

 public:
  int      (*comp)(char*,int,int,char*,int,int);
  FList    *lar;
  int      larmax;
  VFS      *vfs;                    //Current working VFS
  VFS      *vfstack;                //Pointer to the top of VFS stack
  DFS      dfs;
  AFS      afs;
  FTP      ftp;
  char     curdir[L_MAXPATH];       //Current directory
  char     refindchr[L_MAXPATH];
  char     *maskchr;                //File Mask
  FList    fl;                      //File listing
  FList    dl;                      //Directory listing
  FList    *base,*cur;              //Pointer to base item of visible page and current (cursor) item
  FList    *asel;                   //List of selected items (construct only when panel make rescan dir)
  int      aselmax;                 //number of items in asel list
  int      scanfl;                  //Scan directory flag: 1->rescan directory
  time_t   lastscan;                //time_t of last directory scan 
  int      rev_order;               //sort lists in reverse order or not
  int      selfiles;                //number of selected siles and total selected size
  long_size_t selsize;              //total selected bytes 
  int      curn,lcurn;              //Current and last indexes in list of files
  int      col;                     //Number of columns
  int      masks;                   //ORed flags for panel operations

  int      lay;                     //Layout of panel

  BaseCaptain(int icolumns)
    {
      vfs=&dfs;
      last_ff=0;ff_buf[0]=0;
      dfs.set_dir_pointer(curdir,1024);
      afs.set_dir_pointer(0,0);
      ftp.set_dir_pointer(0,0);
      comp=mystrcmp;
      vfstack=0;
      asel=0;
      maskchr=new char[256];
      strcpy(maskchr,"*");
      larmax=0;
      lar=0;
      scanfl=1;
      rev_order=0;
      selfiles=selsize=0;
      lcurn=curn=0;
      cur=base=0;
      col=icolumns;
      items_per_column=1;
      lay=0;
      masks=0;
    }

  // Functions for working with data structures, don't make any call for visualization of data
  void setpath(char *ipath) { strcpy(curdir, ipath); };
  void upperpath();
  void addpathcontent(char*);
  int  findentry(char*);
  int  setcur(FList**,int);
  int  direc(char*);
  void reset_cur() { lcurn=curn=-1;cur=base=0;};
  void reverse_sort() {rev_order^=1;};
  void restore_select();
  void clean_selection();
  void save_select();
  void selected(FList*);
  void setattrs(int,int);
  void set_by_name(char*);
  void step_to_name(char *name);
  void set_cd_done()  { masks|=PANEL_CD_DONE; };

  void push_vfs(VFS*);
  VFS* pop_vfs();
  void delete_vfs_list();

  void sortbyext();
  void sortbysize();
  void sortbytime();
  void sortbyname();
  void sortbyunsort();

  VFS_Info* get_vfs_info_for_file(char* fbuf);

  // Functions that make some data transformations and call virtual methods for visualization 
  void reread();
  void switch_to_prev_vfs();
  void switch_to_vfs(int vfs_type,char *dir);
  VFS* push_n_pop(VFS*);
  void pop_and_show();
  void push_n_switch(int vfs_type, char *dir);
  void check_and_pop(VFS *vf);
  void change_dir(char*);
  void select_by_mask(int);
  void fdelete();
  int  fcopy();
  int  flink();
  void fmove();
  void fmkdir();
  int  pack_files(const char*);
  void select_file_mask(int sel);
  void try_reread_dir();
  void chowns();
  void do_quick_file();
  void go_home();
  void go_end();
  void go_upline();
  void go_downline();
  void page_down();
  void page_up();
  void left();
  void right();
  FList* find_file_entry(char *name);
  void select_file(FList*);
  void compare_panels();
  void setattr();
  void do_ftp_link();
  void do_ftp_link_passwd(char *passwd);
  void set_branch_view_mode()  { masks|=BRANCH_VIEW_MODE;};
  void reset_branch_view_mode() { masks&=~BRANCH_VIEW_MODE;};


  // Virtual functions for representing files info to user
  virtual void showdir();                  // Show contents of direcotry
  virtual void expose_panel();             // Show all panel
  virtual int  am_i_visible();             // Does this panel visible??
  virtual void fserror(char*,FList*);      // Show error window
  virtual void total_expose();             // Redraw all panel and other...
  virtual void total_reread();             // Reread panel directory and other...
  virtual void flush_screen();             // FLushes screen buffer
  virtual void set_recycle_state(int);     // Visualization of recycle operation
  virtual void create_infowin(char *head); // Create progress bar window
  virtual void del_infowin();              // Delete progress window
  virtual void to_infowin(char *str);      // Message to show in window
  virtual void infowin_set_coun(int);      // Set progress length
  virtual void infowin_update_coun(int);   // Update progress bar
  virtual int  is_panel_vfs(VFS*);         // 1 if VFS is registered on any other panel
  virtual BaseCaptain* other();            // Return other(second) panel pointer
  virtual VFS* other_vfs();                // Return other(second) panel current vfs
  virtual void showdirname();              // Show directory name in the header
  virtual void showitem(FList*,int);       // Show one file item
  virtual void showempty(int);             // Show one empty item
  virtual void show_item(FList*);          // Show one item from list
  virtual void showfinfo(FList*, int);     // Show file info to status bar
  virtual void show_ff_item(int,int);      // Show fast find item
  virtual void show_string_info(char*);    // Show given string in status bar
  virtual void showfilelist();             // Show all file list
  virtual void clear_ff();                 // Clear fast file info
  virtual void do_simple_dialog(char*, ManFuncs, int show_text=1);
};


extern int    dirdelay;            //Delay in seconds, for rereading directory
extern int    compare_by_size, compare_by_time;

extern int    create_xnc_tmp_dir(char* to);
extern int    delete_xnc_tmp_dir(char* dir);


#define BaseVirtual(type,name)     type BaseCaptain::name { \
                                   xncdprintf(("Call to pseudo-abstract method: %s BaseCaptain::%s",#type,#name)); \
                                   }
#define rBaseVirtual(type,name,ret)     type BaseCaptain::name { \
                                   xncdprintf(("Call to pseudo-abstract method: %s BaseCaptain::%s",#type,#name)); \
                                   return ret;}

#endif




/* ------------ End of file -------------- */

