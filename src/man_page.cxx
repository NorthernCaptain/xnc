
/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
*  email:   leo@xnc.dubna.su
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
#include "panel.h"
#include "c_externs.h"
#include "query_windows.h"
#include <sys/wait.h>

extern char dbuf[];
extern void viewhighlight(char *);
FList *manf = NULL;


void   man_page()
{
  int    rcode;
  char  *buf;
  char  *buf2;
  struct stat st;
  int    fp, j = 0, i = 0;
  int    fl = 0;
  char   tmps[200];
  char   str[200];
  if (strcmp(viewname, "internal") != 0 && strcmp(editname, "internal") != 0)
    {
      tt_printf("man %s\n", dbuf);
      panel->raise_terminal_if_need();
      return;
    }
  sprintf(str, "man %s >%s/xnc.tmp 2>/dev/null", dbuf, syspath);
  sprintf(tmps, "%s/xnc.tmp", syspath);
  rcode = psystem(str);
  stat(tmps, &st);
  if (st.st_size > 0)
    {
      manf = new FList("Man_page", st.st_mode, st.st_size, st.st_uid, st.st_gid, st.st_mtime);
      buf = new char[st.st_size + 1];
      fp = open(tmps, O_RDWR);
      read(fp, buf, st.st_size);
      close(fp);
      buf[st.st_size] = 0;
      buf2 = new char[st.st_size + 1];
      while (buf[i] != 0)
        {
          if (buf[i + 1] == 8)
            {
              if (fl == 0)
                {
                  buf2[j++] = 8;
                  fl = 1;
                };
            }
          else if (fl)
            {
              buf2[j++] = 7;
              fl = 0;
            };

          buf2[j++] = buf[i++];
          if (buf[i] == 8)
            if (buf[i - 1] != '_')
              i += 2;
            else
              {
                buf2[j - 1] = buf[i + 1];
                i += 2;
              };
        };
      buf[j] = 0;
      manf->size = j;
      delete buf;
      fp = open(tmps, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      write(fp, buf2, j);
      close(fp);
      delete manf;
      delete buf2;
      viewhighlight(tmps);
    }
  else
    {
      unlink(tmps);
      simple_mes(_("Man report"), _("No manual page for this command."));
    };
}
