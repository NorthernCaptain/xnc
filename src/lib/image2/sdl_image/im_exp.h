#ifndef IM_EXP_H
#define IM_EXP_H
#include "globals.h"
#include "picinfo.h"

#ifdef __cplusplus
extern "C" {
#endif


extern int     im_load_gif(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_jpg(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_xpm(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_pcx(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_tga(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_png(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_lbm(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_bmp(char* fname, PICINFO* to_pic, int from_mem_size);
extern int     im_load_tif(char* fname, PICINFO* to_pic, int from_mem_size);


#ifdef __cplusplus
}
#endif

#endif
