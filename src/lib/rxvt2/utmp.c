/*--------------------------------*-C-*---------------------------------*
 * File:	utmp.c
 *
 * Public:
 *	extern void cleanutent (void);
 *	extern void makeutent (const char * pty, const char * hostname);
 *
 * Private:
 *	get_tslot ();
 *	write_utmp ();
 *	update_wtmp ();
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 * ---------------------------------------------------------------------*/
/*{{{ notes: */
/*----------------------------------------------------------------------*
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions are applied.
 *
 * Revision 1.6  07DEC95:16	Piet W. Plomp,
 * wtmp support for (at least linux)
 *
 * Revision 1.55  1995/10/16  16:04:16  rgg
 * now works in Solaris 2.x and Linux 1.2.x
 *
 * Revision 1.5 08/09/1993 stempien
 * Something was wrong with the Linux support!
 * I fixed it using the provided utmp manipulation routines.
 * I also surrounded many varables that were not needed for Linux
 * in the BSD defines to reduce the memory needed to run.
 * I didn't touch the Sun part of the code so it should still work.
 *
 * Revision 1.4  1993/08/09  11:54:15  lipka
 * now works both on Linux and SunOs 4.1.3.
 * Brians clean-ups incorporated
 *
 * Revision 1.3  1993/08/09  07:16:42  lipka
 * nearly correct (running) linux-version.
 * Delete-Window is still a problem
 *
 * Revision 1.1  1993/08/07  18:03:53  lipka
 * Initial revision
 *
 * Clean-ups according to suggestions of Brian Stempien <stempien@cs.wmich.edu>
 *
 *    Bugs:
 *	UTMP should be locked from call to utmp_end() 'til makeutent() (?).
 *	Maybe the child should tell the parent, where the entry is made.
 *	Tested only on Linux.
 *
 *	Gives weird inital idle times. They go away after one uses the
 *	window, but......
 * ------------------------------------------------------------------------
 * This version has SYSV wtmp support for (at least Linux) added by:
 *    Piet W. Plomp,
 *    ICCE - Institute for educational technology,
 *    State University of Groningen, The Netherlands,
 *    <piet@icce.rug.nl> or (faster) <piet@idefix.icce.rug.nl>
 *
 * all WTMP specific code is #ifdef'd WTMP_SUPPORT, which currently depends
 * on UTMP_SUPPORT.
 * This code is valid and tested on linux (libc.so.4.6.27). but is
 * POSIX compliant.
 *
 * My additions are tagged with an entry like "... pwp 95-12-07", where the
 * former are my initials and the latter the date in yy-mm-dd format.
 *----------------------------------------------------------------------*/
/*}}}*/
/*{{{ includes, defines */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "feature.h"

#ifndef UTMP_SUPPORT
/* Dummy routines if utmp support isn't wanted */
void cleanutent (void) {}
void makeutent (const char * pty, const char * hostname) {}
#else /* UTMP_SUPPORT */

#include <stdio.h>
#include <string.h>

#ifdef HAVE_UTMPX_H
# include <utmpx.h>
# define USE_SYSV_UTMP
#else
# include <utmp.h>
# ifdef HAVE_SETUTENT
#  define USE_SYSV_UTMP
# endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_LASTLOG_H
# include <lastlog.h>
#endif
#include <pwd.h>

/* WTMP_SUPPORT added pwp 95-12-07 */
#define WTMP_SUPPORT

#include <errno.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifndef UTMP_FILENAME
# ifdef UTMP_FILE
#  define UTMP_FILENAME UTMP_FILE
# else
#  ifdef _PATH_UTMP
#   define UTMP_FILENAME _PATH_UTMP
#  else
#   define UTMP_FILENAME "/etc/utmp"
#  endif
# endif
#endif

#ifndef LASTLOG_FILENAME
# ifdef _PATH_LASTLOG
#  define LASTLOG_FILENAME _PATH_LASTLOG
# else
#  define LASTLOG_FILENAME "/usr/adm/lastlog"  /* only on BSD systems */
# endif
#endif

#ifndef WTMP_FILENAME
# ifdef WTMP_FILE
#  define WTMP_FILENAME WTMP_FILE
# else
#  ifdef _PATH_WTMP
#   define WTMP_FILENAME _PATH_WTMP
#  else
#   ifdef SYSV
#    define WTMP_FILENAME "/etc/wtmp"
#   else
#    define WTMP_FILENAME "/usr/adm/wtmp"
#   endif
#  endif
# endif
#endif

#ifndef TTYTAB_FILENAME
# ifdef TTYTAB
#  define TTYTAB_FILENAME TTYTAB_FILENAME
# else
#  define TTYTAB_FILENAME "/etc/ttytab"
# endif
#endif

#ifndef USER_PROCESS
# define USER_PROCESS 7
#endif
#ifndef DEAD_PROCESS
# define DEAD_PROCESS 8
#endif
/*}}}*/

/*{{{ extern functions referenced */
/*}}}*/

/*{{{ extern variables referenced */
/*}}}*/

/*{{{ extern variables declared here */
/*}}}*/

/*{{{ local variables */
/* don't go off end of ut_id & remember if an entry has been made */
static char ut_id [5] = "";
/*}}}*/

/*{{{ local functions referenced */
/*}}}*/

/*----------------------------------------------------------------------*/

/*
 * HAVE_SETUTENT corresponds to SYSV-style utmp support.
 * Without it corresponds to using BSD utmp support.
 *
 * SYSV-style utmp support is further divided in normal utmp support
 * and utmpx support (Solaris 2.x) by HAVE_UTMPX_H
 */
#ifdef USE_SYSV_UTMP

/*{{{ update_wtmp() */
/*
 *  added by pwp 95 12 07
 * called by makeutent() and cleanutent() below
 */
# ifdef WTMP_SUPPORT
#  ifdef HAVE_UTMPX_H
#   undef WTMP_FILENAME
#   define WTMP_FILENAME WTMPX_FILE
#   define update_wtmp updwtmpx
#  else		/* HAVE_UTMPX_H */
static void
update_wtmp (char * fname, struct utmp * putmp)
{
   int fd, retry = 10;		/* 10 attempts at locking */
   struct flock lck;		/* fcntl locking scheme */

   if ((fd = open (fname, O_WRONLY|O_APPEND, 0)) < 0)
     return;

   lck.l_whence = SEEK_END;	/* start lock at current eof */
   lck.l_len    = 0;		/* end at ``largest possible eof'' */
   lck.l_start  = 0;
   lck.l_type   = F_WRLCK;	/* we want a write lock */

#   ifndef EACCESS
#    ifdef EAGAIN
#     define EACCESS EAGAIN
#    endif
#   endif

   /* attempt lock with F_SETLK - F_SETLKW would cause a deadlock! */
   while (retry--)
     if ((fcntl (fd, F_SETLK, &lck) < 0) && errno != EACCESS) {
	close (fd);
	return;			/* failed for unknown reason: give up */
     }
   write (fd, putmp, sizeof(struct utmp));

   /* unlocking the file */
   lck.l_type = F_UNLCK;
   fcntl (fd, F_SETLK, &lck);

   close (fd);
}
#  endif	/* HAVE_UTMPX_H */
# else		/* WTMP_SUPPORT */
#  define update_wtmp(fname,putmp) ((void)0)
# endif	/* WTMP_SUPPORT */
/*}}}*/

/*{{{ makeutent() - make a utmp entry */
void
makeutent (const char * pty, const char * hostname)
{
   struct passwd * pwent = getpwuid (getuid ());
#ifdef HAVE_UTMPX_H
   struct utmpx utmp;
   struct utmp utmp2;
   memset (&utmp, 0, sizeof(struct utmpx));
#else
   struct utmp utmp;
   memset (&utmp, 0, sizeof(struct utmp));
#endif

   if (!strncmp (pty, "/dev/", 5)) pty += 5;	/* skip /dev/ prefix */
   if (!strncmp (pty, "pty", 3) || !strncmp (pty, "tty", 3))
     strncpy (ut_id, (pty+3), sizeof(ut_id));	/* bsd naming */
   else
     {
	int n;
	if (sscanf (pty, "pts/%d", &n) == 1)
	  sprintf (ut_id, "vt%02x", n);		/* sysv naming */
	else
	  {
	     print_error ("can't parse tty name \"%s\"", pty);
	     ut_id [0] = '\0';	/* entry not made */
	     return;
	  }
     }
   strncpy (utmp.ut_id, ut_id, sizeof(utmp.ut_id));
   utmp.ut_type = DEAD_PROCESS;

#ifdef HAVE_UTMPX_H
   getutmp (&utmp, &utmp2);
   getutid (&utmp2);		/* position to entry in utmp file */
#else
   getutid (&utmp);		/* position to entry in utmp file */
#endif

   /* set up the new entry */
   strncpy (utmp.ut_id, ut_id, sizeof(utmp.ut_id));
   strncpy (utmp.ut_line, pty, sizeof(utmp.ut_line));
   strncpy (utmp.ut_name, pwent->pw_name, sizeof(utmp.ut_name));
   strncpy (utmp.ut_user, pwent->pw_name, sizeof(utmp.ut_user));
   strncpy (utmp.ut_host, hostname, sizeof(utmp.ut_host));
#ifndef linux
     {
	char * colon = strrchr (utmp.ut_host, ':');
	if (colon) *colon = '\0';
     }
#endif

   utmp.ut_type = USER_PROCESS;
   utmp.ut_pid = getpid ();

#ifdef HAVE_UTMPX_H
   utmp.ut_session = getsid (0);
   utmp.ut_xtime = time (NULL);
   utmp.ut_tv.tv_usec = 0;
#else
   utmp.ut_time = time (NULL);
#endif

   /*
    * write a utmp entry to the utmp file
    */
   utmpname (UTMP_FILENAME);
#ifdef HAVE_UTMPX_H
   getutmp (&utmp, &utmp2);
   pututline (&utmp2);
   pututxline (&utmp);
#else
   /* if (!utmpInhibit) */
   pututline (&utmp);
#endif
   update_wtmp (WTMP_FILENAME, &utmp);
   endutent ();			/* close the file */
}
/*}}}*/

/*{{{ cleanutent() - remove a utmp entry */
/*----------------------------------------------------------------------*
 * there is a nice function "endutent" defined in <utmp.h>;
 * like "setutent" it takes no arguments, so I think it gets all information
 * from library-calls.
 * That's why "setutent" has to be called by the child-process with
 * file-descriptors 0/1 set to the pty. But that child execs to the
 * application-program and therefore can't clean it's own utmp-entry!(?)
 * The master on the other hand doesn't have the correct process-id
 * and io-channels... I'll do it by hand:
 * (what's the position of the utmp-entry, the child wrote? :-)
 *----------------------------------------------------------------------*/
void
cleanutent (void)
{
#ifdef HAVE_UTMPX_H
   struct utmp utmp;
   struct utmpx utmpx;

   if (!ut_id[0]) return;	/* entry not made */

   utmpname (UTMP_FILENAME);
   setutent ();
   if (getutid (&utmp) == NULL) return;
   utmp.ut_type = DEAD_PROCESS;
   utmp.ut_time = time (NULL);
   pututline (&utmp);
   getutmpx (&utmp, &utmpx);
   update_wtmp (WTMP_FILENAME, &utmpx);
   endutent ();

#else	/* HAVE_UTMPX_H */
   struct utmp * putmp;
   pid_t pid = getpid ();

   if (!ut_id[0]) return;	/* entry not made */

   utmpname (UTMP_FILENAME);
   setutent ();
   /*
    * The following code waw copied from the poeigl-1.20 login/init package.
    * Special thanks to poe for the code examples.
    */
   while ((putmp = getutent ()) != NULL)
     {
	if (putmp->ut_pid == pid)
	  {
	     putmp->ut_type = DEAD_PROCESS;
	     putmp->ut_pid = 0;
	     putmp->ut_user[0] = '\0';
	     putmp->ut_time = time (NULL);
	     pututline (putmp);
             update_wtmp (WTMP_FILENAME, putmp);
	     break;
	  }
     }
   endutent ();
#endif	/* HAVE_UTMPX_H */
}
/*}}}*/

#else	/* USE_SYSV_UTMP */
/*{{{ BSD utmp support */
static int utmp_pos = 0;		/* position of utmp-stamp */

/*----------------------------------------------------------------------*
 * get_tslot() - grabbed from xvt-1.0 - modified by David Perry
 *
 * find ttyname in /etc/ttytab and return a slot number that can be used to
 * access the utmp file.  Can't use ttyslot() because the tty name is not
 * that of fd 0.
 *----------------------------------------------------------------------*/
static int
get_tslot (const char * ttyname)
{
   char buf [256], name [256];
   FILE *fd;

   if ((fd = fopen (UTMP_FILENAME, "r")) != NULL)
     {
	int i;
	for (i = 1; fgets (buf, sizeof(buf), fd) != NULL; i++)
	  {
	     if (*buf == '#' || sscanf (buf, "%s", name) != 1) continue;
	     if (!strcmp (ttyname, name))
	       {
		  fclose (fd);
		  return i;
	       }
	  }
	fclose (fd);
     }
   return -1;
}

/*
 * write utmp entry to UTMP_FILENAME
 */
static int
write_utmp (struct utmp * putmp)
{
   int rval = 0;
   FILE * fd;

   if ((fd = fopen (UTMP_FILENAME, "r+")) != NULL)
     {
	utmp_pos = get_tslot (putmp->ut_line) * sizeof(struct utmp);
	if (utmp_pos >= 0)
	  {
	     fseek (fd, utmp_pos, 0);
	     fwrite (putmp, sizeof(struct utmp), 1, fd);
	     rval = 1;
	  }
	fclose (fd);
     }
   return rval;
}

/*
 * make a utmp entry
 */
void
makeutent (const char * pty, const char * hostname)
{
   struct passwd * pwent = getpwuid (getuid ());
   struct utmp utmp;

   memset (&utmp, 0, sizeof(struct utmp));

   if (!strncmp (pty, "/dev/", 5)) pty += 5;	/* skip /dev/ prefix */
   if (!strncmp (pty, "pty", 3) || !strncmp (pty, "tty", 3))
     strncpy (ut_id, (pty+3), sizeof(ut_id));	/* bsd naming */
   else
     {
	print_error ("can't parse tty name \"%s\"", pty);
	ut_id [0] = '\0';	/* entry not made */
	return;
     }

   strncpy (utmp.ut_line, pty, sizeof(utmp.ut_line));
   strncpy (utmp.ut_name, pwent->pw_name, sizeof(utmp.ut_name));
   strncpy (utmp.ut_host, hostname, sizeof(utmp.ut_host));
   utmp.ut_time = time (NULL);

   if (write_utmp (&utmp) < 0)
     ut_id [0] = '\0';		/* entry not made */
}

/*
 * remove a utmp entry
 */
void
cleanutent (void)
{
   FILE * fd;

   if (!ut_id [0] && (fd = fopen (UTMP_FILENAME, "r+")) != NULL)
     {
	struct utmp utmp;
	memset (&utmp, 0, sizeof(struct utmp));
	fseek (fd, utmp_pos, 0);
	fwrite (&utmp, sizeof(struct utmp), 1, fd);
	fclose (fd);
     }
}
/*}}}*/
#endif	/* USE_SYSV_UTMP */
#endif	/* UTMP_SUPPORT */
/*----------------------- end-of-file (C source) -----------------------*/
