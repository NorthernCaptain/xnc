/*
 * file - find type of a file or files - main program.
 *
 * Copyright (c) Ian F. Darwin, 1987.
 * Written by Ian F. Darwin.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or of the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. The author is not responsible for the consequences of use of this
 *    software, no matter how awful, even if they arise from flaws in it.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Since few users ever read sources,
 *    credits must appear in the documentation.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.  Since few users
 *    ever read sources, credits must appear in the documentation.
 *
 * 4. This notice may not be removed or altered.
 */
#ifndef        lint
static char *moduleid = 
        "@(#)$Id: file.c,v 1.2 2002/08/05 10:43:05 leo Exp $";
#endif        /* lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>        /* for MAXPATHLEN */
#include <sys/stat.h>
#include <fcntl.h>        /* for open() */
#include <errno.h>
#if (__COHERENT__ >= 0x420)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <unistd.h>        /* for read() */

#include "readelf.h"
#include <netinet/in.h>                /* for byte swapping */

#include "patchlevel.h"
#include "file.h"

#ifdef S_IFLNK
# define USAGE  "Usage: %s [-vczL] [-f namefile] [-m magicfiles] file...\n"
#else
# define USAGE  "Usage: %s [-vcz] [-f namefile] [-m magicfiles] file...\n"
#endif

#ifndef MAGIC
# define MAGIC "/etc/magic"
#endif

int                         /* Global command-line options                 */
        debug = 0,         /* debugging                                 */
        lflag = 0,        /* follow Symlinks (BSD only)                 */
        zflag = 0;        /* follow (uncompress) compressed files */

int                        /* Misc globals                                */
        nmagic = 0;        /* number of valid magic[]s                 */

struct  magic *magic=NULL;        /* array of magic entries                */

char *magicfile;        /* where magic be found                 */

char *progname;                /* used throughout                         */
int lineno;                /* line number in the magic file        */


static void        unwrap                __P((char *fn));
static int        byteconv4        __P((int, int, int));
static short        byteconv2        __P((int, int, int));


#ifndef MAGIC_FOR_XNC
/*
 * main - parse arguments and handle options
 */
int
main(argc, argv)
int argc;
char *argv[];
{
        int c;
        int check = 0, didsomefiles = 0, errflg = 0, ret = 0, app = 0;

        if ((progname = strrchr(argv[0], '/')) != NULL)
                progname++;
        else
                progname = argv[0];

        if (!(magicfile = getenv("MAGIC")))
                magicfile = MAGIC;

        while ((c = getopt(argc, argv, "vcdf:Lm:z")) != EOF)
                switch (c) {
                case 'v':
                        (void) fprintf(stdout, "%s-%d.%d\n", progname,
                                       FILE_VERSION_MAJOR, patchlevel);
                        return 1;
                case 'c':
                        ++check;
                        break;
                case 'd':
                        ++debug;
                        break;
                case 'f':
                        if (!app) {
                                ret = apprentice(magicfile, check);
                                if (check)
                                        exit(ret);
                                app = 1;
                        }
                        unwrap(optarg);
                        ++didsomefiles;
                        break;
#ifdef S_IFLNK
                case 'L':
                        ++lflag;
                        break;
#endif
                case 'm':
                        magicfile = optarg;
                        break;
                case 'z':
                        zflag++;
                        break;
                case '?':
                default:
                        errflg++;
                        break;
                }

        if (errflg) {
                (void) fprintf(stderr, USAGE, progname);
                exit(2);
        }

        if (!app) {
                ret = apprentice(magicfile, check);
                if (check)
                        exit(ret);
                app = 1;
        }

        if (optind == argc) {
                if (!didsomefiles) {
                        (void)fprintf(stderr, USAGE, progname);
                        exit(2);
                }
        }
        else {
                int i, wid, nw;
                for (wid = 0, i = optind; i < argc; i++) {
                        nw = strlen(argv[i]);
                        if (nw > wid)
                                wid = nw;
                }
                for (; optind < argc; optind++)
                        process(argv[optind], wid);
        }

        return 0;
}

#else

#define MAGIC2  "/etc/magic"

char magic_out[1024];
int magic_ready=0;

/* Return zero if initialization fails */
int
magic_init(char *magic_loc, int show_load)
{
    int ret;
    int check = 0;

    if(magic)
    {
        free(magic);
        magic=NULL;
    }

    if(show_load)    
        fprintf(stderr, "Loading magic.............");
    if (!(magicfile = getenv("MAGIC")))
                 magicfile = magic_loc;
    ret = apprentice(magicfile, check);
    if(ret==-1)
    {
        magicfile=MAGIC;
        ret = apprentice(magicfile, check);
    }
    if(ret==-1)
    {
        magicfile=MAGIC2;
        ret = apprentice(magicfile, check);
    }
    
    if(show_load)
    {
      if(ret==-1)
        fprintf(stderr, "not found (Magic disabled)\n");
      else
        fprintf(stderr, "OK [%s]\n", magicfile);
    }  
    magic_ready=(ret==-1) ? 0 : 1;
    return magic_ready;
}

char *
magic_process(char *fname, struct stat *sb)
{
  magic_out[0]='\0';
  if(magic_ready)
     process(fname, strlen(fname), sb);
  return magic_out;
}

#endif

/*
 * unwrap -- read a file of filenames, do each one.
 */
#ifndef MAGIC_FOR_XNC
static void
unwrap(fn)
char *fn;
{
        char buf[MAXPATHLEN];
        FILE *f;
        int wid = 0, cwid;

        if (strcmp("-", fn) == 0) {
                f = stdin;
                wid = 1;
        } else {
                if ((f = fopen(fn, "r")) == NULL) {
                        error("Cannot open `%s' (%s).\n", fn, strerror(errno));
                        /*NOTREACHED*/
                }

                while (fgets(buf, MAXPATHLEN, f) != NULL) {
                        cwid = strlen(buf) - 1;
                        if (cwid > wid)
                                wid = cwid;
                }

                rewind(f);
        }

        while (fgets(buf, MAXPATHLEN, f) != NULL) {
                buf[strlen(buf)-1] = '\0';
                process(buf, wid);
        }

        (void) fclose(f);
}
#else
static void
unwrap(fn)
char *fn;
{
}
#endif

/*
 * byteconv4
 * Input:
 *        from                4 byte quantity to convert
 *        same                whether to perform byte swapping
 *        big_endian        whether we are a big endian host
 */
static int
byteconv4(from, same, big_endian)
    int from;
    int same;
    int big_endian;
{
  if (same)
    return from;
  else if (big_endian)                /* lsb -> msb conversion on msb */
  {
    union {
      int i;
      char c[4];
    } retval, tmpval;

    tmpval.i = from;
    retval.c[0] = tmpval.c[3];
    retval.c[1] = tmpval.c[2];
    retval.c[2] = tmpval.c[1];
    retval.c[3] = tmpval.c[0];

    return retval.i;
  }
  else
    return ntohl(from);                /* msb -> lsb conversion on lsb */
}

/*
 * byteconv2
 * Same as byteconv4, but for shorts
 */
static short
byteconv2(from, same, big_endian)
        int from;
        int same;
        int big_endian;
{
  if (same)
    return from;
  else if (big_endian)                /* lsb -> msb conversion on msb */
  {
    union {
      short s;
      char c[2];
    } retval, tmpval;

    tmpval.s = (short) from;
    retval.c[0] = tmpval.c[1];
    retval.c[1] = tmpval.c[0];

    return retval.s;
  }
  else
    return ntohs(from);                /* msb -> lsb conversion on lsb */
}

/*
 * process - process input file
 */
#ifndef MAGIC_FOR_XNC
void
process(inname, wid)
const char        *inname;
int wid;
#else
void
process(inname, wid, sb)
const char        *inname;
int wid;
struct stat *sb;
#endif
{
        int        fd = 0;
        static  const char stdname[] = "standard input";
        unsigned char        buf[HOWMANY+1];        /* one extra for terminating '\0' */
        struct utimbuf  utbuf;
#ifndef MAGIC_FOR_XNC
        struct stat        sb;
#endif
        int nbytes = 0;        /* number of bytes read from a datafile */
        char match = '\0';
#ifndef MAGIC_FOR_XNC
        if (strcmp("-", inname) == 0) {
                if (fstat(0, &sb)<0) {
                        error("cannot fstat `%s' (%s).\n", stdname,
                              strerror(errno));
                        /*NOTREACHED*/
                }
                inname = stdname;
        }
#endif
        if (wid > 0)
#ifndef MAGIC_FOR_XNC
             (void) printf("%s:%*s ", inname, 
                           (int) (wid - strlen(inname)), "");
#endif

        if (inname != stdname) {
            /*
             * first try judging the file based on its filesystem status
             */
#ifndef MAGIC_FOR_XNC
            if (fsmagic(inname, &sb) != 0) {
                    putchar('\n');
#else
            if (fsmagic(inname, sb) != 0) {
#endif
                    return;
            }

            if ((fd = open(inname, O_RDONLY)) < 0) {
                    /* We can't open it, but we were able to stat it. */
#ifndef MAGIC_FOR_XNC
                    if (sb.st_mode & 0002) ckfputs("writeable, ", stdout);
                    if (sb.st_mode & 0111) ckfputs("executable, ", stdout);
#else
                    if (sb->st_mode & 0002) ckfputs("writeable, ", stdout);
                    if (sb->st_mode & 0111) ckfputs("executable, ", stdout);
#endif
                    ckfprintf(stdout, "can't read `%s' (%s).\n",
                        inname, strerror(errno));
                    return;
            }
        }


        /*
         * try looking at the first HOWMANY bytes
         */
        if ((nbytes = read(fd, (char *)buf, HOWMANY)) == -1) {
                error("read failed (%s).\n", strerror(errno));
                /*NOTREACHED*/
        }

        if (nbytes == 0)
                ckfputs("empty", stdout);
        else {
                buf[nbytes++] = '\0';        /* null-terminate it */
                match = tryit(buf, nbytes, zflag);
        }

        /*
         * ELF executables have multiple section headers in arbitrary
         * file locations and thus file(1) cannot determine it from easily.
         * Instead we traverse thru all section headers until a symbol table
         * one is found or else the binary is stripped.
         */

        if (match == 's' && nbytes > sizeof (Elf32_Ehdr) &&
            buf[EI_MAG0] == ELFMAG0 &&
            buf[EI_MAG1] == ELFMAG1 &&
            buf[EI_MAG2] == ELFMAG2 &&
            buf[EI_MAG3] == ELFMAG3) {

                union {
                        int l;
                        char c[sizeof (int)];
                } u;

                Elf32_Ehdr elfhdr;
                int stripped = 1;
                int be,same;
                short shnum;

                u.l = 1;
                (void) memcpy(&elfhdr, buf, sizeof elfhdr);

                /*
                 * If the system byteorder does not equal the object byteorder
                 * then need to do byte-swapping.
                 */
                be = u.c[sizeof(int) - 1] == 1; /* are we big endian? */
                same = (u.c[sizeof(int) - 1] + 1) == elfhdr.e_ident[5];
                                /* are we the same endianness? */;

                if (lseek(fd, byteconv4(elfhdr.e_shoff,same,be), SEEK_SET)<0)
                        error("lseek failed (%s).\n", strerror(errno));

                for (shnum = byteconv2(elfhdr.e_shnum,same,be);
                     shnum; shnum--) {
                    if (read(fd, buf,
                             byteconv2(elfhdr.e_shentsize,same,be))<0)
                        error("read failed (%s).\n", strerror(errno));
                    if (byteconv4(((Elf32_Shdr *)buf)->sh_type,same,be)
                        == SHT_SYMTAB) {
                        stripped = 0;
                        break;
                    }
                }
#ifndef MAGIC_FOR_XNC
                if (stripped)
                    (void) printf (", stripped");
                else
                    (void) printf (", not stripped");
#else
                if (stripped)
                    strcat(magic_out, ", stripped");
                else
                    strcat(magic_out, ", not stripped");
#endif
        }

        if (inname != stdname) {
                /*
                 * Try to restore access, modification times if read it.
                 */
#ifndef MAGIC_FOR_XNC
                utbuf.actime = sb.st_atime;
                utbuf.modtime = sb.st_mtime;
#else
                utbuf.actime = sb->st_atime;
                utbuf.modtime = sb->st_mtime;
#endif
                (void) utime(inname, &utbuf); /* don't care if loses */
                (void) close(fd);
        }
#ifndef MAGIC_FOR_XNC
        (void) putchar('\n');
#endif
}


int
tryit(buf, nb, zflag)
unsigned char *buf;
int nb, zflag;
{
        /* try compression stuff */
        if (zflag && zmagic(buf, nb))
                return 'z';

        /* try tests in /etc/magic (or surrogate magic file) */
        if (softmagic(buf, nb))
                return 's';

        /* try known keywords, check whether it is ASCII */
        if (ascmagic(buf, nb))
                return 'a';

        /* abandon hope, all ye who remain here */
        ckfputs("data", stdout);
                return '\0';
}
