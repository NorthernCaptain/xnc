#ifndef XSHRLOADER_H
#define XSHRLOADER_H

#include "globals.h"
#include "xheaders.h"
#include "image.h"

enum ResType {resPixmap, resBitmap, resBitPixmap, resUnknown};

//structure that describe resource

const int L_RES_NAME=32;

struct ResId
{
    char      name[L_RES_NAME];
    int       img_size;
    char      *img_data;
    char      *fname;
    ResType   type;
    unsigned int img_l, img_h;
    unsigned long fg_color, bg_color;

    ResId(char* iname,char* iimg_data=0,int iimg_size=0): img_size(iimg_size)
    {
	strncpy(name,iname,L_RES_NAME);
	name[L_RES_NAME-1]=0;
	fname=0;
	img_data=iimg_data;
	type=resPixmap;
    };

    ResId(char* iname,char* iimg_data, 
	  unsigned int il, unsigned int ih,
	  unsigned long ifg_color=0, unsigned long ibg_color=0)
    {
	strncpy(name,iname,L_RES_NAME);
	name[L_RES_NAME - 1]=0;
	fname=0;
	img_data=iimg_data;
	if(ifg_color==ibg_color)
	    type=resBitmap;
	else
	    type=resBitPixmap;
	img_l=il;
	img_h=ih;
	fg_color=ifg_color;
	bg_color=ibg_color;
    };    
};

//Working modes for loader
enum LoaderMode { SharedOverride, SharedWrite, SharedRead, NonShared };

//Class that implement XSharedLoader itself
class xsharedLoader
{
  int screen;
  Window root;
  Colormap defcmp;
  int (*prev_handler)(Display*, XErrorEvent*);
  LoaderMode mode;

  int    make_shared(Atom xa_prop, void* data,Atom xa_type, int quant);
  void*  get_shared_res_by_atom(Atom xa_res,Atom xa_type);
  void   ignore_xerror();
  void   restore_xerror();
  Pixmap real_load_pixmap(ResId res, int& l,int& h);
  xSprite real_load_xsprite(ResId res);
  int    make_shared_pixmap(ResId res,int x, int y,void* data);
  int    make_shared_xsprite(ResId res, xSprite sprite);
  Pixmap get_shared_pixmap(ResId res,int& x, int& y);
  xSprite get_shared_xsprite(ResId res);
  void   save_pixmap_to_tmp(ResId res);

public:
  xsharedLoader(LoaderMode m=SharedRead) { set_workmode(m);};
  void    init();
  Pixmap  load_pixmap(ResId, int&l, int& h);
  xSprite load_xsprite(ResId res);
  void    free_pixmap(Pixmap);
  void    exit_now();
  LoaderMode set_workmode(LoaderMode m) { LoaderMode oldmode=mode;mode=m; return oldmode; };
};

extern xsharedLoader *default_loader;

#endif
