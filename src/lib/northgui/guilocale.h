#ifndef GUILOCALE_H
#define GUILOCALE_H

#include "globals.h"
#include "xheaders.h"

class GuiLocale
{

    static bool have_locale;

    static XIMStyle best_style;
    static XIM      im;

    XIC        ic;
    XRectangle preedit_area;
    XRectangle status_area;
    XFontSet   fontset;

    Window     win;

 public:

    static bool init_locale();

    GuiLocale();
    void init(Window, long event_mask);
    void new_fontset(char*);
    int  LookupString(XKeyEvent* ev, char* buf, int bytes, KeySym* ks,
		      XComposeStatus* cs, Status* st);
    void setFocus();
    void unsetFocus();
    void DrawString(GC gc, int x, int y, char* str, int l);
};

#endif

/* ------------ End of file -------------- */

