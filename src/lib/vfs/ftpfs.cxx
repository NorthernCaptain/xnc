/****************************************************************************
*  Copyright (C) 1996-98 by Leo Khramov
*  email:     leo@xnc.dubna.su
*  
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
 ****************************************************************************/

//This file contains routines for FTP filesystem (FTPFS).
//There are two parts: Child and parent.
//Parent sends commands to child, and child connects and talking to actual ftp.

#include "xh.h"
#include "panel.h"
#include "ftplib/ftp_communication.h"
#include "bookmark.h"
#include "query_windows.h"
#include "infowin.h"
#include "ftpfs.h"
#include "ftpvisuals.h"
#include "commonfuncs.h"
#include "ftpcfg.h"
#include "mesqueue.h"
#include "main.h"
#include "c_externs.h"
#include "internals.h"


#define         DEBDELAY        20        //DEBUG delay - FTP operation delay ->sleep(DEBDELAY)

int ftp_cache_disabled=0;                //If set to 1 then we skip cache routines.

int is_my_child(int,int);

static char ftptmpstr[2*L_MAXPATH];
static char ftptmpstr2[2*L_MAXPATH];

int ftp_timeout=200;   //Idle timeout in seconds


FTP *ftparr[MaxFtps];        //Array of active FTPs



void   (*xnc_look_at_pipes)(fd_set*,int)=ftp_backgrounds;        //Function for update FTP state in bg.
int    (*xnc_set_pipes)(fd_set*, int)=ftp_backgrounds_fdset;        //Function for select FTP bg pipes.
int    (*xnc_is_my_child)(int,int)=is_my_child;                //Function for catch unplanned child dies.


static int wlogp=-1;               //Loging pipe (for writing) - slave part
static MesQue<char> *log_que=NULL; //FTP-child logging queue, not used in parent.


//Looking through ftparr and see 'is there empty place?'
int is_ftp_place()
{
 int i;
 for(i=0;i<MaxFtps;i++)
   if(ftparr[i]==NULL)
      return 1;
 return 0;
}


int is_this_ftp(FTP* o)
{
  xncdprintf(("is_this_ftp() dup_entry=%d\n", o->dup_entry));
  if(o->dup_entry<0)
      return 0;
  xncdprintf(("is_this_ftp() obj=%X, arr[%d]=%X\n", o, o->dup_entry,
              ftparr[o->dup_entry]));
  if(ftparr[o->dup_entry]==o)
      return 1;
  return 0;
}

int where_is_ftp_place(FTP *o)
{
 int i;
 for(i=0;i<MaxFtps;i++)
   if(ftparr[i]==0)
      return i;
 return -1;
}


//Set background operation code to FTP and to its dup in ftparr.
void ftp_set_bgcode(FTP *ftp,int opcode)
{
  ftp->bgbit=opcode;
  if(ftparr[ftp->dup_entry])
    ftparr[ftp->dup_entry]->bgbit=opcode;
}

//Register FTP in ftparr.
int register_ftp(FTP* ftp)
{ 
  int i;
  xncdprintf(("register_ftp() Object=%X\n", ftp));
  fsw1->switch_to(1);        //Switch MenuBar to FtpVisual.
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i]==NULL)
    {
       ftparr[i]=ftp;
       fvis->rescan();
       xncdprintf(("register_ftp() Obj placed to %d\n", i));
       return i;
    }
  xncdprintf(("register_ftp() Object not registered!\n"));
  return -1;
}


int is_my_child(int dpid, int status)
{
   int i;
   for(i=0;i<MaxFtps;i++)
     if(ftparr[i]!=NULL && ftparr[i]->ftpid==dpid)
     {
       xncdprintf(("Got FTP child DIE, filling message\n"));
       ftparr[i]->ftpid=ftparr[i]->bgbit;  //Remember current operation.
       ftparr[i]->bgbit=FF_DIE;        //Unplanned closing connection.
       ftparr[i]->options|=VFSDIED_BIT;
       sprintf(ftparr[i]->com.reason,_("Unplaning Child DIE with code %d"),status);
       ftparr[i]->com.response=-1;
       return 1;
     }
   return 0;
}

//Find 'prev' in ftparr and change it to 'ftp'
int reregister_ftp(FTP* prev, FTP* ftp)
{ 
  int i;
  xncdprintf(("reregister_ftp() prevObject=%X Object=%X\n", prev, ftp));
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i]==prev)
    {
       ftparr[i]=ftp;
       xncdprintf(("reregister_ftp() found at %d\n", i));
       return i;
    }
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i]==NULL)
    {
       xncdprintf(("reregister_ftp() missed but placed at %d\n", i));
       ftparr[i]=ftp;
       return i;
    }
 xncdprintf(("reregister_ftp() missed at all\n"));
 return -1;
}

//Delete 'ftp' from ftparr
void unregister_ftp(FTP* ftp)
{ 
  int i;
  xncdprintf(("unregister_ftp() Object=%X\n", ftp));
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i]==ftp)
    {
       ftparr[i]=NULL;
       xncdprintf(("unregister_ftp() entry found at %d and deleted\n", i));
       fvis->rescan();
       break;
    }
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i])
            return;
//If we are here then no active FTP found -> switch MenuBar to MainMenu.
  fsw1->switch_to(0);
}

//Looking through all background FTP
//and receive events from its.
void ftp_backgrounds(fd_set *fds, int select_ret)
{
 int i;

 // internals_check_fds(fds);

 for(i=0;i<MaxFtps;i++)
   if(ftparr[i])
      ftparr[i]->bg_select_if_set(fds,select_ret);
}


void errno2ftp(Ftp_Com* com)
{
        com->response=-1;
        strcpy(com->reason,strerror(errno));
}

int ftp_backgrounds_fdset(fd_set *fds, int fd)
{
 int i;

 // fd=internals_set_fds(fd, fds);

 for(i=0;i<MaxFtps;i++)
   if(ftparr[i])
      fd = ftparr[i]->bg_fd_set(fd, fds);
 return fd;
}

void init_ftp_globals()
{
  int i;
  for(i=0;i<MaxFtps;i++)
    ftparr[i]=NULL;
}

//This will be run on exit. (Clean up ftp).
void deinit_all_ftp()
{
  int i;
  for(i=0;i<MaxFtps;i++)
    if(ftparr[i])
       ftparr[i]->close_fs();
}

int ftp_detect_host(char *hostname)
{
    if(str_casecmp(hostname, FTP_HOST_UNIX_CHR)==0)
        return FTP_HOST_UNIX;
    if(str_casecmp(hostname, FTP_HOST_SUNOS_CHR)==0)
        return FTP_HOST_UNIX;
    if(str_casecmp(hostname, FTP_HOST_VMS_CHR)==0)
        return FTP_HOST_VMS;
    if(str_casecmp(hostname, FTP_HOST_NT_CHR)==0)
        return FTP_HOST_NT;
    if(str_casecmp(hostname, FTP_HOST_WIN32_CHR)==0)
        return FTP_HOST_NT;
    return 0;
}    

int write_select(int fp)
{
  fd_set fds;
  struct timeval tv;
  tv.tv_sec=tv.tv_usec=0;
  FD_ZERO(&fds);
  FD_SET(fp,&fds);
  if(select(fp+1,NULL,&fds,NULL,&tv)>0)
          return FD_ISSET(fp,&fds);
  return 0;
}

int read_select(int fp)
{
  fd_set fds;
  struct timeval tv;
  tv.tv_sec=tv.tv_usec=0;
  FD_ZERO(&fds);
  FD_SET(fp,&fds);
  if(select(fp+1,&fds,NULL,NULL,&tv)>0)
    return FD_ISSET(fp,&fds);
  return 0;
}

int write_select_wait(int fp)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fp,&fds);
  if(select(fp+1,NULL,&fds,NULL,NULL)>0)
          return FD_ISSET(fp,&fds);
  return 0;
}

int read_select_wait(int fp)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fp,&fds);
  if(select(fp+1,&fds,NULL,NULL,NULL)>0)
          return FD_ISSET(fp,&fds);
  return 0;
}

int ftp_pipe_xfer(netbuf *ctl, int xfered, void *arg)
{
  time_t t;
  if(wlogp==-1)
          return 1;
  if(write_select(wlogp)==0)
          return 1;
  Ftp_Xfer xfer;
  xfer.type=FTP_XBYTES;
  xfer.xfered=xfered;
  xfer.total=*(int*)arg;
  time(&t);
  xfer.time=t;
  write(wlogp,&xfer,S_Ftp_Xfer);
  return 1;
}

void ftp_pipe_errmsg(const char *msg)
{
  time_t t;
  sprintf(ftp_reason, "%s: %s", msg, strerror(errno));
  if(wlogp==-1)
          return;
  if(write_select(wlogp)==0)
          return;
  Ftp_Xfer xfer;
  xfer.type=FTP_LOGMSG;
  xfer.xfered=0;
  xfer.total=0;
  strncpy(xfer.msg, ftp_reason, FTP_MSGLEN);
  time(&t);
  xfer.time=t;
  write(wlogp,&xfer,S_Ftp_Xfer);
  return;
}

void ftp_pipe_msg(const char *msg)
{
  time_t t;
  int l;
  char *tmps;
  xncdprintf(("ftp_pipe_msg() SEND: %s\n", msg));
  if(wlogp==-1)
          return;
  if(msg)
  {
      tmps=strdup(msg);
      log_que->push_mes(tmps);
      if(write_select(wlogp)==0)
      {
          xncdprintf(("ftp_pipe_msg() BUSY!\n"));
          return;
      }
  }
  tmps=log_que->pop_mes();
  if(tmps==NULL)
      return;
  Ftp_Xfer xfer;
  xfer.type=FTP_LOGMSG;
  xfer.xfered=0;
  xfer.total=0;
  strncpy(xfer.msg, tmps, FTP_MSGLEN);
  free(tmps);
  l=strlen(xfer.msg);
  if(xfer.msg[l-1]=='\n')
    xfer.msg[l-1]=0;
  time(&t);
  xfer.time=t;
  write(wlogp,&xfer,S_Ftp_Xfer);
  return;
}




/******************************************************************

                        FTP Class functions

*******************************************************************/





void dummy_func() {};

int FTP::ftp_log_msg(Ftp_Xfer& xfer)
{
  xncdprintf(("ftp_log_msg() RECV: [%s]\n",xfer.msg));
  return 1;
}

int FTP::ftp_log_xfer(int rlogp)
{
    Ftp_Xfer xfer;
    read(rlogp, &xfer, S_Ftp_Xfer);
    if(xfer.type==FTP_LOGMSG)
       return ftp_log_msg(xfer);
       
    if(infowin && bgbit==0)
       infowin->draw_coun(xfer.total, xfer.xfered, xfer.time);
    return 1;
}

int FTP::piperead(int pp,void *buf,int size)
{
  int ret;
        ret=read(pp,buf,size);
        if(ret==-1)
        {
                errno2mes();
                if(errno==EPIPE)
                {
                        waitpid(ftpid,NULL,WNOHANG);
                        errno=EPIPE;
                }
         }
    return ret;
}

//Open pipes and fork ftp child.
int FTP::ftp_fork()
{
  fd_set fds;
  fd_set wfds;
  struct timeval tv;

  if(pipe(wp)==-1)
  {
          errno2mes();
          return -1;
  }
  if(pipe(rp)==-1)
  {
          errno2mes();
          return -1;
  }
  if(pipe(logp)==-1)
  {
          errno2mes();
          return -1;
  }
  if((ftpid=fork())==-1)
  {
          errno2mes();
          return -1;
  }
  if(ftpid)        //We are parent process.
    return 1;      //Fork init done -> return to caller

//Child part of FTP begins here
//  close(1);
//  close(2);
  close(XConnectionNumber(disp));   //Closing connection to X server.
  xnc_ex=0;                //Disable Xlib.. functions on exit.
  ftplib_debug=2;          //Ftp debugging ON -> stderr
  signal (SIGHUP, SIG_DFL);
  signal (SIGINT, SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGTERM, SIG_DFL);
  signal (SIGCHLD, SIG_DFL);
  ftpid=getpid();
  default_cb=(FtpCallback) ftp_pipe_xfer;
  ftp_error=ftp_pipe_errmsg;
  ftp_log_handler=ftp_pipe_msg;
  wlogp=logp[1];
  log_que=new MesQue<char>(100);  //New logging queue for 100 messages
  while(1)
  {
    int fp=-1, ret;
    tv.tv_sec=ftp_timeout;
    tv.tv_usec=0;
    FD_ZERO(&fds);
    FD_SET(wp[0],&fds);
    fp=MAX2(wp[0], fp);
    if(log_que->is_not_empty())
    {
        FD_ZERO(&wfds);
        FD_SET(wlogp, &wfds);
        fp=MAX2(fp, wlogp);
    }
    if((ret=select(fp+1, &fds, log_que->is_not_empty() ? &wfds : NULL,
                   NULL,&tv))!=-1)
    {
        if(ret==0)
        {
            xncdprintf(("FTP::ftp_fork()-child_main_loop Timeout - do NOOP\n"));
            com.command=FTP_NOOP;
            child_ftp_command();
        } else
        {
            if(FD_ISSET(wlogp, &wfds))
            {
                ftp_pipe_msg(NULL);
            }
            if(FD_ISSET(wp[0], &fds))
            {
                read(wp[0],&com,S_Ftp_Com);
                child_ftp_command();
            }
        }
    }
  }
  return 0;
}

int FTP::child_get_single_file(int mode, char *fname)
{
  char ftptmpstr3[L_MAXPATH];
  int ret;
  add_path_content(com.remotepath, fname, is_vms());
  add_path_content(com.path, fname, is_vms());
      
  if(mode & S_IFDIR)
  {
    if(FtpGetDir(netb, com.remotepath, com.path, ftptmpstr, ftptmpstr2, ftptmpstr3)!=0)
    {
      com.response=-1;
      upper_path(com.remotepath, is_vms());
      upper_path(com.path);
      strcpy(com.reason,ftp_reason);
      return 1;
    } else
      strcpy(com.reason, netb->response);
    upper_path(com.remotepath, is_vms());
    upper_path(com.path);
    return 0;
  }
  
  ret=FtpBinGet(com.path,com.remotepath,mode,netb,0);
  strcpy(com.reason,netb->response);
  
  upper_path(com.remotepath, is_vms());
  upper_path(com.path);
  return ret ? 0 : 1;
}  


int FTP::child_put_single_file(int mode, char *fname)
{
  int ret;
  add_path_content(com.path, fname);
  add_path_content(com.remotepath, fname, is_vms());
  if(mode & S_IFDIR)
        {
                    if(FtpPutDir(netb, com.path, com.remotepath)!=0)
                    {
                      com.response=-1;
                      upper_path(com.remotepath, is_vms());
                      upper_path(com.path);
                      strcpy(com.reason,ftp_reason);
                      return 1;
                    } else
                           strcpy(com.reason, netb->response);
                   upper_path(com.remotepath, is_vms());
                   upper_path(com.path);
                   return 0;
        }
        
         ret=FtpPut(com.path,com.remotepath,'I',netb,0);
         strcpy(com.reason,netb->response);

         upper_path(com.remotepath, is_vms());
         upper_path(com.path);
         return ret ? 0 : 1;
}  


int FTP::child_del_single_file(int mode, char *fname)
{
  char ftptmpstr3[L_MAXPATH];
  char tmps[L_MAXPATH];
  int ret;
  strcpy(tmps,com.remotepath);
  add_path_content(tmps, fname, is_vms());
  if(mode & S_IFLNK)
  {
         ret=FtpDelete(tmps,netb);
         strcpy(com.reason,netb->response);
         return ret ? 0 : 1;
  } else
  if(mode & S_IFDIR)
        {
                    if(FtpDelDir(netb, tmps, ftptmpstr, ftptmpstr2, ftptmpstr3)!=0)
                    {
                      com.response=-1;
                      strcpy(com.reason,ftp_reason);
                      return 1;
                    } else
                           strcpy(com.reason, netb->response);
                   return 0;
        }
        
         ret=FtpDelete(tmps,netb);
         strcpy(com.reason,netb->response);

         return ret ? 0 : 1;
}  


void FTP::child_ftp_command()
{
  char tstr[L_MAXPATH];
  com.response=1;
  int len,mode,l,ret;
  char localtmp1[L_MAXPATH];
  char localtmp2[L_MAXPATH];
  char localtmp3[L_MAXPATH];
  char err_str[256];
  FILE *fp,*ofp;
  switch(com.command)
  {
  case FTP_CONNECT:
    if(com.options[0]==USE_PROXY)
    {
      if(FtpConnect(com.proxyhost, &netb)==0)
      {
	com.response=-1;
	if(netb)
	{
	  strcpy(com.reason, netb->response);
	  free(netb);
	} else
	  strcpy(com.reason, ftp_reason);
	com.command=FTP_BYE;
	break;
      }
      if(FtpLogin(com.proxyuser,com.proxypasswd,netb)==0)
      {
	com.response=-1;
	strcpy(com.reason,netb->response);
	FtpQuit(netb);
	com.command=FTP_BYE;
	break;
      }
      sprintf(localtmp1,"%s@%s", com.user, com.hostname);
      if(strchr(localtmp1, ':'))
	*strchr(localtmp1, ':')=0;
      xncdprintf(("child_ftp_command(): Connect throught proxy to %s\n", localtmp1));
      if(FtpLogin(localtmp1,com.passwd,netb)==0)
      {
	com.response=-1;
	strcpy(com.reason,netb->response);
	FtpQuit(netb);
	com.command=FTP_BYE;
	break;
      }
    } else        //elif  USE_PROXY
    {
      if(FtpConnect(com.hostname, &netb)==0)
      {
	com.response=-1;
	if(netb)
	{
	  strcpy(com.reason, netb->response);
	  free(netb);
	} else
	  strcpy(com.reason, ftp_reason);
	com.command=FTP_BYE;
	break;
      }
      if(FtpLogin(com.user,com.passwd,netb)==0)
      {
	com.response=-1;
	strcpy(com.reason,netb->response);
	FtpQuit(netb);
	com.command=FTP_BYE;
	break;
      }
    }
                 
    if(FtpSysType(err_str, 255, netb)==0)
    {
      com.response=-1;
      strcpy(com.reason,netb->response);
      FtpQuit(netb);
      com.command=FTP_BYE;
      break;
    }
                 
    com.options[1]=netb->host_type=ftp_detect_host(err_str);

#ifdef ONLY_UNIX                 
    if(com.options[1]!=FTP_HOST_UNIX &&
       com.options[1]!=FTP_HOST_VMS)
    {
      com.response=-1;
      sprintf(com.reason,"Connected to '%s' but only UNIX hosts currently supported",err_str);
      FtpQuit(netb);
      com.command=FTP_BYE;
      break;
    }
#endif
                 
    if(strcmp(com.remotepath,"*")==0)
    {
      if(FtpPwd(com.remotepath, 1024, netb)==0)
      {
	com.response=-1;
	strcpy(com.reason,netb->response);
	FtpQuit(netb);
	com.command=FTP_BYE;
	break;
      }
    }
                         
    create_ftp_cachedir(curdir);
    if(ftp_cache_disabled)
      sprintf(com.tmppath,"%s/FTPFS/.Xnc.Ftp%d.ls",syspath,ftpid);
    else
      sprintf(com.tmppath,"%s/FTPFS/%s/%s/.Xnc.Ftp.ls",syspath,com.hostname,com.remotepath);
   
  case FTP_LS:
#if FDEB
    fprintf(stderr,"FTP_LS: '%s'\n",com.remotepath);
#endif
    //                 sprintf(tstr,"Ftp.%d.ls",getpid());
    //               add_path_content(com.tmppath,tstr);
    if(FtpChdir(com.remotepath,netb)==0)
    {
      com.response=-1;
      upper_path(com.tmppath);
      strcpy(com.reason,netb->response);
      FtpPwd(com.remotepath, 1024, netb);
      break;
    }
    if(FtpLS(com.tmppath,NULL,netb)==0)
    {
      com.response=-1;
    }
    //                 upper_path(com.tmppath);
    strcpy(com.reason, netb->response);
    break;
                 
  case FTP_LCD:
    break;
                 
  case FTP_CD:
#if FDEB
    fprintf(stderr,"FTP_CD: '%s'\n",com.remotepath);
#endif
    if(FtpChdir(com.remotepath,netb)==0)
    {
      com.response=-1;
    }
    strcpy(com.reason, netb->response);
    FtpPwd(com.remotepath, 1024, netb);
    break;
  case FTP_MGET:
    fp=fopen(com.tmppath,"r");
    if(fp==NULL)
    {
      strcpy(com.reason,_("Can't read list file!"));
      com.response=-1;
      break;
    }
    strcpy(localtmp1,com.tmppath);
    strcat(com.tmppath,".ret");
    ofp=fopen(com.tmppath,"w");
    if(ofp==NULL)
    {
      fclose(fp);
      rename(localtmp1,com.tmppath);
      strcpy(com.reason, _("Can't create return list!"));
      com.response=-1;
      break;
    }
                 
    sprintf(ftptmpstr,"%s/%s.%d.ls", syspath, "FTPMGET",this);
    ret=0;
    while(fgets(localtmp2,L_MAXPATH,fp)!=NULL)
    {
      sscanf(localtmp2,"%o %[^\n]\n", &mode, localtmp3);
      l=strlen(localtmp3);
      if(localtmp3[l-1]=='\r')
	localtmp3[l-1]=0;
      xncdprintf(("Ftp-child: GET %s\n",localtmp3));
      if(child_get_single_file(mode, localtmp3)!=0)
      {
	strcpy(err_str,com.reason);
	fprintf(ofp,"%s\n",localtmp3);
	ret=-1;
      }
    }
    fclose(fp);
    fclose(ofp);
    unlink(localtmp1);
    //                 unlink(ftptmpstr);
    com.response=ret;
    if(ret==-1)
      strcpy(com.reason,err_str);
    else
      strcpy(com.reason,netb->response);
    break;
                 
  case FTP_MPUT:
    fp=fopen(com.tmppath,"r");
    if(fp==NULL)
    {
      strcpy(com.reason,_("Can't read list file!"));
      com.response=-1;
      break;
    }
    strcpy(localtmp1,com.tmppath);
    strcat(com.tmppath,".ret");
    ofp=fopen(com.tmppath,"w");
    if(ofp==NULL)
    {
      fclose(fp);
      rename(localtmp1,com.tmppath);
      strcpy(com.reason, _("Can't create return list!"));
      com.response=-1;
      break;
    }
                 
    ret=0;
    while(fgets(localtmp2,L_MAXPATH,fp)!=NULL)
    {
      sscanf(localtmp2,"%o %[^\n]\n", &mode, localtmp3);
      l=strlen(localtmp3);
      if(localtmp3[l-1]=='\r')
	localtmp3[l-1]=0;
      if(child_put_single_file(mode, localtmp3)!=0)
      {
	strcpy(err_str,com.reason);
	fprintf(ofp,"%s\n",localtmp3);
	ret=-1;
      }
    }
    fclose(fp);
    fclose(ofp);
    unlink(localtmp1);
    unlink(ftptmpstr);
    com.response=ret;
    if(ret==-1)
      strcpy(com.reason,err_str);
    else
      strcpy(com.reason,netb->response);
    break;
                 
  case FTP_MDEL:
    //                 sleep(DEBDELAY);
    fp=fopen(com.tmppath,"r");
    if(fp==NULL)
    {
      strcpy(com.reason,_("Can't read list file!"));
      com.response=-1;
      break;
    }
    strcpy(localtmp1,com.tmppath);
    strcat(com.tmppath,".ret");
    ofp=fopen(com.tmppath,"w");
    if(ofp==NULL)
    {
      fclose(fp);
      rename(localtmp1,com.tmppath);
      strcpy(com.reason, _("Can't create return list!"));
      com.response=-1;
      break;
    }
                 
    sprintf(ftptmpstr,"%s/%s.%d.ls", syspath, "FTPMDEL",this);
    ret=0;
    strcpy(tstr,com.remotepath);
    while(fgets(localtmp2,L_MAXPATH,fp)!=NULL)
    {
      sscanf(localtmp2,"%o %[^\n]\n", &mode, localtmp3);
      l=strlen(localtmp3);
      if(localtmp3[l-1]=='\r')
	localtmp3[l-1]=0;
      if(child_del_single_file(mode, localtmp3)!=0)
      {
	strcpy(err_str,com.reason);
	fprintf(ofp,"%s\n",localtmp3);
	ret=-1;
      }
    }
    fclose(fp);
    fclose(ofp);
    unlink(localtmp1);
    unlink(ftptmpstr);
    com.response=ret;
    if(ret==-1)
      strcpy(com.reason,err_str);
    else
      strcpy(com.reason,netb->response);
    strcpy(com.remotepath,tstr);
    break;
                 
  case FTP_GET:
    if(com.file_attr & S_IFDIR)
    {
      sprintf(tstr,"FtpTMP.%d.ls",getpid());
      add_path_content(com.tmppath,tstr);
      strcpy(localtmp1, com.remotepath);
      strcpy(localtmp2, com.path);
      if(FtpGetDir(netb, localtmp1, localtmp2, com.tmppath, ftptmpstr,ftptmpstr2)!=0)
      {
	com.response=-1;
	strcpy(com.reason,ftp_reason);
      } else
	strcpy(com.reason, netb->response);
      upper_path(com.remotepath, is_vms());
      upper_path(com.path);
      upper_path(com.tmppath);
      break;
    }
    com.response=FtpBinGet(com.path,com.remotepath,com.file_attr,netb,0) ? 0 : -1;
    strcpy(com.reason, netb->response);
    upper_path(com.remotepath, is_vms());
    upper_path(com.path);
    break;

  case FTP_PUT:
    //                 sleep(DEBDELAY);
    if(com.file_attr & S_IFDIR)
    {
      if(FtpPutDir(netb, com.path, com.remotepath)!=0)
      {
	com.response=-1;
	strcpy(com.reason,ftp_reason);
      } else
	strcpy(com.reason, netb->response);
      upper_path(com.remotepath);
      upper_path(com.path);
      break;
    }
    com.response=FtpPut(com.path,com.remotepath,'I',netb,0) ? 0 : -1;
    strcpy(com.reason, netb->response);
    upper_path(com.remotepath, is_vms());
    upper_path(com.path);
    break;

  case FTP_MKDIR:

    com.response=FtpMkdir(com.remotepath,netb) ? 0 : -1;
    strcpy(com.reason, netb->response);
    break;
                 
  case FTP_DEL:
    //                 sleep(DEBDELAY);
    if(com.file_attr & S_IFLNK)
    {
      com.response=FtpDelete(com.remotepath,netb) ? 0 : -1;
      strcpy(com.reason, netb->response);
      upper_path(com.remotepath, is_vms());
      break;
    } else
      if(com.file_attr & S_IFDIR)
      {
	sprintf(tstr,"%s/FTPFS/%s/FtpTMP.%d.ls",syspath,host,getpid());
	strcpy(localtmp1, com.remotepath);
	if(FtpDelDir(netb, localtmp1, tstr, ftptmpstr,ftptmpstr2)!=0)
	{
	  com.response=-1;
	  strcpy(com.reason,ftp_reason);
	} else
	  strcpy(com.reason, netb->response);
	upper_path(com.remotepath, is_vms());
	break;
      }
    com.response=FtpDelete(com.remotepath,netb) ? 0 : -1;
    strcpy(com.reason, netb->response);
    upper_path(com.remotepath, is_vms());
    break;
                 
  case FTP_NOOP:
    FtpPwd(com.remotepath, 1024, netb);
    //    FtpNoop(netb);
    return;

  case FTP_BYE:
    FtpQuit(netb);
    strcpy(com.reason, netb->response);
    break;
  };                

  //Send reply to parent.
  len=strlen(com.reason);
  if(com.reason[len-2]=='\r')
    com.reason[len-2]='\0';
  if(com.reason[len-1]=='\n')
    com.reason[len-1]='\0';
  if(write_select_wait(rp[1])==0)
  {
    xncdprintf(("FTP::child_ftp_command() write fd busy\n"));
  }
  len=write(rp[1],&com, S_Ftp_Com);
  if(len!=S_Ftp_Com)
  {
    xncdprintf(("FTP::child_ftp_command() response sent %d of %d\n", len, S_Ftp_Com));
  }
  if(com.command==FTP_BYE)
  {
    close(wp[0]);
    close(wp[1]);
    close(rp[0]);
    close(rp[1]);
    exit(0);
  }
}

//Send data to ftp child.
int FTP::send_to_ftpchild(Ftp_Com *com)
{
  while(!write_select_wait(wp[1]));
  if(write(wp[1],com,S_Ftp_Com)!=S_Ftp_Com)
  {
    xncdprintf(("FTP::send_to_ftpchild write failed (%s)\n", strerror(errno)));
    errno2mes();
    return 0;
  }
  return 1;
}

//If reply from child is ready, then process it!
void FTP::bg_select()
{
  int i;
  if(read_select(rp[0]))
  {
          if(piperead(rp[0],&com,S_Ftp_Com))
                  errno2ftp(&com);
          work=0;
          fvis->refresh(this);
          if(autoraise==0 || panel->vfs->fstype==FTP_TYPE)
                  return;
          i=where_is_ftp_place(this);
          if(i!=-1)
             fvis->animate_moving(i);
          bg_commands();
  }
}                           

void FTP::bg_select_if_set(fd_set *fds, int select_ret)
{
  int i,fdr=0;
  if(select_ret<0)
  {
    if(options & VFSDIED_BIT)  //Ftp connection was closed
    {
      work=0;
      fvis->refresh(this);
      bg_commands();
    }
    return;
  }

  if(FD_ISSET(logp[0], fds))
          ftp_log_xfer(logp[0]);
  if(bgbit==0 && (options & VFSDIED_BIT)==0)
          return;
  if((options & VFSDIED_BIT)==0 && FD_ISSET(rp[0], fds))
  {
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          fdr=1;
  }
  if(fdr || (options & VFSDIED_BIT))
  {
          work=0;
          fvis->refresh(this);
          if(autoraise==0)
                  return;
          i=where_is_ftp_place(this);
          if(i!=-1)
             fvis->animate_moving(i);
          bg_commands();
  }
}                           

int FTP::bg_fd_set(int fd, fd_set *fds)
{
  FD_SET(logp[0], fds);
  fd= fd > logp[0] ? fd : logp[0];
  if(bgbit)
  {
      fd= fd > rp[0] ? fd : rp[0];
      FD_SET(rp[0], fds);
  }
  return fd;
}


void FTP::bg_switch()
{
  if(work)
  {
    simple_mes("FTP",_("Operation not complete!"));
    return;
  }
  bg_commands();
}

void FTP::ftp_to_bg(FTP *o)
{
   curdir = com.remotepath;
   host = com.hostname;
   xncdprintf(("ftp_to_bg() Object=%X with dup_entry=%d\n", o, o->dup_entry));
   reregister_ftp(o, this);
}

void FTP::bg_commands()
{
        FTP *fv;
        FList *ko;
        FILE* fp;
          switch(bgbit)
          {
                  case FF_NON:
                          if(panel->vfs->fstype==FTP_TYPE)
                          {
                              vfs_error(_("Pusher"),_("Panel already in FTP mode. Close/Minimize first."));
                              show_vfs_error();
                              return;
                          }
                          if(panel->panel2->vfs==this)
                          {
                                  panel->header_blink_other();
                                  return;
                          }
                          need_reread=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          if(&panel->ftp!=this && &panel->panel2->ftp!=this)
                                delete this;
                          return;
                  case FF_CON:
                          if(com.response==-1)
                          {
                                  vfs_error("FTP",com.reason);
                                  show_vfs_error();
                          }
                          else
                          {
                            need_reread=0;
                            bgbit=0;
                            panel->push_n_pop(this);
                            reregister_ftp(this,&panel->ftp);
                            create_ftp_cachedir(curdir);
                            delete this;
                          }
                          break;
                  case FF_CLO:
                          close(wp[0]);
                          close(wp[1]);
                          close(rp[0]);
                          close(rp[1]);
                          bgbit=0;
                          unregister_ftp(this);
                          wait(NULL);
                          delete this;
                          break;
                  case FF_CD:
                          need_reread = 1;
                          if(com.response==-1)
                          {
                               vfs_error("FTP",com.reason);
                               show_vfs_error();
                          } else
                          {
                                  bgbit=0;
                                  panel->push_n_pop(this);
                                  reregister_ftp(this,&panel->ftp);
                                  create_ftp_cachedir(curdir);
                                  delete this;
                          }
                          break;
                  case FF_DIR:
                          need_reread=0;
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_VIE:
                  case FF_SVI:
                          need_reread=0;
                          if(com.response!=-1)
                                  panel->bg_view(tmps);
                          else
                            {
                                 vfs_error("FTP",com.reason);
                                 show_vfs_error();
                            }
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_EDI:
                          need_reread=0;
                          if(com.response!=-1)
                                  panel->bg_edit(tmps);
                          else
                            {
                                 vfs_error("FTP",com.reason);
                                 show_vfs_error();
                            }
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_GET:
                          need_reread=0;
                          if(com.response==-1)
                          {
                              vfs_error("FTP",com.reason);
                              show_vfs_error();
                          }
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_PUT:
                          need_reread=1;
                          if(options & VFSDIFF_BIT)
                          {
                            strcpy(com.remotepath, dest);
                            options&=~VFSDIFF_BIT;
                          }
                          options|=REALREAD_BIT;
                          if(com.response==-1)
                          {
                              vfs_error("FTP",com.reason);
                              show_vfs_error();
                          }
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_DEL:
                  case FF_MKD:
                          need_reread=1;
                          options|=REALREAD_BIT;
                          strcpy(com.remotepath,com.tmppath);  /* we store remotepath in tmppath during this oper*/
                          if(com.response==-1)
                          {
                              vfs_error("FTP",com.reason);
                              show_vfs_error();
                          }
                          bgbit=0;
                          panel->push_n_pop(this);
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_MGET:
                          need_reread=0;
                          bgbit=0;
                          fv=(FTP*)panel->push_n_pop(this);
                          if(com.response==-1)
                          {
                             vfs_error("FTP",com.reason);
                             fp=fopen(com.tmppath,"r");        //read list of files return by child and select these files.
                             if(fp==NULL)
                                     errno2mes();
                             else
                             {
                             while(fgets(tmps,L_MAXPATH,fp))
                             {
                                ko=fv->dl.next;
                                tmps[strlen(tmps)-1]=0;
                                while(ko)
                                {
                                        if(strcmp(ko->name,tmps)==0)
                                                break;
                                        ko=ko->next;
                                }
                                if(ko)
                                {
                                        ko->options|=S_SELECT;
                                        panel->selfiles++;
                                        panel->selsize+=ko->size;
                                }
                             }
                             fclose(fp);
                             unlink(com.tmppath);
                             show_vfs_error();
                             panel->showdir();
                             }
                          }
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_MPUT:
                          bgbit=0;
                          fv=(FTP*)panel->push_n_pop(this);
                          fv->need_reread=1;
                          fv->options|=REALREAD_BIT;
                          if(options & VFSDIFF_BIT)
                          {
                            strcpy(com.remotepath, dest);
                            options&=~VFSDIFF_BIT;
                          }
                          if(com.response==-1)
                          {
                             vfs_error("FTP",com.reason);
                             unlink(com.tmppath);
                             show_vfs_error();
                          }
                          panel->reread();
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                  case FF_MDEL:
                          need_reread=0;
                          bgbit=0;
                          fv=(FTP*)panel->push_n_pop(this);
                          fv->need_reread=1;
                          fv->options|=REALREAD_BIT;
                          if(com.response==-1)
                          {
                             vfs_error("FTP",com.reason);
                             fp=fopen(com.tmppath,"r");        //read list of files return by child and select these files.
                             if(fp==NULL)
                                     errno2mes();
                             else
                             {
                             while(fgets(tmps,L_MAXPATH,fp))
                             {
                                ko=fv->dl.next;
                                tmps[strlen(tmps)-1]=0;
                                while(ko)
                                {
                                        if(strcmp(ko->name,tmps)==0)
                                                break;
                                        ko=ko->next;
                                }
                                if(ko)
                                {
                                        ko->options|=S_SELECT;
                                        panel->selfiles++;
                                        panel->selsize+=ko->size;
                                }
                             }
                             fclose(fp);
                             unlink(com.tmppath);
                             show_vfs_error();
                             }
                          }
                          panel->reread();
                          reregister_ftp(this,&panel->ftp);
                          delete this;
                          break;
                 case FF_DIE:
		          xncdprintf(("FTP is DIEing -> let's make clean ups\n"));
                          vfs_error(_("FTP FATAL"),com.reason);
                          delete_vfs_list();
                          close_fs();
                          show_vfs_error();
                          options|=VFSDIED_BIT;
                          break;
          }
   bgbit=0;
}

//Wait while child complete operation
//This routine can change FTP status to BG.
int FTP::ftp_wait(char *mes,int opcode)
{
  int logging=NO;
  time_t tt,tt2;
  disable_reread=1;
  work=1;
  fvis->refresh(this);
  if(mes)
  {
          create_ftp_infowin("FTP");
          to_infowin(mes);
  }
  time(&tt);
  while(!read_select(rp[0]))
  {
      if (read_select(logp[0]))
          ftp_log_xfer(logp[0]);
      if (XPending(disp))
        {
          XNextEvent(disp, &ev);
          process_x_event(&ev);
        }
      time(&tt2);
      if(options & VFSDIED_BIT)
      {
              vfs_error("FTP FATAL",com.reason);
              delete_vfs_list();
              if(mes)
                    del_infowin();
              close_fs();
              show_vfs_error();
              options^=VFSDIED_BIT;
              return BGM;
      }
      if(bgbit==0 && infowin->bgbit && xnc_ex)
      {
              bgbit=opcode;
              delete_vfs_list();
              FTP *ftp=new FTP;
              *ftp=*this;
              ftp->init_internals();
              dup_entry=reregister_ftp(this,ftp);
              if(mes)
                    del_infowin();
              return BGM;        //We are in background mode! (operation not completed yet)
      }
  }
  disable_reread=0;
  if(mes)
    del_infowin();
  work=0;
  fvis->refresh(this);
  return FGM;
}

void FTP::create_ftp_cachedir(char *dir)
{
   char str[L_MAXPATH];
   char str2[FLIST_NAME];
         if(ftp_cache_disabled)
                   return;
         sprintf(str,"%s/FTPFS/%s",syspath,host);
         if(::mkdir(str,0755)==-1 && errno!=EEXIST)
                 return;
         if(strcmp(dir,"/"))
         {
                 const char *s=dir;
                 if(*s=='/')         //Skip leading '/'
                         s++;
                 do
                 {
                         s=get_first_content(s,str2);
                         add_path_content(str,str2);
                         if(::mkdir(str,0755)==-1 && errno!=EEXIST)
                                 break;
                 } while(s);
         }
}

//Initialize FTPlink and run ftp.xnc if need.
//Return 1 if OK and 0 if failed.    
int    FTP::init_support(char *ftprec_chr, char* nullval)
{
  char  *b;
  FTPCFG_S *frec=(FTPCFG_S*)ftprec_chr;
  ftpcfg=*frec;
  if(is_ftp_place()==0)
  {
          vfs_error("FTP",_("Too many active connections"));
          return 0;
  }
  com.command = FTP_CONNECT;
  com.xncpid = getpid();

  sprintf(host,"%s:%d", frec->ftphost, frec->ftpport);
  xncdprintf(("FTP::init_support() connecting to %s\n", host));
  strcpy(com.remotepath, frec->ftppath);
  strcpy(com.user,frec->ftplogin);
  strcpy(com.passwd,frec->ftppasswd);
  
  if(frec->bool_prox)
  {
      strcpy(com.proxyhost,frec->proxhost);
      strcpy(com.proxyuser,frec->proxlogin);
      strcpy(com.proxypasswd,frec->proxpasswd);
      com.options[0]=USE_PROXY;
  }
  strcpy(com.tmppath, syspath);
// *INDENT-OFF*        
  ::getcwd(com.path, 1024);
// *INDENT-ON*        

  if(ftp_fork()==-1)
  {
        vfs_error("FTP",_("Forking Child"));
        return 0;
  }
  bgbit=0;
  if (send_to_ftpchild(&com))
    {
          make_empty_list();
          if(ftp_wait(_("Connecting"), FF_CON)==BGM)
                  return 1;
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          if(com.response==-1)
          {
                  vfs_error("FTP",com.reason);
                  return 0;
          }
          need_reread=0;
          dup_entry=register_ftp(this);
          create_ftp_cachedir(curdir);
          return 1;
    }
  vfs_error("FTP",_("Child process failed"));
  return 0;
}

extern "C" void set_cmdchild_signal();

//Close ftp and wait for child DIE!
void   FTP::close_fs()
{
  if((options & VFSDIED_BIT)==0)
  {
  signal(SIGCHLD, SIG_DFL);
  com.xncpid = getpid();
  com.command = FTP_BYE;
  send_to_ftpchild(&com);
  if(ftp_wait(_("Closing connection"), FF_CLO)==BGM)
          return;
  piperead(rp[0],&com,S_Ftp_Com);
  waitpid(ftpid,NULL,WNOHANG);
  set_cmdchild_signal();
  }
  close(wp[0]);
  close(wp[1]);
  close(rp[0]);
  close(rp[1]);
  unregister_ftp(this);
}

char*   FTP::get_execute_dir(FList* ptr)
{
  sprintf(tmps,"%s/FTPFS/%s%s",syspath,host,com.remotepath);
  return tmps;
}

char*   FTP::get_file_for_execute(FList* ptr)
{
  char *fname=ptr->name;
  com.xncpid = getpid();
  com.command = FTP_GET;
  com.file_attr=0755;
  strcpy(tmps,com.path);
  if(is_vms())
      sprintf(com.path,"%s/FTPFS/%s/%s",syspath,host,com.remotepath);
  else
      sprintf(com.path,"%s/FTPFS/%s%s",syspath,host,com.remotepath);
  add_path_content(com.path,fname);
  xncdprintf(("get_file_for_execute() path: %s\n", com.path));
  if(is_cache_out_time(com.path, transfer_cache_timeout)==NO) 
      {
          return fname;
      }
  add_path_content(com.remotepath,fname, is_vms());
  xncdprintf(("get_file_for_execute() remote path: %s\n", com.remotepath));
  send_to_ftpchild(&com);
  upper_path(com.remotepath, is_vms());
  strcpy(com.path,tmps);
  if(ftp_wait(_("Getting file"), FF_GET)==BGM)
  {
          panel->switch_to_prev_vfs();
          return NULL;
  }
  if(piperead(rp[0],&com,S_Ftp_Com)==-1)
         errno2ftp(&com);
  if(com.response==-1)
  {
     vfs_error("FTP",com.reason);
     return NULL;
  }
  return fname;
}

int  FTP::remove(FList *cur)
{
  com.xncpid = getpid();
  del_infowin();
  if(opt==0) //If no optimization then we delete only one file from FTP
  {  
  com.command = FTP_DEL;
  com.file_attr=cur->mode;
  strcpy(com.tmppath,com.remotepath);
  strcpy(com.remotepath,cur->name);
  send_to_ftpchild(&com);
  strcpy(com.remotepath,com.tmppath);
  if(ftp_wait(_("Deleting file"), FF_DEL)==BGM)
  {
          panel->switch_to_prev_vfs();
          return 0;
  }
  if(piperead(rp[0],&com,S_Ftp_Com)==-1)
         errno2ftp(&com);
  if(com.response==-1)
  {
     strcpy(com.remotepath,com.tmppath);
     vfs_error("FTP",com.reason);
     return 255;
  }
  strcpy(com.remotepath,com.tmppath);

  } else
  {
          if(selected_list_to_file())
          {
                  com.command = FTP_MDEL;
                  send_to_ftpchild(&com);
                  if(ftp_wait(_("Deleting files"), FF_MDEL)==BGM)
                  {
                          panel->switch_to_prev_vfs();
                          return 0;
                  }
                  if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                         errno2ftp(&com);
                  need_reread=1;
                  options|=REALREAD_BIT;
                  if(com.response==-1)
                  {
                     file_to_selected_list(dl.next);
                     vfs_error("FTP",com.reason);
                     return 255;
                  }
          }
  }
          
  need_reread=1;
  options|=REALREAD_BIT;
  return 0;
}

int  FTP::mkdir(char *dir, mode_t mode)
{
  com.xncpid = getpid();
  del_infowin();
  com.command = FTP_MKDIR;
  com.file_attr=mode;
  strcpy(com.tmppath,com.remotepath);
  strcpy(com.remotepath,dir);
  send_to_ftpchild(&com);
  strcpy(com.remotepath,com.tmppath);
  if(ftp_wait(_("Making directory"), FF_MKD)==BGM)
  {
          panel->switch_to_prev_vfs();
          return 0;
  }
  if(piperead(rp[0],&com,S_Ftp_Com)==-1)
         errno2ftp(&com);
  if(com.response==-1)
  {
     strcpy(com.remotepath,com.tmppath);
     vfs_error("FTP",com.reason);
     return 255;
  }
  strcpy(com.remotepath,com.tmppath);
  return 0;
}


int   FTP::selected_list_to_file()
{
   FList* ko;
   FILE *fp;
          ko = dl.next;
          opt=0;
          sprintf(com.tmppath,"%s/FTPFS/%s/TMPLIST.%d",syspath,host,this);
          fp=fopen(com.tmppath,"w");
          if(fp==NULL)
          {
             vfs_error("FTP",_("Can't create temporary list file!"));
             return 0;
          }
          while (ko != NULL)
            {
              if (ko->options & S_SELECT)
                {
                    fprintf(fp,"%o %s\n",ko->mode, ko->name);
                }
              ko = ko->next;
            }
          fclose(fp);
     return 1;
}


int   FTP::file_to_selected_list(FList *oo)
{
        FILE *fp;
        FList *ko;
        ko=oo;
        char tmps[L_MAXPATH];
          while(ko)
          {
                  ko->options &= ~S_SELECT;
                  ko=ko->next;
          }
             panel->selfiles=0;
             panel->selsize=0;
             fp=fopen(com.tmppath,"r");
             if(fp==NULL)
                     return 0;
             while(fgets(tmps,L_MAXPATH,fp))
             {
                ko=oo;
                tmps[strlen(tmps)-1]=0;
                while(ko)
                {
                        if(strcmp(ko->name,tmps)==0)
                                break;
                        ko=ko->next;
                }
                if(ko)
                {
                        ko->options|=S_SELECT;
                        panel->selfiles++;
                        panel->selsize+=ko->size;
                }
             }
             fclose(fp);
             unlink(com.tmppath);
             return 1;
}


//copy DFS->FTP
int FTP::add_to_vfs(FList *o) 
{
  int l;
  char str1[L_MAXPATH];
  char rootpath[]="/";
  char *b;
  
  del_infowin();
  
          FList *oo=dl.next;
          struct stat dstat;
          int checkres=0;

//Check file existance.
          if(options & VFSDIFF_BIT)
          {
             strcpy(str1, dest);
             b=strrchr(str1, '/');
             if(b)
               *b=0;
             else
               b=str1-1;
             if(path_cmp(curdir, b==str1 ? rootpath : str1)==0)
             {
              while(oo)
                      if(strcmp(oo->name, b+1)==0)
                      {
                              checkres=1;
                              break;
                      } else
                              oo=oo->next;
              if (ow_all == 0 && dont_ask == 0)
                if (checkres)
                  {
                    init_overwrite_query(_("Copy file"), o->name);
                    wait_for_query_done();
                    if (ow_all == 0 && ow_file == 0)
                      return ow_cancel;        //if cancel we return as error!!!
        
                  }
             }
          } else
            {
              while(oo)
                      if(strcmp(oo->name,o->name)==0)
                      {
                              checkres=1;
                              break;
                      } else
                              oo=oo->next;
              if (ow_all == 0 && dont_ask == 0)
                if (checkres)
                  {
                    init_overwrite_query(_("Copy file"), o->name);
                    wait_for_query_done();
                    if (ow_all == 0 && ow_file == 0)
                      return ow_cancel;        //if cancel we return as error!!!
        
                  }
            }
          ow_file = 0;


          com.xncpid = getpid();
          com.command = FTP_PUT;
          com.file_attr=o->mode;
          strcpy(tmps, com.path);
          ::getcwd(com.path,1024);
          sprintf(com.tmppath,"%s/FTPFS/%s/",syspath,host);
          if (!S_ISDIR(o->mode) || (::stat(com.path, &dstat) != -1 && S_ISDIR(dstat.st_mode)))
                add_path_content(com.path,o->name);
          if(options & VFSDIFF_BIT)
          {
            str_swap(com.remotepath, dest);
            l=strlen(com.remotepath);
            if(com.remotepath[l-1]=='/')
              add_path_content(com.remotepath, o->name, is_vms());
          } else
            add_path_content(com.remotepath,o->name, is_vms());
          send_to_ftpchild(&com);
          upper_path(com.remotepath, is_vms());
          strcpy(com.path,tmps);
          if(options & VFSDIFF_BIT)
            strcpy(com.remotepath, dest);
          if(ftp_wait(_("Putting file"), FF_PUT)==BGM)
          {
                  panel->panel2->switch_to_prev_vfs();
                  return 0;
          }
          need_reread=1;
          options|=REALREAD_BIT;
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          if(options & VFSDIFF_BIT)
          {
            strcpy(com.remotepath, dest);
            options&=~VFSDIFF_BIT;
          }
          if(com.response==-1)
          {
             vfs_error("FTP",com.reason);
             strcpy(com.path,tmps);
             return 255;
          }
          strcpy(com.path,tmps);
          need_reread=1;
          options|=REALREAD_BIT;
          return 0;
}

int   FTP::add_to_vfs_by_list(FList *ino)
{
  char tmps[L_MAXPATH];
  FList *o;
  FILE *fp;
  FList *ko=ino;
  del_infowin();
  
          sprintf(com.tmppath,"%s/FTPFS/%s/TMPLIST.%d",syspath,host,this);
          fp=fopen(com.tmppath,"w");
          if(fp==NULL)
          {
             vfs_error("FTP",_("Can't create temporary list file!"));
             return 255;
          }
          ow_no=0;
          while (ko != NULL)
            {
              if (ko->options & S_SELECT)
                {
                      //Check file existance.
                      if (ow_all == 0 && dont_ask == 0)
                      {
                        o=dl.next;
                        while(o)
                         if (strcmp(o->name,ko->name)==0)
                          {
                            init_overwrite_query(_("Copy file"), ko->name);
                            wait_for_query_done();
                            if (ow_cancel)
                            {
                              fclose(fp);
                              unlink(tmps);
                              return ow_cancel;        //if cancel we return as error!!!
                            }
                            break;
                          } else
                                  o=o->next;
                       ow_file = 0;
                       }
                       if(ow_no)
                       {
                         ko->options^=S_SELECT;
                         ow_no=0;
                       } else
                          fprintf(fp,"%o %s\n",ko->mode, ko->name);
                }
              ko = ko->next;
            }
          fclose(fp);

          com.xncpid = getpid();
          com.command = FTP_MPUT;
          ::getcwd(com.path,1024);
          if(options & VFSDIFF_BIT)
            str_swap(com.remotepath, dest);
          send_to_ftpchild(&com);
          if(options & VFSDIFF_BIT)
            strcpy(com.remotepath, dest);
          if(ftp_wait(_("MPutting files"), FF_MPUT)==BGM)
          {
                  panel->panel2->switch_to_prev_vfs();
                  return 0;
          }
          need_reread=1;
          options|=REALREAD_BIT;
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          if(options & VFSDIFF_BIT)
          {
            strcpy(com.remotepath, dest);
            options&=~VFSDIFF_BIT;
          }
          if(com.response==-1)
          {
             file_to_selected_list(ino);
             vfs_error("FTP",com.reason);
             strcpy(com.path,tmps);
             return 255;
          }
          strcpy(com.path,tmps);
          need_reread=1;
          options|=REALREAD_BIT;
          panel->selfiles=panel->selsize=0;
          ko=ino;
          while(ko)
          {
               ko->options &= ~S_SELECT;
               ko=ko->next;
          }
          return 0;
}


//Copy FTP->DFS
int   FTP::copy(FList *o, VFS* vfs)
{
  if(vfs->fstype!=DFS_TYPE)        //Currently we can copy only to DFS
          return 255;
  del_infowin();
  
  struct stat dstat;
  char tmps[L_MAXPATH];
  FList *ko;
  FILE *fp;
  int ret;
  if(opt==0) //If no optimization then we copy only one file from FTP
  {  
          int checkres=vfs->is_file_exist(vfs->curdir, o->name);
//Check file existance.
              if (ow_all == 0 && dont_ask == 0)
                if (checkres)
                  {
                    init_overwrite_query(_("Copy file"), o->name);
                    wait_for_query_done();
                    if (ow_all == 0 && ow_file == 0)
                      return ow_cancel;        //if cancel we return as error!!!
        
                  }
              ow_file = 0;
        
          com.xncpid = getpid();
          com.command = FTP_GET;
          com.file_attr=o->mode;
          strcpy(com.tmppath, syspath);
          strcpy(com.path,vfs->curdir);
          ret=::stat(vfs->curdir, &dstat);
          if (ret!=-1 && (!S_ISDIR(o->mode) ||  S_ISDIR(dstat.st_mode)))
                add_path_content(com.path,o->name);
          add_path_content(com.remotepath,o->name, is_vms());
          send_to_ftpchild(&com);
          upper_path(com.remotepath, is_vms());
          upper_path(com.path);
          if(ftp_wait(_("Getting file"), FF_GET)==BGM)
          {
                  panel->switch_to_prev_vfs();
                  return 0;
          }
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          if(com.response==-1)
          {
             vfs_error("FTP",com.reason);
             return 255;
          }
  } else         //If we are here then doing list file copy
  {
          ko = dl.next;
          opt=0;
          sprintf(com.tmppath,"%s/FTPFS/%s/TMPLIST.%d",syspath,host,this);
          fp=fopen(com.tmppath,"w");
          if(fp==NULL)
          {
             vfs_error("FTP",_("Can't create temporary list file!"));
             return 255;
          }
          while (ko != NULL)
            {
              if (ko->options & S_SELECT)
                {
                      //Check file existance.
                      if (ow_all == 0 && dont_ask == 0)
                        if (vfs->is_file_exist(vfs->curdir,ko->name))
                          {
                            init_overwrite_query(_("Copy file"), ko->name);
                            wait_for_query_done();
                            if (ow_cancel)
                            {
                              fclose(fp);
                              unlink(tmps);
                              return ow_cancel;        //if cancel we return as error!!!
                            }
                          }
                       ow_file = 0;
                       if(ow_no)
                       {
                         ko->options ^= S_SELECT;
                         ow_no=0;
                       } else
                          fprintf(fp,"%o %s\n",ko->mode, ko->name);
                }
              ko = ko->next;
            }
          fclose(fp);

          com.xncpid = getpid();
          com.command = FTP_MGET;
          strcpy(com.path,vfs->curdir);

          send_to_ftpchild(&com);

          if(ftp_wait(_("MGetting files"), FF_MGET)==BGM)
          {
                  panel->switch_to_prev_vfs();
                  return 0;
          }
          if(piperead(rp[0],&com,S_Ftp_Com)==-1)
                  errno2ftp(&com);
          ko=dl.next;
          while(ko)
          {
                  if(ko->options & S_SELECT)
                          ko->options^=S_SELECT;
                  ko=ko->next;
          }
          if(com.response==-1)
          {
             vfs_error("FTP",com.reason);
             file_to_selected_list(dl.next);
             return 255;
          }
      
      opt=0;
  }
     
  return 0;
}

int    FTP::chdir(char *d)
{
  if(strcmp(d,"..")==0 && strcmp(curdir,"/")==0)
  {
    need_change_vfs=1;
    return 0;
  }
  need_change_vfs=0;
  if(strcmp(com.remotepath,d)==0)
          return 0;
  com.command = FTP_CD;
  strcpy(com.remotepath, d);
  if (send_to_ftpchild(&com))
    {
      if(ftp_wait(_("Changing directory"),FF_CD)==BGM)
      {
              panel->switch_to_prev_vfs();
              return 0;
      }
      if(piperead(rp[0],&com,S_Ftp_Com)==-1)
              errno2ftp(&com);
      need_reread = 1;
      if(com.response==-1)
      {
         vfs_error("FTP",com.reason);
         return -1;
      }
      create_ftp_cachedir(curdir);
      return 0;
    }
  return -1;
}

char  *FTP::get_info_for_bookmark()
{
  if(strcmp(ftpcfg.ftplogin,"anonymous")==0 ||
     strcmp(ftpcfg.ftplogin,"ftp")==0)
    sprintf(tmps,"%s%s",host,curdir);
  else
    sprintf(tmps,"%s@%s%s",ftpcfg.ftplogin,host,curdir);
  return tmps;
}


char  *FTP::get_dir_header()
{
  sprintf(ftptmpstr, "ftp://%s%s", host, curdir);
  return ftptmpstr;
}

//Convert file info from list file to stat struct.
char  *convert_ftpstring(char *s, struct stat *st, char *user, char *group)
{
  static char strbuf[L_MAXPATH];
  int l;
  sscanf(s, "%d %o %X %s %s %[^\n]\n", &st->st_size, &st->st_mode,
         &st->st_mtime, user, group, strbuf);
  l=strlen(strbuf);
  if(strbuf[l-1]=='\r')
          strbuf[l-1]=0;
  st->st_uid = st->st_gid = 0;
  if (strcmp(strbuf, "..") == 0 || strcmp(strbuf,".")==0)
    return NULL;
  return strbuf;
}

int find_in_dir_by_name(char *list, char *fname, struct stat *st)
{
  char *name;
  char buf[1024];
  char dummy[64];
  FILE *fp = fopen(list, "r");
  while(fgets(buf, 1024, fp)!=NULL)
  {
      name=convert_ftpstring(buf, st, dummy, dummy);
      if(name && strcmp(name, fname)==0)
      {
          fclose(fp);
          return 1;
      }
  }
  fclose(fp);
  return 0;
}

int    FTP::direc(char *dir_ch)
{
  FList *ol, *hd=NULL;
  Namer *namer=NULL,*n;
  char  *d_name;
  FILE  *d;
  struct stat st;
  char   ftpname[1024];
  char   lbuf1[FLIST_NAME];
  char   lbuf2[FLIST_NAME];
  char   user[L_USER];
  char   group[L_GROUP];
  int linkcoun=0;
  int oldcache=NO;

  if(ftp_cache_disabled)
        sprintf(com.tmppath,"%s/FTPFS/.Xnc.Ftp%d.ls",syspath,ftpid);
  else
        sprintf(com.tmppath,"%s/FTPFS/%s/%s/.Xnc.Ftp.ls",syspath,host,curdir);
  oldcache=is_cache_out_time(com.tmppath, ls_timeout);
  if (need_reread || oldcache)
    {
      com.command = FTP_LS;
      if(ftp_cache_disabled || stat(com.tmppath,&st)==-1 || (options & REALREAD_BIT) || oldcache)
      {
       create_ftp_cachedir(curdir);
       options&=~REALREAD_BIT;
       if (send_to_ftpchild(&com))
       {
        make_empty_list();
        if(ftp_wait(_("Directory listing"),FF_DIR)==BGM)
                return 1;
       }
       else
         return 0;
       if(piperead(rp[0],&com,S_Ftp_Com)==-1)
               errno2ftp(&com);
       if(com.response==-1)
               return 0;
      }
      need_reread = 0;
    }
  d = fopen(com.tmppath, "r");
  if (d == NULL)
  {
      com.command = FTP_LS;
      if(ftp_cache_disabled || stat(com.tmppath,&st)==-1)
      {
       create_ftp_cachedir(curdir);
       if (send_to_ftpchild(&com))
       {
        make_empty_list();
        if(ftp_wait(_("Directory listing"),FF_DIR)==BGM)
                return 1;
       }
       else
         return 0;
       if(piperead(rp[0],&com,S_Ftp_Com)==-1)
               errno2ftp(&com);
       if(com.response==-1)
               return 0;
      }
      need_reread = 0;
      d = fopen(com.tmppath, "r");
      if (d == NULL)
      {
              errno2mes();
              return 0;
      }
  }

  delete_vfs_list();
  while (fgets(ftpname, 1024, d) != NULL)
    {
      if ((d_name = convert_ftpstring(ftpname, &st, user, group)) == NULL)
        {
          if (st.st_mode)
            {
              hd = (FList*)new_alloc(S_FList);
              hd->init("..", st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);
            }
          continue;
        }
      if (hide_hidden && d_name[0] == '.')
        continue;
      bmark->animate();
      if(st.st_mode & S_IFLNK)        //Symbolic link
      {
              linkcoun++;
              breakdown_link(d_name,lbuf1,lbuf2);        //breakdown  lbuf1 -> lbuf2
              ol=(FList*)new_alloc(S_FList);
              ol->init(lbuf1, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);        //init link
              n=new Namer(lbuf2,ol);        //and remember pointer for post detection.
              n->next=namer;
              namer=n;
      } else
      {
              ol=(FList*)new_alloc(S_FList);
              ol->init(d_name, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);
      }
      ol->set_user_group_names(user, group);
      if (st.st_mode & S_IFDIR)
        dl.add(ol);
      else
          fl.add(ol);
    }
  fclose(d);


  if(linkcoun)
  {                        //Doing post action linking
        n=namer;
        while(n)
        {
              ol=(FList*)new_alloc(S_FList);
              if(flist_find_by_name(dl.next, n->name, &st) ||
                 flist_find_by_name(fl.next, n->name, &st) ||
                 (hide_hidden && find_in_dir_by_name(com.tmppath, n->name, &st)))
                      ol->init(n->name, st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);
              else
                      ol->init(n->name,0755,0,0,0,st.st_mtime);
              n->o->link=ol;
              ol=n->o;
              if((ol->mode & S_IFDIR)!=S_IFDIR && (st.st_mode & S_IFDIR)==S_IFDIR)
              {        //We are in wrong list. We must be inside dl but now we in fl.
                      ol->mode|=S_IFDIR;
                      if(ol->prev)
                              ol->prev->next=ol->next;
                      if(ol->next)
                              ol->next->prev=ol->prev;
                      ol->next=ol->prev=NULL;
                      dl.add(ol);
              } else
                      if((ol->mode & S_IFDIR)==S_IFDIR && (st.st_mode & S_IFDIR)!=S_IFDIR)
                      {        //We are in wrong list. We must be inside fl but now we in dl.
                              ol->mode&=~S_IFDIR;
                              if(ol->prev)
                                      ol->prev->next=ol->next;
                              if(ol->next)
                                      ol->next->prev=ol->prev;
                              ol->next=ol->prev=NULL;
                              fl.add(ol);
                      }              
              ol->mode=(ol->mode & ~0777) | (st.st_mode & 0777);
              ol->size=st.st_size;
              n=n->next;
              delete namer;
              namer=n;
        }
  }
  
  bmark->animate();
  if(hd==NULL)
  {
              hd = (FList*)new_alloc(S_FList);
              hd->init("..", S_IFDIR | 0755, 0, st.st_uid, st.st_gid, st.st_mtime);
  }
  simple_flist_add(&dl, hd);
  ol = (FList*)new_alloc(S_FList);
  ol->init(".", S_IFDIR | 0755, 0, st.st_uid, st.st_gid, st.st_mtime);
  simple_flist_add(&dl, ol);

  return 1;
}

void FTP::make_empty_list()
{
    FList *ol;
    delete_vfs_list();
    ol=(FList*)new_alloc(S_FList);
    dl.next = fl.next = NULL;
    ol->init("..", S_IFDIR, 0, 0, 0, 5000);
    ol->next=NULL;
    simple_flist_add(&dl, ol);
    panel->dl.next=panel->base=panel->cur=dl.next;
    panel->curn=0;
}

char* FTP::get_dir_for_dnd()
{
    int l=strlen(curdir);
    char *buf=new char[l+256];
    sprintf(buf,"ftp://%s%s",com.hostname,curdir);
    xncdprintf(("FTP_get_dir_for_dnd: [%s]\n",buf));
    return buf;
}
