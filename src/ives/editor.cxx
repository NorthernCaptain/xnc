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
#include "xh.h"
#include "scroll.h"
#include "baseguiplugin.h"

#define TAB_SIZE  4

const int  maxlines=10000;
const int  maxsyms=250;
Atom ast;
extern Atom wmDeleteWindow;
extern Win *vdw;
extern KEY *vdok;
extern KEY *vdcan;
extern Input *vdin;
extern char syspath[];
XIC Input_Context;                /* input context */
KEY *vdex=NULL;
Editor *ew=NULL;
Switch *esw1=NULL,*esw2=NULL;
extern Text *vdt1,*vdt2,*vdt3;
char edbuf[1024]="";
int ednum;
int vcenterx,vcentery;
int internal_lineno=-1;
unsigned char rustbl[256]=  //CP866    -> KOI8
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,
0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0xE1,0xE2,0xF7,0xE7,0xE4,0xE5,0xF6,0xFA,0xE9,0xEA,0xEB,0xEC,
0xED,0xEE,0xEF,0xF0,0xF2,0xF3,0xF4,0xF5,0xE6,0xE8,0xE3,0xFE,0xFB,0xFD,0xFF,0xF9,0xF8,0xFC,0xE0,0xF1,
0xC1,0xC2,0xD7,0xC7,0xC4,0xC5,0xD6,0xDA,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xB0,0xB1,0xB2,0xB3,
0xB4,0xE5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,
0xDC,0xDD,0xDE,0xDF,0xD2,0xD3,0xD4,0xD5,0xC6,0xC8,0xC3,0xDE,0xDB,0xDD,0xDF,0xD9,0xD8,0xDC,0xC0,0xD1,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

unsigned char wrustbl[256]= //WIN1251  -> KOI8
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,
0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,
0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xE5,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,
0xB4,0xB5,0xB6,0xB7,0xC5,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xE1,0xE2,0xF7,0xE7,0xE4,0xE5,0xF6,0xFA,
0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0,0xF2,0xF3,0xF4,0xF5,0xE6,0xE8,0xE3,0xFE,0xFB,0xFD,0xFF,0xF9,
0xF8,0xFC,0xE0,0xF1,0xC1,0xC2,0xD7,0xC7,0xC4,0xC5,0xD6,0xDA,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,
0xD2,0xD3,0xD4,0xD5,0xC6,0xC8,0xC3,0xDE,0xDB,0xDD,0xDF,0xD9,0xD8,0xDC,0xC0,0xD1
};

unsigned char wintbl[256]=  //KOI8     -> WIN1251
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,
0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,
0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xE5,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xC5,
0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xFE,0xE0,0xE1,0xF6,0xE4,0xE5,0xF4,0xE3,
0xF5,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xFF,0xF0,0xF1,0xF2,0xF3,0xE6,0xE2,0xFC,0xFB,0xE7,0xF8,
0xFD,0xF9,0xF7,0xFA,0xDE,0xC0,0xC1,0xD6,0xC4,0xC5,0xD4,0xC3,0xD5,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,
0xCF,0xDF,0xD0,0xD1,0xD2,0xD3,0xC6,0xC2,0xDC,0xDB,0xC7,0xD8,0xDD,0xD9,0xD7,0xDA
};

unsigned char dostbl[256]=  //KOI8     -> CP866
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,
0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,
0x8C,0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA5,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB5,
0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,0xEE,0xA0,0xA1,0xE6,0xA4,0xA5,0xE4,0xA3,
0xE5,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xEF,0xE0,0xE1,0xE2,0xE3,0xA6,0xA2,0xEC,0xEB,0xA7,0xE8,
0xED,0xE9,0xE7,0xEA,0x9E,0x80,0x81,0x96,0x84,0xB5,0x94,0x83,0x95,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,
0x8F,0x9F,0x90,0x91,0x92,0x93,0x86,0x82,0x9C,0x9B,0x87,0x98,0x9D,0x99,0x97,0x9A
};

unsigned char rtbl[128]=  //Russian keyboard mapping
{
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,
0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0xFC,0x2F,0x24,0x3A,0x2E,0xDC,
0x3F,0x25,0x3B,0x2B,0xC2,0x2D,0xC0,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0xF6,0xD6,
0xE2,0x3D,0xE0,0x3F,0x22,0xE6,0xE9,0xF3,0xF7,0xF5,0xE1,0xF0,0xF2,0xFB,0xEF,0xEC,0xE4,0xF8,0xF4,0xFD,
0xFA,0xEA,0xEB,0xF9,0xE5,0xE7,0xED,0xE3,0xFE,0xEE,0xF1,0xC8,0x5C,0xDF,0x2C,0x5F,0x60,0xC6,0xC9,0xD3,
0xD7,0xD5,0xC1,0xD0,0xD2,0xDB,0xCF,0xCC,0xC4,0xD8,0xD4,0xDD,0xDA,0xCA,0xCB,0xD9,0xC5,0xC7,0xCD,0xC3,
0xDE,0xCE,0xD1,0xE8,0x7C,0xFF,0x7E,0x7F
};


void save_file(char* fname, char* buf, int siz)
{
  int fp=creat(fname,0644);
  if(fp==-1)
    return;
  write(fp,buf,siz);
  close(fp);
} 

#ifdef OLD_MAP_GEN
void genrtbl()
{
  char dos[73];
  char koi[73];
  char tmp[1024];
  int i;
  strcpy(tmp,syspath);
  strcat(tmp,"/ru.key1");
  int fp1=open(tmp,O_RDONLY);
  strcpy(tmp,syspath);
  strcat(tmp,"/ru.key2");
  int fp2=open(tmp,O_RDONLY);
  if(fp1== -1 || fp2== -1)
  {
    fprintf(stderr,"Russian mapping failed...\n");
    if(fp1>0)close(fp1);
    if(fp2>0)close(fp2);
    return;
  }
  read(fp1,dos,73);
  read(fp2,koi,73);
  close(fp1);
  close(fp2);
  for(i=0;i<128;i++)
    rtbl[i]=i;
  for(i=0;i<73;i++)
    rtbl[dos[i]]=koi[i];
  save_file("rukeys.bin",rtbl,128);
}


void genrustbl()
{
  unsigned char dos[66];
  unsigned char koi[66];
  unsigned char win[66];
  char tmp[1024];
  int i;
  genrtbl();
  return;

  strcpy(tmp,syspath);
  strcat(tmp,"/ru.dos");
  int fp1=open(tmp,O_RDONLY);
  strcpy(tmp,syspath);
  strcat(tmp,"/ru.koi8");
  int fp2=open(tmp,O_RDONLY);
  strcpy(tmp,syspath);
  strcat(tmp,"/ru.win");
  int fp3=open(tmp,O_RDONLY);
  if(fp1== -1 || fp2== -1 || fp3==-1)
  {
    fprintf(stderr,"Generating Russian mapping failed...\n");
    if(fp1>0)close(fp1);
    if(fp2>0)close(fp2);
    if(fp3>0)close(fp3);
    return;
  }
  read(fp1,dos,66);
  read(fp2,koi,66);
  read(fp3,win,66);
  close(fp1);
  close(fp2);
  close(fp3);
  for(i=0;i<256;i++)
    rustbl[i]=dostbl[i]=wintbl[i]=wrustbl[i]=i;
  for(i=0;i<66;i++)
  {
    rustbl[dos[i]]=koi[i];
    dostbl[koi[i]]=dos[i];
    wrustbl[win[i]]=koi[i];
    wintbl[koi[i]]=win[i];
  }
  save_file("koi2dos.bin",dostbl,256);
  save_file("koi2win.bin",wintbl,256);
  save_file("dos2koi.bin",rustbl,256);
  save_file("win2koi.bin",wrustbl,256);
}

#endif

void showmes(char*); 
void hidemes();

void eok()
{
  if(vdin->bl)
  {
    vdin->hide();
    if(esw1)
    {
      ew->dos=esw1->sw;
      ew->dos|=esw2->sw<<1;
      esw1->hide();
      delete esw1;
      esw1=NULL;
      esw2->hide();
      delete esw2;
      esw2=NULL;
    }
    vdok->hide();
    vdcan->hide();
    vdw->hide();
    if(vdex) {vdex->hide();delete vdex;vdex=NULL;};
    if(esw1) {esw1->hide();delete esw1; esw1=NULL;};
    if(esw2) {esw2->hide();delete esw2; esw2=NULL;};
    delete vdw;
    delete vdin;
    delete vdok;
    delete vdcan;
    delete vdt1;
    XSetInputFocus(disp,ew->w,RevertToParent,CurrentTime);
    vdw=NULL;
    ew->func(ednum);
  }
}

void eexit()
{
  if(vdin->bl)
  {
    vdin->hide();
    ew->dos=esw1->sw;
    ew->dos|=esw2->sw<<1;
    esw1->hide();
    esw2->hide();
    vdok->hide();
    vdcan->hide();
    vdw->hide();
    vdex->hide();
    delete vdw;
    delete vdin;
    delete vdok;
    delete vdcan;
    delete vdt1;
    delete vdex;
    delete esw1;
    delete esw2;
    vdex=NULL;
    XSetInputFocus(disp,ew->w,RevertToParent,CurrentTime);
    vdw=NULL;
    ew->func(ednum+1);
  }
}

void ecan()
{
  vdin->hide();
  if(esw1)
  {
    esw1->hide();
    delete esw1;
    esw1=NULL;
  }
  vdok->hide();
  vdcan->hide();
  vdw->hide();
  if(vdex) {vdex->hide();delete vdex;vdex=NULL;};
  if(esw1) {esw1->hide();delete esw1; esw1=NULL;};
  if(esw2) {esw2->hide();delete esw2; esw2=NULL;};
  delete vdw;
  delete vdin;
  delete vdok;
  delete vdcan;
  delete vdt1;
  vdw=NULL;
  XSetInputFocus(disp,ew->w,RevertToParent,CurrentTime);
}

extern char cfindbuf[];
void eask(char *head,char *mes,Editor* ivw,int inum)
{
  if(vdw==NULL)
  {
    ew=ivw;
    ednum=inum;
    vcenterx=ivw->l/2;
    vcentery=ivw->h/2;
    vdw=new Win(vcenterx-150,vcentery-70,303,130,head,1);
    vdok=new KEY(40,-20,80,25,_("OK"),1,eok);
    vdcan=new KEY(-40,-20,80,25,_("Cancel"),2,ecan);
    vdin=new Input(10,50,280,1,eok);
    vdt1=new Text(10,40,mes,1);
    vdin->set_escapefunc(ecan);
    vdok->set_escapefunc(ecan);
    if(inum==4)        //Goto line num
      vdin->set_histid("GOTO");
    else
      if(inum==5)
	vdin->set_histid("CFIND");
      else
	if(inum==6)
	{
	  vdin->set_histid("CFLIST");
	  vdin->bool_expand=YES;
	}
	else
	{
	  vdin->set_histid("SAVE");
	  vdin->bool_expand=YES;
	}
    vdw->init(ew->w);
    vdin->init(vdw->w);
    vdok->init(vdw->w);
    vdcan->init(vdw->w);
    vdt1->init(vdw->w);
    vdw->link(vdin);
    vdin->link(vdok);
    vdok->link(vdcan);
    vdcan->link(vdin);
    if(ednum==0)  strcpy(edbuf,ew->node->name);
    else
      if(ednum==6)  strcpy(edbuf,cfindbuf);
    vdin->setbuf(edbuf,1020);
    vdw->show();
    vdin->show();
    vdok->show();
    vdcan->show();
    vdt1->show();
  }
}

void eentire()
{
  ew->ent=1;
}

void ebackward()
{
  ew->bw=1;
}
 
void efind(Editor* ivw,int inum)
{
  if(vdw==NULL)
  {
    ew=ivw;
    ednum=inum;
    vcenterx=ivw->l/2;
    vcentery=ivw->h/2;
    vdw=new Win(vcenterx-150,vcentery-70,303,150,_("Find"),1);
    vdok=new KEY(-20,-45,80,25,_("Find"),1,eok);
    vdcan=new KEY(-20,-15,80,25,_("Cancel"),2,ecan);
    vdin=new Input(10,50,280,1,eok);
    vdt1=new Text(10,40,_("Text to find:"),1);
    esw1=new Switch(20, 85,150,_("Entire scope"),1,eentire);
    esw2=new Switch(20,110,150,_("Backwards"),1,ebackward);
    vdin->set_escapefunc(ecan);
    vdok->set_escapefunc(ecan);
    vdin->bool_expand=0;
    vdw->init(ew->w);
    vdin->init(vdw->w);
    vdin->set_histid("FIND");
    vdok->init(vdw->w);
    vdcan->init(vdw->w);
    vdt1->init(vdw->w);
    esw1->init(vdw->w);
    esw2->init(vdw->w);
    vdw->link(vdin);
    vdin->link(esw1);
    esw1->link(esw2);
    esw2->link(vdok);
    vdok->link(vdcan);
    vdcan->link(vdin);
    if(ednum==0)  strcpy(edbuf,ew->node->name);
    else strcpy(edbuf,ew->findbuf);
    vdin->setbuf(edbuf,255);
    vdw->show();
    vdin->show();
    vdok->show();
    vdcan->show();
    vdt1->show();
    esw1->show();
    esw2->show();
  }
}

void eex(Editor* ivw,int inum)
{
  if(vdw==NULL)
  {
    ew=ivw;
    ednum=inum;
    vcenterx=ivw->l/2;
    vcentery=ivw->h/2;
    if(inum)
      vdw=new Win(vcenterx-150,vcentery-100,303,185,_("Save and Exit"),1);
    else
      vdw=new Win(vcenterx-150,vcentery-90,303,185,_("Save"),1);
    esw1=new Switch(55, 83,185,_("DOS text format"),1,NULL);
    esw2=new Switch(55, 108,185,_("KOI8 -> Any RUS"),1,NULL);
    if(inum)
    {
      vdok=new KEY(10,-20,80,25,_("Save"),1,eok);
      vdex=new KEY(100,-20,80,25,_("Don't save"),1,eexit);
      vdcan=new KEY(-10,-20,80,25,_("Cancel"),2,ecan);
    } else
    {
      vdok=new KEY(20,-20,80,25,_("Save"),1,eok);
      vdcan=new KEY(-20,-20,80,25,_("Cancel"),2,ecan);
      vdex=NULL;
    }
    vdin=new Input(10,50,280,1,eok);
    vdt1=new Text(10,40,_("Save as"),1);
    vdin->set_histid("SAVE");
    vdin->bool_expand=YES;
    vdin->set_escapefunc(ecan);
    vdok->set_escapefunc(ecan);
    vdw->init(ew->w);
    esw1->init(vdw->w);
    esw2->init(vdw->w);
    vdin->init(vdw->w);
    vdok->init(vdw->w);
    if(inum)
      vdex->init(vdw->w);
    vdcan->init(vdw->w);
    vdt1->init(vdw->w);
    vdw->link(vdin);
    vdin->link(esw1);
    esw1->link(esw2);
    esw2->link(vdok);
    if(inum)
    {
      vdok->link(vdex);
      vdex->link(vdcan);
    } else 
      vdok->link(vdcan);
    vdcan->link(vdin);
    edbuf[0]=0;
    strcpy(edbuf,ew->node->name);
    vdin->setbuf(edbuf,255);
    esw1->sw=ew->dos;
    if(ew->koi>0) //CP866
    {
      esw2->sw=1;
    }
    vdw->show();
    vdin->show();
    esw1->show();
    esw2->show();
    vdok->show();
    if(inum)
      vdex->show();
    vdcan->show();
    vdt1->show();
  }
}


void vmes(char* head,char* mes,Viewer *ivw);
////////////////////////////C-Find functions//////////////////////////////
char cfindbuf[1024]="/root/.xnc/.ives.cfind";
char cfindlast[1024];

int auto_find=NO;
char *autofind_string=NULL;

char *etags_buf=NULL;
char *etags_end, *etags_cur;
char etags_cur_file[2048];
char etags_last_find[2048];

int load_etags()
{
  int fsiz;
  int fp=open(cfindbuf, O_RDONLY);
  if(fp==-1)
    return -1;
  fsiz=lseek(fp, 0l, SEEK_END);
  lseek(fp, 0l, SEEK_SET);
  if(etags_buf)
    delete etags_buf;
  etags_buf=new char[fsiz+1];
  etags_end=etags_buf+fsiz;
  etags_cur=etags_buf;
  read(fp,etags_buf, fsiz);
  close(fp);
  return 1;
}

char *etags_get_line(char *lineto)
{
  char *b;
  if(etags_cur>=etags_end)
    return NULL;
  b=strchr(etags_cur, '\n');
  if(b==NULL)
    return NULL;
  strncpy(lineto, etags_cur, b-etags_cur);
  etags_cur=b+1;
  return lineto;
}

int cfind(char *src, char *file, int from_start=YES)
{
  char str1[1024];
  char *b;
  if(etags_buf==NULL)
    return -1;
  if(from_start)
  {
    etags_cur=etags_buf;
    strcpy(cfindlast, src);
  }
  while(etags_get_line(str1))
  {
    if(str1[0]==0x0c)        //Next line will be 'filename,totallines'
    {
      etags_get_line(etags_cur_file);
      b=strchr(etags_cur_file, ',');
      if(!b)
	return -2;        //Not a ETAGS file.
      *b=0;
      continue;
    }
    if(strstr(str1,src))        //We found entry in ETAGS, so parse it!
    {
      b=strchr(str1, 0x7f);        //Token separator
      if(!b)
	return -2;        //Not a ETAGS file.
      *b=0;
      strcpy(file, etags_cur_file);
      strcpy(etags_last_find, str1);
      return 1;
    }
  }
  return 0;        //Token not found
}

///////////////////////////////////Editor class///////////////////////////////
void Editor::init(Window ipar)
{
  int tw;
  koi=0;
  rmode=0;
  XTextProperty wname;
  parent=ipar;
  XRectangle rec;
  vcenterx=l/2;
  vcentery=h/2;
  win_resize=1;
  w=create_win(name,x,y,l+20,h,ExposureMask | ButtonPressMask | 
	       FocusChangeMask | KeyPressMask | StructureNotifyMask);
  gl.init(w, ExposureMask | ButtonPressMask | 
	  FocusChangeMask | KeyPressMask | StructureNotifyMask);
  gcv.background=normal_bg_color;
  gcv.foreground=window_text_color;
  gcv.font=fixfontstr->fid;
  gcw=XCreateGC(disp,w,GCBackground | GCFont,&gcv);
  gcv.font=vfontstr->fid;
  gct=XCreateGC(disp,w,GCBackground | GCFont | GCForeground,&gcv);
  rec.x=0;rec.y=0;rec.width=l-10;rec.height=h-46;
  XSetClipRectangles(disp,gct,4,24,&rec,1,YXSorted);
  ty=vfontstr->max_bounds.ascent+vfontstr->max_bounds.descent;
  vish=(h-48)/ty;
  tx=XTextWidth(vfontstr," ",1);
  tl=vfontstr->max_bounds.rbearing-vfontstr->min_bounds.lbearing;
  prflg=row=0;
  XStringListToTextProperty(&name,1,&wname);
  XSetWMIconName(disp,w,&wname);
  scr=baseguiplugin->new_ScrollBar(-2,0,h-23,this);
  scr->setrange(0,10);
  scr->init(w);
  XSetWMProtocols (disp, w, &wmDeleteWindow, 1);
}

void Editor::reconfigure(int il,int ih)
{
  l=il-20;h=ih;
  XRectangle rec;
  vcenterx=l/2;
  vcentery=h/2;
  rec.x=0;rec.y=0;rec.width=l-10;rec.height=h-46;
  XSetClipRectangles(disp,gct,4,24,&rec,1,YXSorted);
  vish=(h-48)/ty;
  cy=cx=0;
  XClearWindow(disp,w);
  expose();
  scr->reconfigure(l+18,0,h-23);
  if(external_reconfigure) external_reconfigure(this,x,y,l+20,h);
}
 

char Editor::trans(int mode,char sym)
{
  if(mode & LockMask)
    switch(sym)
    {
    case '[':sym='{';break;
    case ']':sym='}';break;
    case ';':sym=':';break;
    case '\'':sym='\"';break;
    case ',':sym='<';break;
    case '.':sym='>';break;
    };
  sym=rtbl[sym];
  return sym;
}

void Editor::func(int num)
{
  switch(num)
  {
  case 0: save();break;
  case 1: if(mod) if(save()) break;
  case 2: endfunc(this);break;
  case 3: find();break;
  case 4: gotoln();break;
  case 5: cfindfunc(YES); break;
  case 6: strcpy(cfindbuf,edbuf);
    if(load_etags()==-1)
      vmes(_("Error"), _("TAGS file not found..."),(Viewer*)this);
    break;
  };
}

void Editor::view()
{
  int i,f=25+vfontstr->max_bounds.ascent;
  for(i=0;i<vish;i++)
  {            
    if(i+base>=highb && i+base<highe)
      XSetForeground(disp,gct,dark_bg_color);
    else XSetForeground(disp,gct,normal_bg_color);
    XFillRectangle(disp,w,gct,4,25+i*ty,l-9,ty);
    if(i+base<maxr)
    {
      XSetForeground(disp,gct,window_text_color);
      XDrawString(disp,w,gct,5-stx,f+i*ty,buf[i+base],chl[i+base]);
    }
  }
  scr->val=base;
  scr->expose();
}

void Editor::info()
{
  int nl,k,tx;
  char str[64];
  XSetForeground(disp,gcw,normal_bg_color);
  XFillRectangle(disp,w,gcw,2,2,l-4,18);
  XSetForeground(disp,gcw,window_text_color);
  nl=strlen(node->name);
  tx=fixfontstr->max_bounds.rbearing-fixfontstr->min_bounds.lbearing;
  visl=(l-10)/tx;
  k=visl/3;
  if(nl>k) nl=k-1;
  XDrawString(disp,w,gcw,5,16,node->name,nl);
  sprintf(str,"%4d:%d (%d)",cx+dx+1,base+cy+1,maxr);
  if(mod) strcat(str,"  Mod");
  XDrawString(disp,w,gcw,10+k*tx,16,str,strlen(str));
  sprintf(str,"Size: %d",node->size);
  XDrawString(disp,w,gcw,15+k*2*tx+tx*3,16,str,strlen(str));

  XSetForeground(disp,gcw,normal_bg_color);
  XFillRectangle(disp,w,gcw,l-75,2,75,18);
  XSetForeground(disp,gcw,light_bg_color);
  XDrawLine(disp,w,gcw,l-40,2,l-3,2);
  XDrawLine(disp,w,gcw,l-40,2,l-40,19);
  XDrawLine(disp,w,gcw,l-75,2,l-41,2);
  XDrawLine(disp,w,gcw,l-75,2,l-75,19);
  XSetForeground(disp,gcw,dark_bg_color);
  XDrawLine(disp,w,gcw,l-40,19,l-3,19);
  XDrawLine(disp,w,gcw,l-3,2,l-3,19);
  XDrawLine(disp,w,gcw,l-75,19,l-41,19);
  XDrawLine(disp,w,gcw,l-41,2,l-41,19);
  if(rmode)
  {
    XSetForeground(disp,gcw,window_text_color);
    XDrawString(disp,w,gcw,l-36,16,"Rus",3);
  } else
  {
    XSetForeground(disp,gcw,window_text_color);
    XDrawString(disp,w,gcw,l-36,16,"Eng",3);
  }
  if(koi==0) {
    XSetForeground(disp,gcw,window_text_color);
    XDrawString(disp,w,gcw,l-72,16,"KOI",3);
  }
  else if(koi==1){
    XSetForeground(disp,gcw,window_text_color);
    XDrawString(disp,w,gcw,l-72,16,"DOS",3);
  } else {
    XSetForeground(disp,gcw,window_text_color);
    XDrawString(disp,w,gcw,l-72,16,"WIN",3);
  }
}

/*
void Editor::rusconvert()
{
  int i,j;
  if(koi==0)
  {
  for(i=0;i<maxr;i++)
  {
   for(j=0;j<chl[i];j++)
      buf[i][j]=rustbl[buf[i][j]];
    if(chl[i])
    xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
   }
   koi=1;
   } else if(koi==1)
   {
  for(i=0;i<maxr;i++)
  {
   for(j=0;j<chl[i];j++)
      buf[i][j]=dostbl[buf[i][j]];
    if(chl[i])
    xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
   }
   koi=0;
   } else wrusconvert();
}
*/

void printhbuf(unsigned char *buf, int size);


void Editor::rusconvert()
{
  int i,j;
  unsigned char *bu;
  unsigned char ch;
  if(koi==0)
  {
    xncdprintf(("Converting DOS->KOI\n"));
    for(i=0;i<maxr;i++)
    {
      bu=(unsigned char*)(buf[i]);
      for(j=0;j<chl[i];j++)
      {
	ch=bu[j];
	bu[j]=rustbl[ch];
      }
      if(chl[i])
	xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
    }
    koi=1;
  } else if(koi==1)
  {
    xncdprintf(("Converting KOI->DOS\n"));
    for(i=0;i<maxr;i++)
    {
      bu=(unsigned char*)(buf[i]);
      for(j=0;j<chl[i];j++)
      {
	ch=bu[j];
	bu[j]=dostbl[ch];
      }
      if(chl[i])
	xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
    }
    koi=0;
  } else wrusconvert();
  printhbuf((unsigned char*)(buf[0]),chl[0]);
}

void Editor::wrusconvert()
{
  int i,j;
  unsigned char *bu;
  unsigned char ch;
  if(koi==0)
  {
    xncdprintf(("Converting WIN->KOI\n"));
    for(i=0;i<maxr;i++)
    {
      bu=(unsigned char*)(buf[i]);
      for(j=0;j<chl[i];j++)
      {
	ch=bu[j];
	bu[j]=wrustbl[ch];
      }
      if(chl[i])
	xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
    }
    koi=2;
  } else if(koi==2)
  {
    xncdprintf(("Converting KOI->WIN\n"));
    for(i=0;i<maxr;i++)
    {
      bu=(unsigned char*)(buf[i]);
      for(j=0;j<chl[i];j++)
      {
	ch=bu[j];
	bu[j]=wintbl[ch];
      }
      if(chl[i])
	xl[i]=XTextWidth(vfontstr,buf[i],chl[i]);
    }
    koi=0;
  } else rusconvert();
  printhbuf((unsigned char*)(buf[0]),chl[0]);
}

void Editor::do_convertion()
{
  rusconvert();
  view();
  info();
  showcurs();
}

void Editor::do_wconvertion()
{
  wrusconvert();
  view();
  info();
  showcurs();
}                

void Editor::xyinfo()
{
  int nl,k,tx;
  char str[64];
  tx=fixfontstr->max_bounds.rbearing-fixfontstr->min_bounds.lbearing;
  visl=(l-10)/tx;
  k=visl/3;
  XSetForeground(disp,gcw,normal_bg_color);
  XFillRectangle(disp,w,gcw,k*tx,2,l-76-k*tx,18);
  XSetForeground(disp,gcw,window_text_color);
  sprintf(str,"%4d:%d (%d)",cx+dx+1,base+cy+1,maxr);
  if(mod) strcat(str,"  Mod");
  XDrawString(disp,w,gcw,10+k*tx,16,str,strlen(str));
  sprintf(str,"Size: %d",node->size);
  XDrawString(disp,w,gcw,15+k*2*tx+tx*3,16,str,strlen(str));
  if(maxr-1!=scr->maxval) {scr->maxval=scr->range=maxr-1;scr->setpages(maxr/vish);scr->expose();};
}

void Editor::expose()
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
  view();
  info();
  if(foc) showcurs();
  if(ffl)
  {
    XSetInputFocus(disp,w,RevertToNone,CurrentTime);
    ffl=0;
  }
}

void Editor::pagedown()
{
  if(base+vish<maxr)
  {
    if(ev.xkey.state & ShiftMask)
    {if(base+cy<highb || base+cy>highe)  highb=base+cy;
    highe=base+cy+vish;
    if(highe>maxr) highe=maxr;
    }
    showcurs(0);
    base+=vish;
    if(base+cy>=maxr) cy=maxr-base-1;
    while (calclen(cx+dx,cy)>=l-15) {cx--;dx++;stx+=tx;};
    view();
    showcurs();
  }
}

void Editor::pageup()
{
  if(base>0)
  {
    showcurs(0);
    if(ev.xkey.state & ShiftMask)
    {
      if(base+cy<highb || base+cy>highe)
	highe=base+cy;
      highb=base+cy-vish;
      if(highb<0) highb=0;
    }
    base-=vish;
    if(base<0) base=0;
    while (calclen(cx+dx,cy)>=l-15) {cx--;dx++;stx+=tx;};
    view();
    showcurs();
  }
}
   
void Editor::back()
{
  int i;
  int x=cx+dx,y=base+cy;
  showcurs(0);
  if(x<=chl[y])
  {
    mod=1;node->size--;
    for(i=x;i<chl[y]+1;i++)
      buf[y][i-1]=buf[y][i];
    chl[y]--;
    xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
  }
  cx--;
  if(cx<0) {cx=0;dx--;stx-=tx;view();}
  else
    showline(cy);
  showcurs();
}

void Editor::backln()
{
  int y=cy+base,i,j;
  if(y>0)
  {
    mod=1;node->size--;
    cx=chl[y-1];
    i=chl[y-1];j=0;
    while(j<=chl[y] && i<maxsyms-1)
      buf[y-1][i++]=buf[y][j++];
    if(j<=chl[y])
    {
      buf[y-1][i]=0;
      for(i=j;i<=chl[y];i++)
	buf[y][i-j]=buf[y][i];
      chl[y-1]=strlen(buf[y-1]);
      chl[y]=strlen(buf[y]);
      xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
      xl[y-1]=XTextWidth(vfontstr,buf[y-1],chl[y-1]);
    } else
    {
      chl[y-1]=strlen(buf[y-1]);
      xl[y-1]=XTextWidth(vfontstr,buf[y-1],chl[y-1]);
      if(maxr>1) node->size+=(chl[cy+base]+1);
      delln(cy);
    }
    if(cy>0){if(cy+base+1<maxr) cy--;
    while (calclen(cx+dx,cy)>=l-15) {cx--;dx++;stx+=tx;};
    }
    else if(base>0) {if(cy+base+1>=maxr) cy++;base--;
    while (calclen(cx+dx,cy)>=l-15) {cx--;dx++;stx+=tx;};
    };
    view();showcurs();
  }
}
 
void Editor::delln(int y)
{
  int i;
  if(maxr>1)
  {
    mod=1;
    y+=base;
    node->size-=(chl[y]+1);
    delete buf[y];
    for(i=y+1;i<=maxr;i++)
    {
      buf[i-1]=buf[i];
      chl[i-1]=chl[i];
      xl[i-1]=xl[i];
    }
    maxr--;
    if(cy+base==maxr) {cy--;if(cy<0) cy=0,base--;};
    info();
  }
}

void Editor::insempty(int y)
{
  int i;                  
  for(i=maxr;i>y;i--)
  {
    buf[i]=buf[i-1];
    chl[i]=chl[i-1];
    xl[i]=xl[i-1];
  }
  buf[y]=new char[maxsyms];
  chl[y]=0;
  xl[y]=0;
  maxr++;node->size++;
}

void Editor::insertln()
{
  int y=cy+base,x=dx+cx,i;
  if(maxr+1<maxlines)
  {
    mod=1;
    node->size++;
    for(i=maxr;i>y;i--)
    {                
      buf[i]=buf[i-1];
      chl[i]=chl[i-1];
      xl[i]=xl[i-1];
    }
    buf[y+1]=new char[maxsyms];
    for(i=0;i<maxsyms;i++)
      buf[y+1][i]=0;
    for(i=0;i<chl[y];i++)
      if(buf[y][i]==' ') buf[y+1][i]=' ';
      else break;
    buf[y+1][i]=0;
    chl[y+1]=i;
    if(x<chl[y])
    {
      for(i=x;i<=chl[y];i++)
	buf[y+1][i-x]=buf[y][i];
      buf[y][x]=0;
      buf[y][x+1]=0;
      chl[y]=strlen(buf[y]);
      xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
      cx=0;
    } else cx=i;
    y++;             
    chl[y]=strlen(buf[y]);
    xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
    stx=dx=0;
    maxr++;cy++;
    if(cy+1>vish) {cy--;base++;};
    view();showcurs();
  }
}

void Editor::insert(char sym)
{
  int y=cy+base,x=cx+dx,i;
  if(sym!=0 && x+1<maxsyms)
  {                
    mod=1;
    showcurs(0);node->size++;
    for(i=chl[y];i>x;i--)
      buf[y][i]=buf[y][i-1];
    buf[y][x]=sym;
    chl[y]++;
    xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
    cx++;
    i=0;
    while (calclen(cx+dx,cy)>=l-15) {cx--;dx++;stx+=tx;i=1;};
    if(i) view();
    else showline(cy);
    showcurs();
  }
}

void Editor::insspace()
{
  int y=cy+base,i;
  mod=1;
  for(i=chl[y];i<cx+dx;i++)
  {
    buf[y][i]=' ';
    node->size++;
  }
  buf[y][cx+dx]=0;
  chl[y]=strlen(buf[y]);
  xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
}
      
void Editor::sfind()
{
  ent=bw=0;
  cut_cur_word(findbuf);
  efind(this,3);
}

void Editor::sgoto()
{
  eask(_("Go to line..."),_("Line number:"),this,4);
}
  
void Editor::cfindwin()
{
  cut_cur_word(edbuf);
  eask(_("ETags-Find"),_("String to find:"),this,5);
}

void Editor::cfindpromptwin()
{
  strcpy(edbuf, cfindbuf);
  eask(_("ETAGS file"),_("Enter tags file:"),this,6);
}

void Editor::cfindfunc(int from_start)
{
  char file[2048];
  int err;
  if((err=cfind(from_start ? edbuf : cfindlast, file, from_start))<0)
  {
    switch(err)
    {
    case -2:
      vmes(_("Error"), _("Not a Emacs TAGS file!"),(Viewer*)this);
      break;
    case -1:
      vmes(_("Error"), _("Etags fatal..."),(Viewer*)this);
      break;
    }
    hidemes();
    return;
  }
  if(err==0)
    vmes(_("Sorry!"), _("Token not found..."),(Viewer*)this);
  else            
    if(cfind_func)
    {
      auto_find=YES;
      autofind_string=etags_last_find;
      cfind_func(file);
    }
}
  
void Editor::ssave()
{
  eex(this,0);
}

void Editor::gotoln()
{
  int num=0;
  sscanf(edbuf,"%d",&num);
  if(num<1 || num>maxr)  vmes(_("Error"),_("Wrong line number"),(Viewer*)this);
  else
  {
    num--;
    cy=num%vish;
    base=num-cy;
    dx=stx=cx=0;
    view();
    showcurs();
  }
}

void Editor::cut_cur_word(char *wordto)
{
  int y=cy+base,x=cx+dx;
  int i=x;
  while(i>=0)
  {
    if(isalnum(buf[y][i]) || buf[y][i]=='_')
      i--;
    else
    {
      i++;
      break;
    }
  }
  while(i<chl[y])
  {
    if(isalnum(buf[y][i]) || buf[y][i]=='_')
    {
      *wordto++=buf[y][i];
      i++;
    }
    else
      break;
  }
  *wordto=0;
}

/*
char selection_text[]="Transfer complete!";
void send_selection(int time,int requestor,int target,int property)
{
  XEvent event;
  
  event.xselection.type = SelectionNotify;
  event.xselection.selection = XA_PRIMARY;
  event.xselection.target = target;
  event.xselection.requestor = requestor;
  event.xselection.time = time;
  if (target == XA_STRING)
    {
      XChangeProperty(disp,requestor,property,XA_STRING,8,PropModeReplace,
                      selection_text,strlen(selection_text));
      event.xselection.property = property;
    }
  else
    {
      event.xselection.property = None;
      fprintf(stderr,"Not a string  %X=%d\n",target,target);
      XChangeProperty(disp,requestor,property,target,8,PropModeReplace,
                      selection_text,strlen(selection_text));
      event.xselection.property = property;
    }
   fprintf(stderr,"Send selection\n");
  XSendEvent(disp,requestor,False,0,&event);
}

void paste_primary(int window,int property,int Delete)
{
  Atom actual_type;
  int actual_format,i;
  long nitems, bytes_after, nread;
  unsigned char *data, *data2;

  if (property == None)
    {fprintf(stderr,"Prim None\n");return;};

  nread = 0;
      if (XGetWindowProperty(disp,window,property,nread/4,1024,Delete,
                             AnyPropertyType,&actual_type,&actual_format,
                             &nitems,&bytes_after,(unsigned char **)&data)
          != Success) fprintf(stderr,"XGetProp error\n");
 else fprintf(stderr,"Receive: %s\n",data);
}
*/

void Editor::enter_sym(XEvent* ev)
{
  XComposeStatus cs;
  Status status;
  KeySym ks;
  char sym[4]="";
  gl.LookupString(&ev->xkey,sym,4,&ks,&cs,&status);
  if(sym[0] && ev->xkey.state!=ControlMask)
    insert(rmode ? trans(ev->xkey.state, sym[0]) : sym[0]);
}

void Editor::click()
{
  XComposeStatus cs;
  Status status;
  Atom sel_property;
  KeySym ks;
  char sym[4];
  int j; 
  if(ev.xany.window==w)
  {
    switch(ev.type)
    {
    case Expose: expose();break;
    case UnmapNotify: unmap=1;break;
    case ButtonPress: 
      if(ev.xbutton.y>25) 
      {
	showcurs(0);
	cy=(ev.xbutton.y-25)/ty;
	if(cy>=vish) 
	  cy=vish-1;
	if(cy+base>=maxr) 
	  cy=maxr-base-1;
	cx=chl[cy+base]-dx;
	while(ev.xbutton.x < XTextWidth(vfontstr,buf[cy+base],cx)-1-stx) 
	  cx--;
	showcurs();
      };
      XSetInputFocus(disp,w,RevertToParent,CurrentTime);
      break;

    case ButtonRelease:break;
    case FocusIn: showcurs();foc=1; break;
    case FocusOut:showcurs(0);foc=0; break;
    case ConfigureNotify: 
      if(ev.xconfigure.width!=l+20 || ev.xconfigure.height!=h)
	reconfigure(ev.xconfigure.width,ev.xconfigure.height);
      break;
      //  case SelectionNotify: paste_primary(w,ev.xselection.property,True);break;
      //  case SelectionRequest: send_selection(ev.xselectionrequest.time,ev.xselectionrequest.requestor,
      //                 ev.xselectionrequest.target,ev.xselectionrequest.property);break;
    case KeyPress:
      ks=XLookupKeysym(&ev.xkey,0);
      if(enable_bracket)
      {
	switch(ks)
	{
	case XK_period:
	  cfindwin();
	  break;
	case XK_slash:
	  cfindpromptwin();
	  break;
	case XK_comma:
	  cfindfunc(NO);
	  break;
	case XK_greater:
	  showcurs(0);cx=cy=0;base=maxr-vish;
	  if(base<0) 
	    base=0;
	  view();
	  showcurs();
	  break;
	case XK_less:
	  showcurs(0);base=cx=cy=0;view();showcurs();
	  break;
	}
	enable_bracket=0;
	break;
      }
      switch(ks)
      {
      case XK_Down:
	if(cy+1+base<maxr)
	{ 
	  j=0;
	  if(ev.xkey.state & ShiftMask) 
	  {
	    if(cy+base<highb || cy+base>highe) 
	    {
	      highb=cy+base;
	      highe=highb+1;
	      j=1;
	    } else 
	      if(cy+base<=highe) 
	      {
		highe=cy+base+1;
		j=1;
	      }
	  }
	  showcurs(0);
	  cy++;
	  while (calclen(cx+dx,cy)>=l-15) 
	  {
	    cx--;
	    dx++;
	    stx+=tx;
	    j=1;
	  }
	  if(cy+1>vish) 
	  {
	    cy--;
	    base++;
	    j=1;
	  }
	  if(j) 
	    view();
	  showcurs();
	};
	break;

      case XK_Up: 
	if(cy+base>0)
	{
	  j=0;
	  if(ev.xkey.state & ShiftMask) 
	  {
	    if(cy+base-1<highb || cy+base>highe)
	    {
	      if(cy+base!=highb) 
		highe=cy+base;
	      highb=cy+base-1;
	      j=1;
	    } else 
	    {
	      highe=cy+base-1;
	      j=1;
	    }
	  }
	  if(cy>0) 
	  {
	    showcurs(0);
	    cy--;
	    while (calclen(cx+dx,cy)>=l-15) 
	    {
	      cx--;
	      dx++;
	      stx+=tx;
	      j=1;
	    }
	    if(j) 
	      view();
	    showcurs();
	  }
	  else 
	    if(base>0) 
	    {
	      base--;
	      while (calclen(cx+dx,cy)>=l-15) 
	      {
		cx--;
		dx++;
		stx+=tx;
	      }
	      view();
	      showcurs();
	    }
	}
	break;

      case XK_Next:
	if(ev.xkey.state & ControlMask)
	{
	  showcurs(0);
	  cx=cy=0;
	  base=maxr-vish;
	  if(base<0) 
	    base=0;
	  view();
	  showcurs();
	} else
	  pagedown();
	break;

      case XK_Prior: 
	if(ev.xkey.state & ControlMask)
	{
	  showcurs(0);
	  base=cx=cy=0;
	  view();
	  showcurs();
	} 
	else 
	  pageup();
	break;

      case XK_Tab:  
	if(cx+dx>chl[base+cy]) 
	  insspace();
	for(j=0;j<TAB_SIZE;j++) 
	  insert(' ');
	break;

      case XK_Right:
	if(cx+dx+1<maxsyms)
	{
	  showcurs(0);
	  if(calclen(cx+dx,cy)>=l-15) 
	  {
	    stx+=tx;
	    dx++;
	    view();
	  }
	  else 
	    cx++;
	  showcurs();
	} 
	break;

      case XK_Left: 
	if(cx>0) 
	{
	  showcurs(0);
	  cx--;
	  showcurs();
	}
	else 
	  if(dx>0) 
	  {
	    dx--;
	    stx-=tx;
	    view();
	    showcurs();
	  }
	break;

      case XK_Home: 
	showcurs(0);
	cx=0;
	if(dx>0) 
	{
	  dx=stx=0;
	  view();
	}
	showcurs();
	break;

      case XK_a:
      case XK_A: 
	if(ev.xkey.state & ControlMask) 
	{
	  //Do the same as XK_Home
	  showcurs(0);
	  cx=0;
	  if(dx>0) 
	  {
	    dx=stx=0;
	    view();
	  }
	  showcurs();
	  break;
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_E:
      case XK_e: 
	if(ev.xkey.state & ControlMask) 
	{
	  //Do the same as XK_End
	  if(cx!=chl[cy+base])
	  {
	    showcurs(0);
	    if(dx) 
	      j=1;
	    dx=stx=0;
	    cx=chl[cy+base];
	    while (calclen(cx+dx,cy)>=l-15) 
	    {
	      cx--;
	      dx++;
	      stx+=tx;
	    }
	    if(dx!=0 || j!=0) 
	      view();
	    showcurs();
	  }
	  break;
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_End: 
	if(cx!=chl[cy+base])
	{
	  showcurs(0);
	  if(dx) 
	    j=1;
	  dx=stx=0;
	  cx=chl[cy+base];
	  while (calclen(cx+dx,cy)>=l-15) 
	  {
	    cx--;
	    dx++;
	    stx+=tx;
	  }
	  if(dx!=0 || j!=0) 
	    view();
	  showcurs();
	}
	break;

      case XK_Execute: 
      case XK_Escape:
      case XK_F10:
        if(mod) 
	  eex(this,1);
	else 
	  endfunc(this);
	break;

      case XK_F2: ssave(); break;
      case XK_F3: copy();break;
      case XK_F4: paste();break;
      case XK_F5: cut();break;
      case XK_F6: if(swfunc) swfunc(this);break;
      case XK_F8: sgoto();break;
      case XK_F9: del();break;
	/*     case XK_F11:
	       XSetSelectionOwner(disp,XA_PRIMARY,w,ev.xkey.time);
	       if (XGetSelectionOwner(disp,XA_PRIMARY) != w)
	       fprintf(stderr,"Error owning XA_PRIMARY\n");
	       else fprintf(stderr,"Owned\n");
	       break;
	       case XK_F12:
	       sel_property = XInternAtom(disp,"IVES_SELECTION",False);
	       XConvertSelection(disp,XA_PRIMARY,XA_STRING,sel_property,w,ev.xkey.time);
	*/     
      case XK_F11: 
	if((ev.xkey.state & ControlMask) || (ev.xkey.state & ShiftMask)) 
	{
	  koi++;
	  if(koi>2) 
	    koi=0;
	  info();
	  break;
	}
	rusconvert();
	view();
	info();
	showcurs();
	break;

      case XK_F12: 
	wrusconvert();
	view();
	info();
	showcurs();
	break;

      case XK_Y:
      case XK_y: 
	if(ev.xkey.state & ControlMask) 
	{
	  delln(cy);
	  view();
	  showcurs();
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_B:
      case XK_b: 
	if(ev.xkey.state & ControlMask) 
	{
	  find_brackets();
	}
	else
	  enter_sym(&ev);
	break;

      case XK_C:
      case XK_c: 
	if(ev.xkey.state & ControlMask) 
	{
	  copy();
	  break;
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_V:
      case XK_v: 
	if(ev.xkey.state & ControlMask) 
	{
	  paste();
	  break;
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_bracketleft:
	if(ev.xkey.state & ControlMask) 
	{
	  enable_bracket=1;
	  break;
	}
	else 
	  enter_sym(&ev);
	break;

      case XK_X:
      case XK_x: 
	if(ev.xkey.state & ControlMask) 
	{
	  cut();
	  break;
	}
	else
	  enter_sym(&ev);
	break;

      case XK_F7:
      case XK_Find: sfind();break;
      case XK_BackSpace:
	if(cx+dx>0) 
	  back();
	else 
	  backln();
	break;

      case XK_D:
      case XK_d: 
	if(ev.xkey.state & ControlMask) 
	{
	  showcurs(0);
	  if(cx+dx<chl[base+cy])
	  {
	    cx++;
	    back();
	  }
	  else 
	    if(cy+1+base<maxr) 
	    {
	      cx=dx=stx=0;
	      cy++;
	      backln();
	    } else 
	      showcurs();
	  break;
	}
	else 
	  enter_sym(&ev);
	break;
      case XK_Delete: 
	if(ev.xkey.state & ControlMask) 
	{
	  del();
	  break;
	};
	showcurs(0);
	if(cx+dx<chl[base+cy])
	{
	  cx++;
	  back();
	}
	else 
	  if(cy+1+base<maxr) 
	  {
	    cx=dx=stx=0;
	    cy++;
	    backln();
	  } else 
	    showcurs();
	break;

      case XK_KP_Enter:
      case XK_Return: insertln();break;
#ifndef NO_ALT
      case XK_Mode_switch:
      case XK_Alt_R: rmode^=1;info();break;
#else
      case XK_Alt_R: break;
#endif

      case XK_KP_Multiply:
	if(cx+dx>chl[base+cy]) insspace();
	insert('*');break;

      case XK_KP_Divide:
	if(cx+dx>chl[base+cy]) insspace();
	insert('/');break;

      case XK_KP_Add:
	if(cx+dx>chl[base+cy]) insspace();
	insert('+');break;

      case XK_KP_Subtract:
	if(cx+dx>chl[base+cy]) insspace();
	insert('-');break;

      case XK_Caps_Lock:
      case XK_Control_L:
      case XK_Control_R:
      case XK_Alt_L:
      case XK_Shift_L:
      case XK_Shift_R: break;
      case XK_l:
      case XK_L:if(ev.xkey.state & ControlMask) {find();break;};

      default:
	if(cx+dx>chl[base+cy]) 
	  insspace();

	gl.LookupString(&ev.xkey,sym,4,&ks,&cs,&status);
	j=strlen(sym);
	if(j>=1 && ev.xkey.state!=ControlMask)
	  insert(rmode ? trans(ev.xkey.state,sym[0]) : sym[0]);
	break;
        
      }
      break;

    case ClientMessage:
      if (ev.xclient.format == 32 && ev.xclient.data.l[0] == wmDeleteWindow)
        if(mod) 
	  eex(this,1);
	else if(endfunc) 
	  endfunc(this);
      break;
     
    };
  }
}

void Editor::exit_and_save()
{
  if(mod) eex(this,1);else endfunc(this);
}

void Editor::copy()
{
  char *cutbuf;
  int cl=0,i,j,k;
  if(highb!=highe)
  {
    for(i=highb;i<highe;i++)
      cl+=chl[i]+1;
    cutbuf=new char[cl];
    cl--;
    k=0;
    for(i=highb;i<highe;i++)
    {
      for(j=0;j<chl[i];j++)
	cutbuf[k++]=buf[i][j];
      cutbuf[k++]=0xa;
    }
    XStoreBytes(disp,cutbuf,cl);
    delete cutbuf;
  }
}
  
void Editor::cut()
{
  char *cutbuf;
  int cl=0,i,j,k;
  if(highb!=highe)
  {
    for(i=highb;i<highe;i++)
      cl+=chl[i]+1;
    cutbuf=new char[cl];
    cl--;
    k=0;
    for(i=highb;i<highe;i++)
    {
      for(j=0;j<chl[i];j++)
	cutbuf[k++]=buf[i][j];
      cutbuf[k++]=0xa;
    }
    XStoreBytes(disp,cutbuf,cl);
    delete cutbuf;
    for(i=highb;i<highe;i++)
      delln(highb-base);
    highe=highb;
    if(cy+base>highb) {cy=highb%vish;base=highb-cy;};
    view();
    showcurs();
  }
}
  

void Editor::del()
{
  int i;
  if(highb!=highe)
  {
    for(i=highb;i<highe;i++)
      delln(highb-base);
    highe=highb;
    if(cy+base>highb) {cy=highb%vish;base=highb-cy;};
    view();
    showcurs();
  }
}
  
  
void Editor::paste()
{
  int cl,i,j=0,y;    
  char *cutbuf;
  cutbuf=XFetchBytes(disp,&cl);
  if(cl>0)
  {
    y=base+cy;highb=y;
    cutbuf[cl]=0;
    insempty(y);
    for(i=0;i<cl;i++)
    {
      if(cutbuf[i]!=0xa) buf[y][j++]=cutbuf[i];
      else 
      {
	chl[y]=j;
	xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
	buf[y][j]=0;y++;j=0;node->size+=chl[y];
	insempty(y);
	node->size--;
      }
    }
    chl[y]=j;
    node->size+=cl;
    xl[y]=XTextWidth(vfontstr,buf[y],chl[y]);
    buf[y][j]=0;
    highe=y+1;
    mod=1;
    view();showcurs();
    XFree(cutbuf);
  }
}

void Editor::showcurs(int f)
{
  int curl;
  if(base+cy>=maxr) cy=maxr-base-1;
  if(f) XSetForeground(disp,gct,window_text2_color);
  else 
    if(cy+base>=highb && cy+base<highe)
      XSetForeground(disp,gct,dark_bg_color);
    else XSetForeground(disp,gct,normal_bg_color);
  if(cx+dx<chl[cy+base])
    curl=XTextWidth(vfontstr,buf[cy+base],cx+dx)-1-stx;
  else curl=xl[cy+base]+(cx+dx-chl[cy+base])*tx-1-stx;
  XDrawLine(disp,w,gct,5+curl,25+ty*cy,5+curl,24+ty*cy+ty);
  XDrawLine(disp,w,gct,6+curl,25+ty*cy,6+curl,24+ty*cy+ty);
  if(f==0) 
  {
    XSetForeground(disp,gct,window_text_color);
    XDrawString(disp,w,gct,5-stx,25+vfontstr->max_bounds.ascent+cy*ty,
		buf[cy+base],chl[cy+base]);
  }
  xyinfo();
}

void Editor::scroll(Gui* o)
{
  ScrollBar* oo=(ScrollBar*)o;
  base=oo->val;
  view();showcurs();xyinfo();
}

void Editor::scrollup(Gui* o)
{
  scroll(o);
}

void Editor::scrolling(Gui* o)
{
  scroll(o);
}

void Editor::scrolldown(Gui* o)
{
  scroll(o);
}

void Editor::showline(int nl)
{
  if(nl+base>=highb && nl+base<highe)
    XSetForeground(disp,gct,dark_bg_color);
  else XSetForeground(disp,gct,normal_bg_color);
  XFillRectangle(disp,w,gct,4,25+nl*ty,l-9,ty);
  XSetForeground(disp,gct,window_text_color);
  XDrawString(disp,w,gct,5-stx,25+vfontstr->max_bounds.ascent+nl*ty,
	      buf[nl+base],chl[nl+base]);
}

int Editor::calclen(int ix,int iy)
{
  int il=chl[base+iy];
  if(ix<il) return XTextWidth(vfontstr,buf[iy+base],ix)-stx;
  return xl[iy+base]+(ix-il)*tx-stx;
}

void Editor::show() 
{
  // oldel=el.next;
  // el.next=NULL;
  XMapRaised(disp,w);
  addto_el(this,w);
  scr->show();
  ffl=1;
  buf=new char* [maxlines];
  for(int i=0;i<maxlines;i++)
    buf[i]=NULL;
  xl=new int[maxlines];
  chl=new int[maxlines];
}

void Editor::hide()
{
  int i;
  scr->hide();
  XUnmapWindow(disp,w);
  delfrom_el(this);
  delfrom_exp(this);
  // el.next=oldel;
  for(i=0;i<maxlines;i++)
    if(buf[i]!=NULL) delete buf[i];
  delete buf;
  buf=NULL;
  delete chl;
  delete xl;
  delete scr;
}     


void Editor::newfile(FList* cur)
{
  node=cur;
  maxr=1;
  buf[0]=new char[maxsyms];
  chl[0]=0;
  xl[0]=0;
  cy=base=cx=dx=stx=mod=highb=highe=0;
  dos=0;
}

void Editor::load(FList* cur)
{
  char *tmp;
  int i,j,k,q;
  node=cur;
  dos=0;koi=0;
  int fp=open(cur->name,O_RDONLY);
  if(fp!=-1)
  {
    if(buf!=NULL)
    {
      for(i=0;i<maxlines;i++)
	if(buf[i]){ delete buf[i];buf[i]=NULL;};
    }
    tmp=new char[node->size];
    if(read(fp,tmp,node->size)<0)
      vmes(_("FATAL Error!"),_("Error loading file..."),(Viewer*)this);
    close(fp);
    i=j=k=0;
    buf[i]=new char[maxsyms];
    while(k<node->size)
    {
      if(tmp[k]==0xd) {k++;dos=1;};
      if(tmp[k]==0x9) 
      {
	k++;
	if(j+10>maxsyms) continue;
	for(q=0;q<8;q++)
          buf[i][j++]=' ';
	continue;
      }
      //   if((unsigned char)tmp[k]>=0x80 && (unsigned char)tmp[k]<0xc0) {koi=0;};
      buf[i][j]=tmp[k++];
      if(j+2>=maxsyms)
      {
	j++;
	buf[i][j]=0;
	chl[i]=j;
	if(j>0)
	  xl[i]=XTextWidth(vfontstr,buf[i],j);
	else xl[i]=0;
	j=0;
	buf[++i]=new char[maxsyms];
      } else
	if(buf[i][j]==0xa)
	{
	  buf[i][j]=0;
	  chl[i]=j;
	  if(j>0)
	    xl[i]=XTextWidth(vfontstr,buf[i],j);
	  else xl[i]=0;
	  j=0;
	  buf[++i]=new char[maxsyms];
	} else j++;
      if(i>=maxlines)
      {
	vmes(_("Error!"),_("Maximum lines exeeded!"),(Viewer*)this);
	break;
      }
    }
    delete tmp;
    buf[i][j]=0;chl[i]=j;xl[i]=XTextWidth(vfontstr,buf[i],j);
    maxr=i+1;
    scr->setpages(maxr/vish);
    scr->setrange(0,maxr-1);
    base=cx=cy=stx=dx=mod=0;highb=highe=0;
  } else {vmes(_("FATAL Error"),_("Error loading file!"),(Viewer*)this);newfile(cur);};
}

int Editor::save()
{
  int i,oldkoi;
  char *b;
  FILE* fp;
  if(strcmp(edbuf,node->name)==0)
  {
    strcpy(fname,node->name);
    if(dos&1)
    {
      b=strchr(fname,'.');
      if(b==NULL) b=fname+strlen(fname);
      if(strcmp(b,".bug")==0) strcpy(b,".old");
      else strcpy(b,".bug");
      //   fprintf(stderr,"BNAME: %s\n",fname);
    } else
      strcat(fname,".old");
    rename(node->name,fname);
  }
  strcpy(fname,edbuf);
  fp=fopen(fname,"w");
  if(fp!=NULL)
  {
    if(dos&2)
    {
      oldkoi=koi;
      rusconvert();
    } 
    for(i=0;i<maxr-1;i++)
    {
      buf[i][chl[i]]=0;
      if(dos&1)
	fprintf(fp,"%s\r\n",buf[i]);
      else
	fprintf(fp,"%s\n",buf[i]);
    }
    buf[i][chl[i]]=0;
    fprintf(fp,"%s",buf[i]);
    fclose(fp);
    if(dos&2)
    {
      if(oldkoi==2)
	wrusconvert();
      else
	rusconvert();
    }
    // delete node->name;
    // node->name=new char[strlen(fname)+1];
    strcpy(node->name,fname);
    chmod(node->name,node->mode);
    mod=0;
    info();
  } else  {vmes(_("Error"),_("Error saving file!"),(Viewer*)this);return 1;};
  return 0;
}

void Editor::reverse_find_bracket()
{
  char br1[]="{[(<";
  char br2[]="}])>";
  int i1,i,j,brcoun=0;
  for(i1=0;br2[i1]!=0;i1++)
    if(br2[i1]==buf[base+cy][cx+dx]) 
      break;
  if(br2[i1]==0)
  {
    vmes(_("Warning!"),_("Symbol not a bracket!"),(Viewer*)this);
    return;
  }
  for(i=base+cy;i>=0;i--)
  {
    for(j=(cy+base==i ? cx+dx : chl[i]-1);j>=0;j--)
    {
      if(buf[i][j]==br2[i1])
	brcoun++;
      else
	if(buf[i][j]==br1[i1])
	  brcoun--;
      if(brcoun==0)
      {
	cy=i%vish;
	base=i-cy;
	cx=j;dx=0;stx=0;
	view();
	showcurs();
	return;
      }
    }
  }
  vmes(_("Warning!"),_("Another bracket not found!"),(Viewer*)this);
}

void Editor::find_brackets()
{
  char br1[]="{[(<";
  char br2[]="}])>";
  int i1,i,j,brcoun=0;
  for(i1=0;br1[i1]!=0;i1++)
    if(br1[i1]==buf[base+cy][cx+dx]) 
      break;
  if(br1[i1]==0)
  {
    reverse_find_bracket();
    return;
  }
  for(i=base+cy;i<maxr;i++)
  {
    for(j=(cy+base==i ? cx+dx : 0);j<chl[i];j++)
    {
      if(buf[i][j]==br1[i1])
	brcoun++;
      else
	if(buf[i][j]==br2[i1])
	  brcoun--;
      if(brcoun==0)
      {
	cy=i%vish;
	base=i-cy;
	cx=j;dx=0;stx=0;
	view();
	showcurs();
	return;
      }
    }
  }
  vmes(_("Warning!"),_("Another bracket not found!"),(Viewer*)this);
}

void Editor::set_straight_and_find(char *str)
{
  strcpy(edbuf,str);
  ent=1;
  bw=0;
  find();
}

  
void Editor::find()
{
  int i,j,k=0;
  strcpy(findbuf,edbuf);
  if(bw==0)
    for(i=(ent ? 0 : base+cy);i<maxr;i++)
    {
      for(j=(cy+base==i ? cx+dx : 0);j<chl[i];j++)
	if(findbuf[0]==buf[i][j])
	{
	  k=1;
	  while(findbuf[k]==buf[i][j+k] && findbuf[k]!=0) k++;
	  if(findbuf[k]==0) break;
	}
      if(findbuf[k]==0) break;
    } 
  else
    for(i=(ent ? maxr-1 : base+cy);i>=0;i--)
    {
      for(j=(cy+base==i ? cx+dx : 0);j<chl[i];j++)
	if(findbuf[0]==buf[i][j])
	{
	  k=1;
	  while(findbuf[k]==buf[i][j+k] && findbuf[k]!=0)
	    k++;
	  if(findbuf[k]==0) break;
	}
      if(findbuf[k]==0) break;
    }
  if(findbuf[k]==0)
  {
    cy=i%vish;
    base=i-cy;
    cx=k+j;dx=0;stx=0;
    view();
    showcurs();
  } else vmes(_("Warning!"),_("String not found"),(Viewer*)this);
  ent=0;
}
/////////////////////////////////End of file//////////////////////////////


