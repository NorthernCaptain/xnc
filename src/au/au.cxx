/****************************************************************************
*  Copyright (C) 1998 by Leo Khramov
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


#include "au_sup.h"

#define DEVNULL        " >/dev/null"

extern char home_files_dir[];

int superr=0;
int supwar=0;
int scurerr=0;
char SUP::scanstr[1002];
int afs_disabled=0;

int au_out=1;

/*
 * UNQ_SET defines the set of characters that don't need quoting.
 * Must not necessarily be complete, but should include those
 * characters that are used most often (performance and string length),
 * but _must_not_ include any character that might get interpreted by
 * a shell.
 */
#define UNQ_SET \
 "abcdefghijklmnopqrstuvwxyz" \
 "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
 "0123456789" \
 ".-_/"

#define FIRST_LETTER_SET \
 "abcdefghijklmnopqrstuvwxyz" \
 "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
 "0123456789" \
 "._/"

/*
 * The following kind of quoting should work for every shell:
 *   - the characters in UNQ_SET are copied unchanged
 *   - any other characters is escaped using '\'
 */ 
char* quote_path(char *dest, const char *src)
{
    char *t = dest;
    *t++ = *src++;
    
    while (*src) 
    {
	if (!strchr(UNQ_SET, *src)) 
	{
	    *t++ = '\\';
	}
	*t++ = *src++;
    }
    *t = 0;
    return dest;
}

char* quote_first_path(char *dest, const char *src)
{
    char *t = dest;
    if(!strchr(FIRST_LETTER_SET, *src))
    {
	*t++ = '.';
	*t++ = '/';
    }
    strcpy(t, src);
    return dest;
}

char* strong_quote_path(char *dest, const char *src)
{
    char *t = dest;
    if(!strchr(FIRST_LETTER_SET, *src))
    {
	*t++ = '.';
	*t++ = '/';
    }
    *t++ = *src++;
    
    while (*src) 
    {
	if (!strchr(UNQ_SET, *src)) 
	{
	    *t++ = '\\';
	}
	*t++ = *src++;
    }
    *t = 0;
    return dest;
}


/*
 * The following macro is used to define all SUP::single_* commands.
 */
#define SUP_single(cmd, posarr, fmt) \
char *SUP::single_##cmd (char *dest, const char *archive, const char *file) \
{ \
   char buf1[2*L_MAXPATH];                                  \
                                                           \
   if (posarr[0] < posarr[1])                              \
      sprintf(dest, fmt, strong_quote_path(buf1, archive), file);  \
   else                                                    \
      sprintf(dest, fmt, file, strong_quote_path(buf1, archive));  \
   return dest;                                            \
}

SUP_single( extract_command,     sarc,  sunpack )
SUP_single( listextract_command, luarc, lunpack )
SUP_single( listpack_command,    lparc, lpack   )
SUP_single( pack_file_command,   parc,  spack   )
SUP_single( pack_dir_command,    pdarc, sdpack  )
SUP_single( delete_command,      darc,  sdelpack)
SUP_single( create_command,      create_arc,  create_com)
SUP_single( create_many,         create_many_arc,  create_many_com)

void SUP::skip_header(FILE* fp)
{
 int i,j,l,ls;
 new_line=1;
 if(headln)
  for(i=0;i<headln;i++)
   fgets(sup_tmp,128,fp);
 else if(sep[0]!=0)
 {
   l=strlen(sep);
   while(1)
   {
          if(fgets(sup_tmp,128,fp)==NULL) return;
          ls=strlen(sup_tmp);
          for(i=0;i<ls-l;i++)
             {
                   j=0;
                   while(sup_tmp[i+j]==sep[j]) j++;
                   if(sep[j]==0) return;
             }
   }
 }
 new_line=1;
}

char* SUP::getword(FILE* fp, int& co)
{
  int i=0;
  char *s;
  int rest_of_line;

  if (co==total)
    co=0;
  if (new_line) {
    new_line=0;
    if (fgets(scanstr,1000,fp) == NULL)
      return NULL;
    scancoun=0;
  }
  while (scanstr[scancoun] == ' ')
    scancoun++;
  if (scanstr[scancoun]=='\n' || scanstr[scancoun]==0) {
    if(fgets(scanstr,1000,fp) == NULL)
      return NULL;
    scancoun=0;
    while(scanstr[scancoun] == ' ')
      scancoun++;
  }
  
  s = scanstr + scancoun;
  while (s[i] == sep[i])
    i++;
  if (i!=0 && sep[i]==0)
    return NULL;
  i = 0;
  // if the name is the last "word" on the line, take the rest of the
  // line. This way filename including a ' ' can be used. (And the
  // rest of the code does not see any difference...)
  rest_of_line = ((co == total-1 || name_cr) && co==dname);
  while((rest_of_line || scanstr[scancoun] != ' ')
        && scanstr[scancoun] != '\n'
        && scanstr[scancoun] != 0 ) 
    sup_tmp[i++] = scanstr[scancoun++];
  sup_tmp[i] = 0;
  co++;
  return sup_tmp;
}

void SUP::flush_line()
{
 new_line=1;
}

int SUP::is_yours(char *ar)
{
 int l=strlen(ar),i;
 for(i=0;i<maxext;i++)
  if(strcmp(ar+l-strlen(ext[i]),ext[i])==0) return 1;
 return 0;
}

void SUP::show()
{
 int i;
 fprintf(stderr,"\nSupport: %s\nExtensions:",name);
 for(i=0;i<maxext;i++)
  fprintf(stderr,"  %s",ext[i]);
 fprintf(stderr,"\nHeader lines: %d\n",headln);
 fprintf(stderr,"Separator: '%s'\n",sep);
 fprintf(stderr,"Name pos: %d\n",dname);
 fprintf(stderr,"Size pos: %d\n",dsize);
 fprintf(stderr,"Attr pos: %d\n",dattr);
 fprintf(stderr,"List command: %s\n",list);
 fprintf(stderr,"Unpack command: %s [%d %d]\n",unpack,uarc[0],uarc[1]);
 fprintf(stderr,"Single unpack command: %s [%d %d]\n",sunpack,sarc[0],sarc[1]);
 fprintf(stderr,"List unpack command: %s [%d %d]\n",lunpack,luarc[0],luarc[1]);
 if(next) next->show();
}

void SUP::addseparator(char* s)
{
 int i=0;
 while(s[i]!=0 && i<9) sep[i]=s[i++];
 sep[i]=0;
}

void SUP::addext(char* s)
{
 int l=strlen(s)+1;
 if(maxext<10)
 {
 ext[maxext]=new char[l];
 strcpy(ext[maxext],s);
 maxext++;
 }
}

SUP *sup=NULL;
SUP *cursup=NULL;
LexDemon *dm;
char *sbuf;

void add_support(SUP *s)
{
 s->next=sup;
 sup=s;
}

int calc_rows()
{
 char *b=sbuf;
 int coun=1;
 while(b!=dm->lastinp)
  if(*b++==0xa) coun++;
 return coun;
}

void swarning(char *mes)
{
 fprintf(stderr,"\nWarning (%02d): %s\n",calc_rows(),mes);
 supwar++;
}

void serror(char *mes)
{
 fprintf(stderr,"\nError (%02d): %s\n",calc_rows(),mes);
 superr++;scurerr++;
}

int fendsup()
{
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 if(scurerr==0)
 {
 if(cursup->dname!=-1 && cursup->dname!=cursup->dsize)
 {
 fprintf(stderr," %s",cursup->name);
 add_support(cursup);
 } else
 {
  serror("No syntax found or wrong syntax");
  delete cursup;
  cursup=NULL;
  return 0;
 }
 } else {
 fprintf(stderr,"\nSkip support '%s' because found (%d) errors\n",cursup->name,scurerr);
 delete cursup;
 }
 cursup=NULL;
 return 1;
}

int fhead()
{
 char c;
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 c=dm->process_lex();
 if(c!='c')
 {
  dm->unlex(c);
  serror("Must be digital constant");
  return 0;
 }
 cursup->headln=dm->con;
 return 1;
}
 
int fsep()
{
 char c;
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 c=dm->process_lex();
 if(c!='s')
 {
  dm->unlex(c);
  serror("Must be character string");
  return 0;
 }
 cursup->addseparator(dm->wlast);
 return 1;
}

int fsupport()
{
 char c=dm->process_lex();
 if(c!='s')
 {
   swarning("No support name");
   dm->unlex(c);
   return 1;
  }
 if(cursup)
 {
  serror("Previous support not closed");
  return 0;
 }
 cursup=new SUP;
 strcpy(cursup->name,dm->wlast);
 scurerr=0;
 return 1;
}

int fsyntax()
{
  char c;
  int coun;
  int ok;

  if (cursup==NULL) {
    serror("No current support");
    return 0;
  }
  c=dm->process_lex();
  if (c != ':') {
    serror("Missing separator ':'");
    dm->unlex(c);
    return 0;
  }
  coun = 0;
  ok = 1;
  while (ok) {
    switch ((c = dm->process_lex())) {
    case 12: /* name */
      cursup->dname = coun;
      cursup->name_cr=0;
      break;
    case 13: /* size */
      cursup->dsize = coun;
      break;
    case 15: /* attribute */
      cursup->dattr = coun;
      break;
    case 21: /* date */
      cursup->ddate = coun;
      break;
    case 22: /* time */
      cursup->dtime = coun;
      break;
    case 23: /* month */
      cursup->dmonth = coun;
      break;
    case 24: /* day */
      cursup->dday = coun;
      break;
    case 25: /* year */
      cursup->dyear = coun;
      break;
    case 26: /* name_cr - name to end of line (Carrige Return) */
      cursup->dname = coun;
      cursup->name_cr=1;
      break;
    case 14:
      break;
    default:
      ok = 0;
    }
    coun++;
  }
  coun--;
  if (c == ';') {
    cursup->total = coun;
    return 1;
  }
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int flist()
{
  char c;
  int coun;
  char st[80]="";

  if (cursup==NULL) {
    serror("No current support");
    return 0;
  }
  c=dm->process_lex();
  if(c != ':') {
    serror("Missing separator ':'");
    dm->unlex(c);
    return 0;
  }
  coun=0;
  while ((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@') {
    switch(c) {
    case 10:
      cursup->larc[0] = coun++;
      strcat(st,"%s ");
      break;
    case 11: cursup->larc[1] = coun++;
      strcat(st,"%s ");
      break;
    case '@':
      strcat(st,"@");
      break;
    default:
      strcat(st,dm->wlast);
      strcat(st," ");
      break;
    }
  }
  if (c == ';') {
    strcpy(cursup->list,st);
    return 1;
  }
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int funpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->uarc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->uarc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {strcpy(cursup->unpack,st);
    strcat(cursup->unpack, DEVNULL);
    return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fsunpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@') 
 {
  switch(c) {
   case 10: cursup->sarc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->sarc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->sunpack,st);
     strcat(cursup->sunpack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fcreate_archive()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@') 
 {
  switch(c) {
   case 10: cursup->create_arc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->create_arc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->create_com,st);
     //     strcat(cursup->create_com, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fcreate_many_archive()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@') 
 {
  switch(c) {
   case 10: cursup->create_many_arc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->create_many_arc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->create_many_com,st);
     //     strcat(cursup->create_com, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->parc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->parc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->spack,st);
     strcat(cursup->spack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fdirpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->pdarc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->pdarc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->sdpack,st);
     strcat(cursup->sdpack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int flistex()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->luarc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->luarc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->lunpack,st);
     strcat(cursup->lunpack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int flistpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->lparc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->lparc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->lpack,st);
     strcat(cursup->lpack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fdelpack()
{
 char c;
 int coun;
 char st[80]="";
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 coun=0;
 while((c=dm->process_lex())=='s' || c==10 || c==11 || c=='@')
 {
  switch(c) {
   case 10: cursup->darc[0]=coun++;strcat(st,"%s ");break;
   case 11: cursup->darc[1]=coun++;strcat(st,"%s ");break;
   case '@': strcat(st,"@");break;
   default: strcat(st,dm->wlast);strcat(st," ");break;
   };
  }
  if(c==';') 
   {
     strcpy(cursup->sdelpack,st);
     strcat(cursup->sdelpack, DEVNULL);
     return 1;
   };
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}
   
int fexten()
{
 char c;
 if(cursup==NULL)
 {
  serror("No current support");
  return 0;
 }
 c=dm->process_lex();
 if(c!=':')
 {
  serror("Missing separator ':'");
  dm->unlex(c);
  return 0;
 }
 while((c=dm->process_lex())=='s')
   cursup->addext(dm->wlast);
  if(c==';') return 1;
  dm->unlex(c);
  serror("Missing end of sequence ';'");
  return 0;
}

static struct lexdata_s term[] = {
 { "support",            1  },
 { "extensions",         2  }, 
 { "header",             3  },
 { "separator",          4  },
 { "syntax",             5  },
 { "list",               6  },
 { "extract",            7  },
 { "single_ex",          8  },
 { "endsupport",         9  },
 { "archive",            10 },
 { "archieve",           10 },
 { "file",               11 },
 { "name",               12 },
 { "size",               13 },
 { "word",               14 },
 { "attributes",         15 },
 { ":",                  ':'},
 { ";",                  ';'},
 { ">LSF",               'H'},
 { ">endLSF",            'E'},
 { "delete_file",        16 },
 { "single_packfile",    17 },
 { "single_packdir",     18 },
 { "list_unpack",        19 },
 { "list_pack",          20 },
 { "@",                  '@'},
 { "date",               21 },
 { "time",               22 },
 { "month",              23 },
 { "day",                24 },
 { "year",               25 },
 { "name_cr",            26 },
 { "single_create",      27 },
 { "multi_create",       28 },
 { NULL, 0 }
};
      

static int (*aufuncs[])() = {
/*  0 */  NULL,
/*  1 */  fsupport,
/*  2 */  fexten,
/*  3 */  fhead,
/*  4 */  fsep,
/*  5 */  fsyntax,
/*  6 */  flist,
/*  7 */  funpack,
/*  8 */  fsunpack,
/*  9 */  fendsup,
/* 10 */  NULL,
/* 11 */  NULL,
/* 12 */  NULL,
/* 13 */  NULL,
/* 14 */  NULL,
/* 15 */  NULL,
/* 16 */  fdelpack,
/* 17 */  fpack,
/* 18 */  fdirpack,
/* 19 */  flistex,
/* 20 */  flistpack,
/* 21 */  NULL,
/* 22 */  NULL,
/* 23 */  NULL,
/* 24 */  NULL,
/* 25 */  NULL,
/* 26 */  NULL,
/* 27 */  fcreate_archive,
/* 28 */  fcreate_many_archive
};

int sup_init(char *sbuf)
{
 dm = new LexDemon(term);
 dm->set_input_stream(sbuf);
 dm->set_terminators(";:@");
 char c=dm->process_lex();
 if(c!='H') {
   fprintf(stderr,"Error: Not a support file...\n");
   delete dm;
   return 0;
 }
 return 1;
}

int comp_sup(char *msg, char *sname, int (*sup_init)(char*), int (*aufuncs[])())
{
  int fp;
  int n, siz;

  fp = open(sname,O_RDONLY);
  if (fp == -1) {
    if(au_out)
      fprintf(stderr,"Error: Can't open support '%s'\n",sname);
    return 0;
  }
  siz=lseek(fp, 0l, SEEK_END);
  lseek(fp, 0l, SEEK_SET);
  sbuf = new char[siz];
  read(fp, sbuf, siz);
  close(fp);
  if (!sup_init(sbuf))
    return 0;
  if(au_out)
    fprintf(stderr, "%s", msg);
  while ((n = dm->process_lex())) {
    if (n=='E')
      break;
    if (n > 32)
      serror("Missing operator");
    else if (aufuncs[n])
      aufuncs[n]();
  }
  if(au_out)
  {
    if(msg[strlen(msg)-1]=='.')
       fprintf(stderr, "OK");
    fprintf(stderr,"\n     (%d) warnings, (%d) errors\n",supwar,superr);
  }
  delete dm;
  delete sbuf;
  return 1;
}


//If Archive File System file then return 1 else 0
int is_it_afs_file(char *ar)
{
 SUP *sp=sup;
 if(afs_disabled) return 0;
 while(sp!=NULL)
  if(sp->is_yours(ar)) return 1;
  else sp=sp->next;
 return 0;
}

SUP* find_support(char *ar)
{
 SUP *sp=sup;
 if(afs_disabled) return NULL;
 while(sp!=NULL)
  if(sp->is_yours(ar)) return sp;
  else sp=sp->next;
 return NULL;
}

/*
void do_list(char *ar)
{
 SUP *sp=sup;
 FILE* fp;
 int coun;
 char str[128];
 char name[40];
 char size[20];
 char attrchr[20];
 char *w;
 while(sp!=NULL)
  if(sp->is_yours(ar)) break;
  else sp=sp->next;
 fprintf(stderr,"Using support '%s'\n",sp->name); 
 sprintf(str,sp->list,ar);
 fprintf(stderr,"Command: %s\n",str);
 fp=popen(str,"r");
 if(fp==NULL)
 {
  fprintf(stderr,"Error: popen failed...\n");
  return;
 }
 sp->skip_header(fp);
 coun=0;
 while((w=sp->getword(fp,coun))!=NULL)
 {
  if(coun==sp->dname+1) strcpy(name,w);
  else if(coun==sp->dsize+1) strcpy(size,w);
  else if(coun==sp->dattr+1) strcpy(attrchr,w);
  if(coun==sp->total)
  {
  if(sp->dattr!=-1)
    fprintf(stderr,"file: %-20s size: %10s attr: %s\n",name,size,attrchr);
  else
    fprintf(stderr,"file: %-20s size: %10s attr: no info\n",name,size);
   sp->flush_line();
  }
 }
 pclose(fp);
}
*/

void compile_AFS_supports(char *supname)
{
 char str[1024];
 sprintf(str,"%s/%s",home_files_dir,supname);
 supwar=superr=0;
 if(comp_sup("Compiling AFS supports:", str, sup_init, aufuncs)==0)
 {
  fprintf(stderr,"\n!!!!!XNC PANIC!!!!! Archive file system will be disabled!\n");
  afs_disabled=1;
 }
// else sup->show();
}

extern char *tmppath;
extern "C" int psystem(char*);

void deinit_AFS_supports()
{
 SUP *s;
 while(sup!=NULL)
 {
  s=sup;
  sup=sup->next;
  delete s;
 }
}
