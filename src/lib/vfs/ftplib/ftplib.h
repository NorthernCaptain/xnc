/***************************************************************************/
/*                                                                           */
/* ftplib.h - header file for callable ftp access routines                 */
/* Copyright (C) 1996, 1997 Thomas Pfau, pfau@cnj.digex.net                */
/*        73 Catherine Street, South Bound Brook, NJ, 08880                   */
/*                                                                           */
/* This library is free software; you can redistribute it and/or           */
/* modify it under the terms of the GNU Library General Public                   */
/* License as published by the Free Software Foundation; either                   */
/* version 2 of the License, or (at your option) any later version.           */
/*                                                                            */
/* This library is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           */
/* Library General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU Library General Public           */
/* License along with this progam; if not, write to the                           */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,                   */
/* Boston, MA 02111-1307, USA.                                                   */
/*                                                                           */
/***************************************************************************/

#if !defined(__FTPLIB_H)
#define __FTPLIB_H

#if !defined(_WIN32)
#define GLOBALDEF
#define GLOBALREF extern
#else
#if defined BUILDING_LIBRARY
#define GLOBALDEF __declspec(dllexport)
#define GLOBALREF __declspec(dllexport)
#else
#define GLOBALREF __declspec(dllimport)
#endif
#endif

/* FtpAccess() type codes */
#define FTPLIB_DIR 1
#define FTPLIB_DIR_VERBOSE 2
#define FTPLIB_FILE_READ 3
#define FTPLIB_FILE_WRITE 4

/* FtpAccess() mode codes */
#define FTPLIB_ASCII 'A'
#define FTPLIB_IMAGE 'I'
#define FTPLIB_TEXT FTPLIB_ASCII
#define FTPLIB_BINARY FTPLIB_IMAGE

/* connection modes */
#define FTPLIB_PASSIVE 1
#define FTPLIB_PORT 2

/* connection option names */
#define FTPLIB_CONNMODE 1
#define FTPLIB_CALLBACK 2
#define FTPLIB_IDLETIME 3
#define FTPLIB_CALLBACKARG 4
#define FTPLIB_CALLBACKBYTES 5

#define FTP_XFER_IND        32768


#define FTPLIB_HOST_UNIX  1
#define FTPLIB_HOST_VMS   2
#define FTPLIB_HOST_NT    3

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NetBuf netbuf;
typedef int (*FtpCallback)(netbuf *nControl, int xfered, void *arg);

/* Moved from ftplib.c by Leo */
struct NetBuf {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char *buf;
    int dir;
    netbuf *ctrl;
    int cmode;
    struct timeval idletime;
    FtpCallback idlecb;
    void *idlearg;
    int xfered;
    int cbbytes;
    int xfered1;
    char response[256];
    int host_type;
};
/****end moved****/


#define _FTPLIB_NO_COMPAT        /* We don't need old stuff, Leo.*/
/* v1 compatibility stuff */
#if !defined(_FTPLIB_NO_COMPAT)
netbuf *DefaultNetbuf;

#define ftplib_lastresp FtpLastResponse(DefaultNetbuf)
#define ftpInit FtpInit
#define ftpOpen(x) FtpConnect(x, &DefaultNetbuf)
#define ftpLogin(x,y) FtpLogin(x, y, DefaultNetbuf)
#define ftpSite(x) FtpSite(x, DefaultNetbuf)
#define ftpMkdir(x) FtpMkdir(x, DefaultNetbuf)
#define ftpChdir(x) FtpChdir(x, DefaultNetbuf)
#define ftpRmdir(x) FtpRmdir(x, DefaultNetbuf)
#define ftpNlst(x, y) FtpNlst(x, y, DefaultNetbuf)
#define ftpDir(x, y) FtpDir(x, y, DefaultNetbuf)
#define ftpGet(x, y, z) FtpGet(x, y, z, DefaultNetbuf)
#define ftpPut(x, y, z) FtpPut(x, y, z, DefaultNetbuf)
#define ftpRename(x, y) FtpRename(x, y, DefaultNetbuf)
#define ftpDelete(x) FtpDelete(x, DefaultNetbuf)
#define ftpQuit() FtpQuit(DefaultNetbuf)
#endif /* (_FTPLIB_NO_COMPAT) */
/* end v1 compatibility stuff */

GLOBALREF int ftplib_debug;
GLOBALREF void FtpInit(void);
GLOBALREF char *FtpLastResponse(netbuf *nControl);
GLOBALREF int FtpConnect(const char *host, netbuf **nControl);
GLOBALREF int FtpOptions(int opt, long val, netbuf *nControl);
GLOBALREF int FtpLogin(const char *user, const char *pass, netbuf *nControl);
GLOBALREF int FtpAccess(const char *path, int typ, int mode, netbuf *nControl,
    netbuf **nData);
GLOBALREF int FtpRead(void *buf, int max, netbuf *nData);
GLOBALREF int FtpWrite(void *buf, int len, netbuf *nData);
GLOBALREF int FtpClose(netbuf *nData);
GLOBALREF int FtpSite(const char *cmd, netbuf *nControl);
GLOBALREF int FtpSysType(char *buf, int max, netbuf *nControl);
GLOBALREF int FtpMkdir(const char *path, netbuf *nControl);
GLOBALREF int FtpChdir(const char *path, netbuf *nControl);
GLOBALREF int FtpCDUp(netbuf *nControl);
GLOBALREF int FtpRmdir(const char *path, netbuf *nControl);
GLOBALREF int FtpPwd(char *path, int max, netbuf *nControl);
GLOBALREF int FtpNlst(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpDir(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpSize(const char *path, int *size, char mode, netbuf *nControl);
GLOBALREF int FtpModDate(const char *path, char *dt, int max, netbuf *nControl);
GLOBALREF int FtpGet(const char *output, const char *path, char mode,
        netbuf *nControl);
GLOBALREF int FtpRename(const char *src, const char *dst, netbuf *nControl);
GLOBALREF int FtpDelete(const char *fnm, netbuf *nControl);
GLOBALREF void FtpQuit(netbuf *nControl);

/* Added by Leo for XNC */
GLOBALREF int FtpNoop(netbuf *nControl);
GLOBALREF int FtpCdup(netbuf *nControl);
GLOBALREF int FtpLS(char *output, char *path, netbuf *nControl);
GLOBALREF int FtpBinGet(char *output, char *path, int chmode, netbuf *nControl, int fsz);
GLOBALREF char *Ftp_getword(char *from,char* to);
GLOBALREF char *Ftp_nameword(char *from,char* to);
GLOBALREF int FtpPut(const char *input, const char *path, char mode,
        netbuf *nControl, int fsz);
GLOBALREF int FtpPutDir(netbuf* netb,char *from, char *to);
GLOBALREF int FtpGetDir(netbuf* netb,char *from,char *to,char *TMPFILE,char *tmp1,char *ftpname);
GLOBALREF int FtpDelDir(netbuf* netb,char *from,char *TMPFILE,char *tmp1,char *ftpname);

GLOBALREF char ftp_reason[];
GLOBALREF void (*ftp_error)(const char *);
GLOBALREF void (*ftp_log_handler)(const char *);
GLOBALREF FtpCallback default_cb;

/*******end add*******/

#ifdef __cplusplus
};
#endif

#endif /* __FTPLIB_H */
/* ------------ End of file -------------- */

