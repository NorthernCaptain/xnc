#ifndef IMAGE_H
#define IMAGE_H
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "globals.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "picinfo.h"
#include <math.h>


#define   XSPRITE_LOAD_TRANSPARENT   1
#define   XSPRITE_LOAD_IMAGE_ONLY    2
#define   XSPRITE_IMAGE_LOADED       4
#define   XSPRITE_MASK_LOADED        8

struct xSprite
{
  Pixmap          image;
  Pixmap          image_mask;
  unsigned int    l,h;
  int             flags;
  xSprite(int iflags=XSPRITE_LOAD_IMAGE_ONLY)
  {
    image=image_mask=0;
    flags=iflags;
  };
  void            set_image(Pixmap p, unsigned int il, unsigned int ih);
  void            create_mask_for_image(char* mask_info);
  void            set_mask(Pixmap mask) 
  { 
    image_mask=mask; 
    flags |= (XSPRITE_MASK_LOADED | XSPRITE_LOAD_TRANSPARENT);
  }
};

extern Display* disp;
extern Colormap         defcmp;
extern int conv24;
extern int vclass;
extern void (*im_coding256)(char **,int);

void              init_imagengine();
int               im_convert_true_to_pseudo(PICINFO& pic,int max_colors);
int               im_getpixels(unsigned long* pt);
Colormap          im_get_colormap();
void              im_freecmp(Colormap XColorMap,unsigned long *pt,int iptnum);
void              im_populate_pallete(unsigned long* pal);
void              im_clear_global_pal();
void              im_reset_coding();
void              im_set_coding();
int               im_find_closest_color(XColor* orig, XColor* tocol);
Status            im_AllocColor(Display* d, Colormap cmap, XColor *cl);
void              im_256truecoding(char**,int);
void              im_init_translation();

char*             getpicinfo();

Pixmap            LoadPixmap(char *fname,int type,int& l,int& h,int cmptype=CMP_AUTO);
Pixmap            LoadPixmapFromMem(char *fname,int type,int& l,int& h,
				    int cmptype=CMP_AUTO,int siz=0);
int               LoadPixmapFromMem(char *buf,int size, int type,
				    xSprite& out_image, int cmptype=CMP_AUTO);
XImage*           LoadXImage(char *file,int cmptype,int type);
XImage*           encoding256(PICINFO& im,int ncol);
int               autodetect(char*);

extern "C" void   x11_create_lut(unsigned long, unsigned long, unsigned long);
extern "C" void   set_method(int method);


#endif
//////////////////////////////////////////End of file///////////////////////////////////////////

/* ------------ End of file -------------- */

