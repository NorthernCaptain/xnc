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
#include <sys/types.h>     
#include <stdio.h>
#include <sys/param.h>
#include <grp.h>

int   main()
{
  struct group *gr;
  while ((gr = getgrent()) != NULL)
    printf("%s %d\n", gr->gr_name, gr->gr_gid);
  endgrent();
  return 0;
}




