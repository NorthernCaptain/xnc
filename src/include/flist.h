#ifndef FLIST_H
#define FLIST_H

#include "iconsets.h"

#define FLIST_NAME        160  //Maximum filename

#define FILE_TIMESTAMP 18
#define L_MAGIC        50
#define L_USER         16        //User [8] + '\0'
#define L_GROUP        16        //Group[8] + '\0'

extern int    sort_order;

//Consts for FList options
const   int   S_SELECT=04000000;  //Current file is selected
const   int   S_EXTEND=02000000;  //File has associated extension

enum FileType
    {
	file_is_unknown=0,
	file_is_dir=1,
	file_is_symlink=2,
	file_is_file=3,
	file_is_archive=4,
	file_is_image=5,
	file_is_execatable=6,
	file_is_association=7
    };

struct FList
{
    FList            *next,*prev;
    char             name[FLIST_NAME];
    int              namelen;
    unsigned int     mode;
    int              size,gid,uid;
    time_t           time;
    char             chr_time[FILE_TIMESTAMP];   // DD-MMM-YYYY/HH:MM
    int              options;
    FList*           link;
    struct stat      st;
    char             magic[L_MAGIC];
    char             user[L_USER];
    char             group[L_GROUP];
    FileType         file_type;
    unsigned int     file_color;
    FileTypeIconset  icon_type;
    char*            dir_name;
    
    FList() 
    {
        prev=next=link=NULL;
        mode=0;
        name[0]=0;
        size=0;
        options=0;
        user[0]=0;
        group[0]=0;
        magic[0]=0;
	namelen=0;
	file_type=file_is_unknown;
	file_color=0;
	icon_type=f_normal_icon;
	dir_name=0;
    };
    FList(char *iname,int imode=0,int is=0,int iuid=0,
	  int igid=0,time_t t=0, struct stat *ist=0);
    void init(char *iname,int imode=0,int is=0,int iuid=0,
	      int igid=0,time_t t=0, struct stat *ist=0);
    void add(FList*);
    void add_magic(char *);
    void set_user_group_names(char *fromuser, char *fromgroup);
    void out();
    void set_file_type();
    char* get_dir() { return dir_name;};
    void set_dir(char* d) { dir_name=d;};
};

#define S_FList        sizeof(FList)

extern void   simple_flist_add(FList * base, FList * it);

extern int    extcmp   (char *, int, int, char *, int, int);
extern int    sizecmp  (char *, int, int, char *, int, int);
extern int    timecmp  (char *, int, int, char *, int, int);
extern int    mystrcmp (char *, int, int, char *, int, int);
extern int    unsortcmp(char *, int, int, char *, int, int);

extern int    (*compare)(char*,int,int,char*,int,int);


#endif


/* ------------ End of file -------------- */

