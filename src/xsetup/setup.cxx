/****************************************************************************
*  Copyright (C) 1996-99 by Leo Khramov
*  email:     leo@xnc.dubna.su
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
#include "pager.h"
#include "image.h"
#include "imload.h"
#include "im_exp.h"
#include "scroll.h"
#include "gui.h"
#include "x_actions.h"
#include "key.h"
#include "tartest.h"
#include "geomfuncs.h"
#include "baseguiplugin.h"

char home_files_dir[1024];

#ifdef XNC_DIR
char *xnc_dir=XNC_DIR;
#else
char *xnc_dir="/usr/local/lib/xnc/data";
#endif


char ininame[1024];

static void print_error(char* err)
{
    fprintf(stderr, "%s", err);
}

extern int sfiles_chr_size;
extern char sfiles_chr[];

int oldtar=1;
char tarvers[80]="nonGNU";

extern int detect_tar_output(char*);

void extract_setup_files(char *home)
{
    char tmp[2048];
    char tmp2[2048];
    int fp;
    sprintf(tmp,"%s/.xnc",home);
    mkdir(tmp,0700);
    sprintf(tmp,"%s/.xnc/.setup",home);
    sprintf(tmp2, "rm -rf %s", tmp);
    system(tmp2);
    if(mkdir(tmp,0700)==-1)
	return;
    chdir(tmp);
    fp=open("files.tar",O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(fp==-1)
    {
	fprintf(stderr,"********Setup Fatal Error: Extract files : %s\n",strerror(errno));
	return;
    }
    write(fp,sfiles_chr,sfiles_chr_size);
    close(fp);
    system("tar -xf files.tar");
    unlink("files.tar");
}

int create_file(char *path, char *fname, char *buf, int siz)
{
    char str[2048];
    int fp;
    sprintf(str,"%s/%s",path,fname);
    fp=open(str,O_WRONLY | O_CREAT,0600);
    if(fp==-1)
	return 0;
    write(fp,buf,siz);
    close(fp);
    return 1;
}


char current_dir[512];
char *rel_dir="./data";

void check_source_dirs()
{
    char setup_dir[2048];
    getcwd(current_dir,512);
    strcpy(home_files_dir,getenv("HOME"));
    sprintf(setup_dir,"%s/.xnc/.setup/data",home_files_dir);
    strcat(home_files_dir,"/.xnc");
    if(chdir(rel_dir)==-1)
	if(chdir(setup_dir)==-1)
	{
	    fprintf(stderr,"\n****************************************************************\n"
		    "Global ERROR: Can't find XNC system configuration files in '%s' or '%s'\n"
		    "Exiting.....\n",rel_dir,setup_dir);
	    exit(256);
	}
}

extern Atom wmDeleteWindow;
int vcenterx,vcentery;
char *karr[AEmpty+1];
int kn[AEmpty+1];
int keynum=0;
extern Panel kpanel;
extern void prect(Window w,GC& gcw,int x,int y,int l,int h);
extern void urect(Window w,GC& gcw,int x,int y,int l,int h);
void fqok();
void fmeskok();
void fmes();
void gen_ini();
void inpfont();
KEY qok(-25,-15,80,25,_noop("Exit"),1,fmes);
KEY qapp(-115,-15,80,25,_noop("Save"),2,gen_ini);
Text tsys(10,60,_noop("Sysfiles Path:"),1);
Text tgeom(10,90,_noop("XNC Geometry:"),1);
Text tedit(10,120,_noop("XNC Editor:"),1);
Text tview(10,150,_noop("XNC Viewer:"),1);
Text tfont(10,50,_noop("Font Name:"),1);
Input ifont(10,65,505,1,inpfont);
Input isys(190,50,320,1);
Input igeom(190,80,320,1);
Input iedit(190,110,320,1);
Input iview(190,140,320,1);
char chsys[255]="/xnctmp";
char chgeom[30]="750x550+10+5";
char chedit[100]="internal";
char chview[100]="internal";
char chfont[100]="";
unsigned char rustbl[256];
unsigned char wrustbl[256];
unsigned char wintbl[256];
unsigned char dostbl[256];
extern int done;
int not_saved=1;
int showed=0;
int kmax;

Win mesw(100,100,250,110,_noop("Exit prompt"),1);
Text mest(40,50,_noop("Save configuration?"),1);
KEY meskok(20,-10,100,25,_noop("Save"),2,fmeskok);
KEY meskcan(-20,-10,100,25,_noop("Don't save"),1,fqok);

char kstr[30];
char* convert_key_to_string(unsigned keycode, unsigned mod)
{
    KeySym ks;
    char *b;
    kstr[0]=0;
    if(keycode==0 && mod==0) return NULL;
    ks=XKeycodeToKeysym(disp,keycode,0);
    if(ks==NoSymbol) return NULL;
    if(mod & ShiftMask)
	strcpy(kstr,"Shift+");
    else
	if(mod & ControlMask)
	    strcpy(kstr,"Ctrl+");
    if(mod & Mod1Mask)
	strcpy(kstr,"Alt+");
    b=XKeysymToString(ks);
    strcat(kstr,b);
    return kstr;
}

void reconstruct_key_string(int j)
{
    ActionKey *ac;
    int i;
    char *b=action_name_by_code(kn[j]);
    sprintf(karr[j],"%-20s: ",b);
    ac=action_by_name(kn[j]);
    for(i=0;i<3;i++)
    {
	b=convert_key_to_string(ac->keys[i],ac->mods[i]);
	if(b)
	{
	    strcat(karr[j],b);
	    strcat(karr[j],"; ");
	}
    }
}

void gen_key_file(char *name)
{
    FILE* fp;
    int i,j;
    char *b;
    char n[255];
    strcpy(n,getenv("HOME"));
    strcat(n,"/.xnc");
    strcat(n,name);
    fp=fopen(n,"w");
    if(fp==NULL) return;
    fprintf(fp,">LSF-keys\n");
    fprintf(fp,"#This file generated automatically by XNC Setup....\n");
    for(j=0;j<kmax;j++)
	fprintf(fp,"%s\n",karr[j]);
    fprintf(fp,">endLSF\n");
    fclose(fp);
}

void init_key_array()
{
    int j=1,i;
    char *b;
    ActionKey* ac;
    compile_internal_key_support();
    parse_file_in_replace_mode("/xnc.keys");
    while(j<AEmpty)
    {
	b=action_name_by_code(j);
	if(b)
	{
	    karr[kmax]=new char[100];
	    kn[kmax]=j;
	    sprintf(karr[kmax],"%-20s: ",b);
	    ac=action_by_name(j);
	    if(ac==NULL)
		delete karr[kmax];
	    else
	    {
		for(i=0;i<3;i++)
		{
		    b=convert_key_to_string(ac->keys[i],ac->mods[i]);
		    if(b)
		    {
			strcat(karr[kmax],b);
			strcat(karr[kmax],"; ");
		    }
		}
		kmax++;
	    }
	};j++;
    }
    kpanel.set_max_to(kmax);
}

int eventkey_to_action()
{
    KeySym ks;
    int n=kpanel.get_current_index(),i;
    ActionKey* ac=action_by_name(kn[n]);
    if(ac==NULL) {XBell(disp,0);return 0;};
    ks=XKeycodeToKeysym(disp,ev.xkey.keycode,0);
    if(ks==XK_Alt_L || ks==XK_Alt_R || ks==XK_Control_L || ks==XK_Control_R || ks==XK_Shift_L || ks==XK_Shift_R) 
        return 0;
    for(i=0;i<keynum;i++)
	if(ac->mods[i]==0 && ac->keys[i]==0) break;
    ac->keys[i]=ev.xkey.keycode;
    ac->mods[i]=ev.xkey.state;
    reconstruct_key_string(n);
    // kpanel.expose();
    return 1;
}

void clear_actionkey()
{
    KeySym ks;
    int n=kpanel.get_current_index(),i;
    ActionKey* ac=action_by_name(kn[n]);
    if(ac==NULL) {XBell(disp,0);return;};
    for(i=0;i<3;i++)
	if(ac->mods[i]==0 && ac->keys[i]==0)
	    break;
    i--;
    if(i<=0)
	return;
    ac->keys[i]=0;
    ac->mods[i]=0;
    reconstruct_key_string(n);
    kpanel.expose();
    XFlush(disp);
}

void fmeskok()
{
    gen_ini();
    fqok();
}

void fmes()
{
    if(not_saved)
    {
	mesw.add_toexpose(&mest);
	mesw.link(&meskok);
	meskok.link(&meskcan);
	meskcan.link(&meskok);
	mesw.show();
	mest.show();
	meskok.show();
	meskcan.show();
	showed=1;
    } else done=1;
}

void layfunc();
void fontfunc();
void colfunc();
MenuItem milay[]={menu_none, _noop("Vertical"),1,layfunc,NULL,0, no_icon, AEmpty,
		  menu_none, _noop("Horizontal"),1,layfunc,NULL,0, no_icon, AEmpty,
		  menu_none, _noop("Page"),1,layfunc,NULL,0, no_icon, AEmpty};
                                       
MenuItem mifont[]={menu_none, _noop("Main Font"),1,fontfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Panels Font"),1,fontfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Fixed Font"),1,fontfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("IVES Font"),1,fontfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Mini Fixed Font"),1,fontfunc,NULL,0, no_icon, AEmpty};


MenuItem micols[]={menu_none, _noop("Background"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Shadow color"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Text color"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Text warning"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Text color 2"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Cursor color"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Panel Info"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Selected file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Extension file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Normal file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Directory"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Execution file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Link file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Archive file"),1,colfunc,NULL,0, no_icon, AEmpty,
		   menu_none, _noop("Image file"),1,colfunc,NULL,0, no_icon, AEmpty
};

int transcol[]={7,0,13,15,15,5,16,4,11,12,6,2,10,9,14};
int  startcol[15][3]={{0xb7,0xb0,0xae},
		      {0x00,0x00,0x00},
		      {0xff,0xff,0xff},
		      {0xff,0x00,0x00},
		      {0x00,0xff,0x00},
		      {0x00,0x00,0xff},
		      {0xff,0xff,0x00},
		      {0x86,0x10,0xb6},
		      {0x8e,0xe5,0xee},
		      {0xff,0xff,0xff},
		      {0x13,0x12,0xf8},
		      {0x00,0xff,0x34},
		      {0x00,0xff,0xf2},
		      {0xfb,0x50,0x56},
		      {0xfb,0x50,0x56}
};
////////////////////////////////////////////////////////////////////////////////////////////////
class KeyWin:public Win
{
public:
    KeyWin(): Win(120,120,240,90,_noop("Define Key"),1,0) {};
    virtual void click();
    virtual void expose();
    virtual void init(Window);
};

KeyWin kw;

void keyfunc1()
{
    kw.show();
    XSetInputFocus(disp,kw.w,RevertToParent,CurrentTime);
    keynum=0;
}

void keyfunc2()
{
    kw.show();
    XSetInputFocus(disp,kw.w,RevertToParent,CurrentTime);
    keynum=1;
}
void keyfunc3()
{
    kw.show();
    XSetInputFocus(disp,kw.w,RevertToParent,CurrentTime);
    keynum=2;
}

void KeyWin::init(Window ipar) 
{
    int tw;
    parent=ipar;
    w=XCreateSimpleWindow(disp,parent,x,y,l,h,0,0,keyscol[1]);
    gcv.background=keyscol[1];
    gcv.font=fontstr->fid;
    gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
    XSelectInput(disp,w,ExposureMask | KeyPressMask);
    tl=strlen(name);
    tw=XTextWidth(fontstr,name,tl);
    if(l<tw+40) l=tw+40;
    ty=(21+fontstr->max_bounds.ascent-fontstr->max_bounds.descent)/2;
    prflg=0;
}

void KeyWin::expose()
{
    XSetForeground(disp,gcw,keyscol[2]);
    XDrawLine(disp,w,gcw,0,0,l,0);
    XDrawLine(disp,w,gcw,0,0,0,h);
    XDrawLine(disp,w,gcw,1,1,l-1,1);
    XDrawLine(disp,w,gcw,1,1,1,h-1);
    XDrawLine(disp,w,gcw,1,21,l-1,21);
    XDrawLine(disp,w,gcw,21,2,21,20);
    XSetForeground(disp,gcw,keyscol[0]);
    XDrawLine(disp,w,gcw,0,h-1,l,h-1);
    XDrawLine(disp,w,gcw,l-1,0,l-1,h);
    XDrawLine(disp,w,gcw,1,h-2,l-1,h-2);
    XDrawLine(disp,w,gcw,l-2,1,l-2,h-1);
    XDrawLine(disp,w,gcw,2,20,l-2,20);
    XDrawLine(disp,w,gcw,20,2,20,20);
    XSetForeground(disp,gcw,cols[0]);
    XDrawString(disp,w,gcw,25,ty+1,name,tl);
    XDrawString(disp,w,gcw,6,ty+2,"X",1);
    XSetForeground(disp,gcw,cols[col]);
    XDrawString(disp,w,gcw,24,ty,name,tl);
    XSetForeground(disp,gcw,cols[2]);
    XDrawString(disp,w,gcw,4,ty,"X",1);
    XDrawString(disp,w,gcw,5,ty+1,"X",1);
    XSetForeground(disp,gcw,cols[1]);
    XDrawString(disp,w,gcw,20,60,_("Now Press Key combination..."),
		strlen(_("Now Press Key combination...")));
    if(ffl)
    {
	if(next) if(next->foc==0 && next->w!=w) 
	    XSetInputFocus(disp,next->w,RevertToNone,CurrentTime);
	else if(next->next) XSetInputFocus(disp,next->next->w,RevertToNone,CurrentTime);
	ffl=0;
    }
    if(next) if(next->w==w) next->expose();
    if(guiexpose) guiexpose->expose();
}

void KeyWin::click()
{
    if(ev.xany.window==w)
    {
	switch(ev.type)
	{
	case Expose: expose();break;
	case KeyPress: if(eventkey_to_action()) hide();break;
	};
    }
}

 
class PicShower:public Gui
{
public:
    int shown,max,ptn;
    unsigned long pt[256];
    XImage *im;
    PICINFO pic;
    char *fname;
    PicShower(int ix,int iy,int il,int ih,char *ifname,int maxcol):Gui()
    {x=ix;y=iy;l=il;h=ih;shown=0;fname=ifname;max=maxcol;};
    PicShower() {if(disp) {XFreeGC(disp,gcw);XDestroyImage(im);};};
    virtual void init(Window);
    virtual void click();
    virtual void expose();
    void setcol(int,int ,int,int);
    void recoding();
};

void PicShower::setcol(int n,int r,int g,int b)
{
    if(r>255) r=255;
    if(g>255) g=255;
    if(b>255) b=255;
    n*=3;
    pic.pal[n]=(r&0xff);
    pic.pal[n+1]=(g&0xff);
    pic.pal[n+2]=(b&0xff);
    delete im->data;
    im->data=NULL;
    XDestroyImage(im);
    im_freecmp(0,pt,ptn);
    im=encoding256(pic,max);
    ptn=im_getpixels(pt);
}

void PicShower::recoding()
{
    delete im->data;
    im->data=NULL;
    XDestroyImage(im);
    im=encoding256(pic,max);
}

void PicShower::init(Window ip)
{
    parent=ip;
    im_reset_coding();
    im_load_gif(fname,&pic,0);
    im_set_coding();
    im=encoding256(pic,max);
    ptn=im_getpixels(pt);
    w=XCreateSimpleWindow(disp,parent,x,y,l,h,1,0,cols[1]);
    gcw=XCreateGC(disp,w,0,NULL);
    XSelectInput(disp,w,ExposureMask);
}       

void PicShower::click()
{
    if(ev.xany.window==w && ev.type==Expose)
	expose();
}

void PicShower::expose()
{
    int ty,tx;
    shown=1;
    XPutImage(disp,w,gcw,im,0,0,0,0,im->width,im->height);
}



class ColShower:public Gui
{
public:
    int shown;
    XColor col;
    ColShower(int ix,int iy,int il,int ih):Gui()
    {x=ix;y=iy;l=il;h=ih;shown=0;col.pixel=0;};
    virtual void init(Window);
    virtual void click();
    virtual void expose();
    void setcol(int ,int,int);
};

void ColShower::setcol(int r,int g,int b)
{
    if(col.pixel) XFreeColors(disp,defcmp,&col.pixel,1,0l);
    if(r>255) r=255;
    if(g>255) g=255;
    if(b>255) b=255;
    r*=256;
    g*=256;
    b*=256;
    col.red=r;
    col.green=g;
    col.blue=b;
    XAllocColor(disp,defcmp,&col);
    if(shown)
	XSetWindowBackground(disp,w,col.pixel);
}

void ColShower::init(Window ip)
{
    parent=ip;
    col.red=col.green=col.blue=65535;
    col.flags=DoRed | DoGreen | DoBlue;
    XAllocColor(disp,defcmp,&col);      
    w=XCreateSimpleWindow(disp,parent,x,y,l,h,1,0,col.pixel);
    XSelectInput(disp,w,ExposureMask);
}       

void ColShower::click()
{
    if(ev.xany.window==w && ev.type==Expose)
	expose();
}

void ColShower::expose()
{
    int ty,tx;
    shown=1;
    XSetWindowBackground(disp,w,col.pixel);
    XClearWindow(disp,w);
}



class FontShower:public Gui
{
public:
    int shown;
    XFontStruct *font;
    FontShower(int ix,int iy,int il,int ih):Gui()
    {x=ix;y=iy;l=il;h=ih;font=NULL;shown=0;};
    ~FontShower() {if(disp) {XFreeGC(disp,gcw);XFreeFont(disp,font);};};
    virtual void init(Window);
    virtual void click();
    virtual void expose();
    virtual void hide();
    void setfont(char *nfont);
};


void FontShower::setfont(char *nfont)
{
    if(font) XFreeFont(disp,font);
    font=XLoadQueryFont(disp,nfont);
    if(font)
	XSetFont(disp,gcw,font->fid);
    if(shown) expose();
}

void FontShower::hide()
{
    Gui::hide();
    shown=0;
}

void FontShower::init(Window ip)
{
    parent=ip;
    w=XCreateSimpleWindow(disp,parent,x,y,l,h,1,0,WhitePixel(disp, DefaultScreen(disp)));
    gcw=XCreateGC(disp,w,0,NULL);
    XSetForeground(disp,gcw,cols[1]);
    XSelectInput(disp,w,ExposureMask);
}       

void FontShower::click()
{
    if(ev.xany.window==w && ev.type==Expose)
	expose();
}

void FontShower::expose()
{
    int ty,tx;
    shown=1;
    XClearWindow(disp,w);
    if(font)
    {
	ty=h/2-(font->max_bounds.ascent+font->max_bounds.descent)/2;
	tx=XTextWidth(font,"ABCDEFsdfghjk1234567890{}בג‎צü",30)/2;
	XDrawString(disp,w,gcw,l/2-tx,ty+font->max_bounds.ascent,
		    "ABCDEFsdfghjk1234567890{}בג‎צü",30);
    }
}

Viewer *vrw=NULL;
void vwquit(Viewer* vv)
{
    vv->hide();
    delete vv;
    vrw=NULL;
}

void whats_new()
{
    struct stat buf;
    FList *fo;
    char *umem="../WHATS_NEW";
    if(stat(umem,&buf)!=-1)
    {
	vrw=new Viewer(20,20,600,450,_("WHAT'S NEW from last version"),2,vwquit);
	vrw->init(Main);
	vrw->show();
	fo=new FList(umem,buf.st_mode,buf.st_size,buf.st_uid,buf.st_gid);
	vrw->view(fo);
    }
}

void setcol();
char **nfonts;                                 
int xnclay=2,lastfont;      
char chfonts[5][100]={"-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*",
		      "-*-helvetica-*-r-*-*-14-*-*-*-*-*-*-*",
		      "-*-fixed-*-r-*-*-14-*-*-*-*-*-*-*",
		      "-*-fixed-*-r-*-*-14-*-*-*-*-*-*-*",
		      "-*-fixed-*-r-*-*-10-*-*-*-*-*-*-*"
};
Sw_panel slay(20,220,200,_noop("XNC Layout"),milay,3);
Sw_panel sfont(10,250,160,_noop("Fonts"),mifont,5);
Sw_panel scols(10,30,150,_noop("Colors"),micols,15);
FontShower fsfont(190,275,295,80);
ColShower cscol(370,275,131,80);
IMLoader gifl(250,220,208,104,3);
PicShower clipl(370,67,131,200,"../xncclip.gif",14);
Text tclip(370,60,_noop("Example:"),1);
Input ired(170,350,50,1,setcol);
char chred[6]="255";
Input igreen(230,350,50,1,setcol);
char chgreen[6]="255";
Input iblue(290,350,50,1,setcol);
char chblue[6]="255";
Text tred(190,60,"R:",2);
Text tgreen(250,60,"G:",3);
Text tblue(310,60,"B:",4);
ScrollBar sbred(190,70,260,NULL);
ScrollBar sbgreen(250,70,260,NULL);
ScrollBar sbblue(310,70,260,NULL);
int curcol=2;

void set_start_colors()
{
    int r,g,b,rr,gg,bb;
    for(int i=0;i<14;i++)
    {
        r=startcol[i][0];
        g=startcol[i][1];
        b=startcol[i][2];
        if(transcol[i]==transcol[0])
	{
	    clipl.pic.pal[3]=r*60/100;
	    clipl.pic.pal[4]=g*60/100;
	    clipl.pic.pal[5]=b*60/100;
	    rr=r*130/100;if(rr>255)  rr=255;
	    gg=g*130/100;if(gg>255)  gg=255;
	    bb=b*130/100;if(bb>255)  bb=255;
	    clipl.pic.pal[24]=rr;
	    clipl.pic.pal[25]=gg;
	    clipl.pic.pal[26]=bb;
	}
        clipl.setcol(transcol[i], r, g, b);
    }
}

void rscring(Gui* o)
{
    int r,g,b;
    ScrollBar* oo=(ScrollBar*)o;
    r=oo->val;
    int i=curcol;
    startcol[i][0]=r;
    g=startcol[i][1];
    b=startcol[i][2];
    cscol.setcol(r,g,b);
    cscol.expose();
    sprintf(chred,"%d",r);
    ired.setbuf(chred,3);             
    ired.expose();
}
 
void gscring(Gui* o)
{
    int r,g,b;
    ScrollBar* oo=(ScrollBar*)o;
    g=oo->val;
    int i=curcol;
    startcol[i][1]=g;
    r=startcol[i][0];
    b=startcol[i][2];
    cscol.setcol(r,g,b);
    cscol.expose();
    sprintf(chgreen,"%d",g);
    igreen.setbuf(chgreen,3);
    igreen.expose();
}
 
void bscring(Gui* o)
{
    int r,g,b;
    ScrollBar* oo=(ScrollBar*)o;
    b=oo->val;
    int i=curcol;
    startcol[i][2]=b;
    g=startcol[i][1];
    r=startcol[i][0];
    cscol.setcol(r,g,b);
    cscol.expose();
    sprintf(chblue,"%d",b);
    iblue.setbuf(chblue,3);
    iblue.expose();
}
      
void rscr(Gui* o)
{
    int r,g,b,rr,gg,bb;
    ScrollBar* oo=(ScrollBar*)o;
    r=oo->val;
    int i=curcol;
    startcol[i][0]=r;
    g=startcol[i][1];
    b=startcol[i][2];
    cscol.setcol(r,g,b);
    cscol.expose();
    if(transcol[i]==transcol[0])
    {
	clipl.pic.pal[3]=r*60/100;
	clipl.pic.pal[4]=g*60/100;
	clipl.pic.pal[5]=b*60/100;
	rr=r*130/100;if(rr>255)  rr=255;
	gg=g*130/100;if(gg>255)  gg=255;
	bb=b*130/100;if(bb>255)  bb=255;
	clipl.pic.pal[24]=rr;
	clipl.pic.pal[25]=gg;
	clipl.pic.pal[26]=bb;
    }
    clipl.setcol(transcol[i],r,g,b);
    clipl.expose();
    sprintf(chred,"%d",r);
    sprintf(chgreen,"%d",g);
    sprintf(chblue,"%d",b);
    ired.setbuf(chred,3);
    ired.expose();
    igreen.setbuf(chgreen,3);
    igreen.expose();
    iblue.setbuf(chblue,3);
    iblue.expose();
}
 
void gscr(Gui* o)
{
    int r,g,b,rr,gg,bb;
    ScrollBar* oo=(ScrollBar*)o;
    g=oo->val;
    int i=curcol;
    startcol[i][1]=g;
    r=startcol[i][0];
    b=startcol[i][2];
    cscol.setcol(r,g,b);
    cscol.expose();
    if(transcol[i]==transcol[0])
    {
	clipl.pic.pal[3]=r*60/100;
	clipl.pic.pal[4]=g*60/100;
	clipl.pic.pal[5]=b*60/100;
	rr=r*130/100;if(rr>255)  rr=255;
	gg=g*130/100;if(gg>255)  gg=255;
	bb=b*130/100;if(bb>255)  bb=255;
	clipl.pic.pal[24]=rr;
	clipl.pic.pal[25]=gg;
	clipl.pic.pal[26]=bb;
    }
    clipl.setcol(transcol[i],r,g,b);
    clipl.expose();
    sprintf(chred,"%d",r);
    sprintf(chgreen,"%d",g);
    sprintf(chblue,"%d",b);
    ired.setbuf(chred,3);
    ired.expose();
    igreen.setbuf(chgreen,3);
    igreen.expose();
    iblue.setbuf(chblue,3);
    iblue.expose();
}
 
void bscr(Gui* o)
{
    int r,g,b,rr,gg,bb;
    ScrollBar* oo=(ScrollBar*)o;
    b=oo->val;
    int i=curcol;
    startcol[i][2]=b;
    g=startcol[i][1];
    r=startcol[i][0];
    cscol.setcol(r,g,b);
    cscol.expose();
    if(transcol[i]==transcol[0])
    {
	clipl.pic.pal[3]=r*60/100;
	clipl.pic.pal[4]=g*60/100;
	clipl.pic.pal[5]=b*60/100;
	rr=r*130/100;if(rr>255)  rr=255;
	gg=g*130/100;if(gg>255)  gg=255;
	bb=b*130/100;if(bb>255)  bb=255;
	clipl.pic.pal[24]=rr;
	clipl.pic.pal[25]=gg;
	clipl.pic.pal[26]=bb;
    }
    clipl.setcol(transcol[i],r,g,b);
    clipl.expose();
    sprintf(chred,"%d",r);
    sprintf(chgreen,"%d",g);
    sprintf(chblue,"%d",b);
    ired.setbuf(chred,3);
    ired.expose();
    igreen.setbuf(chgreen,3);
    igreen.expose();
    iblue.setbuf(chblue,3);
    iblue.expose();
}
 
void setcol()
{
    int r,g,b,rr,gg,bb;
    int i;
    for(i=0;i<14;i++)
	if(scols.get(i)) break;
    sscanf(chred,"%d",&r);
    sscanf(chgreen,"%d",&g);
    sscanf(chblue,"%d",&b);
    startcol[i][0]=r;
    startcol[i][1]=g;
    startcol[i][2]=b;
    sbred.val=r;
    sbgreen.val=g;
    sbblue.val=b;
    sbred.expose();
    sbgreen.expose();
    sbblue.expose();
    cscol.setcol(r,g,b);
    cscol.expose();
    if(transcol[i]==transcol[0])
    {
	clipl.pic.pal[3]=r*60/100;
	clipl.pic.pal[4]=g*60/100;
	clipl.pic.pal[5]=b*60/100;
	rr=r*130/100;if(rr>255)  rr=255;
	gg=g*130/100;if(gg>255)  gg=255;
	bb=b*130/100;if(bb>255)  bb=255;
	clipl.pic.pal[24]=rr;
	clipl.pic.pal[25]=gg;
	clipl.pic.pal[26]=bb;
    }
    clipl.setcol(transcol[i],r,g,b);
    clipl.expose();
}

void colfunc()
{
    int r,g,b;
    int i;
    for(i=0;i<14;i++)
	if(scols.get(i)) break;
    sprintf(chred,"%d",startcol[i][0]);
    sprintf(chgreen,"%d",startcol[i][1]);
    sprintf(chblue,"%d",startcol[i][2]);
    cscol.setcol(startcol[i][0],startcol[i][1],startcol[i][2]);
    cscol.expose();
    ired.setbuf(chred,3);
    ired.expose();
    igreen.setbuf(chgreen,3);
    igreen.expose();
    iblue.setbuf(chblue,3);
    iblue.expose();
    sbred.val=startcol[i][0];
    sbgreen.val=startcol[i][1];
    sbblue.val=startcol[i][2];
    sbred.expose();
    sbgreen.expose();
    sbblue.expose();
    curcol=i;
}

void layfunc()
{
    int i;
    for(i=0;i<3;i++)
	if(slay.get(i)) break;
    gifl.setpage(i);
    gifl.expose();
    xnclay=i;
}

int pchoose(int n,char* f)
{
    strcpy(chfont,f);
    ifont.setbuf(chfont,100);
    ifont.expose();
    fsfont.setfont(f);
    strcpy(chfonts[lastfont],chfont);
    return 0;
}

Panel pfont(10,105,505,130,NULL,0,0l,pchoose);

void inpfont()
{
    int i;
    for(i=0;i<4;i++)
	if(sfont.get(i)) break;
    strcpy(chfonts[lastfont],chfont);
    lastfont=i;
    fsfont.setfont(chfont);
    pfont.find_and_set(chfont);
}


void fontfunc()
{
    int i;
    for(i=0;i<5;i++)
	if(sfont.get(i)) break;
    strcpy(chfonts[lastfont],chfont);
    strcpy(chfont,chfonts[i]);
    lastfont=i;
    ifont.setbuf(chfont,100);
    ifont.expose();
    fsfont.setfont(chfont);
    pfont.find_and_set(chfont);
}

int pid;

void fqok()
{
    if(showed)
    {
	mest.hide();
	meskok.hide();
	meskcan.hide();
	mesw.hide();
	showed=0;
    }
    done=1;
}

void MainExpose()
{
    prect(Main,gc,Mainl-210,Mainh-46,203,38);
}

void XEVENT()
{
    Gui *o;
    int fiflg=1;
    XNextEvent(disp,&ev);
    if(ev.xany.window!=Main)
    {
	if(ev.type==Expose)
	{
	    if(ev.xexpose.count==0)
	    {
		o=findexp(ev.xany.window);
		if(o) o->click();
	    }
	}
	else {
	    o=find(ev.xany.window);
	    if(o) o->click();
        }
    }
    if(ev.type==Expose && ev.xexpose.count==0)
	MainExpose();
    else if(ev.type==ClientMessage && ev.xany.window==Main)
	if (ev.xclient.format == 32 && ev.xclient.data.l[0] == wmDeleteWindow)
	    fmes();
    /*   else
	 if(ev.type==FocusIn || (ev.type==Expose && fiflg==1)) 
	 { if(last) XSetInputFocus(disp,last->w,RevertToNone,CurrentTime);fiflg=0;}
    */}

MenuItem miup[]={menu_none, _noop("Always"),0,NULL,NULL,0, no_icon, AEmpty,
		 menu_none, _noop("Never"),0,NULL,NULL,0, no_icon, AEmpty,
		 menu_none, _noop("Prompt"),0,NULL,NULL,0, no_icon, AEmpty};
Pager p1(0,0,530,430,5);
Separator csep(10,185,510,&p1);
Separator csep2(10,165,510,&p1);
Separator csep3(10,170,510,&p1);
Separator csep4(10,105,510,&p1);

Text tkey(10,45,_noop("Hot Keys to XNC Actions mapping:"),1);
Panel kpanel(10,75,500,300,karr,0,0,NULL);
KEY kkey1(25,385,100,25,_noop("Key1"),1,keyfunc1);
KEY kkey2(140,385,100,25,_noop("Key2"),1,keyfunc2);
KEY kkey3(250,385,100,25,_noop("Key3"),1,keyfunc3);
KEY kkey4(390,385,100,25,_noop("Clear"),2,clear_actionkey);
Text tterm(10,45,_noop("IVES->XNC:"),1);
Switch swsticky(20,65,180,_noop("Sticky terminal"),1);
Switch swraise(20,95,240,_noop("Raise terminal on exec"),1);
Switch swfocus(20,65,240,_noop("Return Focus from IVES"),1);
Text tbookmark(10,125,_noop("Bookmark Options:"),1);
Switch swbook(20,145,200,_noop("Enable Bookmark"),1);
Switch swanim(250,200,270,_noop("Allow Bookmark animation"),1);
Text ticons(10,190,_noop("AFS Options:"),1);
Switch swdiricon(20,265,290,_noop("Show icons with directory names"),1);
Switch swfileicon(20,295,230,_noop("Show icons with files"),1);
Sw_panel supdate(60,220,200,_noop("Update archieve on exit"),miup,3);
char *pn[5]={_noop("Common"),_noop("Colors"),_noop("Fonts"),_noop("Keys"),_noop("Other")};
char *traise="yes";
char *tsticky="yes";
char *ret_foc="yes";
char *book_anim="yes";
char *book_show="yes";
char *dir_icon="yes";
char *file_icon="yes";
char *update_chr="never";

void genrgb(char *t,int r,int g,int b)
{
    r<<=16;
    g<<=8;
    r|=g|b;
    sprintf(t,"#%06x\n",r);
}

char *chncols[]={"northgui.color.background",
		 "northgui.color.shadow",
		 "northgui.color.text",
		 "northgui.color.text_warn",
		 "northgui.color.text2",
		 "northgui.color.cursor",
		 "xnc.panel_color.info",
		 "xnc.panel_color.selected_file",
		 "xnc.panel_color.extension_file",
		 "xnc.panel_color.normal_file",
		 "xnc.panel_color.directory_file",
		 "xnc.panel_color.execution_file",
		 "xnc.panel_color.link_file",
		 "xnc.panel_color.afs_file",
		 "xnc.panel_color.image_file"};

void gen_inis(char *name1,char* name2, int make_full_path=1)
{
    char n[1024]="";
    char tmp[100];
    char *clay[]={_noop("vertical"),_noop("horizontal"),_noop("page")};
    FILE* fp;
    int i;
    if(make_full_path)
    {
	strcpy(n,getenv("HOME"));
	strcat(n,"/.xnc");
    }
    strcat(n,name1);
    fp=fopen(n,"w");
    if(fp!=NULL)
    {
	fprintf(fp,"#This is initialisation file for %s\n",name2);
	fprintf(fp,"#COLORS\n");

	for(i=0;i<15;i++)
	{
	    fprintf(fp,"%s:              ",chncols[i]);
	    genrgb(tmp,startcol[i][0],startcol[i][1],startcol[i][2]);
	    fprintf(fp,"%s",tmp);
	}
	if(swsticky.sw) tsticky="yes"; else tsticky="no";
	if(swraise.sw) traise="yes"; else traise="no";
	if(swfocus.sw) ret_foc="yes"; else ret_foc="no";
	if(swbook.sw) book_show="yes"; else book_show="no";
	if(swanim.sw) book_anim="yes"; else book_anim="no";
	if(swdiricon.sw) dir_icon="yes"; else dir_icon="no";
	if(swfileicon.sw) file_icon="yes"; else file_icon="no";
	if(supdate.get(0)) update_chr="always"; else
	    if(supdate.get(1)) update_chr="never"; else update_chr="prompt";
	fprintf(fp,"#FONTS\n");
	fprintf(fp,"leoprogs.font:   %s\n",chfonts[0]);
	fprintf(fp,"leoprogs.list.font:   %s\n",chfonts[1]);
	fprintf(fp,"leoprogs.font.fixed:   %s\n",chfonts[2]);
	fprintf(fp,"leoprogs.viewer.font:   %s\n",chfonts[3]);
	fprintf(fp,"leoprogs.font.minifixed:   %s\n",chfonts[4]);
	fprintf(fp,"#COMMON\n");
	fprintf(fp,"xnc.sysfiles.path:   %s\n",chsys);
	fprintf(fp,"xnc.editor.name:   %s\n",chedit);
	fprintf(fp,"xnc.viewer.name:   %s\n",chview);
	fprintf(fp,"xnc.geometry:   %s\n",chgeom);
	fprintf(fp,"xnc.viewer.geometry:   %s\n",chgeom);
	fprintf(fp,"xnc.panels.layout:     %s\n",clay[xnclay]);
	fprintf(fp,"xnc.bookmark.show_and_use:     %s\n",book_show);
	fprintf(fp,"xnc.afs.update:     %s\n",update_chr);
	fprintf(fp,"leoprogs.focus.return:     %s\n",ret_foc);
	if(name2[0]=='I')
	    fprintf(fp,"leoprogs.text.shadow:   yes\n");
	fprintf(fp,"#File generated by X Northern Captain Setup.....\n");
	fclose(fp);
    }
}
void gen_ini()
{
    char tmp2[1180]="cp -f ../pixs/* ";
    char tmp1[1180]="cp -f ru.* ";
    char tmp3[1180]="chmod a+w ";
    char tmp4[1180];
    int oldtar;
    not_saved=0;
    sprintf(tmp4,"%s/.xnc/.setup",getenv("HOME"));
    oldtar=detect_tar_output(tmp4);
    system("mkdir $HOME/.xnc >/dev/null 2>/dev/null");
    gen_key_file("/xnc.keys");
    gen_inis(ininame,"X Northern Captain.......",0);
    gen_inis("/ives.ini","IVES.......");
    mkdir(chsys,0777);
    strcat(tmp2,chsys);
    strcat(tmp1,chsys);
    strcat(tmp3,chsys);
    system(tmp3);
    system(tmp2);
    system(tmp1);
    switch(oldtar)
    {
    case GNUTAR_1_11:
	fprintf(stderr,"Using GNU old style tar output for AFS\n");
	system("./setup.script ./.xnc.afs");
	break;
    case GNUTAR_1_12:
	fprintf(stderr,"Using new/ISO GNU style tar output for AFS\n");
	system("./setup.script ./xnc.afs.tar112");
	break;
    case OPENTAR_1_11:
	fprintf(stderr,"Using GNU OpenBSD style tar output for AFS\n");
	system("./setup.script ./xnc.afs.openbsdtar");
	break;
    case AIXTAR_1:
	fprintf(stderr,"Using AIX style tar output for AFS\n");
	system("./setup.script ./xnc.afs.aixtar");
	break;
    default:
	fprintf(stderr, "Detected unknown [%d] tar output! Use default...\n",oldtar);
	fprintf(stderr, "**If You will have problems reading TAR archives then\n");
	fprintf(stderr, "**mail code printed above to <leo@xnc.dubna.su>\n");
	system("./setup.script ./.xnc.afs");
	break;
    }
    strcat(chsys,"/AFS");
    mkdir(chsys,0755);
    strcpy(strrchr(chsys, '/'), "/FTP");
    mkdir(chsys,0700);
    system("./postsetup.script");
    fqok();
}


char *colchr;
char colchr1[100];
char colchr2[100];
char colchr3[100];
char colchr4[100];
char colchr5[100];
char colchr6[100];
char colchr7[100];
char colchr8[100];
char colchr9[100];
char colchr10[100];
char colchr11[100];
char colchr12[100];
char colchr13[100];
char colchr14[100];

X_PAR xpcols[]=
    {
	"northgui.color.background","northgui.color.background",colchr1,
	"northgui.color.shadow","northgui.color.shadow",colchr2,
	"northgui.color.text","northgui.color.text",colchr3,
	"northgui.color.text_warn","northgui.color.text_warn",colchr4,
	"northgui.color.text2","northgui.color.text2",colchr5,
	"northgui.color.cursor","northgui.color.cursor",colchr6,
	"xnc.panel_color.info","xnc.panel_color.info",colchr7,
	"xnc.panel_color.selected_file","xnc.panel_color.selected_file",colchr8,
	"xnc.panel_color.extension_file","xnc.panel_color.extension_file",colchr9,
	"xnc.panel_color.normal_file","xnc.panel_color.normal_file",colchr9,
	"xnc.panel_color.directory_file","xnc.panel_color.directory_file",colchr10,
	"xnc.panel_color.execution_file","xnc.panel_color.execution_file",colchr11,
	"xnc.panel_color.link_file","xnc.panel_color.link_file",colchr12,
	"xnc.panel_color.afs_file","xnc.panel_color.afs_file",colchr13,
	"xnc.panel_color.image_file","xnc.panel_color.image_file",colchr14
    };

int xpcoln=15;

X_PAR xpfont[]=
    {
	"leoprogs.font","LeoProgs.Font",colchr,
	"leoprogs.list.font","LeoProgs.List.Font",colchr,
	"leoprogs.font.fixed","LeoProgs.Font.Fixed",colchr,
	"leoprogs.viewer.font","LeoProgs.Viewer.Font",colchr,
	"leoprogs.font.minifixed","LeoProgs.Font.MiniFixed",colchr
    };
int xpfontn=5;
X_PAR xpcom[]=
    {
	"xnc.sysfiles.path","Xnc.SysFiles.Path",colchr,
	"xnc.editor.name","Xnc.Editor.Name",colchr,
	"xnc.viewer.name","Xnc.Viewer.Name",colchr,
	"xnc.geometry","Xnc.Geometry",colchr,
	"leoprogs.focus.return","LeoProgs.Focus.Return",ret_foc,
	"xnc.bookmark.show_and_use","Xnc.BookMark.Show_and_Use",book_show,
	"xnc.afs.update","Xnc.AFS.Update",update_chr
    };
int xpcomn=7;
char chra[5]="yes";
char chst[5]="yes";
char chre[5]="yes";
char chan[5]="yes";
char chbo[5]="yes";
char chdicon[5]="yes";
char chficon[5]="yes";
char chupd[10]="never";
char *chcom[]={chsys,chedit,chview,chgeom,chre,chbo,chupd};
void load(char *rfile)
{
    char *rmt;
    XColor xcol;
    int i,r,g,b;
    rdb=XrmGetFileDatabase(rfile);
    if(rdb!=NULL)
    {
	for(i=0;i<xpcoln;i++)
	    if(XrmGetResource(rdb,xpcols[i].name,xpcols[i].cname,&rmt,&rmv))
	    {
		strcpy(xpcols[i].pstr, rmv.addr);
		XParseColor(disp,defcmp,rmv.addr,&xcol);
		startcol[i][0]=r=xcol.red/256;
		startcol[i][1]=g=xcol.green/256;
		startcol[i][2]=b=xcol.blue/256;
		if(transcol[i]==transcol[0])
		{
		    clipl.pic.pal[3]=r*60/100;
		    clipl.pic.pal[4]=g*60/100;
		    clipl.pic.pal[5]=b*60/100;
		    clipl.pic.pal[24]=r*130/100;
		    clipl.pic.pal[25]=g*130/100;
		    clipl.pic.pal[26]=b*130/100;
		}
		clipl.pic.pal[transcol[i]*3]=r;
		clipl.pic.pal[transcol[i]*3+1]=g;
		clipl.pic.pal[transcol[i]*3+2]=b;
   
	    }
	    else fprintf(stderr,"Error getting resource: %s\n",xpcols[i].name);
	sprintf(chred,"%d",startcol[2][0]);
	sprintf(chgreen,"%d",startcol[2][1]);
	sprintf(chblue,"%d",startcol[2][2]);
	r=startcol[2][0];
	g=startcol[2][1];
	b=startcol[2][2];
	cscol.setcol(r,g,b);
	sbred.val=r;
	sbgreen.val=g;
	sbblue.val=b;
	clipl.recoding();
	for(i=0;i<xpfontn;i++)
	    if(XrmGetResource(rdb,xpfont[i].name,xpfont[i].cname,&rmt,&rmv))
		strcpy(chfonts[i],rmv.addr);
	for(i=0;i<xpcomn;i++)
	    if(XrmGetResource(rdb,xpcom[i].name,xpcom[i].cname,&rmt,&rmv))
		strcpy(chcom[i],rmv.addr);
	slay.set(0,0);
	if(XrmGetResource(rdb,"xnc.panels.layout","Xnc.Panels.Layout",&rmt,&rmv))
	    if(strcmp(rmv.addr,"vertical")==0) {slay.set(0,1);gifl.setpage(0);xnclay=0;}
	    else if(strcmp(rmv.addr,"horizontal")==0) {slay.set(1,1);gifl.setpage(1);xnclay=1;}
	    else {slay.set(2,1);gifl.setpage(2);xnclay=2;};
    }
    tsticky=chst;
    traise=chra;
    ret_foc=chre;
    book_show=chbo;
    book_anim=chan;
    dir_icon=chdicon;
    file_icon=chficon;
    update_chr=chupd;
    supdate.excl();
    if(strcmp(ret_foc,"yes")==0) swfocus.sw=1;else swfocus.sw=0;
    if(strcmp(book_show,"no")==0) swbook.sw=0;else swbook.sw=1;
    if(strcmp(update_chr,"always")==0) supdate.set(0,1); else
	if(strcmp(update_chr,"prompt")==0) supdate.set(2,1); else supdate.set(1,1);
}          
 
char tmpstr[100];
int main(int argc,char** argv)
{
    char *name=_("X Northern Captain Setup");
    int i,nf,fp11,lfl;
    mlook=LOOKFOUR;
    shadow=0;
    fprintf(stderr,"Initialisation:\n");
    extract_setup_files(getenv("HOME"));
    sprintf(chsys,"%s/.xnc/xnctmp",getenv("HOME"));
    margc=argc;margv=argv; 
    check_source_dirs();
    system("./postsetup.script");
    InitXClient(argc,argv,30,70,540,480,name);
    init_basegui_plugin();
#if 0
    pid=fork();
    if(pid==-1)
    {
	fprintf(stderr,"Can't run Setup\n");
	exit(1);
    }
    if(pid!=0)
    {
	fprintf(stderr,"Running in background\n");
	disp=NULL;
	exit(0);
    } else
#endif
    {
	init_imagengine();
	init_scroll();
	init_l10n();

	GuiLocale::init_locale();
    
	sbred.setpages(25);
	sbgreen.setpages(25);
	sbblue.setpages(25);
	sbred.setrange(0,255);
	sbgreen.setrange(0,255);
	sbblue.setrange(0,255);
	swdiricon.sw=1;
	swfileicon.sw=1;

	mesw.init(DefaultRootWindow(disp));
	mest.init(mesw.w);
	meskok.init(mesw.w);
	meskcan.init(mesw.w);


	qok.init(Main);
	qapp.init(Main);
	p1.init(Main);
	kw.init(Main);
	swsticky.init(p1.w);
	swraise.init(p1.w);
	swfocus.init(p1.w);
	swanim.init(p1.w);
	swbook.init(p1.w);
	swdiricon.init(p1.w);
	swfileicon.init(p1.w);
	supdate.init(p1.w);
	kkey1.init(p1.w);
	kkey2.init(p1.w);
	kkey3.init(p1.w);
	kkey4.init(p1.w);
	tsys.init(p1.w);
	tgeom.init(p1.w);
	tedit.init(p1.w);
	ticons.init(p1.w);
	tbookmark.init(p1.w);
	tterm.init(p1.w);
	tview.init(p1.w);
	tkey.init(p1.w);
	tclip.init(p1.w);
	tred.init(p1.w);
	tgreen.init(p1.w);
	tblue.init(p1.w);
	isys.init(p1.w);
	igeom.init(p1.w);
	iedit.init(p1.w);
	iview.init(p1.w);
	slay.init(p1.w);
	sfont.init(p1.w);
	scols.init(p1.w);
	ifont.init(p1.w);
	ired.init(p1.w);
	igreen.init(p1.w);
	cscol.init(p1.w);
	kpanel.init(p1.w);
	iblue.init(p1.w);
	tfont.init(p1.w);
	pfont.init(p1.w);
	csep.init(p1.w);
	csep2.init(p1.w);
	csep3.init(p1.w);
	csep4.init(p1.w);
	fsfont.init(p1.w);
	gifl.init(p1.w);
	clipl.init(p1.w);
	sbred.init(p1.w);
	sbgreen.init(p1.w);
	sbblue.init(p1.w);
	sbred.val=sbgreen.val=sbblue.val=255;
	slay.excl();
	sfont.excl();
	scols.excl();
	supdate.excl();
	gifl.load("../xncvert.gif");
	gifl.load("../xnchor.gif");
	gifl.load("../xncpage.gif");
	gifl.setpage(2);
	slay.set(2,1);
	sfont.set(0,1);
	scols.set(2,1);
	supdate.set(2,1);
	swsticky.sw=0;
	swraise.sw=1;
	swfocus.sw=0;
	swbook.sw=1;
	swanim.sw=1;
	if(vclass==PseudoColor)
	{
	    swdiricon.sw=0;
	    swfileicon.sw=0;
	} else
	{
	    swdiricon.sw=1;
	    swfileicon.sw=1;
	};
	lfl=1;

	set_start_colors();

	if(argc>1)
	    for(i=1;i<argc;i++)
		if(strcmp(argv[i],"-l")==0) 
		{
		    load(argv[i+1]);
		    lfl=0;
		    strncpy(ininame,argv[i+1],1024);
		};
	if(lfl)
	{
	    sprintf(tmpstr,"%s/.xnc/xnc.ini",getenv("HOME"));
	    strcpy(ininame,tmpstr);
	    if((fp11=open(tmpstr,O_RDONLY))!=-1)
	    {
		close(fp11);
		load(tmpstr);
	    } else
	    {
		sprintf(tmpstr,"%s/.xnc/.setup/data/.xnc.ini",getenv("HOME"));
		if((fp11=open(tmpstr,O_RDONLY))!=-1)
		{
		    close(fp11);
		    load(tmpstr);
		}
	    }
	}
	strcpy(chfont,chfonts[0]);
	isys.setbuf(chsys,255);
	ired.setbuf(chred,3);
	igreen.setbuf(chgreen,3);
	iblue.setbuf(chblue,3);
	igeom.setbuf(chgeom,30);
	iedit.setbuf(chedit,100);
	iview.setbuf(chview,100);
	strcpy(chfont,chfonts[0]);
	ifont.setbuf(chfont,100);
	isys.link(&igeom);
	igeom.link(&iedit);
	iedit.link(&iview);
	iview.link(&slay);
	slay.link(&isys);
	sfont.link(&ifont);
	ifont.link(&pfont);
	pfont.link(&sfont);
	ired.link(&igreen);
	igreen.link(&iblue);
	iblue.link(&ired);
	for(i=0;i<5;i++)
	{
	    p1.setpage(i);
	    p1.setpagename(pn[i]);
	}
	p1.setpage(0);
	p1.pagemaxobj(11);
	p1.addobj(&tsys);
	p1.addobj(&tgeom);
	p1.addobj(&tedit);
	p1.addobj(&tview);
	p1.addobj(&isys);
	p1.addobj(&igeom);
	p1.addobj(&iedit);
	p1.addobj(&iview);
	p1.addobj(&slay);
	p1.addobj(&csep);
	p1.addobj(&gifl);
	p1.setpage(1);
	p1.pagemaxobj(13);
	p1.addobj(&scols);
	p1.addobj(&clipl);
	p1.addobj(&tclip);
	p1.addobj(&ired);
	p1.addobj(&igreen);
	p1.addobj(&iblue);
	p1.addobj(&cscol);
	p1.addobj(&sbred);
	p1.addobj(&sbgreen);
	p1.addobj(&sbblue);
	p1.addobj(&tred);
	p1.addobj(&tgreen);
	p1.addobj(&tblue);
	p1.setpage(2);
	p1.pagemaxobj(5);
	p1.addobj(&sfont);
	p1.addobj(&ifont);
	p1.addobj(&pfont);
	p1.addobj(&tfont);
	p1.addobj(&fsfont);
	p1.setpage(3);
	p1.pagemaxobj(6);
	p1.addobj(&tkey);
	p1.addobj(&kpanel);
	p1.addobj(&kkey1);
	p1.addobj(&kkey2);
	p1.addobj(&kkey3);
	p1.addobj(&kkey4);
	p1.setpage(4);
	p1.pagemaxobj(14);
	p1.addobj(&swfocus);
	p1.addobj(&swbook);
	p1.addobj(&csep3);
	p1.addobj(&csep4);
	p1.addobj(&ticons);
	p1.addobj(&tbookmark);
	p1.addobj(&tterm);
	p1.addobj(&supdate);
	p1.setpage(0);
	fsfont.setfont(chfont);
	sbred.setfuncs(rscr,rscr,rscr,rscring);
	sbgreen.setfuncs(gscr,gscr,gscr,gscring);
	sbblue.setfuncs(bscr,bscr,bscr,bscring);
 
	nfonts=XListFonts(disp,"*",2000,&nf);
	pfont.set_names_to(nfonts);
	pfont.set_max_to(nf);
	fprintf(stderr,"Fonts: %d\n",nf);
	init_key_array();
	wmDeleteWindow = XInternAtom (disp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols (disp, Main, &wmDeleteWindow, 1);
	p1.show();
	qok.show();
	qapp.show();
	whats_new();
	while(!done)  XEVENT();
	XFreeFontNames(nfonts);
	for(i=0;i<kmax;i++)
	    delete karr[i];
	deinit_scroll();
	fprintf(stderr,"Exiting from Setup....");
	if(vrw) {vrw->hide();};
	chdir(current_dir);
	fprintf(stderr,"OK\n");
	DeinitXClient();
    }
    return 0;
}

        
