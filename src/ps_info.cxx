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

extern char *panlist[];

int    getps()
{
  char   buf[255];
  int    i = 0, k;
  FILE  *p;
  signal(SIGCHLD,SIG_DFL);
#if defined (__svr4__)
  p = (FILE *) popen("ps -ef", "r");
#else
  p = (FILE *) popen("ps ax", "r");
#endif
  if (p)
    {
      while (fgets(buf, 255, p) != NULL)
        {
          k = strlen(buf);
          if (k > 0)
            {
              buf[k - 1] = 0;
              panlist[i] = new char[k];
              strcpy(panlist[i++], buf);
            }
        }
      (void)pclose(p);
      set_cmdchild_signal();
      return i;
    }
  return 0;
}

void   delps(int max)
{
  for (int i = 0; i < max; i++)
    delete panlist[i];
}

int    getdf(Panel * pan)
{
  char   buf[255];
  int    i = 0, k;
  signal(SIGCHLD,SIG_DFL);
  FILE  *p = (FILE *) popen("df", "r");
  if (p)
    {
      while (fgets(buf, 255, p) != NULL)
        {
          k = strlen(buf);
          if (k > 0)
            {
              buf[k - 1] = 0;
              panlist[i] = new char[k];
              strcpy(panlist[i++], buf);
              pan->add_element(panlist[i - 1]);
            }
        }
      (void)pclose(p);
      set_cmdchild_signal();
      return i;
    }
  return 0;
}

int    getfree(Panel * pan)
{
  char   buf[255];
  int    i = 0, k;
  signal(SIGCHLD,SIG_DFL);
#if defined (__linux__)
  FILE  *p = (FILE *) popen("free -t", "r");
#else
  FILE  *p = (FILE *) popen("vmstat", "r");
#endif
  if (p)
    {
      while (fgets(buf, 255, p) != NULL)
        {
          k = strlen(buf);
          if (k > 0)
            {
              buf[k - 1] = 0;
              panlist[i] = new char[k];
              strcpy(panlist[i++], buf);
              pan->add_element(panlist[i - 1]);
            }
        }
      (void)pclose(p);
      set_cmdchild_signal();
      return i;
    }
  return 0;
}
