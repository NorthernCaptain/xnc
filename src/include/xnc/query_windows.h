#ifndef QUERY_WINDOWS_H
#define QUERY_WINDOWS_H
#include "vfs.h"

class  BookMark;    //real definition in bookmark.h
class  Lister;      //real definition in panel.h
struct FList;       //real definition in flist.h

extern int update_query_work,update_ok;
extern char wfile[];
extern char combuffer[];
extern int comnumber;
extern int attrmode;
extern BookMark *bmark;
extern char   dbuf[];

extern int    ow_file;                //overwrite current file
extern int    ow_all;                //overwrite all files without prompt
extern int    dont_ask;                //don't even ask for overwriting, just do it! Very good choice for me ;)
extern int    ow_query_work;
extern int    ow_cancel;
extern int    ow_no;

extern void   wait_for_query_done();
extern void   ask_for_update();
extern void   init_overwrite_query(char *head,char *file);
extern void   create_meswin(char *head,char *te,ManFuncs n);
extern void   attrib(FList *cur,ManFuncs inum);
extern void   attrib2();
extern void   initquery(char *head,Lister* p1,VFS *vfs,ManFuncs inum);
extern void   init_copymove_query(char *head,Lister* p1,VFS *vfs,ManFuncs inum, char* content=0);
extern void   init_chown_dialog(FList *cur);
extern void   initdialog(char *head,Lister *l1,VFS* vf,ManFuncs inum, char* content=0);
extern void   newtextfile(char *head,Lister *l1,ManFuncs inum,int showtext=1,
			  char *text_title=0);
extern void   simple_mes(char *head,char *mes);
extern void   show_vfs_error();
extern void   show_file_error(char *head,FList *o);
extern int    panexec(int i,char *m);
extern int    panexec2(int i,char *m);
extern int    pankill(int i,char *m);
extern int    panmnt(int i,char *m);
extern int    panmntmnt();
extern void   newpanel(char *head,int n,int (*prfunc)(int,char*),int wl=540);
extern void   newkillpanel(char *head,int n,int (*prfunc)(int,char*),int wl=540);
extern void   dfs_info(int wl=540);
extern void   free_info(int wl=540);
extern void   menupanel(int (*prfunc)(int,char*));
extern void   selectmask(Lister *l1,ManFuncs inum);
extern void   new_ftp_prompt(Lister *l1, ManFuncs inum);
extern void   show_config_win();
extern void   set_dbuf(VFS* vf, char* content=0);
extern int    blank(char* str);

#endif

/* ------------ End of file -------------- */

