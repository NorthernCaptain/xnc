#ifndef MAIN_H
#define MAIN_H

extern int        xnc_ex;         //Placed in main.cxx: if xnc_ex==0 then no exit clean function for XNC
extern FtpVisual* fvis;           //Placed in main.cxx
extern Switcher*  fsw1;           //Placed in main.cxx

void switch_term_fm_state();

#endif

/* ------------ End of file -------------- */

