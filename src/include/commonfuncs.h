#ifndef COMMONFUNCS_H
#define COMMONFUNCS_H

extern char   last_info_message[];
extern char   last_error_message[];
extern char   vfserr[];                //String with error message from VFS
extern char   vfshead[];                //String with vfs header for vfserr


extern void  vfs_error(char *h, char *s);
extern void  errno2mes();
extern void  set_error(char *err);
extern int   is_cache_out_time(char *, int);
extern int   path_cmp(char *dir1, char *dir2);
extern int   path_to_fullpath(char* dest, char *srcdir);
extern void  add_path_content(char *curdir, const char *cont, int vms_host=0);
extern void  upper_path(char *curdir, int vms_host=0);
extern const char* get_first_content(const char *curdir, char *cont);
extern void  get_last_content(char *curdir, char *cont);
extern void  get_last_and_rest_content(char* curdir, char* last, char* rest);
extern void  str_swap(char *str1, char *str2);
extern void  init_xnc_alarm();
extern int   set_xnc_alarm(void (*func)(int));
extern void  remove_xnc_alarm(void (*func)(int));
extern void  convert_one_filename(char* from, char *to);
extern void  break_to_real_dirfile(char* from, char* realdir, char* realfile, char* rest);
extern char  *getseq(char *str, char *w, int nocase = 1);
extern char  *getrest(char *str,char *w, int nocase = 1);
extern int    is_filtered(char *s, char *ff);
extern void  str2version(char *str,int *ver);


#endif

/* ------------ End of file -------------- */

