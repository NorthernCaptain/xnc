#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
struct PCD_IMAGE {
    int            size;
    unsigned char  *mmap;
    int            thumbnails;    /* # of thumbnails, 0 for normal image */

    int            res,nr,gray,verbose;
    int            left,top,width,height,rot;
    unsigned char  **luma;
    unsigned char  **red;
    unsigned char  **blue;
    unsigned char  *data;

    unsigned long  *lut_red;
    unsigned long  *lut_green;
    unsigned long  *lut_blue;

    unsigned char  *seq1; /* huffman tables */
    unsigned char  *len1;
    unsigned char  *seq2;
    unsigned char  *len2;
    unsigned char  *seq3;
    unsigned char  *len3;
};

/* --- file.c --- */

#define PCD_WIDTH(res,rot)   (rot&1?pcd_def_height[res]:pcd_def_width[res])
#define PCD_HEIGHT(res,rot)  (rot&1?pcd_def_width[res]:pcd_def_height[res])

extern char pcd_rotor[];
extern int  pcd_img_start[];
extern int  pcd_def_width[];
extern int  pcd_def_height[];
extern char pcd_errmsg[];

int pcd_open(struct PCD_IMAGE *img, char *filename);
int pcd_get_rot(struct PCD_IMAGE *img, int nr);
int pcd_get_maxres(struct PCD_IMAGE *img);
int pcd_select(struct PCD_IMAGE *img, int res, int nr, int gray, int verbose,
	       int rot, int *left, int *top, int *width, int *height);
int pcd_free(struct PCD_IMAGE *img);
int pcd_close(struct PCD_IMAGE *img);

/* --- yuv2rgb.c --- */
#define RANGE       320
extern unsigned int LUT_range[256+2*RANGE];

extern unsigned long LUT_15_red[256];
extern unsigned long LUT_15_green[256];
extern unsigned long LUT_15_blue[256];

extern unsigned long LUT_16_red[256];
extern unsigned long LUT_16_green[256];
extern unsigned long LUT_16_blue[256];

extern unsigned long LUT_24_red[256];
extern unsigned long LUT_24_green[256];
extern unsigned long LUT_24_blue[256];

#define PCD_TYPE_GRAY       1          /* gray           - 1 byte/pixel */
#define PCD_TYPE_RGB        2          /* red,green,blue - 3 byte/pixel */
#define PCD_TYPE_BGR        3          /* blue,green,red - 3 byte/pixel */
#define PCD_TYPE_LUT_SHORT  4          /* lookup table   - 2 byte/pixel */
#define PCD_TYPE_LUT_LONG   5          /* lookup table   - 4 byte/pixel */

void pcd_get_LUT_init();
void pcd_set_lookup(struct PCD_IMAGE *img, unsigned long *red,
		    unsigned long *green, unsigned long *blue);
int  pcd_get_image_line(struct PCD_IMAGE *img, int line,
			unsigned char *dest, int type, int scale);
int  pcd_get_image(struct PCD_IMAGE *img,
		   unsigned char *dest, int type, int scale);

/* --- huff.c --- */
int  pcd_read_htable(unsigned char *src,
		     unsigned char **pseq, unsigned char **pbits);
int  pcd_decode(struct PCD_IMAGE *img);

/* --- inter.c --- */
int  pcd_inter_m2(struct PCD_IMAGE *img);
int  pcd_inter_pixels(unsigned char **data, int width, int height);
int  pcd_inter_lines(unsigned char **data, int width, int height);

/* ----------------------------------------------------------------- */
#define TELL(x)  { if(img->verbose) fputc(x,stderr); }
#define ROTOR(x) { if(img->verbose) fprintf(stderr,"%c\010",pcd_rotor[x&3]); }


/* ------------ End of file -------------- */

