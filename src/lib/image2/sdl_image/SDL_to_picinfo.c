#define  JPEG_IMAGES
#include "SDL.h"
#include "picinfo.h"
#include "SDL_image.h"

void im_sdl_create_mask_info(SDL_Surface* surface, PICINFO* pic);

void im_sdl_surface_to_picinfo(SDL_Surface* surface, PICINFO* pic)
{
    int   i;
    byte  *pixels,r,g,b;
    int   red_dx,green_dx,blue_dx;

    pic->w=surface->w;
    pic->h=surface->h;

    red_dx  =surface->format->Rshift/8;
    green_dx=surface->format->Gshift/8;
    blue_dx =surface->format->Bshift/8;

    pic->mask=0;

    if(surface->format->BytesPerPixel==1)
    {
	pic->type=PIC8;
	pic->pic=(byte*)surface->pixels;
	if(surface->flags & SDL_SRCCOLORKEY)
	    im_sdl_create_mask_info(surface, pic);
	surface->pixels=0;
	for(i=0;i<256;i++)
	{
	    pic->pal[i*3]  =surface->format->palette->colors[i].r;
	    pic->pal[i*3+1]=surface->format->palette->colors[i].g;
	    pic->pal[i*3+2]=surface->format->palette->colors[i].b;
	}
	SDL_FreeSurface(surface);
    }
    else if(surface->format->BytesPerPixel==4) //32bit
    {
	pixels=(byte*) surface->pixels;
	pic->type=PIC24;
	pic->pic=(byte*) malloc(pic->w*pic->h*3);
	for(i=0;i<pic->w*pic->h;i++)
	{
	    pic->pic[i*3+0]=pixels[i*4+red_dx];
	    pic->pic[i*3+1]=pixels[i*4+green_dx];
	    pic->pic[i*3+2]=pixels[i*4+blue_dx];
	}
	SDL_FreeSurface(surface);
    } else
    {
	pic->type=PIC24;
	pic->pic=(byte*)surface->pixels;
	surface->pixels=0;
	SDL_FreeSurface(surface);
	if(red_dx!=0 || green_dx!=1)
	{
	    for(i=0;i<pic->w*pic->h;i++)
	    {
		r=pic->pic[i*3+red_dx];
		g=pic->pic[i*3+green_dx];
		b=pic->pic[i*3+blue_dx];
		pic->pic[i*3]=r;
		pic->pic[i*3+1]=g;
		pic->pic[i*3+2]=b;
	    }
	}
    }  
}


void write_mask_to_disk(char* fname, char* mask, int l, int h)
{
#ifdef DEBUG_XNC
    FILE* fp;
    int i;
    fp=fopen(fname,"w");
    if(!fp)
	return;
    fprintf(fp, "#define mask_width %d\n", l);
    fprintf(fp, "#define mask_height %d\n", h);
    fprintf(fp, "static char mask_bits[] = {\n");
    for(i=0;i<l*h/8+1;i++)
    {
	fprintf(fp, " 0x%x", (int)(mask[i]) & 0xff);
	if(i+1!=l*h/8+1)
	    fprintf(fp,",");
    }
    fprintf(fp, "};\n");
    fclose(fp);
#endif
}

void im_sdl_create_mask_info(SDL_Surface* surface, PICINFO* pic)
{
    int   i,j,k,x,y;
    char  transparent_pixel;
    char  *mask_info, *pixels;
    char  mask_bit;
    mask_info=(char*)malloc(surface->w*surface->h/8+1);
    transparent_pixel=surface->format->colorkey;
    pixels=(char*)surface->pixels;

    for(y=0,i=0,k=-1;y<surface->h;y++)
	for(x=0,j=8,mask_bit=1;x<surface->w;x++)
	{
	    if(j==8)
	    {
		mask_bit=1;
		k++;
		j=0;
		mask_info[k]=0;
	    }
	    else
		mask_bit<<=1;
	    if(pixels[i++]!=transparent_pixel)
		mask_info[k]|= mask_bit;
	    j++;
	}
    pic->mask=(byte*)mask_info;
    write_mask_to_disk("/tmp/xnc_images/mask.xbm", mask_info, 
		       surface->w, surface->h);
}

int  im_load_image_through_loader(char* fname, PICINFO* to_pic, 
				  int from_mem_size, 
				  SDL_Surface* (*img_loader)(SDL_RWops*))
{
    SDL_RWops    *rw;
    SDL_Surface  *surface;

    if(!from_mem_size)
	rw=SDL_RWFromFile(fname,"rb");
    else
	rw=SDL_RWFromMem((void*)fname, from_mem_size);

    surface=img_loader(rw);

    SDL_FreeRW(rw);

    im_sdl_surface_to_picinfo(surface,to_pic);

    return 1;
}


/******************************************************************/

int  im_load_gif(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadGIF_RW);
}

int  im_load_jpg(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadJPG_RW);
}

int  im_load_xpm(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadXPM_RW);
}

int  im_load_pcx(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadPCX_RW);
}

int  im_load_tga(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadTGA_RW);
}

int  im_load_png(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadPNG_RW);
}

int  im_load_lbm(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadLBM_RW);
}

int  im_load_bmp(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadBMP_RW);
}

int  im_load_tif(char* fname, PICINFO* to_pic, int from_mem_size)
{

    return im_load_image_through_loader(fname,to_pic,
					from_mem_size,IMG_LoadTIF_RW);
}


