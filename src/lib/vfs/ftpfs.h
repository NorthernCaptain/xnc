#ifndef FTPFS_H
#define FTPFS_H
#include "ftplib/ftp_communication.h"
#include "vfs.h"


extern void deinit_all_ftp();
extern void init_ftp_globals();
extern void ftp_backgrounds(fd_set*,int);
extern int  ftp_backgrounds_fdset(fd_set*, int);
extern void ftp_set_bgcode(FTP*,int);
extern int where_is_ftp_place(FTP *o);
extern int is_this_ftp(FTP* o);

/* Here we define FtpFunctions which can be send to background */
#define FF_NON        0        //NON BLOCKED OPERATION
#define FF_CON        1        //CONNECT
#define FF_CLO        2        //CLOSE CONNECT
#define FF_CD         3        //CHANGE DIR
#define FF_DIR        4        //DIRECTORY LISTING
#define FF_GET        5        //GET FILE
#define FF_VIE        6        //VIEW FILE
#define FF_SVI        7        //SIMPLE VIEW
#define FF_EDI       8       //EDIT FILE
#define FF_MGET   9       //MultiGet FILES
#define FF_DEL     10     //DELETE file
#define FF_MDEL  11    //MultiDEL files
#define FF_MKD    12    //Make directory
#define FF_DIE      13   //Unplanning child die :(
#define FF_PUT      14   //Put file to FTP
#define FF_MPUT   15  //MultiPut files

#define FGM        0        //FTP in ForeGround Mode
#define BGM        1        //FTP in BackGround Mode

#endif
/* ------------ End of file -------------- */

