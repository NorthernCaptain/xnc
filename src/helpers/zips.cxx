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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
                        "Sep","Oct","Nov","Dec"};

char attrs[]="rwxrwxrwx";

int main(int argc, char **argv)
{
        struct stat dstat;
        struct tm* tt;
        char fname[2048];
        int i,mask;
        if(argc<3)
                return -1;
        if(stat(argv[1],&dstat)==-1)
                return -1;
        strcpy(fname,strrchr(argv[1],'/')==NULL ? argv[1] : strrchr(argv[1],'/'));
        fname[strlen(fname)-strlen(argv[2])]=0;
        for(i=0,mask=0400;mask>0;i++,mask>>=1)
                if((dstat.st_mode & mask)==0)
                        attrs[i]='-';
        tt=localtime(&dstat.st_mtime);
        printf("%s %02d %s %04d  %02d:%02d %s\n", attrs, tt->tm_mday, months[tt->tm_mon],
                1900+tt->tm_year, tt->tm_hour, tt->tm_min, fname);
        return 0;
}
