#ifndef AU_H
#define AU_H
#include "au_glob.h"
#include "au_lex.h"

extern int      comp_sup(char *msg, char *sname, int (*sup_init)(char*), int (*aufuncs[])());
extern void     swarning(char *mes);
extern void     serror(char *mes);
extern LexDemon *dm;
extern int      superr;
extern int      supwar;
extern int      au_out;    //Placed in au/au.cxx

#endif


/* ------------ End of file -------------- */

