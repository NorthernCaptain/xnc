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
#include <pwd.h>

int   main()
{
  struct passwd *pw;
  while ((pw = getpwent()) != NULL)
    printf("%s %d %d\n", pw->pw_name, pw->pw_uid, pw->pw_gid);
  endpwent();
  return 0;
}
