#ifndef ICONMAN_H
#define ICONMAN_H

#include "globals.h"
#include "xheaders.h"
#include "xshrloader.h"


struct Icon
{
    Pixmap          image;
    Pixmap          mask;
    Pixmap          shadow;
    unsigned int    l,h;
    int             from_x, from_y;
    Icon(int ifrom_x=0, int ifrom_y=0,
	 unsigned int il=0, unsigned int ih=0) 
    { 
	image=mask=shadow=0; 
	l=il;
	h=ih; 
	from_x=ifrom_x;
	from_y=ifrom_y; 
    };
    void init(Pixmap iimage, Pixmap imask, Pixmap ishadow,
	      int ifrom_x=0, int ifrom_y=0,
	      unsigned int il=0, unsigned int ih=0) 
    { 
	image=iimage;
	mask=imask;
	shadow=ishadow;
	l=il;
	h=ih; 
	from_x=ifrom_x;
	from_y=ifrom_y; 
    };
};

const int max_iconsets=10;
const int L_ICONSET_NAME=32;

//Icon Manager  loads iconsets, manage and display icons

class IconManager
{
    struct IconSet
    {
	char            name[L_ICONSET_NAME];
	Pixmap          pixmap;
	Pixmap          mask;
	int             l,h;  //sizes of whole pixmap, not an icon
	int             from_x, from_y;
	unsigned int    icon_l, icon_h;
	int             total_icons;
	Icon            *icons;
	IconSet()       { icons=0; total_icons=0;};
	void            create_icons(int ifrom_x, int ifrom_y,
				     unsigned int il, unsigned int ih,
				     int max_icons);
	void            delete_icons();
    };
    
    IconSet             iset[max_iconsets];
    IconSet             iset_shadow[max_iconsets];
    int                 total_isets;

 public:

    IconManager();
    int load_iconset(int idx, char *iname, ResId respixmap, ResId resmask,
		     int ifrom_x, int ifrom_y, 
		     unsigned int icon_l, unsigned int icon_h,
		     int max_icons);

    void change_iconset_style(int idx, int ifrom_x, int ifrom_y);

    void display_icon_from_set(Window w, GC gc, int left_x, int center_y,
			       int iconset_idx, int icon_idx);

    void display_icon_from_set(Window w, int left_x, int center_y,
			       int iconset_idx, int icon_idx);

    void display_icon_from_set_with_shadow(Window w, GC gc, int left_x, int center_y,
					   int iconset_idx, int icon_idx);

    void display_icon_from_set_with_shadow(Window w, int left_x, int center_y,
					   int iconset_idx, int icon_idx);
};

extern IconManager *default_iconman;

#endif

//============================== End of file ==================================

