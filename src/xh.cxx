/****************************************************************************
*  Copyright (C) 1996-97 by Leo Khramov
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

#include "panel.h"
#include "xnc.icon"
#include "xncm.icon"
#include "bookmark.h"
#include "xncversion.h"
#include "tartest.h"
#include "geomfuncs.h"
#include "image.h"
#include "xnimlib.h"
#include "flist.h"
#include "pluginman.h"
#include "commonfuncs.h"
#include "c_externs.h"
#include "config_file.h"

Window     base_win;   //Base window of the whole xnc application.

/*
base_win (base window of the whole application)
 |
 \--- Main (main window of xnc filemanager part)
 |      |
 |      \- Panel wins
 |      |
 |      \- Bookmark win
 |      |
 |      \- Cmdline win
 |      |
 |      \- Menu bar wins
 |
 \--- main_win (main window of rxvt terminal part)
        |
        \-vt_win
        |
        \-scrollbar_win
*/


extern int detect_tar_output(char *path);

int create_file(char *path, char *fname, char *buf, int siz)
{
  char str[L_MAXPATH];
  int fp;
  xncdprintf(("create_file(%x,%x,%x,%d) called\n",path,fname,buf,siz));
  xncdprintf(("create_file: path=[%s], fname=[%s], size=%d\n",path,fname,siz));
  sprintf(str,"%s/%s",path,fname);
  fp=open(str,O_WRONLY | O_CREAT | O_TRUNC,0600);
  if(fp==-1)
  {
    xncdprintf(("create_file: creating failed for [%s]\n",str));
    xncdprintf(("create_file: reason is %s",strerror(errno)));
    return 0;
  }
  xncdprintf(("create_file: writing data\n"));
  write(fp,buf,siz);
  close(fp);
  xncdprintf(("create_file: [%s] created ok\n",str));
  return 1;
}

int is_new_version()
{
  char tmp[1024];
  FILE *fp;
  if(getenv("HOME")==0)
    return 1;
  sprintf(tmp, "%s/.xnc/current-"XNCVERSION, getenv("HOME"));
  fp=fopen(tmp, "r");
  if(fp)
    {
      fclose(fp);
      return 0;
    }
  fp=fopen(tmp, "w");
  if(fp)
    {
      fputs(XNCVERSION, fp);
      fclose(fp);
    }
  return 1;
}

int auto_create_startup_files(char *path)
{
        int oldtar=detect_tar_output(path);
        if(create_file(path,"xnc.ini",xncini_chr,xncini_chr_size)==0) return 0;
        switch(oldtar)
        {
                case GNUTAR_1_11:
                        if(create_file(path,"xnc.afs",xncafs_chr,xncafs_chr_size)==0) return 0;
                        break;
                case GNUTAR_1_12:
                        if(create_file(path,"xnc.afs",xncafstar_chr,xncafstar_chr_size)==0) return 0;
                        break;
                case OPENTAR_1_11:
                        if(create_file(path,"xnc.afs",xncafsopenbsd_chr,xncafsopenbsd_chr_size)==0) return 0;
                        break;
                case AIXTAR_1:
                        if(create_file(path,"xnc.afs",xncafsaix_chr,xncafsaix_chr_size)==0) return 0;
                        break;
                default:
                        fprintf(stderr, "Detected unknown [%d] tar output! Use default...\n",oldtar);
                        if(create_file(path,"xnc.afs",xncafs_chr,xncafs_chr_size)==0) return 0;
                        break;
        }
        if(create_file(path,"xnc.ext",xncext_chr,xncext_chr_size)==0) return 0;
        if(create_file(path,"xnc.cfg",xnccfg_chr,xnccfg_chr_size)==0) return 0;
        if(create_file(path,"xnc.menu",xncmenu_chr,xncmenu_chr_size)==0) return 0;
        if(create_file(path,"xnc.keys",xnckeys_chr,xnckeys_chr_size)==0) return 0;
        return 1;
}

void   make_home_file_dir()
{
  char   str[L_MAXPATH];
  int    fp;
  if(getenv("HOME")==NULL)
  {
          perror("FATAL ERROR: can't resolve 'HOME' environment");
          exit(-1);
  }
  sprintf(str, "%s/.xnc/xnc.ini", getenv("HOME"));
  fp = open(str, O_RDONLY);
  if (fp == -1)
    {
      sprintf(str,"%s/.xnc",getenv("HOME"));
      mkdir(str,0700);
      if(auto_create_startup_files(str))
      {
          strcpy(home_files_dir, getenv("HOME"));
          strcat(home_files_dir, "/.xnc");
      } else
      {
      strcpy(home_files_dir, XNC_DIR);
      strcat(home_files_dir, "/xnc");
      }

      fprintf(stderr,"\n-------------------------------------------------------------\n");    
      fprintf(stderr, "WARNING: User configuration files not found - using system ones...\n");
      fprintf(stderr, "Please, run **' xncsetup '** for configuring your own settings for XNC.....\n");
      fprintf(stderr,"-------------------------------------------------------------\n\n");    
      sleep(2);
      sprintf(str, "%s/xnc.ini", home_files_dir);
      fp = open(str, O_RDONLY);
      if (fp == -1)
        fprintf(stderr, "\nWARNING: System default configuration files not found ...\n");
      else
        close(fp);
      return;
    }
  else
    close(fp);
  strcpy(home_files_dir, getenv("HOME"));
  strcat(home_files_dir, "/.xnc");
}



extern char build_chr[];
char   xncvt[100];
char   geom[100];
extern char host[];
extern int afs_disabled;
extern int ftp_cache_disabled;

int    margc;
int    centerx, centery;
char **margv;
int    ScreenL, ScreenH;
int    allow_t_raising = 1;

char **ext = NULL;
char **extcom;
char **menunames;
char **menucom;
KeySym *mkeys = NULL;
int    extmax = 0, menumax = 0;
int    term_sticky = 1;
int    hide_hidden = 0;

int    findmenukey(KeySym ks)
{
  for (int i = 0; i < menumax; i++)
    if (mkeys[i] == ks)
      return i;
  return -1;
}

int    fstring(FILE * fp, char *str)
{
  char  *s = str;
  while (!feof(fp))
    {
      *str = (char)fgetc(fp);
      if (*str == '\n')
        {
          *str = 0;
          return 1;
        };
      str++;
    };
  *str = 0;
  if (*s)
    return 1;
  return 0;
}

char  *strscan(char *src, char *dest)
{
  while (*src != 0 && *src != ':')
    *dest++ = *src++;
  *dest = 0;
  return ++src;
}

char  *strscan2(char *src, char *dest)
{
  while (*src == ' ')
    src++;
  while (*src != 0 && *src != ' ' && *src != 0xa)
    *dest++ = *src++;
  *dest = 0;
  return ++src;
}

void   del_ext()
{
  int    i;
  if (ext != NULL)
    {
      for (i = 0; i < extmax; i++)
        {
          delete ext[i];
          delete extcom[i];
        }
      delete ext;
      delete extcom;
      ext = NULL;
    }
}

void   del_menu()
{
  int    i;
  if (mkeys != NULL)
    {
      for (i = 0; i < menumax; i++)
        {
          if (menunames[i])
            delete menunames[i];
          if (menucom[i])
            delete menucom[i];
        }
      delete menunames;
      delete menucom;
      delete mkeys;
      mkeys = NULL;
    }
}

int    ext_find(char *name)
{
  int    l = strlen(name), i, nl, el, fl;
  for (i = 0; i < extmax; i++)
    {
      el = strlen(ext[i]);
      nl = l;
      fl = 1;
      while (el >= 0)
        if (name[nl] != ext[i][el])
          {
            fl = 0;
            break;
          }
        else
          {
            el--;
            nl--;
          };
      if (fl)
        return i;
    }
  return -1;
}

int    ext_find2(char *name, int n)
{
  int    l = strlen(name), i, nl, el, fl, k = 0;
  for (i = 0; i < extmax; i++)
    {
      el = strlen(ext[i]);
      nl = l;
      fl = 1;
      while (el >= 0)
        if (name[nl] != ext[i][el])
          {
            fl = 0;
            break;
          }
        else
          {
            el--;
            nl--;
          };
      if (fl)
        {
          if (k == n)
            return i;
          else
            k++;
        };
    }
  return -1;
}

void   load_ext()
{
  FILE  *fp;
  int    i;
  char   str[120];
  char   str1[80];
  char  *chr;
  int    show_dots = 1;
  if (ext != NULL)
    show_dots = 0;
  del_ext();
  strcpy(rfile, home_files_dir);
  strcat(rfile, "/xnc.ext");
  if ((fp = fopen(rfile, "r")) != NULL)
    {
      i = 0;
      while (fstring(fp, str))
        if (str[0] == '.')
          i++;
      fseek(fp, 0l, SEEK_SET);
      ext = new char *[i];
      extcom = new char *[i];
      i = 0;
      while (fstring(fp, str))
        {
          chr = strscan(str, str1);
          if (str1[0] == '.')
            {
              ext[i] = new char[strlen(str1) + 1];
              strcpy(ext[i], str1);
              strscan(chr, str1);
              extcom[i] = new char[strlen(str1) + 1];
              strcpy(extcom[i], str1);
              i++;
              if (show_dots)
                fprintf(stderr, ".");
            }
        }
      extmax = i;
      fclose(fp);
    }
  else
    fprintf(stderr, "XNC Error: No extension file found...");
}

void   load_menu()
{
  FILE  *fp;
  int    i, j;
  char   str[120];
  char   str1[120];
  char  *chr;
  int    show_dots = 1;
  if (mkeys != NULL)
    show_dots = 0;
  del_menu();
  strcpy(rfile, home_files_dir);
  strcat(rfile, "/xnc.menu");
  if ((fp = fopen(rfile, "r")) != NULL)
    {
      i = 0;
      while (fstring(fp, str))
        if (str[0] != '#' && str[0] != ' ' && str[0] != 0xa && str[0] != 0)
          i++;
      fseek(fp, 0l, SEEK_SET);
      mkeys = new KeySym[i];
      menucom = new char *[i];
      menunames = new char *[i];
      menumax = i;
      for (j = 0; i < j; j++)
        {
          menucom[i] = NULL;
          menunames[i] = NULL;
        }
      i = 0;
      while (fstring(fp, str))
        {
          chr = strscan(str, str1);
          if (str1[0] != '#')
            {
              mkeys[i] = XStringToKeysym(str1);
              chr = strscan(chr, str1);
              *(chr - 1) = 0;
              menunames[i] = new char[strlen(str) + 1];
              strcpy(menunames[i], str);
              strscan(chr, str1);
              menucom[i] = new char[strlen(str1) + 1];
              strcpy(menucom[i], str1);
              i++;
              if (show_dots)
                fprintf(stderr, ".");
            }
        }
      fclose(fp);
    }
  else
    fprintf(stderr, "XNC Error: No menu file found...");
}


//////////////////////////////////////////////////////////////////////////

char  bgchr[50] = "white", 
      fgchr[50] = "black", 
      fontchr[100] = "fixed", 
      lfontchr[100] = "fixed",
      vfontchr[100] = "fixed", 
      fixfontchr[100] = "fixed", 
      mfixfontchr[100] = "fixed",
      kbgchr[50] = "#9cb0c4", 
      *dispchr = NULL, 
      selectchr[50] = "#4000f0", 
      editname[100] = "jed",
      viewname[100] = "less", 
      syspath[2048] = ".", 
      *traise = "yes", 
      *tsticky = "yes",
      geometrychr[100] = "0x1000+0+0", 
      viewgeom[100] = "400x400+20+40", 
      *banim = "yes",
      bshow[50] = "yes", 
      *diriconchr = "yes", 
      *fileiconchr = "yes", 
      updatechr[50] = "never";

char   rfile[L_MAXPATH];
int    allow_animation = 1;
int    allow_bookmark = 1;
int    allow_dir_icon = 1;
extern int dont_update;
int    allow_file_icon = 1;

char bgchr1[50];
char bgchr2[50];
char bgchr3[50];
char bgchr4[50];
char bgchr5[50];
char bgchr6[50];
char bgchr7[50];
char bgchr8[50];
char bgchr9[50];
char bgchr10[50];
char bgchr11[50];

Lister *panel = NULL;

X_PAR  resources[] =
{
  "leoprogs.font", "LeoProgs.Font", fontchr,
  "leoprogs.list.font", "LeoProgs.List.Font", lfontchr,
  "leoprogs.font.fixed", "LeoProgs.Font.Fixed", fixfontchr,
  "xnc.editor.name", "Xnc.Editor.Name", editname,
  "xnc.sysfiles.path", "Xnc.SysFiles.Path", syspath,
  "xnc.geometry", "Xnc.Geometry", geometrychr,
  "leoprogs.viewer.font", "LeoProgs.Viewer.Font", vfontchr,
  "xnc.viewer.name", "Xnc.Viewer.Name", viewname,
  "xnc.viewer.geometry", "Xnc.Viewer.Geometry", viewgeom,
  "xnc.bookmark.show_and_use", "Xnc.BookMark.Show_and_Use", bshow,
  "leoprogs.font.minifixed", "LeoProgs.Font.MiniFixed", mfixfontchr,
  "xnc.afs.update", "Xnc.AFS.Update", updatechr
};

const int resn = 12; //sizeof(resources) / sizeof(X_PAR);

XFontStruct *fontstr;
XFontStruct *lfontstr;
XFontStruct *vfontstr;
XFontStruct *fixfontstr;
XFontStruct *mfixfontstr;
unsigned long bgpix, fgpix, selectpix;

XWMHints *pxwmh;
XSizeHints *pxsh;
XClassHint *pch;
XTextProperty wname;
XTextProperty iname;
Display *disp;
Window Main;
Window Key;
GC     gc;
XGCValues gcv;
XEvent ev;
XrmDatabase rdb;
XrmValue rmv;
XColor color;
Colormap defcmp;
int    done = 0;
int    scr;
int    Mainl, Mainh, Mainx, Mainy;

int    viewx, viewy;
uint   viewl, viewh;

Pixmap picon;
Pixmap piconmask;
char  *icname = "XNC";
int    layout = 1;
char   winame[L_MAXPATH];
extern Display *Xdisplay;
extern char l_bg_string[];
extern const char* display_name;
extern Window main_win;

ConfigManager *iniman;

void   InitXClient(int argc, char **argv, int wx, int wy, int wl, int wh, char
                   *winname)
{
  struct utsname uts;
  int    i, ppx, ppy;
  XWindowAttributes xwinattr;
  XrmDatabase plugin_rdb;
  uint   ppl, pph;
  char  *rmt;
  char   tmppath[256];
  char   plugin_rfile[1024];
  XSetWindowAttributes xswa;

  xncdprintf(("XClient..........OK [Display '%s']\n", display_name));
/*
   for(i=1;i<argc;i++)
   if(strcmp("-display",argv[i])==0 || strcmp("-d",argv[i])==0) dispchr=argv[i+1];
   if((disp=XOpenDisplay(dispchr))==NULL)
   {
   fprintf(stderr,"%s: connection to X server failed on display %s.....\n",argv[0],dispchr);
   exit(1);
   } */
  disp = Xdisplay;
  uname(&uts);
  strncpy(host, uts.nodename, 80);
  sprintf(winame, "%s  launched from host '%s'", winname, host);
  sprintf(xncvt, "VT100 for %s  Host:%s", ver, host);
  strcat(host, ":");
  winname = winame;
  scr = DefaultScreen(disp);
  XrmInitialize();
  fprintf(stderr, "Loading resources....");
  strcpy(rfile, home_files_dir);
  strcat(rfile, "/xnc.ini");
  xncdprintf(("Main resource file [%s], plugin handle - %x\n",rfile,guiplugin));
  strcpy(plugin_rfile, home_files_dir);
  strcat(plugin_rfile, guiplugin->get_ini_filename());
  xncdprintf(("Plugin resource file [%s]\n",plugin_rfile));
  plugin_rdb = XrmGetFileDatabase(plugin_rfile);
  rdb = XrmGetFileDatabase(rfile);

  iniman=new ConfigManager(rfile);
  iniman->merge_file(plugin_rfile);
  
  xncdprintf(("Plugin_db [%x], Main_db [%x]\n",plugin_rdb,rdb));

  if(rdb && plugin_rdb)
  {
    xncdprintf(("Merging resources\n"));
    XrmMergeDatabases(plugin_rdb,&rdb);
  }

  if (rdb != NULL)
  {
    xncdprintf(("Reading resources\n"));
    for (i = 0; i < resn; i++)
      {
        fprintf(stderr, ".");
	xncdprintf(("GettingResource: [%s][%s]\n",resources[i].name, resources[i].cname));
        if (XrmGetResource(rdb, resources[i].name, resources[i].cname, &rmt, &rmv))
          strcpy(resources[i].pstr,rmv.addr);
        else
          fprintf(stderr, "%s: Error getting resource: %s\n", argv[0], 
		  resources[i].name);
      }
  }
  else
    fprintf(stderr, "!!!XNC PANIC!!!: Can't open resource file [%s].\n", rfile);
  XParseGeometry(geometrychr, &ppx, &ppy, &ppl, &pph);
  if (ppl > 0)
    {
      wx = ppx;
      wy = ppy;
      wl = ppl;
      wh = pph;
      if (wl < 640)
        wl = 640;
      if (wh < 300)
        wh = 300;
    }
  XParseGeometry(viewgeom, &viewx, &viewy, &viewl, &viewh);
  defcmp = DefaultColormap(disp, scr);

  xncdprintf(("Loading color schema through ConfigManager\n"));
  load_color_schema(iniman);

  if (XrmGetResource(rdb, "xnc.panels.layout", "Xnc.Panels.Layout", &rmt, &rmv))
    {
      if (strcmp(rmv.addr, "horizontal") == 0)
        layout = 0;
      else if (strcmp(rmv.addr, "vertical") == 0)
        layout = 1;
      else
        layout = 2;
    }
  else
    fprintf(stderr, "!!!XNC Panic!!!: Error getting resource: Xnc.Panels.Layout\n");

  if ((fontstr = XLoadQueryFont(disp, resources[0].pstr)) == NULL)
    {
      fprintf(stderr, "!!!XNC Panic!!!: Error loading font %s\n", fontchr);
      exit(1);
    }

  if ((lfontstr = XLoadQueryFont(disp, resources[1].pstr)) == NULL)
    {
      fprintf(stderr, "!!!XNC Panic!!!: Error loading font %s\n", lfontchr);
      exit(1);
    }

  if ((fixfontstr = XLoadQueryFont(disp, resources[2].pstr)) == NULL)
    {
      fprintf(stderr, "!!!XNC Panic!!!: Error loading font %s\n", fixfontchr);
      exit(1);
    }
  if ((vfontstr = XLoadQueryFont(disp, resources[6].pstr)) == NULL)
    {
      fprintf(stderr, "!!!XNC Panic!!!: Error loading font %s\n", vfontchr);
      exit(1);
    }
  if ((mfixfontstr = XLoadQueryFont(disp, mfixfontchr)) == NULL)
    {
      fprintf(stderr, "!!!XNC Panic!!!: Error loading font %s\n", mfixfontchr);
      exit(1);
    }


  fprintf(stderr, "OK\n");

  if ((pxsh = XAllocSizeHints()) == NULL)
    {
      fprintf(stderr, "%s: Error allocating size hints\n", argv[0]);
      exit(1);
    }

  if (strcmp(bshow, "no") == 0)
    {
      allow_bookmark = 0;
      allow_animation = 0;
    };
  if (strcmp(updatechr, "always") == 0)
    dont_update = 1;
  else if (strcmp(updatechr, "prompt") == 0)
    dont_update = 0;
  else
    dont_update = 2;
  pxsh->flags = (PPosition | PSize | PMinSize);                // | PMaxSize);

  pxsh->height = wh;
  pxsh->width = wl;
  pxsh->min_width = 450;
  pxsh->min_height = 350;
/* pxsh->max_width=pxsh->width;
   pxsh->max_height=pxsh->height; */
  pxsh->x = wx;
  pxsh->y = wy;
  Mainl = wl;
  Mainh = wh;
  sprintf(geom, "%dx%d+%d+%d", pxsh->width, pxsh->height, pxsh->x, pxsh->y);
  i = XGeometry(disp, scr, geom, geom, BDWIDTH, fontstr->max_bounds.width,
                fontstr->max_bounds.ascent + fontstr->max_bounds.descent,
             1, 1, &(pxsh->x), &(pxsh->y), &(pxsh->width), &(pxsh->height));
  if (i & (XValue | YValue))
    pxsh->flags |= USPosition;
  if (i & (WidthValue | HeightValue))
    pxsh->flags |= USSize;
  Mainx = pxsh->x;
  Mainy = pxsh->y;
  base_win = Main = main_win = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
						   pxsh->x, pxsh->y, pxsh->width, pxsh->height,
						   BDWIDTH, fgpix, keyscol[1]);
  centerx = pxsh->width / 2;
  centery = pxsh->height / 2;
  if ((pch = XAllocClassHint()) == NULL)
    {
      fprintf(stderr, "%s: Error allocating ClassHint\n", argv[0]);
      exit(1);
    }
  pch->res_name = "xnc";
  pch->res_class = "XNC";
  if (winname == NULL)
    winname = argv[0];
  if (XStringListToTextProperty(&winname, 1, &wname) == 0)
    {
      fprintf(stderr, "%s: Error creating TextProperty\n", argv[0]);
      exit(1);
    }
  if (XStringListToTextProperty(&icname, 1, &iname) == 0)
    {
      fprintf(stderr, "%s: Error creating TextProperty\n", argv[0]);
      exit(1);
    }
  if ((pxwmh = XAllocWMHints()) == NULL)
    {
      fprintf(stderr, "%s: Error allocating WMHints\n", argv[0]);
      exit(0);
    }
  picon = XCreateBitmapFromData(disp, Main, (char *)xnc_bits, xnc_width, xnc_height);
  piconmask = XCreateBitmapFromData(disp, DefaultRootWindow(disp), (char *)xncm_bits, xnc_width, xnc_height);
  pxwmh->flags = (InputHint | StateHint | IconMaskHint | IconPixmapHint);
  pxwmh->input = True;
  pxwmh->initial_state = NormalState;
  pxwmh->icon_pixmap = picon;
  pxwmh->icon_mask = piconmask;
  XSetWMProperties(disp, Main, &wname, &iname, argv, argc, pxsh, pxwmh, pch);

  gcv.font = fontstr->fid;
  gcv.foreground = fgpix;
  gcv.background = bgpix;
  gc = XCreateGC(disp, Main, (GCFont | GCForeground | GCBackground), &gcv);
  xswa.colormap = defcmp;
  xswa.bit_gravity = CenterGravity;
  XChangeWindowAttributes(disp, Main, (CWColormap | CWBitGravity), &xswa);
// XSelectInput(disp,Main,ExposureMask | StructureNotifyMask);
  XSelectInput(disp, Main, FocusChangeMask | StructureNotifyMask);
  XGetWindowAttributes(disp, DefaultRootWindow(disp), &xwinattr);
  ScreenL = xwinattr.width;
  ScreenH = xwinattr.height;
  if((strcmp(syspath, ".") == 0) || strcmp(syspath, "auto")==0)
      sprintf(syspath,"%s/.xnc", getenv("HOME"));
  sprintf(tmppath, "%s/AFS", syspath);
  if (mkdir(tmppath, 0700) == -1 && errno != EEXIST)
    {
      fprintf(stderr, "!!!XNC Panic!!!!: Can't access tmp directory '%s'!\n", tmppath);
      fprintf(stderr, "Archive file system will be disabled!!!\n");
      afs_disabled = 1;
    }
  sprintf(tmppath, "%s/FTPFS", syspath);
  if (mkdir(tmppath, 0700) == -1 && errno != EEXIST)
    {
      fprintf(stderr, "!!!XNC Warning!!!!: Can't access FTP tmp directory '%s'!\n", tmppath);
      fprintf(stderr, "FTP cache will be disabled!!!\n");
      ftp_cache_disabled= 1;
    }
  Main = XCreateSimpleWindow(disp, base_win,
			     0, 0, Mainl, Mainh,
			     0, fgpix, keyscol[1]);
  XSelectInput(disp, Main, StructureNotifyMask
	       	       	       | FocusChangeMask
	       );
}

void   DeinitXClient()
{
// XFreePixmap(disp,picon);

// XFreePixmap(disp,piconmask);
  delall_el();
// delall_exp();
  XFreeGC(disp, gc);
  XUnloadFont(disp, fontstr->fid);
  XUnloadFont(disp, lfontstr->fid);
  XDestroyWindow(disp, Main);
// XCloseDisplay(disp);

// exit(0);
}


Window create_win(char *winname, int wx, int wy, int wl, int wh, int mask, int inp)
{
  int    i;
  Window W;
  XSetWindowAttributes xswa;
  pxsh->flags = (PPosition | PSize | PMinSize | PMaxSize);
  pxsh->height = wh;
  pxsh->width = wl;
  pxsh->min_width = pxsh->width;
  pxsh->min_height = pxsh->height;
  pxsh->max_width = pxsh->width;
  pxsh->max_height = pxsh->height;
  pxsh->x = wx;
  pxsh->y = wy;
  sprintf(geom, "%dx%d+%d+%d", pxsh->width, pxsh->height, pxsh->x, pxsh->y);
  i = XGeometry(disp, scr, geom, geom, BDWIDTH, fontstr->max_bounds.width,
                fontstr->max_bounds.ascent + fontstr->max_bounds.descent,
             1, 1, &(pxsh->x), &(pxsh->y), &(pxsh->width), &(pxsh->height));
  if (i & (XValue | YValue))
    pxsh->flags |= USPosition;
  if (i & (WidthValue | HeightValue))
    pxsh->flags |= USSize;
  W = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
                          pxsh->x, pxsh->y, pxsh->width, pxsh->height,
                          BDWIDTH, fgpix, keyscol[1]);
  pch->res_name = "leoprogs";
  pch->res_class = "LeoProgs";
  if (XStringListToTextProperty(&winname, 1, &wname) == 0)
    {
      fprintf(stderr, "Error creating TextProperty\n");
      exit(1);
    }
  pxwmh->flags = (InputHint | StateHint);
  if (inp)
    pxwmh->input = True;
  else
    pxwmh->input = False;
  pxwmh->initial_state = NormalState;
  XSetWMProperties(disp, W, &wname, &iname, margv, margc, pxsh, pxwmh, pch);
  xswa.colormap = defcmp;
  xswa.bit_gravity = CenterGravity;
  XChangeWindowAttributes(disp, W, (CWColormap | CWBitGravity), &xswa);
  XSelectInput(disp, W, mask);

  return W;
}







