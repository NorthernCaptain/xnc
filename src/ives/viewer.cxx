/****************************************************************************
*  Copyright (C) 1996 by Leo Khramov
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
#include "globals.h"
#include "xheaders.h"
#include "xh.h"
#include "scroll.h"
#include "baseguiplugin.h"

extern unsigned char rustbl[];
extern unsigned char wrustbl[];
extern unsigned char dostbl[];
extern unsigned char wintbl[];

Atom wmDeleteWindow;

extern void tohex(char*,int,int);

Win *vdw=NULL;
KEY *vdok;
KEY *vdcan;
Input *vdin;
Viewer *vw;
Text *vdt1,*vdt2,*vdt3;
char vdbuf[256];
int vnum;

#define V_FRAME 46


void vok()
{            
    if(vdin->bl)
    {
	vdin->hide();
	vdok->hide();
	vdcan->hide();
	vdw->hide();
	delete vdw;
	delete vdin;
	delete vdok;
	delete vdcan;
	delete vdt1;
	XSetInputFocus(disp,vw->w,RevertToParent,CurrentTime);
	vdw=NULL;
	vw->func(vnum);
    }
}

void vcan()
{
    vdin->hide();
    vdok->hide();
    vdcan->hide();
    vdw->hide();
    delete vdw;
    delete vdin;
    delete vdok;
    delete vdcan;
    delete vdt1;
    vdw=NULL;
    XSetInputFocus(disp,vw->w,RevertToParent,CurrentTime);
}

void saveas(char *head,char *mes,Viewer* ivw,int inum)
{
    if(vdw==NULL)
    {
	vw=ivw;
	vnum=inum;
	vcenterx=ivw->l/2;
	vcentery=ivw->h/2;
	vdw=new Win(vcenterx-150,vcentery-70,303,130,head,1);
	vdok=new KEY(40,-20,80,25,_("OK"),1,vok);
	vdcan=new KEY(-40,-20,80,25,_("Cancel"),2,vcan);
	vdin=new Input(10,50,280,1,vok);
	if(inum==1)
	    vdin->set_histid("FIND");
	else
	    vdin->set_histid("SAVE");
	vdin->set_escapefunc(vcan);
	vdok->set_escapefunc(vcan);
	vdt1=new Text(10,40,mes,5);
	vdw->init(vw->w);
	vdin->init(vdw->w);
	vdok->init(vdw->w);
	vdcan->init(vdw->w);
	vdt1->init(vdw->w);
	vdw->link(vdin);
	vdin->link(vdok);
	vdok->link(vdcan);
	vdcan->link(vdin);
	vdbuf[0]=0;
	if(vnum==0)  strcpy(vdbuf,vw->node->name);
	else strcpy(vdbuf,vw->findbuf);
	vdin->setbuf(vdbuf,255);
	vdw->show();
	vdin->show();
	vdok->show();
	vdcan->show();
	vdt1->show();
    }
}

void vmesok()
{
    vdok->hide();
    vdt1->hide();
    vdw->hide();
    delete vdw;
    delete vdok;
    delete vdt1;
    XSetInputFocus(disp,vw->w,RevertToParent,CurrentTime);
    vdw=NULL;
}

void vmes(char* head,char* mes,Viewer *ivw)
{
    if(vdw==NULL)
    {
	vw=ivw;
	vcenterx=ivw->l/2;
	vcentery=ivw->h/2;
	vdw=new Win(vcenterx-125,vcentery-50,250,100,head,2);
	vdok=new KEY(20,-20,210,25,_("OK"),1,vmesok);
	vdt1=new Text(10,40,mes,5);
	vdok->set_escapefunc(vmesok);
	vdw->init(vw->w);
	vdok->init(vdw->w);
	vdt1->init(vdw->w);
	vdw->link(vdt1);
	vdt1->link(vdok);
	vdok->link(vdok);
	vdw->show();
	vdok->show();
	vdt1->show();
    }
}

///////////////////////////////////Viewer class///////////////////////////////
void Viewer::init(Window ipar)
{
    int tw;
    parent=ipar;
    koi=0;
    XTextProperty wname;
    XRectangle rec;
    win_resize=1;
    w=create_win(name,x,y,l+20,h,ExposureMask | ButtonPressMask | 
		 FocusChangeMask | KeyPressMask | StructureNotifyMask);
    gcv.background=normal_bg_color;
    gcv.foreground=window_text_color;
    gcv.font=fixfontstr->fid;
    gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
    gcv.font=fixfontstr->fid;
    gct=XCreateGC(disp,w,GCBackground | GCFont | GCForeground,&gcv);
    rec.x=0;rec.y=0;rec.width=l-10;rec.height=h-V_FRAME;
    XSetClipRectangles(disp,gct,5,24,&rec,1,YXSorted);
    tl=strlen(name);
    tw=XTextWidth(fontstr,name,tl);
    if(l<tw+40) l=tw+40;
    ty=(21+fixfontstr->max_bounds.ascent-fixfontstr->max_bounds.descent)/2;
    vish=(h-V_FRAME)/ty;
    tx=XTextWidth(fixfontstr,"MMMMMMMMMM",10)/10;
    visl=(l-10)/tx;
    n=(visl-8)/4;
    prflg=row=0;
    XStringListToTextProperty(&name,1,&wname);
    XSetWMIconName(disp,w,&wname);
    scr=baseguiplugin->new_ScrollBar(-2,0,h-23,this);
    scr->setrange(0,10);
    scr->init(w);
    XSetWMProtocols (disp, w, &wmDeleteWindow, 1);
}


void Viewer::reconfigure(int il,int ih)
{
    XRectangle rec;
    l=il-20;h=ih;
    rec.x=0;rec.y=0;rec.width=l-10;rec.height=h-V_FRAME;
    XSetClipRectangles(disp,gct,5,24,&rec,1,YXSorted);
    ty=(21+fixfontstr->max_bounds.ascent-fixfontstr->max_bounds.descent)/2;
    vish=(h-V_FRAME)/ty;
    tx=XTextWidth(fixfontstr,"MMMMMMMMMM",10)/10;
    visl=(l-10)/tx;
    n=(visl-8)/4;
    XClearWindow(disp,w);
    // expose();
    scr->reconfigure(l+18,0,h-23);
    if(external_reconfigure) external_reconfigure(this,x,y,l+20,h);
}

void Viewer::expose()
{
    XSetForeground(disp,gcw,dark_bg_color);
    XDrawLine(disp,w,gcw,0,0,l,0);
    XDrawLine(disp,w,gcw,0,0,0,h-20);
    XDrawLine(disp,w,gcw,1,1,l-1,1);
    XDrawLine(disp,w,gcw,1,1,1,h-21);
    XDrawLine(disp,w,gcw,1,21,l-1,21);
    XSetForeground(disp,gcw,light_bg_color);
    XDrawLine(disp,w,gcw,0,h-21,l,h-21);
    XDrawLine(disp,w,gcw,l-1,0,l-1,h-21);
    XDrawLine(disp,w,gcw,1,h-22,l-1,h-22);
    XDrawLine(disp,w,gcw,l-2,1,l-2,h-21);
    XDrawLine(disp,w,gcw,2,20,l-2,20);
    if(hex)showhexfile();else showasc();
    info();
    if(ffl)
    {
	XSetInputFocus(disp,w,RevertToNone,CurrentTime);
	ffl=0;
    }
}

void Viewer::showhexfile()
{
    char str[150];
    int i,bs,j,k,base1;
    XSetFont(disp,gct,fixfontstr->fid);
    ty=(fixfontstr->max_bounds.ascent+fixfontstr->max_bounds.descent);
    vish=(h-V_FRAME)/ty;
    tx=XTextWidth(fixfontstr,"MMMMMMMMMM",10)/10;
    visl=(l-10)/tx;
    n=(visl-8)/4;
    nn=n*3+8;
    sl=n*4+8;
    for(i=0;i<vish;i++)
    {
	bs=base+i*n;
	tohex(str,bs,6);
	str[7]=' ';
	for(j=0;j<n;j++)
	{
	    k=j*3;
	    if(bs+j<node->size)
	    {
		str[nn+j]=buf[bs+j];
		if(str[nn+j]==0xa) str[nn+j]='.';
		tohex(str+k+7,buf[bs+j],2);
	    } else
		str[nn+j]=str[k+7]=str[k+8]=' ';
	    str[k+9]=' ';
	}
	str[nn-1]=' ';
	XSetForeground(disp,gct,normal_bg_color);
	XFillRectangle(disp,w,gct,5,24+i*ty,(unsigned)l-10,ty);
	XSetForeground(disp,gct,window_text_color);
	XDrawString(disp,w,gct,5,35+i*ty,str,sl);
    }
    scr->val=base/n;scr->expose();
}

void Viewer::scroll(Gui* o)
{
    ScrollBar* oo=(ScrollBar*)o;
    base=0;char *b=buf;int i=0;
    if(hex)
    {
	base=cbase=oo->val*n;
	showhexfile();
	showcurs();
	info();
    } else {
	while(i!=oo->val)
	{
	    if(*b==0xa) i++;
	    *b++;base++;
	}
	row=oo->val;
	info();
	showasc();
    }
}

void Viewer::scrollup(Gui* o)
{
    scroll(o);
}

void Viewer::scrolling(Gui* o)
{
    scroll(o);
}

void Viewer::scrolldown(Gui* o)
{
    scroll(o);
}

void Viewer::back()
{
    if(base>0) 
    {
	base-=2;
	while(base>-1 && buf[base]!=0xa) base--;
	base++;
    }
}

int Viewer::pagedown(int bas)
{
    int i=0;
    while(bas<node->size && i<vish)
    {
	if(buf[bas]==0xa) i++;
	bas++;
    }
    return bas;
}
   
void Viewer::click()
{
    KeySym ks;
    int j; 
    char *st;
    if(ev.xany.window==w)
    {
	switch(ev.type)
	{
	case Expose: expose();break;
	case UnmapNotify: unmap=1;break;
	case ButtonPress:
	    if(ev.xbutton.button!=Button3)
		XSetInputFocus(disp,w,RevertToParent,CurrentTime);
	    else
		if(endfunc) endfunc(this);
	    break;
	case ButtonRelease:break;
	case ConfigureNotify: if(ev.xconfigure.width!=l+20 || ev.xconfigure.height!=h)
	    reconfigure(ev.xconfigure.width,ev.xconfigure.height);break;
	case FocusIn: showcurs();break;
	case ClientMessage:
	    if (ev.xclient.format == 32 && ev.xclient.data.l[0] == wmDeleteWindow && endfunc!=NULL)
		endfunc(this);
	    break;
	case FocusOut: showcurs(0);break;
	case KeyPress:
	    ks=XLookupKeysym(&ev.xkey,0);
	    switch(ks)
	    {
	    case XK_Down:if(hex){ if(cbase+n<node->size)
	    { showcurs(0);cbase+=n;
	    if(cbase-base+1>n*vish)
	    {base+=n;showhexfile();};
	    showcurs();
	    info();
	    };
	    } else
	    {
		j=0;
		while(buf[base+j]!=0xa && buf[base+j]!=0) j++;
		if(base+j<node->size) {base+=j+1;row++;showasc();info();};
	    };
		break;
	    case XK_Up:if(hex){ if(cbase-n>=0) {showcurs(0);cbase-=n;
	    if(cbase<base) {base-=n;showhexfile();};showcurs();info();};
	    }else
		if(base>0) {base-=2;while(base>-1 && buf[base]!=0xa) base--;
		base++;
		row--;
		showasc();
		info();
		};
		break;
	    case XK_End:
	    case XK_Prior:if(hex){ if(base-vish*n>=0)
	    {base-=vish*n;cbase-=vish*n;showhexfile();showcurs();}
	    else if(base!=0) {base=0;cbase-=vish*n;if(cbase<0) cbase=0;
	    showhexfile();showcurs();};
	    }else if(base>0) { for(j=0;j<vish;j++) back();row-=vish;
	    if(row<0) row=0;showasc();};info();break;
       
	    case XK_Next: if(hex){if(base+vish*n<node->size)
	    {base+=vish*n;cbase+=vish*n;
	    if(cbase>=node->size) cbase=base;showhexfile();showcurs();info();};
	    }else if((j=pagedown(base))<node->size) {base=j;row+=vish;showasc();info();};
		break;
	    case XK_Tab: showcurs(0);if(tab==2) tab=0;else tab=2;showcurs();info();break;
	    case XK_Right: if(hex){if(cbase%n+1<n && cbase+1<node->size) 
	    {showcurs(0);cbase++;showcurs();info();};}
	    else {stx+=tx;showasc();};break;
	    case XK_Left: if(hex){
		if(cbase%n-1>=0) {showcurs(0);cbase--;showcurs();info();};
	    } else if(stx>0) {stx-=tx;showasc();};break;
	    case XK_Execute: 
	    case XK_Escape:
	    case XK_F10: 
		if(endfunc) endfunc(this);break;
	    case XK_F2: saveas(_("Save file"),_("Save as:"),this,0);break;
	    case XK_F6: if(swfunc) swfunc(this);break;
	    case XK_F11: rusconvert();break;
	    case XK_F12: wrusconvert();break;
	    case XK_F7:
	    case XK_slash:
	    case XK_Find:  saveas(_("Find"),_("Find text:"),this,1);break;
	    case XK_F3: hex_asc();break;
	    case XK_F8: 
		if(hex) saveas(_("Go to offset..."),_("Hex offset:"),this,2); 
		else saveas(_("Go to line..."),_("Line number:"),this,3);break;
	    case XK_Q:                                                           
	    case XK_q: if(hex==0) {if(endfunc) endfunc(this);break;};
	    default: if(hex) insert(&ks);break;
		//      else {st=XKeysymToString(ks);fprintf(stderr,"Key: %s:%X\n",st,ev.xkey.state);};break;

	    };break;
     
	};
    }
}

void Viewer::hex_asc()
{
    int j;
    hex^=1;
    if(hex)
    {
	base=base-(base%n);cbase=base;
	showhexfile();
	showcurs();info();
	scr->setrange(0,(node->size+n-1)/n);
	scr->val=base/n;
	scr->setpages((node->size+n-1)/n/vish);
	scr->expose();
    }
    else {
	XClearArea(disp,w,5,24,l-10,h-V_FRAME,False);
	while(base>-1 && buf[base]!=0xa) base--;base++;
	showasc();row=0;j=0;
	while(j<base) {if(buf[j]==0xa) row++;j++;};
	info();scr->setrange(0,totallines);scr->val=row;
	scr->setpages(totallines/vish);
	scr->expose();};
}

void Viewer::func(int fn)
{
    switch(fn){
    case 0: save();break;
    case 1: find();break;
    case 2: hexoff();break;
    case 3: linenum();break;
    };
}

void Viewer::line(int bb)
{
    char str[96];
    int bs,i,k,j;
    nn=n*3+8;
    sl=n*4+8;
    bs=bb-bb%n;
    i=(bs-base)/n;
    tohex(str,bs,6);
    str[7]=' ';
    for(j=0;j<n;j++)
    {
	k=j*3;
	if(bs+j<node->size)
	{
	    str[nn+j]=buf[bs+j];
	    if(str[nn+j]==0xa) str[nn+j]='.';
	    tohex(str+k+7,buf[bs+j],2);
	} else
	    str[nn+j]=str[k+7]=str[k+8]=' ';
	str[k+9]=' ';
    }
    str[nn-1]=' ';
    XSetForeground(disp,gct,normal_bg_color);
    XFillRectangle(disp,w,gct,5,24+i*ty,(unsigned)l-10,ty);
    XSetForeground(disp,gct,window_text_color);
    XDrawString(disp,w,gct,5,35+i*ty,str,sl);
}

void Viewer::insert(KeySym *ks)
{
    char *tbl="0123456789ABCDEF";
    XComposeStatus cs;
    char sym[4];
    char str[396];
    int bs,ii,k,j;
    int i,bb=cbase;
    XLookupString(&ev.xkey,sym,4,ks,&cs);
    showcurs(0);
    if(tab==2)
    {
	if(sym[0]<32)
	{
	    showcurs();
	    return;
	}
	buf[cbase]=sym[0];
	nn=n*3+8;
	sl=n*4+8;
	bs=bb-bb%n;
	i=(bs-base)/n;
	tohex(str,bs,6);
	str[7]=' ';
	for(j=0;j<n;j++)
	{
	    k=j*3;
	    if(bs+j<node->size)
	    {
		str[nn+j]=buf[bs+j];
		if(str[nn+j]==0xa) str[nn+j]='.';
		tohex(str+k+7,buf[bs+j],2);
	    } else
		str[nn+j]=str[k+7]=str[k+8]=' ';
	    str[k+9]=' ';
	}
	str[nn-1]=' ';
	XSetForeground(disp,gct,normal_bg_color);
	XFillRectangle(disp,w,gct,5,24+i*ty,(unsigned)l-10,ty);
	XSetForeground(disp,gct,window_text_color);
	XDrawString(disp,w,gct,5,35+i*ty,str,sl);
	if(cbase+1<node->size)
	    cbase++;
    } else
    {
	sym[0]=toupper(sym[0]);
	for(i=0;i<16;i++)
	    if(sym[0]==tbl[i]) break;
	if(i<16)
	{
	    if(tab==0)
	    {
		buf[cbase]&=0x0f;
		buf[cbase]|=(char)(i<<4);
		tab++;
		nn=n*3+8;
		sl=n*4+8;
		bs=bb-bb%n;
		i=(bs-base)/n;
		tohex(str,bs,6);
		str[7]=' ';
		for(j=0;j<n;j++)
		{
		    k=j*3;
		    if(bs+j<node->size)
		    {
			str[nn+j]=buf[bs+j];
			if(str[nn+j]==0xa) str[nn+j]='.';
			tohex(str+k+7,buf[bs+j],2);
		    } else
			str[nn+j]=str[k+7]=str[k+8]=' ';
		    str[k+9]=' ';
		}
		str[nn-1]=' ';
		XSetForeground(disp,gct,normal_bg_color);
		XFillRectangle(disp,w,gct,5,24+i*ty,(unsigned)l-10,ty);
		XSetForeground(disp,gct,window_text_color);
		XDrawString(disp,w,gct,5,35+i*ty,str,sl);
	    } else 
	    {
		buf[cbase]&=0xf0;
		buf[cbase]|=(char)i;
		nn=n*3+8;
		sl=n*4+8;
		bs=bb-bb%n;
		i=(bs-base)/n;
		tohex(str,bs,6);
		str[7]=' ';
		for(j=0;j<n;j++)
		{
		    k=j*3;
		    if(bs+j<node->size)
		    {
			str[nn+j]=buf[bs+j];
			if(str[nn+j]==0xa) str[nn+j]='.';
			tohex(str+k+7,buf[bs+j],2);
		    } else
			str[nn+j]=str[k+7]=str[k+8]=' ';
		    str[k+9]=' ';
		}
		str[nn-1]=' ';
		XSetForeground(disp,gct,normal_bg_color);
		XFillRectangle(disp,w,gct,5,24+i*ty,(unsigned)l-10,ty);
		XSetForeground(disp,gct,window_text_color);
		XDrawString(disp,w,gct,5,35+i*ty,str,sl);
		tab=0;if(cbase+1<node->size) cbase++;
	    }
	}
    }
    if(cbase-base+1>n*vish)
    {base+=n;showhexfile();};
    showcurs();
}

void Viewer::showcurs(int shfl)
{
    int dd,hx,hy;
    if(shfl==0) XSetForeground(disp,gct,normal_bg_color);
    else XSetForeground(disp,gct,window_text2_color);
    if(hex)
    {
	dd=cbase-base;
	hy=dd/n;
	hx=dd%n;
	if(tab==2)
	{
	    XDrawRectangle(disp,w,gct,5+(nn+hx)*tx,24+hy*ty,
			   (unsigned)tx,(unsigned)ty);
	    hx=hx*3+tab+5;
	    XDrawLine(disp,w,gct,5+hx*tx,24+hy*ty+ty,5+hx*tx+tx+tx,24+hy*ty+ty);
	}
	else
	{
	    hx=hx*3+tab+7;
	    XDrawRectangle(disp,w,gct,5+hx*tx,24+hy*ty,(unsigned)tx,
			   (unsigned)ty);
	    hx=dd%n;
	    XDrawLine(disp,w,gct,5+(nn+hx)*tx,24+hy*ty+ty,5+(nn+hx)*tx+tx,24+hy*ty+ty);
	}
    }
    XSetForeground(disp,gct,window_text_color);
}

void Viewer::showasc()
{
    int base1,i,j,bb,lj,jl;
    XSetFont(disp,gct,vfontstr->fid);
    ty=vfontstr->max_bounds.ascent+vfontstr->max_bounds.descent;
    bb=25+vfontstr->max_bounds.ascent;
    vish=(h-V_FRAME)/ty;
    base1=base;
    for(i=0;i<vish;i++)
    {
	XSetForeground(disp,gct,normal_bg_color);
	XFillRectangle(disp,w,gct,5,25+i*ty,(unsigned)l-10,ty);
	XSetForeground(disp,gct,window_text_color);
	if(base1<node->size)
	{
	    j=0;
	    if(highlight)
	    {
		jl=0;
		do {
		    lj=j;
		    while(buf[base1+j]!=0xa && 
			  buf[base1+j]!=0 && 
			  buf[base1+j]!=8 && 
			  buf[base1+j]!=7 && 
			  j<500+lj) j++;
		    XDrawString(disp,w,gct,5-stx+jl,bb+i*ty,buf+base1+lj,j-lj);
		    jl+=XTextWidth(vfontstr,buf+base1+lj,j-lj);
		    XSetForeground(disp,gct,window_text_color);
		    j++;
		} while(buf[base1+j-1]!=0xa && buf[base1+j-1]!=0);
		base1+=j;
	    }
	    else {
		while(buf[base1+j]!=0xa && 
		      buf[base1+j]!=0 && 
		      j<500) j++;
		XDrawString(disp,w,gct,5-stx,bb+i*ty,buf+base1,j);
		base1+=j+1;
	    }
	}
    }
    scr->val=row;
    scr->expose();
}
  
void Viewer::info()
{
    int nl,k;
    char str[32];
    XSetForeground(disp,gcw,normal_bg_color);
    XFillRectangle(disp,w,gcw,2,2,l-4,18);
    XSetForeground(disp,gcw,window_text_color);
    nl=strlen(node->name);
    tx=XTextWidth(fixfontstr,"M",1);
    visl=(l-10)/tx;
    k=visl/3;
    if(nl>k) nl=k-1;
    XDrawString(disp,w,gcw,5,16,node->name,nl);
    if(hex)
    {
	sprintf(str,"%06XH(%08d):%02d%% ",cbase,cbase,(cbase+1)*100/node->size);
	XDrawString(disp,w,gcw,10+k*tx,16,str,22);
    }
    else 
    {
	sprintf(str,"Row:%4d(%06XH):%02d%% ",row+1,base,(base+1)*100/node->size);
	XDrawString(disp,w,gcw,10+k*tx,16,str,22);
    }
    sprintf(str,_("Size: %d"),node->size);
    XDrawString(disp,w,gcw,15+k*2*tx+tx*3,16,str,strlen(str));
}

void Viewer::show() 
{
    //// oldel=el.next;
    // el.next=NULL;
    highlight=0;
    XMapRaised(disp,w);
    addto_el(this,w);
    scr->show();
    ffl=1;
}

void Viewer::hide()
{
    scr->hide();
    XUnmapWindow(disp,w);
    delfrom_el(this);
    // delall_el();
    delfrom_exp(this);
    // el.next=oldel;
    delete buf;
    delete scr;
}


void Viewer::view(FList* cur)
{
    int fp;
    node=cur;
    if(buf) delete buf;
    buf=new char[cur->size+1];
    base=cx=cy=cbase=0;hex=0;tab=stx=row=0;
    fp=open(cur->name,O_RDONLY);
    if(fp!=-1)
    {
	if(read(fp,buf,cur->size)==-1)
	    vmes(_("FATAL ERROR!"),_("Error loading file..."),this);
	close(fp);
	buf[node->size]=0;
	if(scanlines()!=buf+node->size) {hex=1;scr->setrange(0,(node->size+n-1)/n);scr->setpages((node->size+n-1)/n/vish);}
	else
	{
	    if(totallines==0) totallines=1;
	    scr->setpages(totallines/vish);
	    scr->setrange(0,totallines);
	}
	scr->val=base;
	scr->expose();
    } else vmes(_("FATAL ERROR"),_("Permission denied"),this);
}

char* Viewer::scanlines()
{
    char *b=buf;
    totallines=0;
    while(*b!=0)
    {
	if(*b==0xa) totallines++;
	b++;
    }
    return b;
}

void Viewer::viewmem(FList *cur,char *ibuf)
{
    int fp;
    node=cur;
    buf=ibuf;
    base=cx=cy=cbase=0;hex=0;tab=stx=row=0;
    buf[node->size]=0;
    scanlines();
    if(totallines==0) totallines=1;
    scr->setpages(totallines/vish);
    scr->setrange(0,totallines);
    scr->val=base;
}

void Viewer::save()
{
    FILE* fp=fopen(vdbuf,"w");
    if(fp!=NULL)
    {
	if(fwrite(buf,node->size,1,fp)!=1) 
	    fprintf(stderr,"Error:%d\n",strerror(errno));
	fclose(fp);
	//  delete node->name;
	//  node->name=new char[strlen(vdbuf)+1];
	strcpy(node->name,vdbuf);
	chmod(node->name,node->mode);
    } else vmes(_("Error"),_("Can't save this file"),this);
}

void Viewer::hexoff()
{
    int off;
    sscanf(vdbuf,"%X",&off);
    if(off<node->size)
    {
	base=off-off%n;
	cbase=off;
	showhexfile();
	info();
    } else vmes("Error","Wrong address!!!",this);
}
   
  
void Viewer::linenum()
{
    int off,b=0,i=0;
    sscanf(vdbuf,"%d",&off);
    off--;
    if(off<totallines && off>=0)
    {
	while(i<off) {if(buf[b]==0xa) i++;b++;};
	base=b;
	row=off;
	showasc();
	info();
    } else vmes(_("Error"),_("Wrong line number!"),this);
}

void printhbuf(unsigned char *buf, int size)
{
    int i;
    for(i=0;i<size;i++)
	printf("%02X ",buf[i]);
    printf("\n");
}

void Viewer::rusconvert()
{
    int i;
    unsigned char* bu=(unsigned char*)buf;
    if(rusconv_state==KOI8)
    {
	for(i=0;i<node->size;i++)
	    bu[i]=rustbl[bu[i]];
	rusconv_state=CP866;
	xncdprintf(("CP866->KOI8\n"));
    } else
	if(rusconv_state==CP866)
	{
	    for(i=0;i<node->size;i++)
		bu[i]=dostbl[bu[i]];
	    rusconv_state=KOI8;
	    xncdprintf(("KOI8->CP866\n"));
	} else
	{
	    for(i=0;i<node->size;i++)
		bu[i]=wintbl[bu[i]];
	    rusconv_state=KOI8;
	    xncdprintf(("KOI8->WIN1251\n"));
	}
    if(hex) 
    {
	showhexfile();
	showcurs();
    }
    else showasc();

    printhbuf(bu,node->size);
    koi=1;
}

void Viewer::wrusconvert()
{
    int i;
    unsigned char* bu=(unsigned char*)buf;
    if(rusconv_state==KOI8)
    {
	for(i=0;i<node->size;i++)
	    bu[i]=wrustbl[bu[i]];
	rusconv_state=WIN1251;
	xncdprintf(("WIN1251->KOI8\n"));
    } else
	if(rusconv_state==CP866)
	{
	    for(i=0;i<node->size;i++)
		bu[i]=dostbl[bu[i]];
	    rusconv_state=KOI8;
	    xncdprintf(("KOI8->CP866\n"));
	} else
	{
	    for(i=0;i<node->size;i++)
		bu[i]=wintbl[bu[i]];
	    rusconv_state=KOI8;
	    xncdprintf(("KOI8->WIN1251\n"));
	}
    if(hex) 
    {
	showhexfile();
	showcurs();
    }
    else showasc();
    printhbuf(bu,node->size);
    koi=1;
}

void Viewer::find()
{
    int i,j;
    strcpy(findbuf,vdbuf);
    int fl=strlen(findbuf);
    for(i=hex? cbase : base;i<node->size;i++)
	if(buf[i]==findbuf[0])
	{
	    j=1;
	    while(findbuf[j]!=0 && findbuf[j]==buf[i+j]) j++;
	    if(j==fl) break;
	}
    if(j==fl)
    {
	cbase=i;
	base=cbase;
	while(base>-1 && buf[base]!=0xa) base--;
	base++;
	if(hex) {hex=0;hex_asc();showcurs(0);cbase=i;showcurs();}
	else {showasc();row=0;j=0;
	while(j<base) {if(buf[j]==0xa) row++;j++;};
	info();};
    } else vmes(_("Warning"),_("String not found!"),this);
}

/////////////////////////////////End of file//////////////////////////////
