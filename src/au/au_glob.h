#ifndef AU_GLOB
#define AU_GLOB
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <globdef.h>
#ifdef HAVE_SYS_SIGEVENT_H
#include <sys/sigevent.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include "types.h"
/*
const int EQ=-1;
const int LESS=-2;
const int NOP=-3;
const int None=2;
const int False=0;
const int True=1;
*/
extern FILE *ofile;

class WTree;

struct Vars
{
 WTree *addr;
 float val;
};

extern int verbose;
#endif
/* ------------ End of file -------------- */

