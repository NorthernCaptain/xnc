/****************************************************************************
*  Copyright (C) 1998 by Leo Khramov
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
/* $Id: ftpdiving.cxx,v 1.2 2002/08/05 10:43:15 leo Exp $ */
//This file contains routines for FTP File System.
//There are diving routines, i mean diving into subdirs through
//the directory tree and do recursive copy/del ... operations.

#include <xh.h>                //-> ../xh.h
#include <ftplib.h>           //-> ./ftplib.h
#include <systemfuncs.h>

#define MAX_STACKED    256

static char tmpstr[2048];

static char *stack_arr[MAX_STACKED];
static int  stack_lvl=-1;

int push_string(char *str)
{
    if(stack_lvl+1>=MAX_STACKED)
        return 0;
    stack_arr[++stack_lvl]=strdup(str);
    return 1;
}
    

int pop_string(char *str)
{
    if(stack_lvl<0)
        return 0;
    strcpy(str, stack_arr[stack_lvl]);
    free(stack_arr[stack_lvl]);
    stack_lvl--;
    return 1;
}

char *extract_last(char *path,char *to)
{
 int l=strlen(path);
 if(path[l-1]=='/')
         path[l-1]=0;
 char *b=strrchr(path,'/');
 strcpy(to, b ? b+1 : path);
 if(path[l-1]==0) 
        {
                path[l-1]='/';
                path[l]=0;
        };
 return to;
}

void addpathcontent(char *to,char *cont)
{
 int l;
 if(to[strlen(to)-1]!='/')
  strcat(to,"/");
 strcat(to,cont);
 l=strlen(to);
 if(to[l-1]!='/')
   {to[l++]='/';to[l++]='\0';};
}

void upperpath(char *curdir)
{
 int l=strlen(curdir);
 if(l>1 && curdir[l-1]=='/')
   curdir[l-1]='\0';
 char *bb=strrchr(curdir,'/');
 if(bb==NULL) bb=curdir;
 *bb=0;
 if(strlen(curdir)==0) strcpy(curdir,"/");
}

int dummysize;

int Ftpget_direntry(FILE* fp,char *fname,int *mode,char *tmp, int *size=&dummysize)
{
 static char link2[1024]="";
 char dummychr[128];
 int dummy,l;
 char *s;
 if(fp==NULL)
 {
         strcpy(fname,link2);
         return 1;
 }
 if(fgets(tmp,255,fp)==NULL) return 0;
 sscanf(tmp, "%d %o %X %s %s %[^\n]\n", size, mode, &dummy, dummychr, dummychr, fname);
  l=strlen(fname);
  if(fname[l-1]=='\r')
          fname[l-1]=0;
 if(*mode & S_IFLNK) //If link then break it into two parts
 {
     s=strstr(fname," -> ");
     if(s==NULL)         //Error in LS file (link detected but not found)
        return 0;       
     *s=0;
     strncpy(link2,s+4,1023);
 }
 return 1;
}
 
int Ftpget_dirdir(char *lsfile,char *to,int coun,char *tmp1)
{
 FILE *fp=fopen(lsfile,"r");
 if(fp==NULL) return 0;
 int mode;
 while(Ftpget_direntry(fp,to,&mode,tmp1))
 {
   if(mode & S_IFLNK)
           continue;
   if((mode & S_IFDIR)==S_IFDIR)
   {
           if(strcmp(to,".")==0 || strcmp(to,"..")==0) continue;
           if(coun==0)
           {
                 fclose(fp);
                 return 1;        //Waw!!! We found it...
           }
           coun--;
   }
 }
 return 0;   //Not found :(
}

/*Recursive copy dir through FTP (from local to remote)  DFS->FTP
* return 0 if success
* return 255 if err
* Parameters:
*        *netb - pointer to opened FTP connection
*        from  - local path 'from' that we copy files
*        to      - remote path to place files
*  On error: error reason placed not to netb->response but to 'ftp_reason'.
*/


int FtpPutDir(netbuf* netb,char *from, char *to)
{
  DIR   *d;
  struct dirent *dr;
  struct stat dstat;
  int dcoun=0;

   if(chdir(from)==-1)
   {
           strcpy(ftp_reason,strerror(errno));
           return 255;
   }
  d = opendir(".");        //Open local directory.
  if (d == NULL)
    {
      strcpy(ftp_reason,strerror(errno));
      chdir("..");
      return 255;
    }
  if(FtpMkdir(to,netb)==0 && strstr(netb->response,"exist")==NULL)  //If error creating remote dir
  {                                                                    //And directory does not exist -> error
          strcpy(ftp_reason,netb->response);
          chdir("..");
          closedir(d);
          return 255;
  }
  if(!FtpChdir(to,netb))
  {
          strcpy(ftp_reason,netb->response);
          chdir("..");
          closedir(d);
          return 255;
  }        
  while ((dr = readdir(d)) != NULL)        //Copy files only not subdirs
    {
      dcoun++;
    if (strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0)        //Don't even think about copying this..
      {
        lstat(dr->d_name, &dstat);
        if(S_ISLNK(dstat.st_mode))        //We can't create symlinks on remote site :(
        {
               sprintf(ftp_reason, "[%s]: Can't copy symlinks to FTP",dr->d_name);
               closedir(d);
               chdir("..");
               FtpCdup(netb);
               return 255;
       }
       if (S_ISDIR(dstat.st_mode) == 0)        //Copy plane file
         {
            if(FtpPut(dr->d_name,dr->d_name,'I',netb, dstat.st_size)==0)
            {
               strcpy(ftp_reason,netb->response);
               closedir(d);
               chdir("..");
               FtpCdup(netb);
               return 255;
            }
         } else        //Copy subdir to remote.
         {
            strcpy(tmpstr,dr->d_name);
            closedir(d);
            if(FtpPutDir(netb,tmpstr,tmpstr)==255)
            {
               chdir("..");
               FtpCdup(netb);
               return 255;
            }
           d = opendir(".");
           if (d == NULL)
             {
              strcpy(ftp_reason,strerror(errno));
              chdir("..");
              FtpCdup(netb);
              return 255;
             }
           xnc_seekdir(d, dcoun);
        }
      }      
    } 
   closedir(d);
   if(chdir("..")==-1)
   {
              strcpy(ftp_reason,strerror(errno));
              FtpCdup(netb);
              return 255;
   }
   if(!FtpCdup(netb))
   {
               strcpy(ftp_reason,netb->response);
               return 255;
   }
   return 0;
}
           
  
  
/*Recursive copy dir through FTP (from remote to local)  FTP->DFS
* return 0 if success
* return 255 if err
* Parameters:
*        *netb - pointer to opened FTP connection
*        from  - remote path 'from' that we copy files
*        to      - local path to place files
*        TMPFILE - generated, unique file name uses as tempfile for 'ls' commands (overwritten)
*        tmp1  -  char tmp buffer
*        ftpname - char tmp buffer for ftp file names.
*  On error: error reason placed not to netb->response but to 'ftp_reason'.
*/

int FtpGetDir(netbuf* netb,char *from,char *to,char *TMPFILE,char *tmp1,char *ftpname)
{
 int coun,mode,fsiz=0;
 if(!FtpChdir(from,netb))
 {
   strcpy(ftp_reason,netb->response);
   return 255;
 }
 if(::mkdir(to,0755)==-1 && errno!=EEXIST)
  {
        FtpCdup(netb);
        strcpy(netb->response, "Error creating local directory!");
        strcpy(ftp_reason,netb->response);
        return 255;
  }
 if(::chdir(to)==-1)
  {
        FtpCdup(netb);
        strcpy(netb->response, "Error changing to local directory!");
        strcpy(ftp_reason,netb->response);
        return 255;
  }
 if(!FtpLS(TMPFILE,NULL,netb))
  {
  
        strcpy(ftp_reason,netb->response);
        FtpCdup(netb);
        ::chdir("..");
        return 255;
  }
 FILE *fp=fopen(TMPFILE,"r");
 if(fp==NULL)
 {
         FtpCdup(netb);
         ::chdir("..");
         strcpy(netb->response, "Error opening local listing!");
         strcpy(ftp_reason,netb->response);
         unlink(TMPFILE);
         return 255;
 }
 
//First we copy all files from current dir and skip subdirs
 while(Ftpget_direntry(fp,ftpname,&mode,tmp1,&fsiz))
 {
  if((mode & S_IFDIR)==0)
    {
          if(mode & S_IFLNK)
          {
                  Ftpget_direntry(NULL,tmp1,NULL,NULL);  //Get second part of link to tmp1
                  if(symlink(tmp1,ftpname)==-1)
                  {
                    strcpy(ftp_reason,strerror(errno));
                    unlink(TMPFILE);
                    FtpCdup(netb);
                    ::chdir("..");
                    fclose(fp);
                    return 255;
                 }
                 chmod(ftpname,mode);
              continue;
          }
          if(!FtpBinGet(ftpname,ftpname,mode,netb,fsiz))
          {
                  unlink(TMPFILE);
                  strcpy(ftp_reason,netb->response);
                  FtpCdup(netb);
                  ::chdir("..");
                  fclose(fp);
                  return 255;
          }
    }
 }
 fclose(fp);
 coun=0;

//Now start scan subdirs and copying it
 while(Ftpget_dirdir(TMPFILE,ftpname,coun,tmp1))
 {
   addpathcontent(from,ftpname);
   addpathcontent(to,ftpname);
   strcat(TMPFILE,"1");
   if(FtpGetDir(netb,from,to,TMPFILE,tmp1,ftpname))
     {
                  TMPFILE[strlen(TMPFILE)-1]='\0';
                  unlink(TMPFILE);
                  FtpCdup(netb);
                  ::chdir("..");
                  upperpath(from);
                  upperpath(to);
                  return 255;
     }
   upperpath(from);
   upperpath(to);
   TMPFILE[strlen(TMPFILE)-1]='\0';
   coun++;
 }
 FtpCdup(netb);
 strcpy(ftp_reason,netb->response);
 unlink(TMPFILE);
 ::chdir("..");
 return 0;
} 

/*Recursive delete dir through FTP
* return 0 if success
* return 255 if err
* Parameters:
*        *netb - pointer to opened FTP connection
*        from  - remote path 'from' that we delete files
*        TMPFILE - generated, unique file name uses as tempfile for 'ls' commands (overwritten)
*        tmp1  -  char tmp buffer
*        ftpname - char tmp buffer for ftp file names.
*  On error: error reason placed not to netb->response but to 'ftp_reason'.
*/

int FtpDelDir(netbuf* netb,char *from,char *TMPFILE,char *tmp1,char *ftpname)
{
 int coun,mode;                       
 if(FtpRmdir(from,netb))
 {
         strcpy(ftp_reason,netb->response);
         return 0;
 }
 if(!FtpChdir(from,netb))
 {
   strcpy(ftp_reason,netb->response);
   return 255;
 }
 if(!FtpLS(TMPFILE,NULL,netb))
  {
        strcpy(ftp_reason,netb->response);
        FtpCdup(netb);
        return 255;
  }
 FILE *fp=fopen(TMPFILE,"r");
 if(fp==NULL)
 {
         FtpCdup(netb);
         strcpy(netb->response, "Error opening local listing!");
         strcpy(ftp_reason,netb->response);
         unlink(TMPFILE);
         return 255;
 }
 
//First we delete all files from current dir and skip subdirs
 while(Ftpget_direntry(fp,ftpname,&mode,tmp1))
 {
  if((mode & S_IFDIR)==0)
    {
          if(!FtpDelete(ftpname,netb))
          {
                  unlink(TMPFILE);
                  strcpy(ftp_reason,netb->response);
                  FtpCdup(netb);
                  fclose(fp);
                  return 255;
          }
    }
 }
 fclose(fp);
 coun=0;

//Now start scan subdirs and deleting it
 while(Ftpget_dirdir(TMPFILE,ftpname,coun,tmp1))
 {

   if(!push_string(from))
   {
       unlink(TMPFILE);
       strcpy(ftp_reason, "Internal stack overflow");
       FtpCdup(netb);
       return 255;
   }
   
   strcpy(from,ftpname);
   
   strcat(TMPFILE,"1");
   if(FtpDelDir(netb,from,TMPFILE,tmp1,ftpname))
     {
         TMPFILE[strlen(TMPFILE)-1]='\0';
         unlink(TMPFILE);
         FtpCdup(netb);
         upperpath(from);
         return 255;
     }
   pop_string(from);
   TMPFILE[strlen(TMPFILE)-1]='\0';
   coun++;
 }
 FtpCdup(netb);
 extract_last(from,tmp1);        //extracting last name from path
 if(!FtpRmdir(tmp1,netb))
 {
     unlink(TMPFILE);
     strcpy(ftp_reason,netb->response);
     return 255;
 }
 strcpy(ftp_reason,netb->response);
 unlink(TMPFILE);
 return 0;
} 

