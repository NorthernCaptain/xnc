#ifndef VFS_H  // -*- c++ -*-
#define VFS_H

#include "globals.h"
#include "flist.h"
#include "au_sup.h"
#include "ftpcfg.h"
#include "ftp_communication.h"
#include "dircacheman.h"

//Virtual file system types.
#define VFS_TYPE -1
#define DFS_TYPE 0
#define AFS_TYPE 1
#define FTP_TYPE 2

#define VFS_MAX_MEM        500
#define VFS_MEM_BLOCK      50000

#define REALREAD_BIT  1        /* This bit set if we want real reread directory, not a cached image*/
#define VFSDIED_BIT   2        /* This bit set when VFS is completely die (child die or fatal error) */
#define VFSDIFF_BIT   4        /* This bit set when destination and source files have diff names */
#define MAGIC_WORK    (1<<5)

#define PROMPT_EXIT        (1)           /*Prompt on exit from XNC*/
#define PROMPT_COPY        (1<<1)        /*Prompt for copy operation*/
#define PROMPT_MOVE        (1<<2)        /*Prompt for move operation*/
#define PROMPT_DEL         (1<<3)        /*Prompt for delete operation*/
#define PROMPT_OVR         (1<<4)        /*Prompt for overwrite existing files*/
#define PROMPT_PACK        (1<<5)        /*Prompt for packing files*/
#define PROMPT_LINK        (1<<6)        /*Prompt for linking files*/

//File operations than can be called via Lister::funcs(ManFuncs)
enum ManFuncs { 
    FDELETE    =  0,
    FCOPY      =  1,
    FMOVE      =  2,
    FMKDIR     =  3,
    FEDIT      =  4,
    FSELBYMASK =  5,
    FDESELMASK =  6,
    FSETATTR   =  7,
    FLPR       =  8,
    FWIDEEXEC  = 10,
    FMANPAGE   = 11,
    FFTPCON    = 12,
    FFTPCONPWD = 13,
    FCHOWN     = 20,
    FMOUNT     = 22,
    FPACK      = 23,
    FCDDIR     = 24,
    FSYMLINK   = 25
};

struct VFS_Info
{
    int      vfstype;
    char     full_path[L_MAXPATH];
    char     filename[L_MAXPATH];
};

class VFS   //Base Virtual File System class
{
public:
    FList fl;             // List of files
    FList dl;             // List of sub-dirs
    char *lar[VFS_MAX_MEM],*larptr;   //Array of Flist structs (i use my own allocator).
    int larmax;
    char *curdir;         // Current directory
    int fstype;           // Type of the virtual file system
    int need_change_vfs;  // Set to 1 when we need to change to another VFS
    int need_reread;      // Set to 1 when we need to rescan directory
    int opt;              // Optimization flag (uses on AFS).
    char *vfs_prefix;     // VFS prefix (DFS/AFS/FTP...)
    int need_dir_prefix;  // Set to 1 if the prefix is needed
    VFS *next;            // We need this for VFS stack queue
    int bgbit;            // Background bit >0 if operation on vfs is in background.
    int options;          // VFS options - bitwise ORed
    char dest[L_MAXPATH];

    VFS(int fst = VFS_TYPE, char *pref = "VFS", int need_prefix = 1) 
    {
	fstype = fst;
	vfs_prefix = pref;
	need_dir_prefix = need_prefix;
	for(int i=0;i<VFS_MAX_MEM;i++)
	    lar[i]=NULL;
	larmax = 0;
	curdir = NULL;
	next = NULL;
	opt = 0;
	need_change_vfs = 0;
	need_reread = 1;
	bgbit=0;
	options=0;
	dest[0]=0;
    };
    virtual int   direc(char*) {return 0;};              //build directory listings into fl and dl
    virtual int   branch_direc(char* d, DirCacheManager*) {return direc(d);};//build directory listings into fl and dl with branches
    virtual void  set_dir_pointer(char *d,int dlen)      //set directory pointer to external buffer
    {curdir=d;};
    virtual int   chdir(char *) {return -1;};            //Change directory
    virtual int   ch_curdir() {return -1;};              //Change to 'curdir' directory
    virtual int   ch_to_rootdir() {return -1;};          //Change to vfsroot dir if error occured
    virtual int   readlink(FList *o,char *c2,int len)    //Return symlink pointer
    {
	if(o==NULL || o->link==NULL)
	    return -1;
	strncpy(c2,o->link->name,len);
	return strlen(c2);
    };
    virtual int   chmod(char* name,mode_t mode)          //Change attributes
    {return -1;}; 
    virtual int   chown(char *name,int uid, int gid) 
    {return -1;};                     //Change owner 
    virtual char* getcwd(char *d, int len)               //Get current work dir 
    {return NULL;}; 
    virtual int   mkdir(char *,mode_t) {return -1;};     //Make dir
    virtual int   copy(FList* cur,VFS*) {return 255;};   //Copy file or dir to vfs_dest_dir (copy_to -> this)
    virtual int   move(FList* cur,VFS*) {return 255;};   //Move file or dir to vfs_dest_dir (move_to -> this)
    virtual int   remove(FList* cur) {return 255;};      //Delete file or dir from vfs
    virtual int   symlink(FList* cur,VFS*) { return 255;}; //Make symbolic link for file to given vfs
    virtual int   is_copy_supported() {return 0;};       // Does VFS support COPY operation?
    virtual int   is_move_supported() {return 0;};       // Does VFS support MOVE operation?
    virtual int   is_del_supported() {return 0;};        // Does VFS support DELETE operation?
    virtual int   is_exec_supported() {return 0;};       // Does VFS support EXECUTE operation?
    virtual int   is_chmod_supported() {return 0;};      // Does VFS support CHMOD operation?
    virtual int   is_chown_supported() {return 0;};      // Does VFS support CHOWN operation?
    virtual int   is_symlink_supported() {return 0;};    // Does VFS support SYMLINK operation?

    virtual int   is_file_exist(char* dir,char* file)    // 0 - means doesn't exist
    {return 0;}; 
    virtual char* get_file_for_execute(FList* ptr)    //Convert vfs filename to DFS file for execute.
    {return ptr->name;}; 
    virtual char* get_execute_dir(FList*) {return curdir;};    //Get dir to execute converted file.
    virtual char* get_dir_header() {return curdir;};     //Get name for panel header
    virtual void  delete_vfs_list()                      //We need to do it when changing VFSes
    {
	for(int i=0;i<VFS_MAX_MEM;i++)
	    if(lar[i]!=NULL)
	    {
		delete lar[i];
		lar[i]=NULL;
	    }
	larmax=0;
	dl.next=fl.next=NULL;
    };
    virtual void  close_fs() {};                         //Make some clean-ups before deleting fs.
    virtual char* get_info_for_bookmark()                //Construct info string for bookmark
    {return curdir;};
    virtual int   qvrecscan(char *dir) {return 0;};      //Need for QuickScanDir - scan subdirs and count files
    virtual int   qvscan() {return 0;};
    virtual int   add_to_vfs(FList *cur) {return 255;};  //Commonly use in AFS, and may be in FTP, to add to archives
    virtual int   can_we_optimize() {return 0;};         //This func use in AFS only for optimize archive operations.
    virtual int   can_we_optimize_del() {return 0;};     //Can we do pocket deletion?
    virtual void  use_optimization() {opt=1;};
    virtual int   is_pocket_addition_allowed()           //For AFS list additions
    {return 0;}; 
    virtual int   add_to_vfs_by_list(FList* dd)          //Only for 'add' to AFS or FTP
    {return 255;}; 
    virtual int   is_full_extract_allowed() {return 0;}; //Only for AFS
    virtual int   check_for_full_extract(int){return 0;};//Again ;)
    virtual int   full_extract_to_vfs(VFS* vfs)          //Make full extraction to vfs
    {return 255;};
    virtual int   make_fullpath() {return -1;};          //Make full pathname from local one
    virtual int   init_support(char *dir,char *subdir=0) 
    {return 0;};                      //init vfs support files if needed
    virtual void* new_alloc(int size)                    //allocate memory for any
    {
	int si=VFS_MEM_BLOCK;
	void *ptr;
	if(lar[larmax]==NULL)
	{
	    lar[larmax]=new char[si];
	    larptr=lar[larmax];
	}
	if(larptr-lar[larmax]+size>VFS_MEM_BLOCK)
	{
	    lar[++larmax]=new char[VFS_MEM_BLOCK];
	    larptr=lar[larmax];
	}
	ptr=larptr;
	larptr+=size;
	return ptr;
    };
    virtual void  init_internals() {};                   //Do nothing on this VFS
    virtual char* magic_file_process(FList*)             //process one file through Magic database
    {return NULL;};  
    virtual int   magic_scan() {return 0;};              //Scan directory with Magic.
    virtual int   pass_cd_to_terminal() {return 1;};     //Pass 'cd' command to terminal or not?
    virtual char* get_dir_for_dnd() { return 0;};        //Return zero dnddata becouse we are abstract
};

class FTP:public VFS //Ftp File System Class
{
public:
    char *host;
    int work;
    Ftp_Com com;
    int wp[2],rp[2],logp[2];
    int ftpid;
    netbuf *netb;
    int autoraise;
    char tmps[L_MAXPATH];
    int dup_entry;
    int ls_timeout;
    int transfer_cache_timeout; 
    FTPCFG_S  ftpcfg;  

    FTP() : VFS(FTP_TYPE, "FTP") 
    { int i; char *ptr=(char*) &com;
    work=0; bgbit=0;autoraise=0;dup_entry=-1;options=0;
    ls_timeout=FTP_LS_CACHE_TIME;
    transfer_cache_timeout=FTP_TRANSFER_CACHE_TIME;
    for(i=0;i<S_Ftp_Com;i++)
	ptr[i]=0;
    };
    void          op_complete() {work=0;};
    virtual int   init_support(char* dir, char *subdir=0);
    void          do_work();
    virtual void  close_fs();
    virtual int   direc(char*);

    virtual void  set_dir_pointer(char *d,int dlen) 
    {
	curdir = com.remotepath;
	host = com.hostname;
	curdir[0]=0;
    };

    virtual int   chdir(char *d);      //Change directory
    virtual int   ch_curdir() {return 0;};      //Change to 'curdir' directory
    virtual int   ch_to_rootdir() {return 0;}; //Change to vfsroot dir if error occured
    virtual char* get_dir_header();
    virtual char* get_file_for_execute(FList* vfsname); //Convert vfs filename to DFS file for execute.
    virtual int   is_exec_supported() {return 1;}; // Does VFS support EXECUTE operation?
    virtual char* get_execute_dir(FList*); //Get dir to execute converted file.
    virtual int   is_copy_supported() {return 1;}; // Does VFS support COPY operation?
    virtual int   copy(FList* cur,VFS*); //Copy file or dir to vfs_dest_dir
    virtual int   can_we_optimize() {return 1;}; //This func use in AFS/FTP for optimize copy/move/delete operations.
    virtual int   is_del_supported() {return 1;}; // Does VFS support DELETE operation?
    virtual int   can_we_optimize_del() {return 1;}; //Can we do pocket deletion? Yes, we can!
    virtual int   remove(FList* cur); //Delete file or dir from vfs
    virtual int   mkdir(char *,mode_t); //Make remote dir
    virtual int   add_to_vfs(FList *cur);
    virtual int   add_to_vfs_by_list(FList* dd);
    virtual char* get_info_for_bookmark();
    virtual int   pass_cd_to_terminal() {return 0;}; //Pass 'cd' command to terminal or not?
    virtual char* get_dir_for_dnd();        //Return ftp://user@host/path (allocate memory)
    int           send_to_ftpchild(Ftp_Com*);
    int           ftp_wait(char *, int);
    int           ftp_fork();
    void          child_ftp_command();
    void          make_empty_list();
    void          bg_select();
    void          bg_select_if_set(fd_set*,int ret);
    int           bg_fd_set(int, fd_set*);
    void          bg_commands();
    void          bg_switch();
    void          create_ftp_cachedir(char *dir);
    int           child_get_single_file(int mode, char *fname);
    int           child_put_single_file(int mode, char *fname);
    int           child_del_single_file(int mode, char *fname);
    int           piperead(int, void*,int);
    int           selected_list_to_file();
    int           file_to_selected_list(FList*);

    virtual void  init_internals()
    {
	curdir = com.remotepath;
	host = com.hostname;
    };

    void          ftp_to_bg(FTP*);
    int           ftp_log_xfer(int);
    int           ftp_log_msg(Ftp_Xfer&);   
    int           host_type() {return com.options[1];};
    int           is_vms() {return host_type()==FTP_HOST_VMS;};
};

#define AFS_CACHE_ENTRIES    500

class AFS:public VFS //Archieve File System Class
{
public:
    SUP *sp;
    char arcname[FLIST_NAME];
    char fullarcname[L_MAXPATH];
    char listpath[L_MAXPATH];
    char to_dir[FLIST_NAME];
    char prefix[10];
    char tmppath[L_MAXPATH];
    int  sublevel,dont_construct_path,update_on_exit;
    char *cache[AFS_CACHE_ENTRIES];
    char *cachestart,*cacheptr;

    AFS() : VFS(AFS_TYPE, "ARC")         
    { 
        int i;
        dont_construct_path = 0; 
        sublevel=0; curdir=NULL;
        for(i=0;i<40;i++)
	    cache[i]=NULL;
    };
    virtual int direc(char*);
  
    virtual void set_dir_pointer(char *d,int dlen) 
    {
	curdir=new char[L_MAXPATH];
	curdir[0]=0;
    };
  
    virtual int   chdir(char *d);      //Change directory
    virtual int   ch_curdir() {return 0;};      //Change to 'curdir' directory
    virtual int   ch_to_rootdir() {return 0;}; //Change to vfsroot dir if error occured
    virtual char* get_dir_header();
    virtual int   is_exec_supported();
    virtual int   is_copy_supported();
    virtual int   is_move_supported() {return 0;};
    virtual int   is_del_supported();
    virtual int   copy(FList* cur,VFS*);
    virtual int   remove(FList* cur);
    virtual int   init_support(char *dir, char *subdir=0);
    virtual char* get_dir_for_dnd();        //Return ftp://user@host/path (allocate memory)
    void          convert_attributes(int& mode,int, char *at, char *name);
    int           reread();
    int           get_sublevel(char *file,int mode);
    char*         get_lastname(char *file);
    FList*        find_content(char* n); //Find content in dir list by name
    char*         gen_fullname(char *n);
    int           is_in_this_dir(char *n);
    int           try_to_cache(FILE* fp,char *name,int mode,int t); //Cache dirs while list archieve
    void          init_dir_cache();
    void          deinit_dir_cache();
    virtual char* get_file_for_execute(FList* vfsname); //Convert vfs filename to DFS file for execute.
    virtual char* get_execute_dir(FList*);//Get dir to execute converted file.
    virtual void  delete_vfs_list();
    virtual char* get_info_for_bookmark() {return fullarcname;};
    virtual int   qvscan(); //Body in 'qview.cxx'
    virtual int   add_to_vfs(FList *cur);
    virtual int   can_we_optimize();
    int           optimized_copy_to_dfs(VFS*);
    virtual int   is_pocket_addition_allowed();
    virtual int   add_to_vfs_by_list(FList* dd);
    virtual int   is_full_extract_allowed();
    virtual int   check_for_full_extract(int);
    virtual int   full_extract_to_vfs(VFS* vfs);
    void*         cache_alloc(int);
    void*         find_in_cache(char*);
    virtual int   pass_cd_to_terminal() {return 0;}; //Pass 'cd' command to terminal or not?
    virtual int   create_archive_from_files(char* from_dir, int); //Create new archive from given dir
    static  int   construct_archive_name(char*, char*, int, const char*);
};

class DFS:public VFS   //Disk File System Class
{
    int movedir(FList* cur,VFS* vfs,mode_t mode, char *mstr); //Moving subdirs to vfs
    int copydir(FList* cur,VFS* vfs,mode_t mode, char *mstr); //Copying subdirs to vfs
    int read_one_dir_files(char*);

public:
    int dlen;

    DFS() : VFS(DFS_TYPE, "DFS", 0) {};
    virtual int direc(char*);
    virtual int branch_direc(char* d, DirCacheManager*);//build directory listings into fl and dl with branches

    virtual void set_dir_pointer(char *d,int idlen) 
    {
	if (d == NULL)
	    curdir = new char[idlen+5];
	else
	    curdir = d;
	dlen = idlen;
	getcwd(curdir, dlen-2);
    }; 

    virtual int chdir(char *c) {return ::chdir(c);};      //Change directory
    virtual int ch_curdir() {return ::chdir(curdir);};      //Change to 'curdir' directory

    virtual int ch_to_rootdir() 
    {
	strcpy(curdir,getenv("HOME"));
	return ::chdir(curdir);
    };

    virtual int   init_support(char *dir, char *subdir=0) 
    {     
	for(int i=0;i<VFS_MAX_MEM;i++)
	    lar[i]=NULL;
	larmax = 0;
	return 1;
    };

    virtual int   chmod(char* name,mode_t mode) {return ::chmod(name,mode);};
    virtual char* getcwd(char *d, int len) {return ::getcwd(d,len);};
    virtual int   mkdir(char *name,mode_t mode) {return ::mkdir(name,mode);};
    virtual int   chown(char *name,int uid, int gid) { return ::chown(name,uid,gid);}; 
    virtual int   copy(FList* cur,VFS*);
    virtual int   move(FList* cur,VFS*);
    virtual int   symlink(FList* cur,VFS*);
    virtual int   remove(FList* cur);
    virtual int   is_copy_supported() {return 1;};
    virtual int   is_move_supported() {return 1;};
    virtual int   is_del_supported() {return 1;};
    virtual int   is_exec_supported() {return 1;};
    virtual int   is_chmod_supported() {return 1;};       // Does VFS support CHMOD operation?
    virtual int   is_chown_supported() {return 1;};       // Does VFS support CHOWN operation?
    virtual int   is_symlink_supported() {return 1;};     // Does VFS support SYMLINK operation?
    virtual int   is_file_exist(char* dir,char* file);
    virtual char* get_execute_dir(FList*); //Just the same dir.
    virtual char* get_dir_header();
    virtual int   qvrecscan(char *dir); //Body of this func placed in 'qview.cxx'
    virtual int   qvscan(); //Body also in 'qview.cxx'
    virtual int   make_fullpath();
    virtual char* magic_file_process(FList*);
    virtual int   magic_scan();
    virtual char* get_dir_for_dnd();
};


struct Namer
{
    char name[L_MAXPATH];
    Namer* next;
    FList *o;
    int val1,val2;
    Namer() {next=NULL;o=NULL;};
    Namer(char *str,FList *ol)        {strncpy(name,str,L_MAXPATH);o=ol;next=NULL;};
};

extern FList* flist_find_by_name(FList *list, char *name, struct stat *st);
extern int breakdown_link(char *from, char *to1, char *to2);
extern int check_for_link(char *from);

extern DFS    dfstmp;
extern AFS    afstmp;
extern VFS    defvfs;

extern VFS   *define_vfs(char *type, char *d);
extern VFS   *define_vfs(char *d);

extern int    afstmp_defined;

#endif
/* ------------ End of file -------------- */

