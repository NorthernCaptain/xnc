#ifndef FTPLIB_H
#define FTPLIB_H
/***************************************************************************/
/*                                                                           */
/* ftplib.h - header file for callable ftp access routines                 */
/* Copyright (C) 1996, 1997 Thomas Pfau, pfau@cnj.digex.net                */
/*        73 Catherine Street, South Bound Brook, NJ, 08880                   */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify    */
/* it under the terms of the GNU General Public License as published by    */
/* the Free Software Foundation; either version 2 of the License, or       */
/* (at your option) any later version.                                     */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/* GNU General Public License for more details.                            */
/*                                                                           */
/* You should have received a copy of the GNU General Public License       */
/* along with this program; if not, write to the Free Software             */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/*                                                                           */
/***************************************************************************/

#if defined(__unix__) || defined(VMS)
#define GLOBALDEF
#ifdef __cplusplus
#define GLOBALREF extern "C"
#else
#define GLOBALREF extern
#endif
#elif defined(_WIN32)
#define GLOBALDEF __declspec(dllexport)
#define GLOBALREF __declspec(dllimport)
#endif

typedef struct NetBuf netbuf;

extern netbuf *DefaultNetbuf;
#define FTP_BUFSIZ 8192
struct FtpServ
{
        char ftpname[255];
        char TMPFILE[512];
        char tmp1[255];
        char remotepath[1024];
        char path[1024];
};

struct NetBuf {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char buf[FTP_BUFSIZ];
    char response[256];
};

GLOBALREF char ftp_reason[];


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

GLOBALREF int ftplib_debug;
GLOBALREF void FtpInit(void);
GLOBALREF char *FtpLastResponse(netbuf *nControl);
GLOBALREF int FtpConnect(char *host, netbuf **nControl);
GLOBALREF int FtpLogin(char *user, char *pass, netbuf *nControl);
GLOBALREF int FtpSite(char *cmd, netbuf *nControl);
GLOBALREF int FtpMkdir(char *path, netbuf *nControl);
GLOBALREF int FtpChdir(char *path, netbuf *nControl);
GLOBALREF int FtpPwd(char *path, netbuf *nControl);
GLOBALREF int FtpCdup(netbuf *nControl);
GLOBALREF int FtpRmdir(char *path, netbuf *nControl);
GLOBALREF int FtpNlst(char *output, char *path, netbuf *nControl);
GLOBALREF int FtpDir(char *output, char *path, netbuf *nControl);
GLOBALREF int FtpLS(char *output, char *path, netbuf *nControl);
GLOBALREF int FtpGet(char *output, char *path, char mode, netbuf *nControl);
GLOBALREF int FtpBinGet(char *output, char *path, int chmode, netbuf *nControl);
GLOBALREF int FtpPut(char *input, char *path, char mode, netbuf *nControl);
GLOBALREF int FtpRename(char *src, char *dst, netbuf *nControl);
GLOBALREF int FtpDelete(char *fnm, netbuf *nControl);
GLOBALREF void FtpQuit(netbuf *nControl);

GLOBALREF char *Ftp_getword(char *from,char* to);
GLOBALREF char *Ftp_nameword(char *from,char* to);
GLOBALREF int FtpGetDir(netbuf* netb,char *from,char *to,char *TMPFILE,char *tmp1,char *ftpname);
GLOBALREF int FtpDelDir(netbuf* netb,char *from,char *TMPFILE,char *tmp1,char *ftpname);

#endif/* ------------ End of file -------------- */

