/****************************************************************************
*  Copyright (C) 1996-99 by Leo Khramov
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
#include "xh.h"
#include "guilocale.h"
#include "commonfuncs.h"

bool GuiLocale::have_locale=false;

XIMStyle GuiLocale::best_style;
XIM      GuiLocale::im;


static XIMStyle ChooseBetterStyle(XIMStyle style1, XIMStyle style2)
{
    XIMStyle s,t;
    XIMStyle preedit = XIMPreeditArea | XIMPreeditCallbacks |
        XIMPreeditPosition | XIMPreeditNothing | XIMPreeditNone;
    XIMStyle status = XIMStatusArea | XIMStatusCallbacks |
        XIMStatusNothing | XIMStatusNone;

    if (style1 == 0) return style2;
    if (style2 == 0) return style1;
    if ((style1 & (preedit | status)) == (style2 & (preedit | status)))
        return style1;
    
    s = style1 & preedit;
    t = style2 & preedit;
    if (s != t) {
        if (s | t | XIMPreeditCallbacks)
            return (s == XIMPreeditCallbacks)?style1:style2;
        else if (s | t | XIMPreeditPosition)
            return (s == XIMPreeditPosition)?style1:style2;
        else if (s | t | XIMPreeditArea)
            return (s == XIMPreeditArea)?style1:style2;
        else if (s | t | XIMPreeditNothing)
            return (s == XIMPreeditNothing)?style1:style2;
    }
    else { /* if preedit flags are the same, compare status flags */
        s = style1 & status;
        t = style2 & status;
        if (s | t | XIMStatusCallbacks)
            return (s == XIMStatusCallbacks)?style1:style2;
        else if (s | t | XIMStatusArea)
            return (s == XIMStatusArea)?style1:style2;
        else if (s | t | XIMStatusNothing)
            return (s == XIMStatusNothing)?style1:style2;
    }
    return style1;
}


bool GuiLocale::init_locale()
{
    int i;
    XIMStyles *im_supported_styles;
    XIMStyle app_supported_styles;
    XIMStyle style;

    if (setlocale(LC_ALL, "") == NULL) {
        xncdprintf(("GuiLocale: cannot set locale\n"));
        have_locale=false;
	return false;
    }

    if (!XSupportsLocale()) {
        xncdprintf(("GuiLocale: X does not support locale \"%s\"\n",
                     setlocale(LC_ALL, NULL)));
        have_locale=false;
	return false;
    }

    if (XSetLocaleModifiers("@im=none") == NULL) {
        xncdprintf(("GuiLocale: Warning: cannot set locale modifiers\n"));
    }
    if ((im = XOpenIM(disp, NULL, NULL, NULL)) == NULL) {
        xncdprintf(("GuiLocale:Couldn't open input method\n"));
        have_locale=false;
	return false;
    }
    /* set flags for the styles our application can support */
    app_supported_styles = XIMPreeditNone | XIMPreeditNothing | XIMPreeditArea;
    app_supported_styles |= XIMStatusNone | XIMStatusNothing | XIMStatusArea;

    /* figure out which styles the IM can support */
    XGetIMValues(im, XNQueryInputStyle, &im_supported_styles, NULL);

    /*
     * now look at each of the IM supported styles, and
     * chose the "best" one that we can support.
     */
    best_style = 0;
    for(i=0; i < im_supported_styles->count_styles; i++) {
        style = im_supported_styles->supported_styles[i];
        if ((style & app_supported_styles) == style) /* if we can handle it */
            best_style = ChooseBetterStyle(style, best_style);
    }

    /* if we couldn't support any of them, print an error and exit */
    if (best_style == 0) {
        xncdprintf(("GuiLocale: application and program do not share a\n"
                     "commonly supported interaction style.\n"));
        have_locale=false;
	return false;
    }


    xncdprintf(("GuiLocale: successful initialization\n"));
    have_locale=true;
    return true;
}

GuiLocale::GuiLocale()
{
}

void GuiLocale::new_fontset(char *fsname)
{
    int  i;
    char **missing_charsets;
    int  num_missing_charsets = 0;
    char *default_string;

    fontset = XCreateFontSet(disp,
                             fsname,
                             &missing_charsets, &num_missing_charsets,
                             &default_string);
    
    /*
     * if there are charsets for which no fonts can
     * be found, print a warning message.  
     */
    if (num_missing_charsets > 0) 
    {
        xncdprintf(("GuiLocale: The following charsets are missing:\n"));
        for(i=0; i < num_missing_charsets; i++)
            xncdprintf(("\t%s\n", missing_charsets[i]));
        XFreeStringList(missing_charsets);
        
        xncdprintf(("GuiLocale: The string \"%s\" will be used in place\n",
                      default_string));
        xncdprintf(("GuiLocale: of any characters from those sets.\n"));
    }

}

void GuiLocale::setFocus()
{
  if(have_locale)
    XSetICFocus(ic);
}

void GuiLocale::unsetFocus()
{
  if(have_locale)
    XUnsetICFocus(ic);
}

void GuiLocale::DrawString(GC gc, int x, int y, char* str, int l)
{
  if(have_locale)
    XmbDrawString(disp, win, fontset, gc, x, y, str, l);
  else
    XDrawString(disp, win, gc, x, y, str, l);
}

int GuiLocale::LookupString(XKeyEvent* ev, char* buf, int bytes, KeySym* ks,
			    XComposeStatus* cs, Status* st)
{
  if(have_locale)
    return XmbLookupString(ic, ev, buf, bytes, ks, st);
  return XLookupString(ev, buf, bytes, ks, cs);
}

void GuiLocale::init(Window iwin, long mask)
{
    long im_event_mask;
    XVaNestedList list;
    win=iwin;
    if(have_locale)
    {
      list = XVaCreateNestedList(0,XNFontSet,fontset,NULL);
      ic = XCreateIC(im,
		     XNInputStyle, best_style,
		     XNClientWindow, win,
		     XNPreeditAttributes, list,
		     XNStatusAttributes, list,
		     NULL);
      XFree(list);
      if (ic == NULL) 
      {
        xncdprintf(("Couldn't create input context\n"));
        have_locale=false;
	init(iwin, mask);
	return;
      }
      XGetICValues(ic, XNFilterEvents, &im_event_mask, NULL);
      XSelectInput(disp, win, mask | im_event_mask);
    }
    else
      XSelectInput(disp, win, mask);
} 




