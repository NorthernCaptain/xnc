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
#include "imview.h"
#include "guilocale.h"
#include "baseguiplugin.h"
#include "scroll.h"
#include "bv1.xbm"
#include "filebrowser.h"
#include "history.h"
#include "geomfuncs.h"
#include "ivesserver.h"
#include "ivesclient.h"
#include "ivesversion.h"
#include "commonfuncs.h"

IVES_server *iserv;
IVES_client *iclient;

char n1[140]="IVES v"IVES_VERSION;
extern Atom wmDeleteWindow;

Atom DndProtocol, DndSelection;
Atom Old_DndProtocol, Old_DndSelection;


int mapped=0;
int internal_use=0;
int xpml,xpmh;
extern int return_focus;

const int D_LEN=200;

int xfp;

static Window root;
static Window focus_to;

void new_data(IVES_data data);


class IVES_dnd:public DndObject
{
  char fname[L_MAXPATH];
public:
  IVES_dnd():DndObject() {};
  virtual int   dnd_drop(DndData* dd, int, int, Atom)
  {
    convert_one_filename(dd->data,fname);
    xncdprintf(("Opening %s file for editing\n",fname));
    internal_use=1;
    new_data(IVES_data(1,fname+5));
    delete dd;
    return 1;
  };
};


FileBrowser fw(100,100,300,450,_noop("Choose File"),0);

int is_window_available(Window w)
{
  XWindowAttributes xwa;
  if(return_focus && w>0)
  {
    if(XGetWindowAttributes(disp,w,&xwa))
    {
      if(xwa.map_state==IsUnmapped) return 0;
      else return 1;
    }
  }
  return 0;
}

void sendsimpleview();

extern char ch_user[];
extern MenuBar *current_menu_bar;
extern char syspath[];
extern int vclass;
Window mesw;
Pixmap mesxpm;
GC mesgc;
void cfunc();
void pcdfunc();
void options();
extern void askfile(char *head,void (*ff)());
extern void prect(Window w,GC& gcw,int x,int y,int l,int h);
extern void urect(Window w,GC& gcw,int x,int y,int l,int h);
MenuItem miconv[]={
    menu_none, _noop("Fast (Floyd Dithering)"),1,cfunc,NULL,0, no_icon, AEmpty,
    menu_none, _noop("Slow method"),1,cfunc,NULL,0, no_icon, AEmpty,
    menu_none, _noop("Best (PPM method)"),1,cfunc,NULL,0, no_icon, AEmpty};

MenuItem mipcdres[]={
    menu_none, "192x128",1,pcdfunc,NULL,0, no_icon, AEmpty,
    menu_none, "384x256",1,pcdfunc,NULL,0, no_icon, AEmpty,
    menu_none, "768x512",1,pcdfunc,NULL,0, no_icon, AEmpty,
    menu_none, "1536x1024",1,pcdfunc,NULL,0, no_icon, AEmpty};

Window focused;
void showmes(char *mes)
{
  int tl,i;
  XGetInputFocus(disp,&focused,&i);
  XSetForeground(disp,mesgc,keyscol[1]);
  XFillRectangle(disp,mesxpm,mesgc,0,0,250,75);
  for(i=0;i<10;i++)
  {
    urect(mesxpm,mesgc,i*25,0,24,24);
    urect(mesxpm,mesgc,i*25,50,24,24);  
  }
  urect(mesxpm,mesgc,0,25,24,24);  
  urect(mesxpm,mesgc,225,25,24,24);  
  XSetForeground(disp,mesgc,cols[1]);
  tl=strlen(mes);
  XDrawString(disp,mesxpm,mesgc,125-XTextWidth(fontstr,mes,tl)/2,40,mes,tl);
  XMapRaised(disp,mesw);
  XSync(disp,0);
}
 
void hidemes()
{
  XUnmapWindow(disp,mesw);
}

void donefunc();
void vwquit(Viewer*);
void ewquit(Editor*);
void swim(IMViewer*);
extern Viewer vw1;
extern Editor ew1;
Editor *edw;
Viewer *vrw;
GC gcw;
int vcoun=0;
int ecoun=0;
void editor_mbar_reconfigure(Editor*,int,int,int,int);
void viewer_mbar_reconfigure(Viewer*,int,int,int,int);
void showstatus();
void vvwquit();
void ivwquit(IMViewer*);
void eewquit();
void eewpseudoquit();
void vhex();
void vsave();
void vsearch();
void vgo();
void vconv();
void econv();
void vconvwin();
void econvwin();
void esave();
void efind();
void egoto();
void ecopy();
void epaste();
void ecut();
void edel();
void enext();
void swview(Viewer*);
void swedit(Editor*);

Gui *last=NULL;
void fqok();
void fqcan();

MenuItem viewm1[]={
    menu_function, _noop("2.Save"),1,vsave,NULL,0, no_icon, AEmpty,
    menu_function, _noop("3.Hex/Ascii"),1,vhex,NULL,0, no_icon, AEmpty,
    menu_function, _noop("7.Search"),1,vsearch,NULL,0, no_icon, AEmpty,
    menu_function, _noop("8.Go to"),1,vgo,NULL,0, no_icon, AEmpty,
    menu_function, _noop("11.DOSr"),1,vconv,NULL,0, no_icon, AEmpty,
    menu_function, _noop("12.WINr"),1,vconvwin,NULL,0, no_icon, AEmpty,
    menu_function, _noop("-10.Quit"),2,vvwquit,NULL,0, no_icon, AEmpty
};
MenuItem editm1[]={
    menu_function, _noop("2.Save"),1,esave,NULL,0, no_icon, AEmpty,
    menu_function, _noop("3.Copy"),1,ecopy,NULL,0, no_icon, AEmpty,
    menu_function, _noop("4.Paste"),1,epaste,NULL,0, no_icon, AEmpty,
    menu_function, _noop("5.Cut"),1,ecut,NULL,0, no_icon, AEmpty,
    menu_function, _noop("6.Next"),1,enext,NULL,0, no_icon, AEmpty,
    menu_function, _noop("7.Find"),1,efind,NULL,0, no_icon, AEmpty,
    menu_function, _noop("8.Go to"),1,egoto,NULL,0, no_icon, AEmpty,
    menu_function, _noop("9.Del"),1,edel,NULL,0, no_icon, AEmpty,
    menu_function, _noop("11.DOSr"),1,econv,NULL,0, no_icon, AEmpty,
    menu_function, _noop("12.WINr"),1,econvwin,NULL,0, no_icon, AEmpty,
    menu_function, _noop("-10.Quit"),2,eewpseudoquit,NULL,0, no_icon, AEmpty
};

#include "ivestruct.h"

IVESid *sm;
char *smem;
char *umem;
int pid,sid;      

template <class Tmp>
class DList
{
public:
  Tmp *o;
  DList<Tmp> *next,*prev;
  int autopid;
  int retval;
  FList *fo;
  MenuBar* m;
  DList(Tmp *obj,int ppid,int ret,FList* ffo) {o=obj;autopid=ppid;fo=ffo;retval=ret;m=NULL;next=prev=NULL;};
  DList() {next=prev=NULL;};
  void add(DList*);
  void del();
  DList<Tmp>* find(Tmp* oo);
  DList<Tmp>* findmenu(MenuBar* oo);
};

template <class Tmp> void DList<Tmp>::add(DList* n)
{
  if(next!=NULL) next->add(n);
  else {
    next=n;
    n->prev=this;
  }
}

template <class Tmp> void DList<Tmp>::del()
{
  if(prev) prev->next=next;
  if(next) next->prev=prev;
  next=prev=NULL;
}

template <class Tmp> DList<Tmp>* DList<Tmp>::find(Tmp* oo)
{
    if(oo==o) 
	return this;
    else 
	if(next) 
	    return next->find(oo);
  return NULL;
}

template <class Tmp> DList<Tmp>* DList<Tmp>::findmenu(MenuBar* oo)
{
    if(oo==m) 
	return this;
    else 
	if(next) 
	    return next->findmenu(oo);
  return NULL;
}

DList<Editor>   *eo=NULL;
DList<Viewer>   *vo=NULL;
DList<IMViewer> *ivo=NULL;


FList *fo;
void sendedit2(char *fname);

void new_data(IVES_data data)
{
  struct stat buf;
  char name[40];
  char mes[80];
  MenuBar* vmn;
  int itype;
  int lineno=-1;
  IMViewer *ivrw;
  DList<Editor> *ecur;
  DList<Viewer> *vcur;
  DList<IMViewer> *ivcur;
  extern char edbuf[];        //in editor.cxx
  extern int internal_lineno;  //in editor.cxx
  extern int auto_find;  //in editor.cxx
  extern char* autofind_string;  //in editor.cxx
  if(internal_use)
    {
      focused=0;
      lineno=internal_lineno;
      internal_lineno=-1;
    } else
      XGetInputFocus(disp,&focused,&itype);
  internal_use=0;
  focus_to=data.focus_to;
  switch(data.type){
  case 1:
    sprintf(name,_("%s: IVES Editor %d-%s"),ch_user,ecoun+1,strrchr(data.data,'/')+1);
    edw=new Editor(viewx,viewy,viewl,viewh,name,2,ewquit);
    edw->init(Main);
    edw->set_external_reconfigure(editor_mbar_reconfigure);
    edw->show();
    if(stat(data.data,&buf)==-1)
      {
	mode_t um;
	(void)umask(um = umask(S_IWGRP|S_IWOTH));
	fo=new FList(data.data,0666&~um,0,getuid(),getgid());
	edw->newfile(fo);
      } else                
	{
	  fo=new FList(data.data,buf.st_mode,buf.st_size,buf.st_uid,buf.st_gid);
	  edw->load(fo);
	}
    ecur=new DList<Editor>(edw,data.retw,data.retval,fo);
    vmn=new MenuBar(0,viewh-20,editm1,11);
    vmn->init(edw->w);
    ecur->m=vmn;
    edw->setswfunc(swedit);
    edw->set_cfind_func(sendedit2);
    vmn->show();
    if(eo==NULL) eo=ecur;else eo->add(ecur);
    last=edw;
    ecoun++;
    showstatus();
    if(lineno!=-1)
      {
	sprintf(edbuf,"%d",lineno);
	edw->gotoln();
      } else if(auto_find && autofind_string)
	{
	  edw->set_straight_and_find(autofind_string);
	  auto_find=NO;
	}
    XFlush(disp);
    break;
  case 2:
  case 3:
  case 4:
      if(data.type==4) itype=-1; else itype=autodetect(data.data);
      if(itype==-1)
	{
	  if(stat(data.data,&buf)!=-1)
	    {
	      sprintf(name,_("%s: IVES Viewer %d-%s"),ch_user,vcoun+1,strrchr(data.data,'/')+1);
	      vrw=new Viewer(viewx,viewy,viewl,viewh,name,2,vwquit);
	      vrw->init(Main);
	      vrw->set_external_reconfigure(viewer_mbar_reconfigure);
	      vrw->show();
	      if(data.type==3) vrw->highlight=1;
	      fo=new FList(data.data,buf.st_mode,buf.st_size,buf.st_uid,buf.st_gid);
	      vmn=new MenuBar(0,viewh-20,viewm1,7);
	      vmn->init(vrw->w);
	      vcur=new DList<Viewer>(vrw,data.retw,data.retval,fo);
	      vcur->m=vmn;
	      vmn->show();
	      vrw->setswfunc(swview);
	      if(vo==NULL) vo=vcur;else vo->add(vcur);
	      vcoun++;
	      showstatus();
	      vrw->view(fo);
	    }
	  last=vrw;
	} else {
	  sprintf(name,_("%s: IVES Image Viewer %d-%s"),ch_user,vcoun+1,strrchr(data.data,'/')+1);
	  strcpy(mes,_("Loading "));
	  switch(itype){
	  case F_GIF: strcat(mes,_("GIF Image...."));break;
	  case F_JPEG: strcat(mes,_("JPEG Image...."));break;
	  case F_PCX: strcat(mes,_("PCX Image...."));break;
	  case F_TIFF: strcat(mes,_("TIFF Image...."));break;
	  case F_XBM: strcat(mes,_("XBM Image...."));break;
	  case F_XPM: strcat(mes,_("XPM Image...."));break;
	  case F_BMP: strcat(mes,_("BMP Image...."));break;
	  case F_PCD: strcat(mes,_("Photo CD...."));break;
	  case F_PNG: strcat(mes,_("PNG Image...."));break;
	  };
	  showmes(mes);
	  ivrw=new IMViewer(name,data.data,itype,ivwquit);
	  ivrw->error_func(sendsimpleview);
	  ivrw->init(Main);
	  ivrw->setswfunc(swim);
	  //   if(ivrw->im)
	  //   {
	  ivrw->show();
	  ivcur=new DList<IMViewer>(ivrw,data.retw,data.retval,NULL);
	  if(ivo==NULL) ivo=ivcur;else ivo->add(ivcur);
	  vcoun++;
	  showstatus();
	//   } else {delete ivrw;XBell(disp,0);};
	}
      hidemes();
      XFlush(disp);
      break;
  case 6: //External exit
    exit(0);
    break;
  }
  //  delete sm.data;
}

extern char dbuf[];  
void sendview()
{
  char data[2048];
  internal_use=1;
  if(fw.fname[0]!='/')
  {
    getcwd(data,1500);
    strcat(data,"/");
    strcat(data,dbuf);
  }else
    strcpy(data,fw.fname);
  new_data(IVES_data(2,data));
}

void sendsimpleview()
{
}

void sendedit()
{
  char data[2048];
  internal_use=1;
  if(fw.fname[0]!='/')
  {
    getcwd(data,1500);
    strcat(data,"/");
    strcat(data,dbuf);
  }else
    strcpy(data,fw.fname);
  new_data(IVES_data(1,data));
}

void sendedit2(char *fname)
{
  char data[2048];
  internal_use=1;
  if(fname[0]!='/')
  {
    getcwd(data,1500);
    strcat(data,"/");
    strcat(data,fname);
  }else
    strcpy(data,fname);
  new_data(IVES_data(1,data));
}

void fied()
{    
  askfile(_("Edit file"),sendedit);
}

void fivd()
{
  askfile(_("View file"),sendview);
}

KEY *qok;
KEY *iop;

KEY *ied;
KEY *ivd;


void fqok()
{
  char tmpchr[1024];
  done=1;
  sprintf(tmpchr,"%s/.xnc/ives.history",getenv("HOME"));
  hist_saveall(tmpchr);
}

void usr2(int)
{
  //  fprintf(stderr,"Recieve SIGUSR2\n");
  signal(SIGUSR2,usr2);
}

void sendsig()
{
  if(sm->autopid!=-1)
  {
    sm->ready=0;
    kill(sm->autopid,SIGUSR1);
  }
}

void swview(Viewer* vv)
{
  DList<Viewer>* t;
  t=vo->find(vv);
  t=t->next;
  if(t==NULL) t=vo;
  XMapRaised(disp,t->o->w);
  if(t->o->unmap!=1)
    XSetInputFocus(disp,t->o->w,RevertToNone,CurrentTime);
  else t->o->unmap=0;
}

void swedit(Editor* vv)
{
  DList<Editor>* t;
  t=eo->find(vv);
  t=t->next;
  if(t==NULL) t=eo;
  XMapRaised(disp,t->o->w);
  if(t->o->unmap!=1)
    XSetInputFocus(disp,t->o->w,RevertToNone,CurrentTime);
  else t->o->unmap=0;
}

void swim(IMViewer* vv)
{
  DList<IMViewer>* t;
  t=ivo->find(vv);
  t=t->next;
  if(t==NULL) t=ivo;
  XMapRaised(disp,t->o->w);
  if(t->o->unmap!=1)
    XSetInputFocus(disp,t->o->w,RevertToNone,CurrentTime);
  else t->o->unmap=0;
}           

void viewer_mbar_reconfigure(Viewer *vv,int x,int y,int l,int h)
{
  DList<Viewer>* t;
  t=vo->find(vv);
  t->m->reconfigure(0,h-20,l,h);
}

void vwquit(Viewer* vv)
{
  DList<Viewer>* t;
  t=vo->find(vv);
  t->m->hide();
  vv->hide();
  delete vv;
  delete t->fo;
  delete  t->m;
  if(t->autopid)
    iserv->send_retval((Window)t->autopid,t->retval);
  if(vo==t) vo=t->next;
  t->del();
  delete t;
  vcoun--;
  showstatus();
  if(focus_to)
  {
    if(is_window_available(focus_to))
      XSetInputFocus(disp,focus_to,RevertToParent,CurrentTime);
  }
  else
  {
    if(is_window_available(focused))
      XSetInputFocus(disp,focused,RevertToParent,CurrentTime);
  }
}         

void ivwquit(IMViewer* vv)
{
  DList<IMViewer>* t;
  t=ivo->find(vv);
  vv->hide();
  delete vv;
  if(t->autopid)
    iserv->send_retval((Window)t->autopid,t->retval);
  if(ivo==t) ivo=t->next;
  t->del();
  delete t;                      
  vcoun--;
  showstatus();
  if(focus_to)
  {
    if(is_window_available(focus_to))
      XSetInputFocus(disp,focus_to,RevertToParent,CurrentTime);
  }
  else
  {
    if(is_window_available(focused))
      XSetInputFocus(disp,focused,RevertToParent,CurrentTime);
  }
}
          
void vvwquit()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  t->m->hide();
  t->o->hide();
  delete t->o;
  delete t->fo;
  delete  t->m;
  if(t->autopid)
    iserv->send_retval((Window)t->autopid,t->retval);
  if(vo==t) vo=t->next;
  t->del();
  delete t;
  vcoun--;
  showstatus();
  if(focus_to)
  {
    if(is_window_available(focus_to))
      XSetInputFocus(disp,focus_to,RevertToParent,CurrentTime);
  }
  else
  {
    if(is_window_available(focused))
      XSetInputFocus(disp,focused,RevertToParent,CurrentTime);
  }
}         

extern void saveas(char*,char*,Viewer*,int);
void vsave()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  saveas(_("Save file"),_("Save as:"),t->o,0);
}

void vsearch()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  saveas(_("Find"),_("Find text:"),t->o,1);
}

void vconv()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  if(t->o->koi==0)
    t->o->rusconvert();
}

void vconvwin()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  if(t->o->koi==0)
    t->o->wrusconvert();
}

void vgo()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  if(t->o->hex)
    saveas(_("Go to offset..."),_("Hex offset:"),t->o,2);
  else
    saveas(_("Go to line..."),_("Line number:"),t->o,3);
}

void vhex()
{
  DList<Viewer>* t;
  t=vo->findmenu(current_menu_bar);
  t->o->hex_asc();
}

void esave()
{        
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->ssave();
}

void efind()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->sfind();
}

void ecopy()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->copy();
}

void egoto()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->sgoto();
}

void econv()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->do_convertion();
}

void econvwin()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->do_wconvertion();
}

void ecut()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->cut();
}

void enext()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  swedit(t->o);
}

void epaste()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->paste();
}

void edel()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->del();
}

void editor_mbar_reconfigure(Editor *vv,int x,int y,int l,int h)
{
  DList<Editor>* t;
  t=eo->find(vv);
  t->m->reconfigure(0,h-20,l,h);
}

void ewquit(Editor* vv)
{
  DList<Editor>* t;
  t=eo->find(vv);
  t->m->hide();
  vv->hide();
  delete vv;
  delete t->fo;
  delete t->m;
  if(t->autopid)
    iserv->send_retval((Window)t->autopid,t->retval);
  if(eo==t) eo=t->next;
  t->del();
  delete t;
  ecoun--;
  showstatus();
  if(focus_to)
  {
    if(is_window_available(focus_to))
      XSetInputFocus(disp,focus_to,RevertToParent,CurrentTime);
  }
  else
  {
    if(is_window_available(focused))
      XSetInputFocus(disp,focused,RevertToParent,CurrentTime);
  }
}         


void eewquit()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->m->hide();
  t->o->hide();
  delete t->o;
  delete t->fo;
  delete t->m;
  if(t->autopid)
    iserv->send_retval((Window)t->autopid,t->retval);
  if(eo==t) eo=t->next;
  t->del();
  delete t;
  ecoun--;
  showstatus();
  if(focus_to)
  {
    if(is_window_available(focus_to))
      XSetInputFocus(disp,focus_to,RevertToParent,CurrentTime);
  }
  else
  {
    if(is_window_available(focused))
      XSetInputFocus(disp,focused,RevertToParent,CurrentTime);
  }
}         

void eewpseudoquit()
{
  DList<Editor>* t;
  t=eo->findmenu(current_menu_bar);
  t->o->exit_and_save();
}


void CText(GC gcw,int dx,int iy,char* name,int n,int col)
{
  int l=XTextWidth(fontstr,name,n);
  int ix=dx-l/2;
  if(shadow)
  {
    XSetForeground(disp,gcw,cols[0]);
    XDrawString(disp,Main,gcw,ix+1,iy+1,name,n);
  }
  XSetForeground(disp,gcw,cols[col]);
  XDrawString(disp,Main,gcw,ix,iy,name,n);
}

void TText(GC gcw,int ix,int iy,char* name,int n,int col)
{
  if(shadow)
  {
    XSetForeground(disp,gcw,cols[0]);
    XDrawString(disp,Main,gcw,ix+1,iy+1,name,n);
  }
  XSetForeground(disp,gcw,cols[col]);
  XDrawString(disp,Main,gcw,ix,iy,name,n);
}

void showstatus()
{
  char n6[300];
  XSetForeground(disp,gcw,normal_bg_color);
  XFillRectangle(disp,Main,gcw,xpml+1,0,D_LEN - 2, 54);
  prect(Main,gcw,0,xpmh+2,xpml-2,54);
  sprintf(n6,_("Viewers:  %d"),vcoun); 
  CText(gcw,xpml+D_LEN/2,18,n6,strlen(n6),1);
  sprintf(n6,_("Editors:  %d"),ecoun); 
  CText(gcw,xpml+D_LEN/2,38,n6,strlen(n6),1);
}

Pixmap xpm;

void MainExpose()
{
  XSetForeground(disp,gcw,window_text_color);
  XSetFont(disp,gcw,fontstr->fid);
  XCopyArea(disp,xpm,Main,gcw,0,0,xpml,xpmh,0,0);
  TText(gcw,10,20,n1,11,1);
  // CText(gcw,60,n2,strlen(n2),5);
  // CText(gcw,80,n3,strlen(n3),5);
  // CText(gcw,100,n4,strlen(n4),5);
  // CText(gcw,120,n5,strlen(n5),5);
  // XSetFont(disp,gcw,fixfontstr->fid);
  showstatus();
}


void wait_for_x_event(XEvent *ev)
{
  int ret;
  fd_set fds;
  struct timeval tv;
  while(1) 
  {
    tv.tv_sec=0;
    tv.tv_usec=500;
    FD_ZERO(&fds);
    FD_SET(xfp,&fds);
    ret=select(xfp+1,&fds,NULL,NULL,&tv);
    if(ret>0 && FD_ISSET(xfp,&fds))
      break;
  }
  XNextEvent(disp, ev);
}
  


void XEVENT()       
{
  Gui *o;
  int fiflg=1;
  Atom acttype;
  int actform;
  unsigned long remby;
  unsigned char *data;
  unsigned long size;
 
 
  // wait_for_x_event(&ev);

  XNextEvent(disp, &ev);

  default_dnd_man->process_event(&ev);
  //    return;

  if (ev.type == ConfigureNotify)
    default_dnd_man->update_geoms();

  if (ev.type == ClientMessage) 
  {
      if(iserv->check_event(&ev))
	  return;
      fprintf(stderr,"ClientMessage Event for 0x%X\n",ev.xany.window);
  }
  
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
  else
    switch(ev.type)
    {
    case Expose:
      if(ev.xexpose.count==0)
	MainExpose();break;
    case MapNotify:
      mapped=1;
      alarm(600);break;
    case UnmapNotify:
      mapped=0;
      alarm(0);break;
    };
  /*   else
       if(ev.type==FocusIn || (ev.type==Expose && fiflg==1)) 
       { if(last) XSetInputFocus(disp,last->w,RevertToNone,CurrentTime);fiflg=0;}
  */
}

void (*ffunc)();
Win *dw=NULL;
KEY *dok;
KEY *dcan;
Input *din;
Text *dt1,*dt2,*dt3;
char dbuf[256];

void nfok()
{
  if(din->bl)
  {
    din->hide();
    dok->hide();
    dcan->hide();
    dw->hide();
    delete dw;
    delete din;
    delete dok;
    delete dcan;
    delete dt1;
    if(mapped)XSetInputFocus(disp,Main,RevertToParent,CurrentTime);
    dw=NULL;
    if(ffunc) ffunc();
  }
}

void nfcan()
{
  din->hide();
  dok->hide();
  dcan->hide();
  dw->hide();
  delete dw;
  delete din;
  delete dok;
  delete dcan;
  delete dt1;
  dw=NULL;
  if(mapped) XSetInputFocus(disp,Main,RevertToParent,CurrentTime);
}

void askfile(char *head,void (*ff)())
{
  fw.show();
  fw.set_func(ff);
}

Sw_panel *dpan;
Sw_panel *dpan2;
extern int pcd_resolution;
void ocan()
{
  dpan->hide();
  dpan2->hide();
  dok->hide();
  dw->hide();
  delete dw;
  delete dpan;
  delete dpan2;
  delete dok;
  dw=NULL;
  if(mapped) XSetInputFocus(disp,Main,RevertToParent,CurrentTime);
}



void options()
{
  if(dw==NULL)
  {
    dw=new Win(350,220,250,350,_("IVES Options"),1);
    dok=new KEY(20,-20,210,25,_("OK"),1,ocan);
    dpan=new Sw_panel(10,35,230,_("Image 24->8 convertion"),miconv,3);
    dpan2=new Sw_panel(10,160,230,_("Photo CD resolution"),mipcdres,4);
    dw->init(DefaultRootWindow(disp));
    dw->nonresizable();
    dpan->init(dw->w);
    dpan2->init(dw->w);
    dok->init(dw->w);
    dw->link(dpan);
    dpan->link(dpan2->ar[0]);
    dpan2->link(dok);
    dok->link(dpan->ar[0]);
    dpan->excl();
    dpan2->excl();
    dw->add_toexpose(dpan2);
    switch(conv24)
    {
    case CONV24_FAST: 
	dpan->set(0,1);break;
    case CONV24_SLOW: 
	dpan->set(1,1);break;
    default: 
	dpan->set(2,1);break;
    }
    dpan2->set(pcd_resolution-1,1);
    dw->show();
    dpan2->show();
    dok->show();  
    dpan->show();
  }              
}

void cfunc()
{
  int i;
  for(i=0;i<3;i++)
    if(dpan->get(i)) break;
  conv24=5+i;
}

void pcdfunc()
{
  int i;
  for(i=0;i<3;i++)
    if(dpan2->get(i)) break;
  pcd_resolution=i+1;
}

void sig_al(int)
{
  XIconifyWindow(disp,Main,DefaultScreen(disp));
  XFlush(disp);
  signal(SIGALRM,sig_al);
}


int main(int argc,char** argv)
{            
  char tmpchr[1024];
  GC xpmgc;
  mlook=LOOKFOUR;
  fprintf(stderr,"Initialisation:\n");
  margc=argc;margv=argv; 
  iserv=new IVES_server;
  iclient=new IVES_client;
  InitXClient(argc,argv,10,60,500,166,n1);
  init_basegui_plugin();
  vcenterx=viewl/2;
  if(iclient->Init())
    {
      fprintf(stderr,_("Another IVES already running\n"));
      disp=0;
      return 0;
    }
  iserv->Init(Main,new_data);
  iclient->Init();
  pid=fork();
  if(pid==-1)
  {
    fprintf(stderr,"Can't run IVES\n");
    exit(1);
  }
  if(pid!=0)
  {
    fprintf(stderr,"Running in background\n");
    disp=NULL;
    exit(0);
    } else
  {
    default_dnd_man=new DndManager(disp,Main);
    IVES_dnd* ives_dnd=new IVES_dnd;
    DndWin*   ives_dwin=default_dnd_man->add_dnd_object(ives_dnd,Main);
    ives_dwin->add_type("text/uri-list");
    init_imagengine();
    init_scroll();
    init_history();
    init_l10n();
    GuiLocale::init_locale();
    sprintf(tmpchr,"%s/.xnc/ives.history",getenv("HOME"));
    hist_loadall(tmpchr);
    shadow=0;
#ifdef HAVE_ATEXIT
    atexit (fqok);
#else
# if defined (__sun__)
    on_exit (fqok, NULL);                /* non-ANSI exit handler */
# endif
#endif

    conv24=CONV24_SLOW;
    root = DefaultRootWindow(disp);
    DndProtocol = XInternAtom(disp, "_DND_PROTOCOL", 0);
    DndSelection = XInternAtom(disp, "_DND_SELECTION", 0);
    Old_DndProtocol = XInternAtom(disp, "DndProtocol", 0); 
    Old_DndSelection = XInternAtom(disp, "DndSelection", 0);
    gcw=XCreateGC(disp,Main,0,NULL);
    fw.init(root);
    win_inp=0;
    mesw=create_win(_("Loading..."),ScreenL/2-125,ScreenH/2-75,250,75,ExposureMask);
    win_nodecor(mesw);
    mesxpm=XCreatePixmap(disp,mesw,250,75,
			 DefaultDepth(disp,DefaultScreen(disp)));
    mesgc=XCreateGC(disp,mesxpm,0,NULL);
    XSetFont(disp,mesgc,fontstr->fid);
    XSetWindowBackgroundPixmap(disp,mesw,mesxpm);
    signal(SIGALRM,sig_al);
    strcpy(tmpchr,syspath);
    strcat(tmpchr,"/ives.gif");
    if(vclass==TrueColor)
      xpm=LoadPixmap(tmpchr,F_AUTO,xpml,xpmh);
    else xpm=0;
    if(xpm==0)
    {
      xpm=XCreatePixmapFromBitmapData(disp,Main,(char*)bv1_bits,bv1_width,bv1_height,keyscol[0],keyscol[1],
				      DefaultDepth(disp,DefaultScreen(disp)));
      xpml=bv1_width;
      xpmh=bv1_height;
    }
    Mainl=xpml+D_LEN;Mainh=xpmh;
    XResizeWindow(disp,Main,Mainl,Mainh);
    xncdprintf(("New geometry is %dx%d\n", Mainl, Mainh));
    XFlush(disp);
    centerx=Mainl/2;centery=Mainh/2;
    qok=new KEY(xpml+5,Mainh-30,D_LEN-10,25,_("Exit"),2,fqok);
    iop=new KEY(xpml+5,Mainh-60,D_LEN-10,25,_("Options"),1,options);
    ied=new KEY(xpml+5,Mainh-90,D_LEN-10,25,_("Edit file"),1,fied);
    ivd=new KEY(xpml+5,Mainh-120,D_LEN-10,25,_("View file"),1,fivd);
    qok->init(Main);
    ied->init(Main);
    ivd->init(Main);
    iop->init(Main);
    qok->show();
    ivd->show();
    ied->show();
    iop->show();
    wmDeleteWindow = XInternAtom (disp, "WM_DELETE_WINDOW", False);
    xfp=XConnectionNumber(disp);
    while(!done)  XEVENT();
    fprintf(stderr,"Exiting from IVES....");
    fprintf(stderr,"OK\n");
    XFreeGC(disp,gcw);
    XFreePixmap(disp,xpm);
    deinit_scroll();
    DeinitXClient();
  }
  return 0;
}

        
