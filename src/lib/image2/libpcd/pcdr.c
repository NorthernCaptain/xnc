#include "pcd.h"
#define PCD_DEFS
#include "picinfo.h"
#include "accel.h"

unsigned long x11_lut_red[256];
unsigned long x11_lut_green[256];
unsigned long x11_lut_blue[256];
unsigned long x11_lut_gray[256];

extern int pcd_resolution;

void x11_create_lut(unsigned long red_mask,
	       unsigned long green_mask,
	       unsigned long blue_mask)
{
    int rgb_red_bits = 0;
    int rgb_red_shift = 0;
    int rgb_green_bits = 0;
    int rgb_green_shift = 0;
    int rgb_blue_bits = 0;
    int rgb_blue_shift = 0;
    int i;
    unsigned long mask;

    for (i = 0; i < 24; i++) {
	mask = (1 << i);
	if (red_mask & mask) rgb_red_bits++;
	else if (!rgb_red_bits) rgb_red_shift++;
	if (green_mask & mask) rgb_green_bits++;
	else if (!rgb_green_bits) rgb_green_shift++;
	if (blue_mask & mask) rgb_blue_bits++;
	else if (!rgb_blue_bits) rgb_blue_shift++;
    }
#if 0
    printf("color: bits shift\n");
    printf("red  : %04i %05i\n",rgb_red_bits,rgb_red_shift);
    printf("green: %04i %05i\n",rgb_green_bits,rgb_green_shift);
    printf("blue : %04i %05i\n",rgb_blue_bits,rgb_blue_shift);
#endif
    
    for (i = 0; i < 256; i++) {
	x11_lut_red[i]   = (i >> (8-rgb_red_bits  )) << rgb_red_shift;
	x11_lut_green[i] = (i >> (8-rgb_green_bits)) << rgb_green_shift;
	x11_lut_blue[i]  = (i >> (8-rgb_blue_bits )) << rgb_blue_shift;
	x11_lut_gray[i]  =
	    x11_lut_red[i] | x11_lut_green[i] | x11_lut_blue[i];
    }
}

struct PCD_IMAGE   pcdstr;
struct PCD_IMAGE   *pcdi=&pcdstr;

int pcd_left,pcd_top,pcd_width,pcd_height;
int LoadPCD(char* fname,PICINFO* pinfo)
{
 byte *p,*buf;
 int pcd_width4,i,j,res;
 if(pcd_open(pcdi,fname)==-1) return 0;
 pcd_left=pcd_top=pcd_width=pcd_height=0;
 res=pcd_get_maxres(pcdi);
 if(pcd_resolution<res) res=pcd_resolution;
	if (pcd_select(pcdi,res,0,0,0,pcd_get_rot(pcdi,0),
                         &pcd_left,&pcd_top,&pcd_width,&pcd_height)==-1)
    {
         pcd_close(pcdi);
         return 0;
     }
 pcd_set_lookup(pcdi,x11_lut_red,x11_lut_green,x11_lut_blue);
 if(translation)
 pinfo->pic=p=(byte*)malloc(pcd_width*pcd_height*bdep);
 else
 pinfo->pic=p=(byte*)malloc(pcd_width*pcd_height*3);
 pinfo->h=pcd_height;
 pinfo->w=pcd_width;
 pcd_width4=pcd_width*4;
 buf=(byte*) malloc(pcd_width4);
 pinfo->type=PIC24;
 i=0;
 if(res>3) pcd_decode(pcdi);
 if(translation)
 {
 while(i<pcd_height)
 {
 if(pcd_get_image_line(pcdi,i,buf,PCD_TYPE_LUT_LONG,0)==-1)
  {
   free(buf);
   pcd_close(pcdi);
   return 0;
  }
  for(j=0;j<pcd_width4;j+=4)
  {
   *p++=buf[j+1];
   *p++=buf[j];
   }
   i++;
 }
 }
 else
 if(pcd_get_image(pcdi,buf,PCD_TYPE_RGB,0)==-1)
  {
   free(buf);
   pcd_close(pcdi);
   return 0;
  }
  
 free(buf);
 sprintf(pinfo->fullInfo,"Photo CD. Now %dx%d",pcd_width,pcd_height);
 return 1;
}  
