#ifndef FTPCFG_H
#define FTPCFG_H

#include "globals.h"

struct FTPCFG_S
{
  char ftpbookmark[80];
  char ftphost[80];
  char ftppath[1024];
  char ftplogin[40];
  char ftppasswd[40];
  int  ftpport;
  int  bool_prox;
  char proxhost[80];
  char proxlogin[80];
  char proxpasswd[80];
  FTPCFG_S() 
  {
    strcpy(ftpbookmark, "X Northern Captain Ftp-Site");
    strcpy(ftphost, "xnc.dubna.su");
    strcpy(ftppath, "*");
    strcpy(ftplogin,"anonymous");
    strcpy(ftppasswd,"xnc_user@durakov.unas.net");
    bool_prox=0;
    ftpport=2021;
    proxhost[0]=0;
    proxlogin[0]=0;
    proxpasswd[0]=0;
  };
  int init(char* ftpstring);
};

extern FTPCFG_S ftprec;

extern int  ftpcfg_addhost(FTPCFG_S *frec);
extern int  ftpcfg_save_all_hosts();
extern FTPCFG_S* ftpcfg_findhost(FTPCFG_S *frec, int flags);
extern void compile_FTP_supports(char *supname);
extern void ftpcfg_deinit_hosts();
extern void ftpcfg_reinit_hosts();


#define FTPCFG_MAX_HOSTS  200

/* Flags for search criteries */
#define FCFG_FHOST       0x1
#define FCFG_FPATH       0x2
#define FCFG_FLOGIN      0x4
#define FCFG_FPASSWD     0x8
#define FCFG_FPHOST      0x10
#define FCFG_FPLOGIN     0x20
#define FCFG_FPPASSWD    0x40
#define FCFG_FBOOK       0x80

extern  char* hosts_list[];
extern  int   max_hosts;


#endif
/* ------------ End of file -------------- */

