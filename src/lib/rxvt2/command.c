/*--------------------------------*-C-*---------------------------------*
 * File:        command.c
 */
/*{{{ notes: */
/*----------------------------------------------------------------------*
 * Copyright 1992 John Bovey, University of Kent at Canterbury.
 *
 * You can do what you like with this source code as long as
 * you don't try to make money out of it and you include an
 * unaltered copy of this message (including the copyright).
 *
 * This module has been very heavily modified by R. Nation
 * <nation@rocket.sanders.lockheed.com>
 * No additional restrictions are applied
 *
 * Additional modification by Garrett D'Amore <garrett@netcom.com> to
 * allow vt100 printing.  No additional restrictions are applied.
 *
 * Integrated modifications by Steven Hirsch <hirsch@emba.uvm.edu> to
 * properly support X11 mouse report mode and support for DEC
 * "private mode" save/restore functions.
 *
 * Integrated key-related changes by Jakub Jelinek <jj@gnu.ai.mit.edu>
 * to handle Shift+function keys properly.
 * Should be used with enclosed termcap / terminfo database.
 *
 * Extensive modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions.
 *
 * Further modification and cleanups for Solaris 2.x and Linux 1.2.x
 * by Raul Garcia Garcia <rgg@tid.es>. No additional restrictions.
 *
 * As usual, the author accepts no responsibility for anything, nor does
 * he guarantee anything whatsoever.
 *----------------------------------------------------------------------*/
/*}}}*/
/*{{{ includes: */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "main.h"
#include "xdefaults.h"
#include "../au/x_actions.h"
#ifdef OFFIX_DND
# include <X11/Xatom.h>
# define DndFile        2
# define DndDir                5
# define DndLink        7
#endif

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

#include <X11/keysym.h>

#ifndef NO_XLOCALE
# if (XtSpecificationRelease < 6)
#  define NO_XLOCALE
# else
#  define X_LOCALE
#  include <X11/Xlocale.h>
# endif
#endif        /* NO_XLOCALE */

#include <errno.h>
#include <signal.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_UNIX_H
#include <unix.h>
#endif

#include <sys/types.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <stdarg.h>

/* #define USE_GETGRNAME */
/*
 * this seems like a bad way to go, since there's no guarantee that
 * /dev/tty belongs to the group "tty" and not "system" or "wheel"
 */

int rxvt_ttyfd=-1;

extern int main_show;
int vt_show=1;
int focus_fl=0;
extern Window Main;
extern int    (*xnc_event)(XEvent*);
extern void   (*xnc_setsignals)();
extern int    (*xnc_look_for_key)(XEvent*);
extern int    (*xnc_process_special_action_in_term)(XEvent*);
extern void   (*xnc_setdir)();
extern void   (*xnc_work_anim)();
extern void   (*xnc_look_at_pipes)(fd_set*,int);
extern int    (*xnc_set_pipes)(fd_set*, int);
extern int    (*xnc_is_my_child)(int,int);
extern void   (*xnc_run_once)();
extern void   (*xnc_switch_term)();
int onlyrun=1;
int disable_modifiers=0;
extern Window main_win;
int xnc_now_on_top=1;

static short    changettyowner = 1;

#ifdef USE_GETGRNAME
# include <grp.h>
#endif

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

#if defined (__svr4__)
# include <sys/resource.h>        /* for struct rlimit */

#if defined(PTYS_ARE_PTMX) && !defined(__CYGWIN32__)
# include <sys/stropts.h>      /* for I_PUSH */
#endif

#define _NEW_TTY_CTRL                /* to get proper defines in <termios.h> */
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#else
# include <sgtty.h>
#endif

#include <sys/wait.h>
#include <sys/stat.h>

#include "command.h"
#include "debug.h"
#include "graphics.h"
#include "grkelot.h"
#include "menubar.h"
#include "scrollbar.h"
#include "screen.h"
#include "defaults.h"

/*}}}*/

/* #define DEBUG_TTYMODE */
/* #define DEBUG_CMD */

/*{{{ terminal mode defines: */
/* use the fastest baud-rate */
#ifdef B38400
# define BAUDRATE        B38400
#else
# ifdef B19200
#  define BAUDRATE        B19200
# else
#  define BAUDRATE        B9600
# endif
#endif

/* Disable special character functions */
#ifdef _POSIX_VDISABLE
# define VDISABLE        _POSIX_VDISABLE
#else
# define VDISABLE        255
#endif

/*----------------------------------------------------------------------*
 * system default characters if defined and reasonable
 */
#ifndef CINTR
# define CINTR                '\003'                /* ^C */
#endif
#ifndef CQUIT
# define CQUIT                '\034'                /* ^\ */
#endif
#ifndef CERASE
# define CERASE                '\010'                /* ^H */
#endif
#ifndef CKILL
# define CKILL                '\025'                /* ^U */
#endif
#ifndef CEOF
# define CEOF                '\004'                /* ^D */
#endif
#ifndef CSTART
# define CSTART                '\021'                /* ^Q */
#endif
#ifndef CSTOP
# define CSTOP                '\023'                /* ^S */
#endif
#ifndef CSUSP
# define CSUSP                '\032'                /* ^Z */
#endif
#ifndef CDSUSP
# define CDSUSP                '\031'                /* ^Y */
#endif
#ifndef CRPRNT
# define CRPRNT                '\022'                /* ^R */
#endif
#ifndef CFLUSH
# define CFLUSH                '\017'                /* ^O */
#endif
#ifndef CWERASE
# define CWERASE        '\027'                /* ^W */
#endif
#ifndef CLNEXT
# define CLNEXT                '\026'                /* ^V */
#endif

#ifndef VDISCRD
# ifdef VDISCARD
#  define VDISCRD        VDISCARD
# endif
#endif

#ifndef VWERSE
# ifdef VWERASE
#  define VWERSE        VWERASE
# endif
#endif
/*}}}*/

/*{{{ defines: */

#define KBUFSZ                8        /* size of keyboard mapping buffer */
#define STRING_MAX        512        /* max string size for process_xterm_seq() */
#define ESC_ARGS        32        /* max # of args for esc sequences */

/* a large REFRESH_PERIOD causes problems with `cat' */
#define REFRESH_PERIOD        1

#ifndef REFRESH_PERIOD
# define REFRESH_PERIOD        10
#endif

#ifndef MULTICLICK_TIME
# define MULTICLICK_TIME        500
#endif

/* time factor to slow down a `jumpy' mouse */
#define MOUSE_THRESHOLD                50
#define CONSOLE                "/dev/console"        /* console device */

/*
 * key-strings: if only these keys were standardized <sigh>
 */
#ifdef LINUX_KEYS
# define KS_HOME        "\033[1~"        /* Home == Find */
# define KS_END                "\033[4~"        /* End == Select */
#else
# define KS_HOME        "\033[7~"        /* Home */
# define KS_END                "\033[8~"        /* End */
#endif

/* and this one too! */
#ifdef NO_DELETE_KEY
# undef KS_DELETE                        /* use X server definition */
#else
# ifndef KS_DELETE
#  define KS_DELETE        "\033[3~"        /* Delete = Execute */
# endif
#endif

/*
 * ESC-Z processing:
 *
 * By stealing a sequence to which other xterms respond, and sending the
 * same number of characters, but having a distinguishable sequence,
 * we can avoid having a timeout (when not under an rxvt) for every login
 * shell to auto-set its DISPLAY.
 *
 * This particular sequence is even explicitly stated as obsolete since
 * about 1985, so only very old software is likely to be confused, a
 * confusion which can likely be remedied through termcap or TERM. Frankly,
 * I doubt anyone will even notice.  We provide a #ifdef just in case they
 * don't care about auto-display setting.  Just in case the ancient
 * software in question is broken enough to be case insensitive to the 'c'
 * character in the answerback string, we make the distinguishing
 * characteristic be capitalization of that character. The length of the
 * two strings should be the same so that identical read(2) calls may be
 * used.
 */
#define VT100_ANS        "\033[?1;2c"        /* vt100 answerback */
#ifndef ESCZ_ANSWER
# define ESCZ_ANSWER        VT100_ANS        /* obsolete ANSI ESC[c */
#endif
/*}}}*/

/*{{{ extern functions referenced */
extern void cleanutent (void);
extern void makeutent (const char * pty, const char * hostname);
/*}}}*/
/* extern variables referenced */
/* extern variables declared here */
/*{{{ local variables */
static char * ptydev = NULL, * ttydev = NULL;        /* pty/tty name */
static int cmd_fd = -1;                /* file descriptor connected to the command */
static pid_t cmd_pid = -1;        /* process id if child */
static int Xfd = -1;                /* file descriptor of X server connection */
static unsigned int num_fds = 0;/* number of file descriptors being used */
static struct stat ttyfd_stat;        /* original status of the tty we will use */

#ifdef META8_OPTION
static unsigned char meta_char = 033;        /* Alt-key prefix */
#endif

/* DEC private modes */
#define PrivMode_132                (1LU<<0)
#define PrivMode_132OK                (1LU<<1)
#define PrivMode_rVideo                (1LU<<2)
#define PrivMode_relOrigin        (1LU<<3)
#define PrivMode_Screen                (1LU<<4)
#define PrivMode_Autowrap        (1LU<<5)
#define PrivMode_aplCUR                (1LU<<6)
#define PrivMode_aplKP                (1LU<<7)
#define PrivMode_BackSpace        (1LU<<8)
#define PrivMode_ShiftKeys        (1LU<<9)
#define PrivMode_VisibleCursor        (1LU<<10)
#define PrivMode_MouseX10        (1LU<<11)
#define PrivMode_MouseX11        (1LU<<12)
/* too annoying to implement X11 highlight tracking */
/* #define PrivMode_MouseX11Track        (1LU<<13) */
#define PrivMode_scrollBar        (1LU<<14)
#define PrivMode_menuBar        (1LU<<15)

#define PrivMode_mouse_report        (PrivMode_MouseX10|PrivMode_MouseX11)
#define PrivMode(test,bit) do {\
if (test) PrivateModes |= (bit); else PrivateModes &= ~(bit);} while (0)

#define PrivMode_Default \
(PrivMode_Autowrap|PrivMode_aplKP|PrivMode_ShiftKeys|PrivMode_VisibleCursor)

static unsigned long PrivateModes = PrivMode_Default;
static unsigned long SavedModes = PrivMode_Default;

#undef PrivMode_Default

static int refresh_count = 0, refresh_limit = 1, refresh_type = SLOW_REFRESH;

Atom wmDeleteWindow;
/* OffiX Dnd (drag 'n' drop) support */
Atom DndProtocol, DndSelection;

#ifndef NO_XLOCALE
static char * rs_inputMethod = "";        /* XtNinputMethod */
static char * rs_preeditType = NULL;        /* XtNpreeditType */
static XIC Input_Context;                /* input context */
#endif        /* NO_XLOCALE */

/* command input buffering */
#ifndef BUFSIZ
# define BUFSIZ                4096
#endif
static unsigned char cmdbuf_base [BUFSIZ], * cmdbuf_ptr, * cmdbuf_endp;
/*}}}*/
/*{{{ local functions referenced */
static void        privileges (int mode);

static RETSIGTYPE        Child_signal (int);
static RETSIGTYPE        Exit_signal (int);
static int        get_pty (void);
static int        get_tty (void);
static int        run_command (char * /* argv */ []);
static unsigned char cmd_getc (void);
static void        lookup_key (XEvent * /* ev */);
void        process_x_event (XEvent * /* ev */);
/*static void        process_string (int);*/
#ifdef PRINTPIPE
static void        process_print_pipe (void);
#endif
static void        process_escape_seq (void);
static void        process_csi_seq (void);
static void        process_xterm_seq (void);
static void        process_terminal_mode (int /* mode */,
                                       int /* priv */,
                                       unsigned int /* nargs */,
                                       int /* arg */ []);
static void        process_sgr_mode (unsigned int /* nargs */,
                                  int /* arg */ []);
static void        process_graphics (void);

static void        tt_winsize (int /* fd */);

#ifndef NO_XLOCALE
static void        init_xlocale (void);
#else
# define init_xlocale() ((void)0)
#endif
/*----------------------------------------------------------------------*/
/*}}}*/

/*{{{ substitute system functions */
#ifndef _POSIX_VERSION
# if defined (__svr4__)
static int
getdtablesize (void)
{
   struct rlimit rlim;
   getrlimit (RLIMIT_NOFILE, &rlim);
   return rlim.rlim_cur;
}
# endif
#endif
/*}}}*/


/*{{{ take care of suid/sgid super-user (root) privileges */
static void
privileges (int mode)
{
#ifdef HAVE_SETEUID
   static uid_t euid;
   static gid_t egid;

   switch (mode) {
    case IGNORE:
      /*
       * change effective uid/gid - not real uid/gid - so we can switch
       * back to root later, as required
       */
      seteuid (getuid ());
      setegid (getgid ());
      break;

    case SAVE:
      euid = geteuid ();
      egid = getegid ();
      break;

    case RESTORE:
      seteuid (euid);
      setegid (egid);
      break;
   }
# else
   switch (mode) {
    case IGNORE:
      setuid (getuid ());
      setgid (getgid ());
      break;

    case SAVE:                break;
    case RESTORE:        break;
   }
#endif
}
/*}}}*/

/*{{{ signal handling, exit handler */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
static RETSIGTYPE
Child_signal (int unused)
{
   int pid, save_errno = errno, status;
/*
   do
     {
        errno = 0;
     }
   while ((-1 == (pid = waitpid (cmd_pid, NULL, WNOHANG))) &&
          (errno == EINTR));
*/
   pid=wait(&status);
   if(pid!=-1)
   {
     if (pid == cmd_pid) exit (EXIT_SUCCESS);
     errno = save_errno;

     xnc_is_my_child(pid,status);
   }
   signal (SIGCHLD, Child_signal);
}

void set_cmdchild_signal()
{
   signal (SIGCHLD, Child_signal);
}  

int psystem(char *str)
{
/*  char bu[80];
  FILE *fpp;
  fpp=(FILE*)popen(str,"r");
  while(fgets(bu,80,fpp)!=NULL);
  pclose(fpp);*/
  int ret;
  signal(SIGCHLD,SIG_DFL);
  ret=system(str);
  signal(SIGCHLD,Child_signal);
  return ret;
}           

static int child_work=0;
static int waitret=0;

static RETSIGTYPE Child_die(int unused)
{
  /*  fprintf(stderr,"Child_die: got SIGCHLD signal, processing...\n"); */
  child_work=0;
  wait(&waitret);
  signal(SIGCHLD,Child_signal);
}

void   m_delay(int usec)
{
  struct timeval time;
  usec *= 1000;
  time.tv_sec = usec / 1000000L;
  time.tv_usec = usec % 1000000L;
  select(0, (fd_set *) NULL, (fd_set *) NULL, (fd_set *) NULL, &time);
}


extern int disable_reread;
int psystem2(char *str, ...)
{
  int pid,i;
  XEvent ev;
  char *argv[10];
  va_list ap;
  va_start(ap,str);
  argv[0]=str;
  i=1;
  while(i<9)
  {
          argv[i]=va_arg(ap,char*);
          if(argv[i]==NULL) break;
          i++;
  }
  va_end(ap);
  signal(SIGCHLD,Child_die);
  child_work=1;
  pid=fork();
  if(pid==-1)
  {
     child_work=0;
     signal(SIGCHLD,Child_signal);
     return 255;
  }
  if(pid==0)
  {
      //     close(1);
      //     close(2);
     execvp(argv[0],argv);
     exit(254);
  }
  disable_reread=1;
  /*  fprintf(stderr,"Waiting for child die....\n"); */
  while(child_work)
  {
    if(XPending(Xdisplay))
    {
      XNextEvent(Xdisplay,&ev);
      process_x_event(&ev);
    } else
      m_delay(10);
  }
  disable_reread=1;
  return WEXITSTATUS(waitret);
}           

/*
 * Catch a fatal signal and tidy up before quitting
 */
static RETSIGTYPE
Exit_signal (int sig)
{
#ifdef DEBUG_CMD
   print_error ("signal %d", sig);
#endif
   signal (sig, SIG_DFL);

   privileges (RESTORE);
   cleanutent ();
   privileges (IGNORE);

   kill (getpid (), sig);
}

/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 */
extern void (*xnc_kill)();
void
clean_exit (void)
{
#ifdef DEBUG_CMD       
   fprintf (stderr, "Restoring \"%s\" to mode %03o, uid %d, gid %d\n",
            ttydev, ttyfd_stat.st_mode, ttyfd_stat.st_uid, ttyfd_stat.st_gid);
#endif
   privileges (RESTORE);
   chmod (ttydev, ttyfd_stat.st_mode);
   chown (ttydev, ttyfd_stat.st_uid, ttyfd_stat.st_gid);
   cleanutent ();
   xnc_kill();
   privileges (IGNORE);
}
/*}}}*/

/*{{{ Acquire a pseudo-teletype from the system. */
/*
 * On failure, returns -1.
 * On success, returns the file descriptor.
 *
 * If successful, ttydev and ptydev point to the names of the
 * master and slave parts
 */
int
get_pty(void)
{
    int             fd;
    char           *ptydev;

#ifdef PTYS_ARE_OPENPTY
    char            tty_name[sizeof "/dev/pts/????\0"];

    if (openpty(&fd, &rxvt_ttyfd, tty_name, NULL, NULL) != -1) {
        ttydev = strdup(tty_name);
        goto Found;
    }
#endif

#ifdef PTYS_ARE__GETPTY
    if ((ptydev = ttydev = _getpty(&fd, O_RDWR | O_NDELAY, 0622, 0)) != NULL)
	goto Found;
#endif
#ifdef PTYS_ARE_GETPTY
    while ((ptydev = getpty()) != NULL)
	if ((fd = open(ptydev, O_RDWR)) >= 0) {
	   ttydev = ptydev;
	   goto Found;
	}
#endif
#if defined(HAVE_GRANTPT) && defined(HAVE_UNLOCKPT)
# if defined(PTYS_ARE_GETPT) || defined(PTYS_ARE_PTMX)
    {
	extern char    *ptsname();

#  ifdef PTYS_ARE_GETPT
	if ((fd = getpt()) >= 0) {
#  else
	if ((fd = open("/dev/ptmx", O_RDWR)) >= 0) {
#  endif
	    if (grantpt(fd) == 0 	/* change slave permissions */
		&& unlockpt(fd) == 0) {	/* slave now unlocked */
		ptydev = ttydev = ptsname(fd);	/* get slave's name */
		changettyowner = 0;
		goto Found;
	    }
	    close(fd);
	}
    }
# endif
#endif
#ifdef PTYS_ARE_PTC
    if ((fd = open("/dev/ptc", O_RDWR)) >= 0) {
	ptydev = ttydev = ttyname(fd);
	goto Found;
    }
#endif
#ifdef PTYS_ARE_CLONE
    if ((fd = open("/dev/ptym/clone", O_RDWR)) >= 0) {
	ptydev = ttydev = ptsname(fd);
	goto Found;
    }
#endif
#ifdef PTYS_ARE_NUMERIC
    {
	int             idx;
	char           *c1, *c2;
	char            pty_name[] = "/dev/ptyp???";
	char            tty_name[] = "/dev/ttyp???";

	ptydev = pty_name;
	ttydev = tty_name;

	c1 = &(pty_name[sizeof(pty_name) - 4]);
	c2 = &(tty_name[sizeof(tty_name) - 4]);
	for (idx = 0; idx < 256; idx++) {
	    sprintf(c1, "%d", idx);
	    sprintf(c2, "%d", idx);
	    if (access(ttydev, F_OK) < 0) {
		idx = 256;
		break;
	    }
	    if ((fd = open(ptydev, O_RDWR)) >= 0) {
		if (access(ttydev, R_OK | W_OK) == 0) {
		    ttydev = strdup(tty_name);
		    goto Found;
		}
		close(fd);
	    }
	}
    }
#endif
#ifdef PTYS_ARE_SEARCHED
    {
	int             len;
	const char     *c1, *c2;
	char            pty_name[] = "/dev/pty??";
	char            tty_name[] = "/dev/tty??";

	len = sizeof(pty_name) - 3;
	ptydev = pty_name;
	ttydev = tty_name;

# define PTYCHAR1	"pqrstuvwxyz"
# define PTYCHAR2	"0123456789abcdef"
	for (c1 = PTYCHAR1; *c1; c1++) {
	    ptydev[len] = ttydev[len] = *c1;
	    for (c2 = PTYCHAR2; *c2; c2++) {
		ptydev[len + 1] = ttydev[len + 1] = *c2;
	        if ((fd = open(ptydev, O_RDWR)) >= 0) {
		    if (access(ttydev, R_OK | W_OK) == 0) {
			ttydev = strdup(tty_name);
			goto Found;
		    }
		    close(fd);
		}
	    }
	}
    }
#endif

    print_error("can't open pseudo-tty");
    return -1;

  Found:
    fcntl(fd, F_SETFL, O_NDELAY);
    return fd;
}
/*}}} */

/*{{{ establish a controlling teletype for new session */
/*
 * On some systems this can be done with ioctl() but on others we
 * need to re-open the slave tty.
 */
/* INTPROTO */
int
get_tty(void)
{
    int             fd, i;
    pid_t           pid;

/*
 * setsid() [or setpgrp] must be before open of the terminal,
 * otherwise there is no controlling terminal (Solaris 2.4, HP-UX 9)
 */
#ifndef ultrix
# ifdef NO_SETSID
    pid = setpgrp(0, 0);
# else
    pid = setsid();
# endif
    if (pid < 0)
	perror("SetSID failed:");
# ifdef DEBUG_TTYMODE
    print_error("(%s: line %d): PID = %d\n", __FILE__, __LINE__, pid);
# endif
#endif				/* ultrix */
    
    if(rxvt_ttyfd==-1) /* we are already open tty*/
    {
      if ((fd = open(ttydev, O_RDWR)) < 0
	  && (fd = open("/dev/tty", O_RDWR)) < 0) 
      {
	print_error("can't open slave tty %s", ttydev);
	exit(EXIT_FAILURE);
      }
    } else
      fd=rxvt_ttyfd;

#if defined(PTYS_ARE_PTMX) && defined(I_PUSH)
/*
 * Push STREAMS modules:
 *    ptem: pseudo-terminal hardware emulation module.
 *    ldterm: standard terminal line discipline.
 *    ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
 */
# ifdef HAVE_ISASTREAM
    if (isastream(fd) == 1)
# endif
    {
        ioctl(fd, I_PUSH, "ptem");
        ioctl(fd, I_PUSH, "ldterm");
        ioctl(fd, I_PUSH, "ttcompat");
    }
#endif

    if (changettyowner) {
    /* change ownership of tty to real uid and real group */
	unsigned int    mode = 0622;
	gid_t           gid = getgid();

#ifdef TTY_GID_SUPPORT
	{
	    struct group   *gr = getgrnam("tty");

	    if (gr) {
	    /* change ownership of tty to real uid, "tty" gid */
		gid = gr->gr_gid;
		mode = 0620;
	    }
	}
#endif				/* TTY_GID_SUPPORT */
#ifndef __CYGWIN32__
	privileges(RESTORE);
	fchown(fd, getuid(), gid);	/* fail silently */
	fchmod(fd, mode);
	privileges(IGNORE);
#endif
    }

/*
 * Close all file descriptors.  If only stdin/out/err are closed,
 * child processes remain alive upon deletion of the window.
 */
    for (i = 0; i < num_fds; i++)
	if (i != fd)
	    close(i);

/* Reopen stdin, stdout and stderr over the tty file descriptor */
    dup2(fd, 0);		/* stdin */
    dup2(fd, 1);		/* stdout */
    dup2(fd, 2);		/* stderr */

    if (fd > 2)
	close(fd);

#ifdef ultrix
    if ((fd = open("/dev/tty", O_RDONLY)) >= 0) {
	ioctl(fd, TIOCNOTTY, 0);
	close(fd);
    } else {
	pid = setpgrp(0, 0);
	if (pid < 0)
	    perror(rs[Rs_name]);
    }

/* no error, we could run with no tty to begin with */
#else				/* ultrix */
# ifdef TIOCSCTTY
    ioctl(0, TIOCSCTTY, 0);
# endif

/* set process group */
# if defined (_POSIX_VERSION) || defined (__svr4__)
    tcsetpgrp(0, pid);
# elif defined (TIOCSPGRP)
    ioctl(0, TIOCSPGRP, &pid);
# endif

/* svr4 problems: reports no tty, no job control */
/* # if !defined (__svr4__) && defined (TIOCSPGRP) */
    close(open(ttydev, O_RDWR, 0));
/* # endif */
#endif				/* ultrix */

    return fd;
}





/* Previous version of get_pty and get_tty */
#if 0
static int
get_pty (void)
{
   int fd = -1;
#if defined (__sgi)
   ptydev = ttydev = _getpty (&fd, O_RDWR|O_NDELAY, 0622, 0);
   if (ptydev == NULL)
     goto Failed;
#elif defined (__svr4__)
     {
        extern char * ptsname ();

        /* open the STREAMS, clone device /dev/ptmx (master pty) */
        if ((fd = open ("/dev/ptmx", O_RDWR)) < 0)
          {
             goto Failed;
          }
        else
          {
             grantpt (fd);                /* change slave permissions */
             unlockpt (fd);                /* unlock slave */
             ptydev = ttydev = ptsname (fd);        /* get slave's name */
          }
     }
#elif defined (_AIX)
   if ((fd = open ("/dev/ptc", O_RDWR)) < 0)
     goto Failed;
   else
     ptydev = ttydev = ttyname (fd);
#else
   static char pty_name[] = "/dev/pty??";
   static char tty_name[] = "/dev/tty??";
   int len = strlen (tty_name);
   char * c1, * c2;

   ptydev = pty_name;
   ttydev = tty_name;

# define        PTYCHAR1        "pqrstuvwxyz"
# define        PTYCHAR2        "0123456789abcdef"
   for (c1 = PTYCHAR1; *c1; c1++)
     {
        ptydev [len-2] = ttydev [len-2] = *c1;
        for (c2 = PTYCHAR2; *c2; c2++)
          {
             ptydev [len-1] = ttydev [len-1] = *c2;
             if ((fd = open (ptydev, O_RDWR)) >= 0)
               {
                  if (access (ttydev, R_OK|W_OK) == 0)
                    goto Found;
                  close (fd);
               }
          }
     }
   goto Failed;
#endif

   Found:
   fcntl (fd, F_SETFL, O_NDELAY);
   return fd;

   Failed:
   print_error ("can't open pseudo-tty");
   return -1;
}
/*}}}*/

/*{{{ establish a controlling teletype for new session */
/*
 * On some systems this can be done with ioctl() but on others we
 * need to re-open the slave tty.
 */
static int
get_tty (void)
{
   int fd;
   pid_t pid;

   /*
    * setsid() [or setpgrp] must be before open of the terminal,
    * otherwise there is no controlling terminal (Solaris 2.4, HP-UX 9)
    */
#ifndef ultrix
# ifdef NO_SETSID
   pid = setpgrp (0, 0);
# else
   pid = setsid ();
# endif
   if (pid < 0)
     perror (rs_name);
# ifdef DEBUG_TTYMODE
   print_error ("(%s: line %d): PID = %d\n", __FILE__, __LINE__, pid);
# endif
#endif        /* ultrix */

   if ((fd = open (ttydev, O_RDWR)) < 0)
     {
        print_error ("can't open slave tty %s", ttydev);
        exit (EXIT_FAILURE);
     }
#if defined (__svr4__)
  /*
   * Push STREAMS modules:
   *        ptem: pseudo-terminal hardware emulation module.
   *        ldterm: standard terminal line discipline.
   *        ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
   */
   ioctl (fd, I_PUSH, "ptem");
   ioctl (fd, I_PUSH, "ldterm");
   ioctl (fd, I_PUSH, "ttcompat");
#else        /* __svr4__ */
     {
        /* change ownership of tty to real uid and real group */
        unsigned int mode = 0622;
        gid_t gid = getgid ();
# ifdef USE_GETGRNAME
          {
             struct group * gr = getgrnam ("tty");
             if (gr)
               {
                  /* change ownership of tty to real uid, "tty" gid */
                  gid = gr->gr_gid;
                  mode = 0620;
               }
          }
# endif        /* USE_GETGRNAME */

        privileges (RESTORE);
        fchown (fd, getuid (), gid);    /* fail silently */
        fchmod (fd, mode);
        privileges (IGNORE);
     }
#endif        /* __svr4__ */

   /*
    * Close all file descriptors.  If only stdin/out/err are closed,
    * child processes remain alive upon deletion of the window.
    */
     {
        int i;
        for (i = 0; i < num_fds; i++)
          if (i != fd)
            close (i);
     }

   /* Reopen stdin, stdout and stderr over the tty file descriptor */
   dup (fd);                /* 0: stdin */
   dup (fd);                /* 1: stdout */
   dup (fd);                /* 2: stderr */

   if (fd > 2)
     close (fd);

#ifdef ultrix
   if ((fd = open ("/dev/tty", O_RDONLY)) >= 0)
     {
        ioctl (fd, TIOCNOTTY, 0);
        close (fd);
     }
   else
     {
        pid = setpgrp (0, 0);
        if (pid < 0)
          perror (rs_name);
     }

   /* no error, we could run with no tty to begin with */
#else        /* ultrix */

# ifdef TIOCSCTTY
   ioctl (0, TIOCSCTTY, 0);
# endif

   /* set process group */
# if defined (_POSIX_VERSION) || defined (__svr4__)
   tcsetpgrp (0, pid);
# elif defined (TIOCSPGRP)
   ioctl (0, TIOCSPGRP, &pid);
# endif

   /* svr4 problems: reports no tty, no job control */
   /* # if !defined (__svr4__) && defined (TIOCSPGRP) */

   close (open (ttydev, O_RDWR, 0));
   /* # endif */
#endif        /* ultrix */

   privileges (IGNORE);

   return fd;
}
#endif              /* End of previous version */
/*}}}*/





/*{{{ ways to deal with getting/setting termios structure */
#ifdef HAVE_TERMIOS_H
typedef struct termios ttymode_t;
# ifdef TCSANOW                  /* POSIX */
#  define GET_TERMIOS(fd,tios)        tcgetattr (fd, tios)
#  define SET_TERMIOS(fd,tios)        do {\
cfsetospeed (tios, BAUDRATE);\
cfsetispeed (tios, BAUDRATE);\
tcsetattr (fd, TCSANOW, tios);\
} while (0)
# else
#  ifdef TIOCSETA
#   define GET_TERMIOS(fd,tios)        ioctl (fd, TIOCGETA, tios)
#   define SET_TERMIOS(fd,tios)        do {\
tios->c_cflag |= BAUDRATE;\
ioctl (fd, TIOCSETA, tios);\
} while (0)
#  else
#   define GET_TERMIOS(fd,tios)        ioctl (fd, TCGETS, tios)
#   define SET_TERMIOS(fd,tios)        do {\
tios->c_cflag |= BAUDRATE;\
ioctl (fd, TCSETS, tios);\
} while (0)
#  endif
# endif
# define SET_TTYMODE(fd,tios)                SET_TERMIOS (fd, tios)
#else
/* sgtty interface */
typedef struct {
   struct sgttyb sg;
   struct tchars tc;
   struct ltchars lc;
   int line;
   int local;
} ttymode_t;

# define SET_TTYMODE(fd,tt)        do {        \
tt->sg.sg_ispeed = tt->sg.sg_ospeed = BAUDRATE;\
ioctl (fd, TIOCSETP, &(tt->sg));\
ioctl (fd, TIOCSETC, &(tt->tc));\
ioctl (fd, TIOCSLTC, &(tt->lc));\
ioctl (fd, TIOCSETD, &(tt->line));\
ioctl (fd, TIOCLSET, &(tt->local));\
} while (0)
#endif /* HAVE_TERMIOS_H */
/*}}}*/

/*{{{ debug_ttymode() */
#ifdef DEBUG_TTYMODE
static void
debug_ttymode (ttymode_t * ttymode)
{
# ifdef HAVE_TERMIOS_H
   /* c_iflag bits */
   fprintf (stderr, "Input flags\n");

   /* cpp token stringize doesn't work on all machines <sigh> */
#  define FOO(flag,name) \
if ((ttymode->c_iflag) & flag) fprintf (stderr, "%s ", name)

   /* c_iflag bits */
   FOO (IGNBRK, "IGNBRK");
   FOO (BRKINT, "BRKINT");
   FOO (IGNPAR, "IGNPAR");
   FOO (PARMRK, "PARMRK");
   FOO (INPCK, "INPCK");
   FOO (ISTRIP, "ISTRIP");
   FOO (INLCR, "INLCR");
   FOO (IGNCR, "IGNCR");
   FOO (ICRNL, "ICRNL");
   FOO (IXON, "IXON");
   FOO (IXOFF, "IXOFF");
#  ifdef IUCLC
   FOO (IUCLC, "IUCLC");
#  endif
#  ifdef IXANY
   FOO (IXANY, "IXANY");
#  endif
#  ifdef IMAXBEL
   FOO (IMAXBEL, "IMAXBEL");
#  endif
   fprintf (stderr, "\n\n");

#  undef FOO
#  define FOO(entry, name) \
fprintf (stderr, "%s = %#3o\n", name, ttymode->c_cc [entry])
   FOO (VINTR, "VINTR");
   FOO (VQUIT, "VQUIT");
   FOO (VERASE, "VERASE");
   FOO (VKILL, "VKILL");
   FOO (VEOF, "VEOF");
   FOO (VEOL, "VEOL");
#  ifdef VEOL2
   FOO (VEOL2, "VEOL2");
#  endif
#  ifdef VSWTC
   FOO (VSWTC, "VSWTC");
#  endif
#  ifdef VSWTCH
   FOO (VSWTCH, "VSWTCH");
#  endif
   FOO (VSTART, "VSTART");
   FOO (VSTOP, "VSTOP");
   FOO (VSUSP, "VSUSP");
#  ifdef VDSUSP
   FOO (VDSUSP, "VDSUSP");
#  endif
#  ifdef VREPRINT
   FOO (VREPRINT, "VREPRINT");
#  endif
#  ifdef VDISCRD
   FOO (VDISCRD, "VDISCRD");
#  endif
#  ifdef VWERSE
   FOO (VWERSE, "VWERSE");
#  endif
#  ifdef VLNEXT
   FOO (VLNEXT, "VLNEXT");
#  endif
   fprintf (stderr, "\n\n");

#  undef FOO

# endif        /* HAVE_TERMIOS_H */
}
#endif        /* DEBUG_TTYMODE */
/*}}}*/

/*{{{ get_ttymode() */
static void
get_ttymode (ttymode_t * tio)
{
#ifdef HAVE_TERMIOS_H
   /*
    * standard System V termios interface
    */
   if (GET_TERMIOS (0, tio) < 0)
     {
        /* return error - use system defaults */
        tio->c_cc [VINTR]        = CINTR;
        tio->c_cc [VQUIT]        = CQUIT;
        tio->c_cc [VERASE]        = CERASE;
        tio->c_cc [VKILL]        = CKILL;
        tio->c_cc [VSTART]        = CSTART;
        tio->c_cc [VSTOP]        = CSTOP;
        tio->c_cc [VSUSP]        = CSUSP;
# ifdef VDSUSP
        tio->c_cc [VDSUSP]        = CDSUSP;
# endif
# ifdef VREPRINT
        tio->c_cc [VREPRINT]        = CRPRNT;
# endif
# ifdef VDISCRD
        tio->c_cc [VDISCRD]        = CFLUSH;
# endif
# ifdef VWERSE
        tio->c_cc [VWERSE]        = CWERASE;
# endif
# ifdef VLNEXT
        tio->c_cc [VLNEXT]        = CLNEXT;
# endif
     }

   tio->c_cc [VEOF]        = CEOF;
   tio->c_cc [VEOL]        = VDISABLE;
# ifdef VEOL2
   tio->c_cc [VEOL2]        = VDISABLE;
# endif
# ifdef VSWTC
   tio->c_cc [VSWTC]        = VDISABLE;
# endif
# ifdef VSWTCH
   tio->c_cc [VSWTCH]        = VDISABLE;
# endif
# if VMIN != VEOF
   tio->c_cc [VMIN]        = 1;
# endif
# if VTIME != VEOL
   tio->c_cc [VTIME]        = 0;
# endif

   /* input modes */
   tio->c_iflag = (BRKINT | IGNPAR | ICRNL | IXON
# ifdef IMAXBEL
                   | IMAXBEL
# endif
                   );

   /* output modes */
   tio->c_oflag = (OPOST | ONLCR);

   /* control modes */
   tio->c_cflag = (CS8 | CREAD);

   /* line discipline modes */
   tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK
# if defined (ECHOCTL) && defined (ECHOKE)
                   | ECHOCTL | ECHOKE
# endif
                   );

   /*
    * guess an appropriate value for Backspace
    */
#ifdef DONT_GUESS_BACKSPACE
   PrivMode (1, PrivMode_BackSpace);        /* always ^H */
#else
   PrivMode ((tio->c_cc [VERASE] == '\b'), PrivMode_BackSpace);
#endif        /* DONT_GUESS_BACKSPACE */

#else        /* HAVE_TERMIOS_H */

   /*
    * sgtty interface
    */

   /* get parameters -- gtty */
   if (ioctl (0, TIOCGETP, &(tio->sg)) < 0)
     {
        tio->sg.sg_erase = CERASE;        /* ^H */
        tio->sg.sg_kill        = CKILL;        /* ^U */
     }
   tio->sg.sg_flags = (CRMOD | ECHO | EVENP | ODDP);

   /* get special characters */
   if (ioctl (0, TIOCGETC, &(tio->tc)) < 0)
     {
        tio->tc.t_intrc  = CINTR;        /* ^C */
        tio->tc.t_quitc  = CQUIT;        /* ^\ */
        tio->tc.t_startc = CSTART;        /* ^Q */
        tio->tc.t_stopc  = CSTOP;        /* ^S */
        tio->tc.t_eofc   = CEOF;        /* ^D */
        tio->tc.t_brkc         = -1;
     }

   /* get local special chars */
   if (ioctl (0, TIOCGLTC, &(tio->lc)) < 0)
     {
        tio->lc.t_suspc         = CSUSP;        /* ^Z */
        tio->lc.t_dsuspc = CDSUSP;        /* ^Y */
        tio->lc.t_rprntc = CRPRNT;        /* ^R */
        tio->lc.t_flushc = CFLUSH;        /* ^O */
        tio->lc.t_werasc = CWERASE;        /* ^W */
        tio->lc.t_lnextc = CLNEXT;        /* ^V */
     }

   /* get line discipline */
   ioctl (0, TIOCGETD, &(tio->line));
# ifdef NTTYDISC
   tio->line = NTTYDISC;
# endif /* NTTYDISC */
   tio->local = (LCRTBS | LCRTERA | LCTLECH | LPASS8 | LCRTKIL);

   /*
    * guess an appropriate value for Backspace
    */
#ifdef DONT_GUESS_BACKSPACE
   PrivMode (1, PrivMode_BackSpace);        /* always ^H */
#else
   PrivMode ((tio->sg.sg_erase == '\b'), PrivMode_BackSpace);
#endif        /* DONT_GUESS_BACKSPACE */

#endif        /* HAVE_TERMIOS_H */
}
/*}}}*/

int is_show_intro()
{
        return Options & Opt_showIntro;
}

int is_term_on()
{
        return (Options & Opt_termOff) ? 0 : 1;
}

int is_look_five()
{
        return (Options & Opt_lookFive) ? 1 : 0;
}

int is_fast_load()
{
        return Options & Opt_fastLoad;
}

void reset_fast_loading()
{
        Options&= ~Opt_fastLoad;
}

int is_back_load()
{
  return (Options & Opt_backLoad) ? 1 : 0;
}

int is_shared_load()
{
  return (Options & Opt_sharedLoad) ? 1 : 0;
}

void unset_back_load()
{
  Options&= ~Opt_backLoad;
}

/*{{{ run_command() */
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
static int
run_command (char * argv [])
{
   ttymode_t tio;
   int ptyfd;

   /* Save and then give up any super-user privileges */
   privileges (SAVE);
   privileges (IGNORE);

   ptyfd = get_pty ();
   if (ptyfd < 0)
     return -1;

   /* store original tty status for restoration clean_exit() -- rgg 04/12/95 */
   lstat (ttydev, &ttyfd_stat);
#ifdef DEBUG_CMD
   fprintf (stderr, "Original settings of %s are mode %o, uid %d, gid %d\n",
            ttydev, ttyfd_stat.st_mode, ttyfd_stat.st_uid, ttyfd_stat.st_gid);
#endif

   /* install exit handler for cleanup */
#ifdef HAVE_ATEXIT
   atexit (clean_exit);
#else
# if defined (__sun__)
   on_exit (clean_exit, NULL);                /* non-ANSI exit handler */
# else
   print_error ("no atexit(), UTMP entries can't be cleaned");
# endif
#endif

   /*
    * get tty settings before fork()
    * and make a reasonable guess at the value for BackSpace
    */
   get_ttymode (&tio);
   /* add Backspace value */
   SavedModes |= (PrivateModes & PrivMode_BackSpace);

   /* add value for scrollBar */
   if (scrollbar_visible ())
     {
        PrivateModes |= PrivMode_scrollBar;
        SavedModes |= PrivMode_scrollBar;
     }

#ifdef DEBUG_TTYMODE
   debug_ttymode (&tio);
#endif

   /* spin off the command interpreter */
   signal (SIGHUP, Exit_signal);
#ifndef __svr4__
   signal (SIGINT, Exit_signal);
#endif
   signal (SIGQUIT, Exit_signal);
   signal (SIGTERM, Exit_signal);
   signal (SIGCHLD, Child_signal);

   /* need to trap SIGURG for SVR4 (Unixware) rlogin */
   /* signal (SIGURG, SIG_DFL); */
  xnc_setsignals();

   cmd_pid = fork ();
   if (cmd_pid < 0)
     {
        print_error ("can't fork");
        return -1;
     }
   if (cmd_pid == 0)                /* child */
     {
        /* signal (SIGHUP, Exit_signal); */
        /* signal (SIGINT, Exit_signal); */
#ifdef HAVE_UNSETENV
        /* avoid passing old settings and confusing term size */
        unsetenv ("LINES");
        unsetenv ("COLUMNS");
        /* avoid passing termcap since terminfo should be okay */
        unsetenv ("TERMCAP");
#endif        /* HAVE_UNSETENV */
        /* establish a controlling teletype for the new session */
        get_tty ();

        /* initialize terminal attributes */
        SET_TTYMODE (0, &tio);

        /* become virtual console, fail silently */
        if (Options & Opt_console)
          {
#ifdef TIOCCONS
             unsigned int on = 1;
             ioctl (0, TIOCCONS, &on);
#elif defined (SRIOCREDIR)
             int fd = open (CONSOLE, O_WRONLY);
             if (fd < 0 || ioctl (0, SRIOCSREDIR, fd) < 0)
               {
                  if (fd >= 0)
                    close (fd);
               }
#endif        /* SRIOCSREDIR */
          }
        tt_winsize (0);                /* set window size */

        /* reset signals and spin off the command interpreter */
        signal (SIGINT,  SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        /*
         * mimick login's behavior by disabling the job control signals
         * a shell that wants them can turn them back on
         */
#ifdef SIGTSTP
        signal (SIGTSTP, SIG_IGN);
        signal (SIGTTIN, SIG_IGN);
        signal (SIGTTOU, SIG_IGN);
#endif /* SIGTSTP */

        /* command interpreter path */
        if (argv != NULL)
          {
#ifdef DEBUG_CMD
             int i;
             for (i = 0; argv [i]; i++)
               fprintf (stderr, "argv [%d] = \"%s\"\n", i, argv [i]);
#endif
             execvp (argv [0], argv);
             print_error ("can't execute \"%s\"", argv [0]);
          }
        else
          {
             const char * argv0, * shell;

             if ((shell = getenv ("SHELL")) == NULL || *shell == '\0')
               shell = "/bin/sh";

             argv0 = my_basename (shell);
             if (Options & Opt_loginShell)
               {
                  char * p = MALLOC ((strlen (argv0)+2)*sizeof(char), argv0);
                  p [0] = '-';
                  strcpy (&p [1], argv0);
                  argv0 = p;
               }
             execlp (shell, argv0, NULL);
             print_error ("can't execute \"%s\"", shell);
          }
        exit (EXIT_FAILURE);
     }

   privileges (RESTORE);
   if (!(Options & Opt_utmpInhibit))
     makeutent (ttydev, display_name);                /* stamp /etc/utmp */
   privileges (IGNORE);

   return ptyfd;
}
/*}}}*/

/*{{{ init_command() */
void
init_command (char * argv [])
{
   /*
    * Initialize the command connection.
    * This should be called after the X server connection is established.
    */

   /* Enable delete window protocol */
   wmDeleteWindow = XInternAtom (Xdisplay, "WM_DELETE_WINDOW", False);
   XSetWMProtocols (Xdisplay, TermWin.parent, &wmDeleteWindow, 1);

   /* Enable OffiX Dnd (drag 'n' drop) protocol */
   DndProtocol  = XInternAtom (Xdisplay, "DndProtocol", False);
   DndSelection = XInternAtom (Xdisplay, "DndSelection", False);

   init_xlocale ();

   /* get number of available file descriptors */
#ifdef _POSIX_VERSION
   num_fds = sysconf (_SC_OPEN_MAX);
#else
   num_fds = getdtablesize ();
#endif

#ifdef META8_OPTION
   meta_char = (Options & Opt_meta8 ? 0x80 : 033);
#endif

#ifdef GREEK_SUPPORT
   greek_init ();
#endif

   Xfd = XConnectionNumber (Xdisplay);
   cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

   if ((cmd_fd = run_command (argv)) < 0)
     {
        print_error ("aborting");
        exit (EXIT_FAILURE);
     }
}
/*}}}*/

/*{{{ Xlocale */
/*
 * This is more or less stolen straight from XFree86 xterm.
 * This should support all European type languages.
 */
#ifndef NO_XLOCALE
static void
init_xlocale (void)
{
   char * p, * s, buf [32], tmp [1024];
   XIM xim = NULL;
   XIMStyle input_style = 0;
   XIMStyles * xim_styles = NULL;
   int found;

   Input_Context = NULL;

#ifdef KANJI
   setlocale (LC_CTYPE, "");
#endif

   if (rs_inputMethod == NULL
#ifndef KANJI
       || !*rs_inputMethod        /* required ? */
#endif
       )
     {
        if ((p = XSetLocaleModifiers ("@im=none")) != NULL && *p)
          xim = XOpenIM (Xdisplay, NULL, NULL, NULL);
     }
   else
     {
        strcpy (tmp, rs_inputMethod);
        for (s = tmp; *s; /*nil*/)
          {
             char * end, * next_s;
             while (*s && isspace (*s)) s++;
             if (!*s) break;
             end = s;
             while (*end && (*end != ',')) end++;
             next_s = end--;
             while ((end >= s) && isspace (*end)) end--;
             *(end + 1) = '\0';

             if (*s)
               {
                  strcpy (buf, "@im=");
                  strcat (buf, s);
                  if ((p = XSetLocaleModifiers (buf)) != NULL && *p &&
                      (xim = XOpenIM (Xdisplay, NULL, NULL, NULL)) != NULL)
                    break;
               }
             if (!*next_s) break;
             s = (next_s + 1);
          }
     }

   if (xim == NULL && (p = XSetLocaleModifiers ("")) != NULL && *p)
     xim = XOpenIM (Xdisplay, NULL, NULL, NULL);

   if (xim == NULL)
     {
        print_error ("Failed to open input method");
        return;
     }

   if (XGetIMValues (xim, XNQueryInputStyle, &xim_styles, NULL) || !xim_styles)
     {
        print_error ("input method doesn't support any style");
        XCloseIM (xim);
        return;
    }

   strcpy (tmp, (rs_preeditType ? rs_preeditType : "Root"));
   for (found = 0, s = tmp; *s && !found; /*nil*/)
     {
        unsigned short i;
        char * end, * next_s;

        while (*s && isspace (*s)) s++;
        if (!*s) break;
        end = s;
        while (*end && (*end != ',')) end++;
        next_s = end--;
        while ((end >= s) && isspace (*end)) *end-- = 0;

        if (!strcmp (s, "OverTheSpot"))
          input_style = (XIMPreeditPosition | XIMStatusArea);
        else if (!strcmp (s, "OffTheSpot"))
          input_style = (XIMPreeditArea | XIMStatusArea);
        else if (!strcmp (s, "Root"))
          input_style = (XIMPreeditNothing | XIMStatusNothing);

        for (i = 0; i < xim_styles->count_styles; i++)
          {
             if (input_style == xim_styles->supported_styles [i])
               {
                  found = 1;
                  break;
               }
          }
        s = next_s;
     }
   XFree (xim_styles);

   if (found == 0)
     {
        print_error ("input method doesn't support my preedit type");
        XCloseIM (xim);
        return;
     }

    /*
     * This program only understands the Root preedit_style yet
     * Then misc.preedit_type should default to:
     *          "OverTheSpot,OffTheSpot,Root"
     *  /MaF
     */
   if (input_style != (XIMPreeditNothing | XIMStatusNothing))
     {
        print_error ("This program only supports the \"Root\" preedit type");
        XCloseIM (xim);
        return;
     }

   Input_Context = XCreateIC (xim, XNInputStyle, input_style,
                              XNClientWindow, TermWin.parent,
                              XNFocusWindow, TermWin.parent,
                              NULL);

   if (Input_Context == NULL)
     {
        print_error ("Failed to create input context");
        XCloseIM (xim);
     }
}
#endif        /* NO_XLOCALE */
/*}}}*/

/*{{{ window resizing */
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
static void
tt_winsize (int fd)
{
   struct winsize ws;

   if (fd < 0) return;

   ws.ws_col = (unsigned short) TermWin.ncol;
   ws.ws_row = (unsigned short) TermWin.nrow;
   ws.ws_xpixel = ws.ws_ypixel = 0;
   ioctl (fd, TIOCSWINSZ, &ws);
}


void
tt_resize (void)
{
   tt_winsize (cmd_fd);
}
/*}}}*/

/*{{{ Convert the keypress event into a string */
static void
lookup_key (XEvent * ev)
{
   static int numlock_state = 0;
#ifdef DEBUG_CMD
   static int debug_key = 1;        /* accessible by a debugger only */
#endif
#ifdef GREEK_SUPPORT
   static short greek_mode = 0;
#endif
   static XComposeStatus compose = {NULL, 0};
   static unsigned char kbuf [KBUFSZ];
   int ctrl, meta, shft, len;
   KeySym keysym;

   /*
    * use Num_Lock to toggle Keypad on/off.  If Num_Lock is off, allow an
    * escape sequence to toggle the Keypad.
    *
    * Always permit `shift' to override the current setting
    */
   shft = (ev->xkey.state & ShiftMask);
   ctrl = (ev->xkey.state & ControlMask);
   meta = (ev->xkey.state & Mod1Mask);
   if (numlock_state || (ev->xkey.state & Mod5Mask))
     {
        numlock_state = (ev->xkey.state & Mod5Mask);        /* numlock toggle */
        PrivMode ((!numlock_state), PrivMode_aplKP);
     }

  if(xnc_look_for_key(ev)==ASwitchtoTerm)
   {
       xnc_switch_term();
       return;
   }

  if(!xnc_now_on_top &&
     xnc_process_special_action_in_term(ev))
    return;

#ifndef NO_XLOCALE
   if (!XFilterEvent (ev, *(&ev->xkey.window)))
     {
        if (Input_Context != NULL)
          {
             Status status_return;
             len = XmbLookupString (Input_Context, &ev->xkey, kbuf,
                                    sizeof(kbuf), &keysym,
                                    &status_return);
          }
        else
          {
             len = XLookupString (&ev->xkey, kbuf,
                                  sizeof(kbuf), &keysym,
                                  &compose);
          }
     }
   else
     len = 0;
#else        /* NO_XLOCALE */
   len = XLookupString (&ev->xkey, kbuf, sizeof(kbuf), &keysym, &compose);
   /*
    * have unmapped Latin[2-4] entries -> Latin1
    * good for installations  with correct fonts, but without XLOCAL
    */
   if (!len && (keysym >= 0x0100) && (keysym < 0x0400))
     {
        len = 1;
        kbuf [0] = (keysym & 0xFF);
     }
#endif        /* NO_XLOCALE */

   /* for some backwards compatibility */
#if defined (HOTKEY_CTRL) || defined (HOTKEY_META)
# ifdef HOTKEY_CTRL
#  define HOTKEY        ctrl
# else
#  ifdef HOTKEY_META
#   define HOTKEY        meta
#  endif
# endif
   if (HOTKEY) {
      if (keysym == ks_bigfont) { change_font (0, FONT_UP); return; }
      else if (keysym == ks_smallfont) { change_font (0, FONT_DN); return; }
   }
# undef HOTKEY
#endif

   if (shft)
     {
        /* Shift + F1 - F10 generates F11 - F20 */
        if (keysym >= XK_F1 && keysym <= XK_F10)
          {
             keysym += (XK_F11 - XK_F1);
             shft = 0;                /* turn off Shift */
          }
        else if (!ctrl && !meta && (PrivateModes & PrivMode_ShiftKeys))
          {
             switch (keysym) {
                /* normal XTerm key bindings */
              case XK_Prior:        /* Shift+Prior = scroll back */
                if (TermWin.saveLines)
                  {
                     scr_page (UP, TermWin.nrow * 4/5);
                     return;
                  }
                break;

              case XK_Next:        /* Shift+Next = scroll forward */
                if (TermWin.saveLines)
                  {
                     scr_page (DN, TermWin.nrow * 4/5);
                     return;
                  }
                break;

              case XK_Insert:        /* Shift+Insert = paste mouse selection */
                selection_request (ev->xkey.time, ev->xkey.x, ev->xkey.y);
                return;
                break;

                /* rxvt extras */
              case XK_KP_Add:        /* Shift+KP_Add = bigger font */
                change_font (0, FONT_UP);
                return;
                break;

              case XK_KP_Subtract:/* Shift+KP_Subtract = smaller font */
                change_font (0, FONT_DN);
                return;
                break;
             }
          }
     }

   switch (keysym) {
    case XK_Print:
#ifdef DEBUG_SELECTION
      debug_selection ();
      return;
#else
# ifdef PRINTPIPE
      scr_printscreen (ctrl|shft);
      return;
# endif
#endif
      break;

    case XK_Mode_switch:
# ifdef GREEK_SUPPORT
      greek_mode = !greek_mode;
      if (greek_mode)
        {
           xterm_seq (XTerm_title, (greek_getmode () == GREEK_ELOT928 ?
                                    "[Greek: iso]" : "[Greek: ibm]"));
           greek_reset ();
        }
      else
        xterm_seq (XTerm_title, APL_NAME "-" VERSION);
      return;
#endif
      break;
   }

   if (keysym >= 0xFF00 && keysym <= 0xFFFF) {
#ifdef KEYSYM_RESOURCE
      if (!(shft|ctrl) && KeySym_map [keysym - 0xFF00] != NULL)
        {
           const unsigned char * kbuf;
           unsigned int len;

           kbuf = (KeySym_map [keysym - 0xFF00]);
           len = *kbuf++;

           /* escape prefix */
           if (meta
# ifdef META8_OPTION
               && (meta_char == 033)
# endif
               )
             {
                const unsigned char ch = '\033';
                tt_write (&ch, 1);
             }
           tt_write (kbuf, len);
           return;
        }
      else
#endif
        switch (keysym) {
         case XK_BackSpace:
           len = 1;
           kbuf [0] = (((PrivateModes & PrivMode_BackSpace) ?
                        !(shft|ctrl) : (shft|ctrl)) ? '\b' : '\177');
           break;

         case XK_Tab:        if (shft) { len = 3; strcpy (kbuf,"\033[Z"); }        break;

         case XK_Home:                len = strlen (strcpy (kbuf,KS_HOME));        break;
          case XK_Left:                /* "\033[D" */
         case XK_Up:                /* "\033[A" */
         case XK_Right:                /* "\033[C" */
         case XK_Down:                /* "\033[B" */
           len = 3; strcpy (kbuf,"\033[@");
           kbuf [2] = ("DACB" [keysym - XK_Left]);
           if (PrivateModes & PrivMode_aplCUR)
             { kbuf [1] = 'O'; }
           /* do Shift first */
           else if (shft && disable_modifiers==0)
             { kbuf [2] = ("dacb" [keysym - XK_Left]); }
           else if (ctrl && disable_modifiers==0)
             { kbuf [1] = 'O'; kbuf [2] = ("dacb" [keysym - XK_Left]); }
           break;
         case XK_Prior:                len = 4; strcpy (kbuf,"\033[5~");        break;
         case XK_Next:                len = 4; strcpy (kbuf,"\033[6~");        break;
         case XK_End:                len = strlen (strcpy (kbuf,KS_END));        break;

         case XK_Select:        len = 4; strcpy (kbuf,"\033[4~");        break;
         case XK_Execute:        len = 4; strcpy (kbuf,"\033[3~");        break;
         case XK_Insert:        len = 4; strcpy (kbuf,"\033[2~");        break;

         case XK_Menu:                len = 5; strcpy (kbuf,"\033[29~");        break;
         case XK_Find:                len = 4; strcpy (kbuf,"\033[1~");        break;
         case XK_Help:                len = 5; strcpy (kbuf,"\033[28~");        break;

         case XK_KP_Enter:
           /* allow shift to override */
           if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
             {
                len = 3; strcpy (kbuf,"\033OM");
             }
           else
             {
                len = 1; kbuf [0] = '\r';
             }
           break;

         case XK_KP_F1:                /* "\033OP" */
         case XK_KP_F2:                /* "\033OQ" */
         case XK_KP_F3:                /* "\033OR" */
         case XK_KP_F4:                /* "\033OS" */
           len = 3; strcpy (kbuf,"\033OP");
           kbuf [2] += (keysym - XK_KP_F1);
           break;

         case XK_KP_Multiply:        /* "\033Oj" : "*" */
         case XK_KP_Add:        /* "\033Ok" : "+" */
         case XK_KP_Separator:        /* "\033Ol" : "," */
         case XK_KP_Subtract:        /* "\033Om" : "-" */
         case XK_KP_Decimal:        /* "\033On" : "." */
         case XK_KP_Divide:        /* "\033Oo" : "/" */
         case XK_KP_0:                /* "\033Op" : "0" */
         case XK_KP_1:                /* "\033Oq" : "1" */
         case XK_KP_2:                /* "\033Or" : "2" */
         case XK_KP_3:                /* "\033Os" : "3" */
         case XK_KP_4:                /* "\033Ot" : "4" */
         case XK_KP_5:                /* "\033Ou" : "5" */
         case XK_KP_6:                /* "\033Ov" : "6" */
         case XK_KP_7:                /* "\033Ow" : "7" */
         case XK_KP_8:                /* "\033Ox" : "8" */
         case XK_KP_9:                /* "\033Oy" : "9" */
           /* allow shift to override */
           if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
             {
                len = 3; strcpy (kbuf,"\033Oj");
                kbuf [2] += (keysym - XK_KP_Multiply);
             }
           else
             {
                len = 1; kbuf [0] = ('*' + (keysym - XK_KP_Multiply));
             }
           break;

#define FKEY(n,fkey) len = 5;\
sprintf (kbuf,"\033[%02d~", (int)((n) + (keysym - fkey)))

         case XK_F1:                /* "\033[11~" */
         case XK_F2:                /* "\033[12~" */
         case XK_F3:                /* "\033[13~" */
         case XK_F4:                /* "\033[14~" */
         case XK_F5:                /* "\033[15~" */
           FKEY (11, XK_F1);        break;

         case XK_F6:                /* "\033[17~" */
         case XK_F7:                /* "\033[18~" */
         case XK_F8:                /* "\033[19~" */
         case XK_F9:                /* "\033[20~" */
         case XK_F10:                /* "\033[21~" */
           FKEY (17, XK_F6);        break;

         case XK_F11:                /* "\033[23~" */
         case XK_F12:                /* "\033[24~" */
         case XK_F13:                /* "\033[25~" */
         case XK_F14:                /* "\033[26~" */
           FKEY (23, XK_F11);        break;

         case XK_F15:                /* "\033[28~" */
         case XK_F16:                /* "\033[29~" */
           FKEY (28, XK_F15);        break;

         case XK_F17:                /* "\033[31~" */
         case XK_F18:                /* "\033[32~" */
         case XK_F19:                /* "\033[33~" */
         case XK_F20:                /* "\033[34~" */
         case XK_F21:                /* "\033[35~" */
         case XK_F22:                /* "\033[36~" */
         case XK_F23:                /* "\033[37~" */
         case XK_F24:                /* "\033[38~" */
         case XK_F25:                /* "\033[39~" */
         case XK_F26:                /* "\033[40~" */
         case XK_F27:                /* "\033[41~" */
         case XK_F28:                /* "\033[42~" */
         case XK_F29:                /* "\033[43~" */
         case XK_F30:                /* "\033[44~" */
         case XK_F31:                /* "\033[45~" */
         case XK_F32:                /* "\033[46~" */
         case XK_F33:                /* "\033[47~" */
         case XK_F34:                /* "\033[48~" */
         case XK_F35:                /* "\033[49~" */
           FKEY (31, XK_F17);        break;
#undef FKEY
#ifdef KS_DELETE
         case XK_Delete:        len = strlen (strcpy (kbuf,KS_DELETE));        break;
#endif
        }
   }
   else if (ctrl && keysym == XK_minus) {
      len = 1; kbuf [0] = '\037';        /* Ctrl-Minus generates ^_ (31) */
   } else {
#ifdef META8_OPTION
      /* set 8-bit on */
      if (meta && (meta_char == 0x80))
        {
           unsigned char * ch;
           for (ch = kbuf; ch < kbuf + len; ch++)
             *ch |= 0x80;
           meta = 0;
        }
#endif
#ifdef GREEK_SUPPORT
      if (greek_mode)
        len = greek_xlat (kbuf, len);
#endif
      /*nil*/;
   }
   disable_modifiers=0;
   if (len <= 0) return;        /* not mapped */

   /*
    * these modifications only affect the static keybuffer
    * pass Shift/Control indicators for function keys ending with `~'
    *
    * eg,
    *        Prior = "ESC[5~"
    *        Shift+Prior = "ESC[5~"
    *        Ctrl+Prior = "ESC[5^"
    *        Ctrl+Shift+Prior = "ESC[5@"
    */
   if (kbuf [0] == '\033' && kbuf [1] == '[' && kbuf [len-1] == '~')
     kbuf [len-1] = (shft ? (ctrl ? '@' : '$') : (ctrl ? '^' : '~'));

   /* escape prefix */
   if (meta
#ifdef META8_OPTION
       && (meta_char == 033)
#endif
       )
     {
        const unsigned char ch = '\033';
        tt_write (&ch, 1);
     }

#ifdef DEBUG_CMD
   if (debug_key)                /* Display keyboard buffer contents */
     {
        char * p;
        int i;

        fprintf (stderr, "key 0x%04X [%d]: `", (unsigned int)keysym, len);
        for (i = 0, p = kbuf; i < len; i++, p++)
          fprintf (stderr, (*p >= ' ' && *p < '\177' ? "%c" : "\\%03o"), *p);
        fprintf (stderr, "'\n");
     }
# endif /* DEBUG_CMD */
   tt_write (kbuf, len);
   disable_modifiers=0;
}
/*}}}*/

/*{{{ cmd_ungetc(), cmd_getc() */
static void                 
cmd_ungetc (unsigned char ch)
{
   if (cmdbuf_ptr > cmdbuf_base)
     {
        /* sneak one in */
        cmdbuf_ptr--;
        *cmdbuf_ptr = ch;
     }
   else
     {
        int n = (cmdbuf_endp - cmdbuf_base);
        if (n > 0)
          {
             if (n == sizeof (cmdbuf_base))
               {
                  /* drop one off the end to make room */
                  n--;
                  cmdbuf_endp--;
               }
             memmove ((cmdbuf_base+1), cmdbuf_base, n);
          }
        *cmdbuf_ptr = ch;
        cmdbuf_endp++;
     }
}

/* attempt to `write' COUNT to the input buffer */
unsigned int
cmd_write (const unsigned char * str, unsigned int count)
{
   while (count--)
     cmd_ungetc (str [count]);

   return 0;
}
/* cmd_getc() - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
static unsigned char
cmd_getc (void)
{
#define TIMEOUT_USEC        5000
   static short refreshed = 0;
   fd_set readfds;
   int retval;
   int maxfd=0;
   struct itimerval value;

   /* If there have been a lot of new lines, then update the screen
    * What the heck I'll cheat and only refresh less than every page-full.
    * the number of pages between refreshes is refresh_limit, which
    * is incremented here because we must be doing flat-out scrolling.
    *
    * refreshing should be correct for small scrolls, because of the
    * time-out */
   if (refresh_count > (refresh_limit * TermWin.nrow))
     {
        if (refresh_limit < REFRESH_PERIOD)
          refresh_limit++;
        refresh_count = 0;
        refreshed = 1;
        scr_refresh (refresh_type);
     }

   /* characters already read in */
   if (cmdbuf_ptr < cmdbuf_endp)
     goto Return_Char;

   while (1)
     {
/*  xnc_work_anim();*/
        while (XPending (Xdisplay))        /* process pending X events */
          {
             XEvent ev;
             refreshed = 0;
             XNextEvent (Xdisplay, &ev);
             process_x_event (&ev);

             /* in case button actions pushed chars to cmdbuf */
             if (cmdbuf_ptr < cmdbuf_endp)
               goto Return_Char;
          }
        /* Nothing to do! */
	if(xnc_run_once)
	  {
	     xnc_run_once();
	     xnc_run_once=0;
	  }
        FD_ZERO (&readfds);
        FD_SET (cmd_fd, &readfds);
        FD_SET (Xfd, &readfds);
        value.it_value.tv_usec = TIMEOUT_USEC;
        value.it_value.tv_sec = 0;

        maxfd=Xfd > cmd_fd ? Xfd : cmd_fd;

        maxfd=xnc_set_pipes(&readfds, maxfd)+1;
        retval = select (maxfd, &readfds, NULL, NULL,
                                (refreshed ? NULL : &value.it_value));
        if(retval!=0)  //We look at pipes if got info or if got error
	  xnc_look_at_pipes(&readfds,retval);

        /* See if we can read from the application */
        if (retval>0 && FD_ISSET (cmd_fd, &readfds))
          {
	    unsigned int count = sizeof(cmdbuf_base)-1;
	    cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

             /* while (count > sizeof(cmdbuf_base) / 2) */
             while (count)
               {
                  int n = read (cmd_fd, cmdbuf_endp, count);
                  if (n <= 0)
                    break;
                  cmdbuf_endp += n;
                  count -= n;
               }
             /* some characters read in */
             if (cmdbuf_ptr < cmdbuf_endp)
               goto Return_Char;
          }
        /* select statement timed out - better update the screen */
        if (retval == 0)
          {
             refresh_count = 0;
             refresh_limit = 1;
             if (!refreshed)
               {
                  refreshed = 1;
                  scr_refresh (refresh_type);
                  scrollbar_show (1);
               }
          }
     }
      if(onlyrun) {xnc_setdir();onlyrun=0;};
   return 0;

   Return_Char:
   refreshed = 0;
      if(onlyrun) {xnc_setdir();onlyrun=0;};
   return (*cmdbuf_ptr++);
}
/*}}}*/

void process_X()
{
  while(XPending(Xdisplay))
  {
             XEvent ev;
             XNextEvent (Xdisplay, &ev);
             process_x_event (&ev);
   }
}



/*{{{ process an X event */
void
process_x_event (XEvent * ev)
{
   static Time buttonpress_time;
   static int clicks = 0;
#define clickOnce()        (clicks <= 1)
   static int bypass_keystate = 0;
   int reportmode;
  if(xnc_event(ev)) return;
   switch (ev->type) {
    case KeyPress:
      lookup_key (ev);
      break;

    case ClientMessage:
      if (ev->xclient.format == 32 && ev->xclient.data.l[0] == wmDeleteWindow)
        exit (EXIT_SUCCESS);
#ifdef OFFIX_DND
      /* OffiX Dnd (drag 'n' drop) protocol */
      if (ev->xclient.message_type == DndProtocol &&
          (ev->xclient.data.l[0] == DndFile) ||
          (ev->xclient.data.l[0] == DndDir) ||
          (ev->xclient.data.l[0] == DndLink))
        {
           /* Get Dnd data */
           Atom ActualType;
           int ActualFormat;
           unsigned char * data;
           unsigned long Size, RemainingBytes;
           XGetWindowProperty (Xdisplay, Xroot,
                               DndSelection,
                               0L, 1000000L,
                               False, AnyPropertyType,
                               &ActualType, &ActualFormat,
                               &Size, &RemainingBytes,
                               &data);
    fprintf(stderr,"Drop on %X [r:%X]: %s\n",ev->xclient.window,Xroot,data);
           XChangeProperty (Xdisplay, Xroot,
                            XA_CUT_BUFFER0, XA_STRING,
                            8, PropModeReplace,
                            data, strlen (data));
           selection_paste (Xroot, XA_CUT_BUFFER0, True);
           XSetInputFocus (Xdisplay, Xroot, RevertToNone, CurrentTime);
        }
#endif        /* OFFIX_DND */
      break;

    case MapNotify: vt_show=1;break;
    case UnmapNotify: vt_show=0;break;
    case MappingNotify:
      XRefreshKeyboardMapping (&(ev->xmapping));
      break;

      /* Here's my conclusiion:
       * If the window is completely unobscured, use bitblt's
       * to scroll. Even then, they're only used when doing partial
       * screen scrolling. When partially obscured, we have to fill
       * in the GraphicsExpose parts, which means that after each refresh,
       * we need to wait for the graphics expose or Noexpose events,
       * which ought to make things real slow!
       */
    case VisibilityNotify:
      switch (ev->xvisibility.state) {
       case VisibilityUnobscured:
         refresh_type = FAST_REFRESH;
         break;

       case VisibilityPartiallyObscured:
         refresh_type = SLOW_REFRESH;
         break;

       default:
         refresh_type = NO_REFRESH;
         break;
      }
      break;

    case FocusIn:
      if (!TermWin.focus)
        {
	    focus_fl=0;
	    /*	    xnc_now_on_top=0;*/
	    TermWin.focus = 1;
#ifndef NO_XLOCALE
	    if (Input_Context != NULL) XSetICFocus (Input_Context);
#endif
        }
      break;

    case FocusOut:
      if (TermWin.focus)
        {
           TermWin.focus = 0;
#ifndef NO_XLOCALE
           if (Input_Context != NULL) XUnsetICFocus (Input_Context);
#endif
        }
      break;

    case ConfigureNotify:
      resize_window ();
      menubar_expose ();
      break;

    case SelectionClear:
      selection_clear ();
      break;

    case SelectionNotify:
      selection_paste (ev->xselection.requestor, ev->xselection.property, True);
      break;

    case SelectionRequest:
      selection_send (&(ev->xselectionrequest));
      break;

    case GraphicsExpose:
    case Expose:
	if (ev->xany.window == TermWin.vt)
        {
	    scr_expose (ev->xexpose.x, ev->xexpose.y,
			ev->xexpose.width, ev->xexpose.height);
        }
	else
        {
	    XEvent unused_xevent;
	    
	    while (XCheckTypedWindowEvent (Xdisplay, ev->xany.window,
					   Expose,
					   &unused_xevent));
	    while (XCheckTypedWindowEvent (Xdisplay, ev->xany.window,
					   GraphicsExpose,
					   &unused_xevent));
	    if (isScrollbarWindow (ev->xany.window))
	    {
                scrollbar_setNone ();
                scrollbar_show (0);
	    }
#ifndef NO_MENUBAR
	    if (menubar_visible () && isMenuBarWindow (ev->xany.window))
		menubar_expose ();
#endif
	    Gr_expose (ev->xany.window);
        }
	break;
	
    case ButtonPress:
      if(ev->xbutton.button==Button4)
      {
	if (TermWin.saveLines)
	{
	  scr_page (UP, TermWin.nrow * 2/5);
	  break;
	}
      }
      if(ev->xbutton.button==Button5)
      {
	if (TermWin.saveLines)
	{
	  scr_page (DN, TermWin.nrow * 2/5);
	  break;
	}
      }


      bypass_keystate = (ev->xbutton.state & (Mod1Mask|ShiftMask));
      reportmode = (bypass_keystate ?
                    0 : (PrivateModes & PrivMode_mouse_report));

      if (ev->xany.window == TermWin.vt)
        {
           if (ev->xbutton.subwindow != None)
             Gr_ButtonPress (ev->xbutton.x, ev->xbutton.y);
           else
             {
                if (reportmode)
                  {
                     if (reportmode & PrivMode_MouseX10)
                       {
                          /* no state info allowed */
                          ev->xbutton.state = 0;
                       }
#ifdef MOUSE_REPORT_DOUBLECLICK
                     if (ev->xbutton.button == Button1)
                       {
                          if (ev->xbutton.time - buttonpress_time < MULTICLICK_TIME)
                            clicks++;
                          else
                            clicks = 1;
                       }
#else
                     clicks = 1;
#endif        /* MOUSE_REPORT_DOUBLECLICK */
                     mouse_report (&(ev->xbutton));
                  }
                else
                  switch (ev->xbutton.button) {
                   case Button1:
                     if (ev->xbutton.time - buttonpress_time < MULTICLICK_TIME)
                       clicks++;
                     else
                       clicks = 1;
                     selection_click (clicks, ev->xbutton.x, ev->xbutton.y);
                     break;

                   case Button3:
                     selection_extend (ev->xbutton.x, ev->xbutton.y);
                     break;
                  }
                buttonpress_time = ev->xbutton.time;
                return;
             }
        }

      if (isScrollbarWindow (ev->xany.window))
        {
           scrollbar_setNone ();
           /*
            * Rxvt-style scrollbar:
            * move up if mouse if above slider
            * move dn if mouse if below slider
            *
            * XTerm-style scrollbar:
            * Move display proportional to pointer location
            * pointer near top -> scroll one line
            * pointer near bot -> scroll full page
            */
#ifndef NO_SCROLLBAR_REPORT
           if (reportmode)
             {
                /*
                 * Mouse report disabled scrollbar:
                 * arrow buttons - send up/down
                 * click on scrollbar - send pageup/down
                 */
                if (scrollbar_upButton (ev->xbutton.y))
                  tt_printf ("\033[A");
                else if (scrollbar_dnButton (ev->xbutton.y))
                  tt_printf ("\033[B");
                else
                  switch (ev->xbutton.button) {
                   case Button2: tt_printf ("\014"); break;
                   case Button1: tt_printf ("\033[6~"); break;
                   case Button3: tt_printf ("\033[5~"); break;
                  }
             }
           else
#endif        /* NO_SCROLLBAR_REPORT */
             {
                if (scrollbar_upButton (ev->xbutton.y))
                  {
                     /* I would like continuous scrolling */
                     if (scr_page (UP, 1)) scrollbar_setUp ();
                  }
                else if (scrollbar_dnButton (ev->xbutton.y))
                  {
                     if (scr_page (DN, 1)) scrollbar_setDn ();
                  }
                else
                  switch (ev->xbutton.button) {
                   case Button2:
#ifndef XTERM_SCROLLBAR
                     if (scrollbar_above_slider (ev->xbutton.y) ||
                         scrollbar_below_slider (ev->xbutton.y))
#endif
                       scr_move_to (scrollbar_position (ev->xbutton.y),
                                    scrollbar_size ());
                     scrollbar_setMotion ();
                     break;

                   case Button1:
                     /*drop*/

                   case Button3:
#ifndef XTERM_SCROLLBAR
                     if (scrollbar_above_slider (ev->xbutton.y))
                       scr_page (UP, TermWin.nrow / 4);
                     else if (scrollbar_below_slider (ev->xbutton.y))
                       scr_page (DN, TermWin.nrow / 4);
                     else
                       scrollbar_setMotion ();
#else        /* XTERM_SCROLLBAR */
                     scr_page ((ev->xbutton.button == Button1 ? DN : UP),
                               (TermWin.nrow *
                                scrollbar_position (ev->xbutton.y) /
                                scrollbar_size ())
                               );
#endif        /* XTERM_SCROLLBAR */
                     break;
                  }
             }
           return;
        }

#ifndef NO_MENUBAR
      if (isMenuBarWindow (ev->xany.window))
        {
           menubar_control (&(ev->xbutton));
           return;
        }
#endif        /* NO_MENUBAR */
      break;

    case ButtonRelease:
      reportmode = (bypass_keystate ?
                    0 : (PrivateModes & PrivMode_mouse_report));

      if (scrollbar_isUpDn ())
        {
           scrollbar_setNone ();
           scrollbar_show (0);
        }

      if (ev->xany.window == TermWin.vt)
        {
           if (ev->xbutton.subwindow != None)
             Gr_ButtonRelease (ev->xbutton.x, ev->xbutton.y);
           else
             {
                if (reportmode)
                  {
                     switch (reportmode & PrivMode_mouse_report) {
                      case PrivMode_MouseX10:
                        break;

                      case PrivMode_MouseX11:
                        ev->xbutton.state = bypass_keystate;
                        ev->xbutton.button = AnyButton;
                        mouse_report (&(ev->xbutton));
                        break;
                     }
                     return;
                  }

                /*
                 * dumb hack to compensate for the failure of click-and-drag
                 * when overriding mouse reporting
                 */
                if ((PrivateModes & PrivMode_mouse_report) &&
                    (bypass_keystate) &&
                    (ev->xbutton.button == Button1) &&
                    (clickOnce ()))
                  selection_extend (ev->xbutton.x, ev->xbutton.y);

                switch (ev->xbutton.button) {
                 case Button1:
                 case Button3:
                   selection_make (ev->xbutton.time);
                   break;

                 case Button2:
                   selection_request (ev->xbutton.time,
                                      ev->xbutton.x, ev->xbutton.y);
                   break;
                }
             }
        }
#ifndef NO_MENUBAR
      else if (isMenuBarWindow (ev->xany.window))
        {
           menubar_control (&(ev->xbutton));
        }
#endif        /* NO_MENUBAR */
      break;

    case MotionNotify:
#ifndef NO_MENUBAR
      if (isMenuBarWindow (ev->xany.window))
        {
           menubar_control (&(ev->xbutton));
           break;
        }
#endif
      if ((PrivateModes & PrivMode_mouse_report) && !(bypass_keystate))
        break;

      if (ev->xany.window == TermWin.vt)
        {
           if ((ev->xbutton.state & (Button1Mask|Button3Mask)) &&
               clickOnce ())
             {
                Window unused_root, unused_child;
                int unused_root_x, unused_root_y;
                unsigned int unused_mask;

                while (XCheckTypedWindowEvent (Xdisplay, TermWin.vt,
                                               MotionNotify, ev));
                XQueryPointer (Xdisplay, TermWin.vt,
                               &unused_root, &unused_child,
                               &unused_root_x, &unused_root_y,
                               &(ev->xbutton.x), &(ev->xbutton.y),
                               &unused_mask);
#ifdef MOUSE_THRESHOLD
                /* deal with a `jumpy' mouse */
                if ((ev->xmotion.time - buttonpress_time) > MOUSE_THRESHOLD)
#endif
                  selection_extend ((ev->xbutton.x), (ev->xbutton.y));
             }
        }
      else if ((ev->xany.window == scrollBar.win) && scrollbar_isMotion ())
        {
           Window unused_root, unused_child;
           int unused_root_x, unused_root_y;
           unsigned int unused_mask;

           while (XCheckTypedWindowEvent (Xdisplay, scrollBar.win,
                                          MotionNotify, ev));
           XQueryPointer (Xdisplay, scrollBar.win,
                          &unused_root, &unused_child,
                          &unused_root_x, &unused_root_y,
                          &(ev->xbutton.x), &(ev->xbutton.y),
                          &unused_mask);
           scr_move_to (scrollbar_position (ev->xbutton.y),
                        scrollbar_size ());
           scr_refresh (refresh_type);
           refresh_count = refresh_limit = 0;
           scrollbar_show (1);
        }
      break;
   }
}
/*}}}*/

/*{{{ tt_write(), tt_printf() - output to command */
/*
 * Send count characters directly to the command
 */

int bool_term_on=1; 

#define  MAX_OUT 2048
char off_buf[MAX_OUT];
int off_count=0;

void
tt_write (const unsigned char * buf, unsigned int count)
{
  if(bool_term_on)
  {
   while (count > 0)
     {
        int n = write (cmd_fd, buf, count);
        if (n > 0)
          {
             count -= n;
             buf += n;
          }
     }
  } else
  {
     int i=0;
     while(i<count && off_count<MAX_OUT && buf[i]!=0)
     {
       if(buf[i]==8)
       {
          if(off_count>0)
                  off_count--;
          i++;
       } else
       if(buf[i]!='\n' && buf[i]!='\r')
               off_buf[off_count++]=buf[i++];
       else
       {
               i++;
               off_buf[off_count]=0;
               if(off_count)
                       psystem(off_buf);
               off_count=0;
       }
     }
  }
}

/*
 * Send printf() formatted output to the command.
 * Only use for small ammounts of data.
 */
void
tt_printf (const unsigned char * fmt, ...)
{
   static unsigned char buf [512];
   va_list arg_ptr;

   va_start (arg_ptr, fmt);
   vsprintf (buf, fmt, arg_ptr);
   va_end (arg_ptr);
   tt_write (buf, strlen (buf));
}
/*}}}*/

/*{{{ print pipe */
/*----------------------------------------------------------------------*/
#ifdef PRINTPIPE
FILE *
popen_printer (void)
{
   FILE * stream = popen (rs_print_pipe, "w");
   if (stream == NULL)
     print_error ("can't open printer pipe");
   return stream;
}

int
pclose_printer (FILE * stream)
{
   fflush (stream);
   /* pclose() reported not to work on SunOS 4.1.3 */
#if defined (__sun__)
   /* pclose works provided SIGCHLD handler uses waitpid */
   return pclose (stream);        /* return fclose (stream); */
#else
   return pclose (stream);
#endif
}

/*
 * simulate attached vt100 printer
 */
static void
process_print_pipe (void)
{
   const char * const escape_seq = "\033[4i";
   const char * const rev_escape_seq = "i4[\033";
   int index;
   FILE * fd;

   if ((fd = popen_printer ()) != NULL)
     {
        for (index = 0; index < 4; /* nil*/)
          {
             unsigned char ch = cmd_getc ();

             if (ch == escape_seq [index]) index++;
             else if (index)
               for (/*nil*/; index > 0; index--)
                 fputc (rev_escape_seq [index-1], fd);

             if (index == 0) fputc (ch, fd);
          }
        pclose_printer (fd);
     }
}
#endif        /* PRINTPIPE */
/*}}}*/

/*{{{ process escape sequences */
static void
process_escape_seq (void)
{
   unsigned char ch = cmd_getc ();

   switch (ch) {
      /* case 1:        do_tek_mode ();        break; */
    case '#':        if (cmd_getc () == '8') scr_E ();        break;
    case '(':        scr_charset_set (0, cmd_getc ());        break;
    case ')':        scr_charset_set (1, cmd_getc ());        break;
    case '*':        scr_charset_set (2, cmd_getc ());        break;
    case '+':        scr_charset_set (3, cmd_getc ());        break;
#ifdef KANJI
    case '$':        scr_charset_set (-2, cmd_getc ());        break;
#endif
    case '7':        scr_cursor (SAVE);        break;
    case '8':        scr_cursor (RESTORE);        break;
    case '=':
    case '>':
      PrivMode ((ch == '='), PrivMode_aplKP);
      break;
    case '@':        (void)cmd_getc ();        break;
    case 'D':        scr_index (UP);                break;
    case 'E':        scr_add_lines ("\n\r", 1, 2);        break;
    case 'G':        process_graphics ();        break;
    case 'H':        scr_set_tab (1);        break;
    case 'M':        scr_index (DN);                break;
    /*case 'N':        scr_single_shift (2);        break;*/
    /*case 'O':        scr_single_shift (3);        break;*/
    case 'Z':        tt_printf (ESCZ_ANSWER);        break;        /* steal obsolete ESC [ c */
    case '[':        process_csi_seq ();        break;
    case ']':        process_xterm_seq ();        break;
    case 'c':        scr_poweron ();                break;
    case 'n':        scr_charset_choose (2);        break;
    case 'o':        scr_charset_choose (3);        break;
   }
}
/*}}}*/

/*{{{ process CSI (code sequence introducer) sequences `ESC[' */
static void
process_csi_seq (void)
{
   unsigned char ch, priv;
   unsigned int nargs;
   int arg [ESC_ARGS];

   nargs = 0;
   arg [0] = 0;
   arg [1] = 0;

   priv = 0;
   ch = cmd_getc ();
   if (ch >= '<' && ch <= '?')
     {
        priv = ch;
        ch = cmd_getc ();
     }

   /* read any numerical arguments */
   do
     {
        int n;
        for (n = 0; isdigit (ch); ch = cmd_getc ())
          n = n * 10 + (ch - '0');

        if (nargs < ESC_ARGS)
          arg [nargs++] = n;
        if (ch == '\b')
          {
             scr_backspace ();
          }
        else if (ch == 033)
          {
             process_escape_seq ();
             return;
          }
             else if (ch < ' ')
          {
             scr_add_lines (&ch, 0, 1);
             return;
          }
        if (ch < '@')
          ch = cmd_getc ();
     }
   while (ch >= ' ' && ch < '@');
   if (ch == 033)
     {
        process_escape_seq ();
        return;
     }
   else if (ch < ' ')
     return;

   switch (ch) {
#ifdef PRINTPIPE
    case 'i':                        /* printing */
      switch (arg [0]) {
       case 0: scr_printscreen (0);                break;
       case 5: process_print_pipe ();                break;
      }
      break;
#endif
    case 'A': case 'e':                /* up <n> */
      scr_gotorc ((arg[0] ? -arg[0] : -1), 0, RELATIVE);        break;
    case 'B':                        /* down <n> */
      scr_gotorc ((arg[0] ? +arg[0] : +1), 0, RELATIVE);        break;
    case 'C': case 'a':                /* right <n> */
      scr_gotorc (0, (arg[0] ? +arg[0] : +1), RELATIVE);        break;
    case 'D':                        /* left <n> */
      scr_gotorc (0, (arg[0] ? -arg[0] : -1), RELATIVE);        break;
    case 'E':                        /* down <n> & to first column */
      scr_gotorc ((arg[0] ? +arg[0] : +1), 0, R_RELATIVE);        break;
    case 'F':                        /* up <n> & to first column */
      scr_gotorc ((arg[0] ? -arg[0] : -1), 0, R_RELATIVE);        break;
    case 'G': case '`':                /* move to col <n> */
      scr_gotorc (0, (arg[0] ? +arg[0] : +1), R_RELATIVE);        break;
    case 'd':                        /* move to row <n> */
      scr_gotorc ((arg[0] ? +arg[0] : +1), 0, C_RELATIVE);        break;
    case 'H': case 'f':                /* position cursor */
      switch (nargs) {
       case 0: scr_gotorc (0, 0, 0); break;
       case 1: scr_gotorc ((arg[0] ? arg[0]-1 : 0), 0, 0);        break;
       default: scr_gotorc (arg[0]-1, arg[1]-1, 0);        break;
      }
      break;
    case 'I':        scr_tab (arg[0] ? +arg[0] : +1);        break;
    case 'Z':        scr_tab (arg[0] ? -arg[0] : -1);        break;
    case 'J':        scr_erase_screen (arg[0]);        break;
    case 'K':        scr_erase_line (arg[0]);        break;
    case '@':        scr_insdel_chars ((arg[0] ? arg [0] : 1), INSERT);        break;
    case 'L':        scr_insdel_lines ((arg[0] ? arg [0] : 1), INSERT);        break;
    case 'M':        scr_insdel_lines ((arg[0] ? arg [0] : 1), DELETE);        break;
    case 'X':        scr_insdel_chars ((arg[0] ? arg [0] : 1), ERASE);        break;
    case 'P':        scr_insdel_chars ((arg[0] ? arg [0] : 1), DELETE);        break;

    case 'c':        tt_printf (VT100_ANS);                break;
    case 'm':        process_sgr_mode (nargs, arg);        break;
    case 'n':                        /* request for information */
      switch (arg[0]) {
       case 5: tt_printf ("\033[0n");                break;        /* ready */
       case 6: scr_report_position ();                break;
#if defined (ENABLE_DISPLAY_ANSWER)
       case 7: tt_printf ("%s\n", display_name);        break;
#endif
       case 8:
         xterm_seq (XTerm_title, APL_NAME "-" VERSION);
         break;
      }
      break;
    case 'r':                        /* set top and bottom margins */
      if (priv != '?')
        {
           if (nargs < 2 || arg[0] >= arg[1])
             scr_scroll_region (0, 10000);
           else
             scr_scroll_region (arg[0] - 1, arg[1] - 1);
           break;
        }
      /* drop */
    case 's':
    case 't':
    case 'h':
    case 'l':
      process_terminal_mode (ch, priv, nargs, arg);
      break;
    case 'g':
      switch (arg[0]) {
       case 0: scr_set_tab (0);                break;        /* delete tab */
       case 3: scr_set_tab (-1);        break;        /* clear all tabs */
      }
      break;
    case 'W':
      switch (arg [0]) {
       case 0: scr_set_tab (1);                break;        /* = ESC H */
       case 2: scr_set_tab (0);                break;        /* = ESC [ 0 g */
       case 5: scr_set_tab (-1);        break;        /* = ESC [ 3 g */
      }
      break;
   }
}
/*}}}*/

/*{{{ process xterm text parameters sequences `ESC ] Ps ; Pt BEL' */
static void
process_xterm_seq (void)
{
   unsigned char ch, string [STRING_MAX];
   int arg;

   ch = cmd_getc ();
   for (arg = 0; isdigit (ch); ch = cmd_getc ())
     arg = arg * 10 + (ch - '0');

   if (ch == ';')
     {
        int n = 0;
        while ((ch = cmd_getc ()) != 007)
          {
             if ((n < sizeof(string)-1))
               {
                  /* silently translate whitespace to space char */
                  if (isspace (ch)) ch = ' ';
                  if (ch >= ' ')
                    string [n++] = ch;
               }
          }
        string [n] = '\0';
        xterm_seq (arg, string);
     }
}
/*}}}*/

/*{{{ process DEC private mode sequences `ESC [ ? Ps mode' */
/*
 * mode can only have the following values:
 *        'l' = low
 *        'h' = high
 *        's' = save
 *        'r' = restore
 *        't' = toggle
 * so no need for fancy checking
 */
static void
process_terminal_mode (int mode, int priv,
                       unsigned int nargs, int arg [])
{
   unsigned int i;
   int state;

   if (nargs == 0) return;

   /* make lo/hi boolean */
   switch (mode) {
    case 'l': mode = 0; break;
    case 'h': mode = 1; break;
   }

   switch (priv) {
    case 0:
      if (mode && mode != 1) return;                /* only do high/low */
      for (i = 0; i < nargs; i++)
        switch (arg [i]) {
         case 4:
           scr_insert_mode (mode);
           break;

         case 36:
           PrivMode (mode, PrivMode_BackSpace);
           break;
           /* case 38:        TEK mode */
        }
      break;

#define PrivCases(bit)        \
if (mode == 't') state = !(PrivateModes & bit); else state = mode;\
switch (state) {\
case 's': SavedModes |= (PrivateModes & bit); continue; break;\
case 'r': state = (SavedModes & bit) ? 1 : 0;/*drop*/\
default:  PrivMode (state, bit); }

    case '?':
      for (i = 0; i < nargs; i++)
        switch (arg [i]) {
         case 1:                /* application cursor keys */
           PrivCases (PrivMode_aplCUR);
           break;

           /* case 2:        - reset charsets to USASCII */

         case 3:                /* 80/132 */
           PrivCases (PrivMode_132);
           if (PrivateModes & PrivMode_132OK)
             set_width (state ? 132 : 80);
           break;

           /* case 4:        - smooth scrolling */

         case 5:                /* reverse video */
           PrivCases (PrivMode_rVideo);
           scr_rvideo_mode (state);
           break;

         case 6:                /* relative/absolute origins  */
           PrivCases (PrivMode_relOrigin);
           scr_relative_origin (state);
           break;

         case 7:                /* autowrap */
           PrivCases (PrivMode_Autowrap);
           scr_autowrap (state);
           break;

           /* case 8:        - auto repeat, can't do on a per window basis */

         case 9:                /* X10 mouse reporting */
           PrivCases (PrivMode_MouseX10);
           /* orthogonal */
           if (PrivateModes & PrivMode_MouseX10)
             PrivateModes &= ~(PrivMode_MouseX11);
           break;

#ifndef NO_MENUBAR
# ifdef menuBar_esc
         case menuBar_esc:
           PrivCases (PrivMode_menuBar);
           map_menuBar (state);
           break;
# endif
#endif        /* NO_MENUBAR */

#ifdef scrollBar_esc
         case scrollBar_esc:
           PrivCases (PrivMode_scrollBar);
           map_scrollBar (state);
           break;
#endif

         case 25:                /* visible/invisible cursor */
           PrivCases (PrivMode_VisibleCursor);
           scr_cursor_visible (state);
           break;

         case 35:
           PrivCases (PrivMode_ShiftKeys);
           break;

         case 36:
           PrivCases (PrivMode_BackSpace);
           break;

         case 40:                /* 80 <--> 132 mode */
           PrivCases (PrivMode_132OK);
           break;

         case 47:                /* secondary screen */
           PrivCases (PrivMode_Screen);
           scr_change_screen (state);
           break;

         case 66:                /* application key pad */
           PrivCases (PrivMode_aplKP);
           break;

         case 1000:                /* X11 mouse reporting */
           PrivCases (PrivMode_MouseX11);
           /* orthogonal */
           if (PrivateModes & PrivMode_MouseX11)
             PrivateModes &= ~(PrivMode_MouseX10);
           break;

#if 0
         case 1001: break;        /* X11 mouse highlighting */
#endif
        }
#undef PrivCases
      break;
   }
}
/*}}}*/

/*{{{ process sgr sequences */
static void
process_sgr_mode (unsigned int nargs, int arg [])
{
   unsigned int i;

   if (nargs == 0)
     {
        scr_rendition (0, ~RS_None);
        return;
     }
   for (i = 0; i < nargs; i++)
     switch (arg [i]) {
      case 0:        scr_rendition (0, ~RS_None);        break;
      case 1:        scr_rendition (1, RS_Bold);        break;
      case 4:        scr_rendition (1, RS_Uline);        break;
      case 5:        scr_rendition (1, RS_Blink);        break;
      case 7:        scr_rendition (1, RS_RVid);        break;
      case 22:        scr_rendition (0, RS_Bold);        break;
      case 24:        scr_rendition (0, RS_Uline);        break;
      case 25:        scr_rendition (0, RS_Blink);        break;
      case 27:        scr_rendition (0, RS_RVid);        break;

      case 30: case 31:                /* set fg color */
      case 32: case 33:
      case 34: case 35:
      case 36: case 37:
        scr_color (minColor + (arg [i] - 30), RS_Bold);
        break;
      case 39:                        /* default fg */
        scr_color (restoreFG, RS_Bold);
        break;

      case 40: case 41:                /* set bg color */
      case 42: case 43:
      case 44: case 45:
      case 46: case 47:
        scr_color (minColor + (arg [i] - 40), RS_Blink);
        break;
      case 49:                        /* default bg */
        scr_color (restoreBG, RS_Blink);
        break;
     }
}
/*}}}*/

/*{{{ process Rob Nation's own graphics mode sequences */
static void
process_graphics (void)
{
   unsigned char ch, cmd = cmd_getc ();
#ifndef RXVT_GRAPHICS
   if (cmd == 'Q')                        /* query graphics */
     {
        tt_printf ("\033G0\n");                /* no graphics */
        return;
     }
   /* swallow other graphics sequences until terminating ':' */
   do ch = cmd_getc (); while (ch != ':');
#else
   int nargs;
   int args [NGRX_PTS];
   unsigned char * text = NULL;

   if (cmd == 'Q')                        /* query graphics */
     {
        tt_printf ("\033G1\n");                /* yes, graphics (color) */
        return;
     }

   for (nargs = 0; nargs < (sizeof(args)/sizeof(args[0]))-1; /*nil*/)
     {
        int neg;

        ch = cmd_getc ();
        neg = (ch == '-');
        if (neg || ch == '+')
          ch = cmd_getc ();

        for (args [nargs] = 0; isdigit (ch); ch = cmd_getc ())
          args [nargs] = args [nargs] * 10 + (ch - '0');
        if (neg) args [nargs] = -args [nargs];

        nargs++;
        args [nargs] = 0;
        if (ch !=  ';')
          break;
     }

   if ((cmd == 'T') && (nargs >= 5))
     {
        int i, len = args [4];

        text = MALLOC ((len+1) * sizeof(char), "text");
        if (text != NULL)
          {
             for (i = 0; i < len; i++)
               text [i] = cmd_getc ();
             text [len] = '\0';
          }
     }

   Gr_do_graphics (cmd, nargs, args, text);
#endif
}
/*}}}*/
void        process_whole_sequence_event()
{
   int ch;
  while ((ch = cmd_getc ()) == 0);        /* wait for something */
      if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        {
           /* Read a text string from the input buffer */
           int nlines = 0;
           unsigned char * str;

           /*
            * point to the start of the string,
            * decrement first since already did get_com_char ()
            */
           str = --cmdbuf_ptr;
           while (cmdbuf_ptr < cmdbuf_endp)
             {
                ch = *cmdbuf_ptr;
                if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r')
                  {
                     cmdbuf_ptr++;
                     if (ch == '\n')
                       {
                          nlines++;
                          refresh_count++;

                          if (refresh_count > (refresh_limit * TermWin.nrow))
                            break;
                       }
                  }
                else                        /* unprintable */
                  {
                     break;
                  }
             }
           scr_add_lines (str, nlines, (cmdbuf_ptr - str));
        }
      else
        {
           switch (ch) {
            case 005: tt_printf (VT100_ANS);        break;        /* terminal Status */
            case 007: scr_bell ();                break;        /* bell */
            case '\b': scr_backspace ();        break;        /* backspace */
            case 013:
            case 014: scr_index (UP);        break;        /* vertical tab, form feed */
            case 016: scr_charset_choose (1);        break;        /* shift out - acs */
            case 017: scr_charset_choose (0);        break;        /* shift in - acs */
            case 033: process_escape_seq ();        break;
           }
        }
}

/*{{{ Read and process output from the application */
void
main_loop (void)
{                                               
   int ch;

   do {
      while ((ch = cmd_getc ()) == 0);        /* wait for something */
      if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        {
           /* Read a text string from the input buffer */
           int nlines = 0;
           unsigned char * str;

           /*
            * point to the start of the string,
            * decrement first since already did get_com_char ()
            */
           str = --cmdbuf_ptr;
           while (cmdbuf_ptr < cmdbuf_endp)
             {
                ch = *cmdbuf_ptr;
                if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r')
                  {
                     cmdbuf_ptr++;
                     if (ch == '\n')
                       {
                          nlines++;
                          refresh_count++;

                          if (refresh_count > (refresh_limit * TermWin.nrow))
                            break;
                       }
                  }
                else                        /* unprintable */
                  {
                     break;
                  }
             }
           scr_add_lines (str, nlines, (cmdbuf_ptr - str));
        }
      else
        {
           switch (ch) {
            case 005: tt_printf (VT100_ANS);        break;        /* terminal Status */
            case 007: scr_bell ();                break;        /* bell */
            case '\b': scr_backspace ();        break;        /* backspace */
            case 013:
            case 014: scr_index (UP);        break;        /* vertical tab, form feed */
            case 016: scr_charset_choose (1);        break;        /* shift out - acs */
            case 017: scr_charset_choose (0);        break;        /* shift in - acs */
            case 033: process_escape_seq ();        break;
           }
        }
   } while (ch != EOF);
}
/*}}}*/
/*----------------------- end-of-file (C source) -----------------------*/
