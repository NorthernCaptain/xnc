#ifndef MYTYPES_H
#define MYTYPES_H

#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef ulong
typedef unsigned long ulong;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

#define L_MAXPATH   2048

#define YES        1
#define NO          0

#define DONT_CARE        0x100000

/* Defines for option_bits variable */
#define FILLED_CURSOR    0x000001
#define IVES_EXIT_TOO    0x000002

/* Status line options */
#define STATUS_TIME      0x000004
#define STATUS_ATTR      0x000008
#define STATUS_SIZE      0x000010
#define STATUS_NAME      0x000020
#define STATUS_OWNER     0x000040
#define STATUS_INODE     0x000080

#define LYNX_NAVIGATION  0x000100

#ifdef HAVE_LONG_LONG
typedef long long        long_size_t;
#else
typedef long        long_size_t;
#endif

#endif
/* ------------ End of file -------------- */

