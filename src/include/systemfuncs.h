#ifndef SYSTEM_FUNCS_H
#define SYSTEM_FUNCS_H
#include "globals.h"
#include "flist.h"

struct USR
{
 char name[30];
 int gid;
 int uid;
 USR *next;
 USR() {next=NULL;};
};

extern USR users;
extern USR groups;
extern char   shdir[];
extern char* mnt[];
extern int mntmax;
extern int    mainuid, maingid;

extern void     load_mnt();
extern void     del_mnt();
extern void     init_users();
extern void     deinit_users();

extern char*    finduser(int);
extern int      finduserid_by_name(char *n);
extern char*    findgroup(int);
extern int      findgroupid_by_name(char *n);
extern char*    findmntent(char*);

extern void     senddir(char *d);
extern int      is_exec(FList * o);
extern void     printperm(char *str, int mode);
extern int      dig2ascii2(char *chr, long_size_t n, int radix=10);
extern void     dig2ascii(char *chr, long_size_t n, int radix=10);
extern void     dig2ascii_r(char *chr, long_size_t n, int total, int radix=10, char fill=' ');
extern char     *findsubstr(char *str, char *sub);
extern int      is_exist(char *fname);
extern char     *strscan2(char *src, char *dest);
extern void     xnc_seekdir(DIR *d, int offset);

#endif
/* ------------ End of file -------------- */

