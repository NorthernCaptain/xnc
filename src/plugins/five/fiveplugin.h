#ifndef FIVEPLUGIN_H
#define FIVEPLUGIN_H
#include "guiplugin.h"

#define PLUGIN_VER  "1.1.2"

//This class is some kind of wrapper or interface between main program and plugin
class FivePlugin:public GuiPlugin
{
 public:
  FivePlugin(char *pl_name,char* pl_ver):GuiPlugin(pl_name,pl_ver) {};
  virtual Panel*     new_Panel(int ix,int iy,int il,int ih,char **iname,int imax,ulong icol,int (*ifunc)(int,char*));
  virtual Switch*    new_Switch(int ix,int iy,int il,char *iname,ulong icol,void (*ifunc)()=NULL);
  virtual Sw_panel*  new_Sw_panel(int ix,int iy,int il,char *ihead,MenuItem* mn,int imax, int icolumn=1);
  virtual Win*       new_Win(int ix,int iy,int il,int ih,char *iname,int icol,int hfl=0);
  virtual Separator* new_Separator(int ix,int iy,int il,Gui *ww,int ivert=NO);
  virtual Input*     new_Input(int ix,int iy,int il,int icol,void (*ifunc)()=NULL);
  virtual Text*      new_Text(int ix,int iy,char *iname,int icol);
  virtual MenuBar*   new_MenuBar(int ix,int iy,MenuItem *ii,int maxi);
  virtual Menu*      new_Menu(MenuItem *ii,int maxi);
  virtual Pager*     new_Pager(int ix,int iy,int il,int ih,int imax);
  virtual InfoWin*   new_InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey=0);
  virtual KEY*       new_KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)());
  virtual BookMark*  new_BookMark(int il,int ih);
  virtual Lister*    new_Lister(int ix,int iy,int il,int ih,int icolumns=2);
  virtual Cmdline*   new_Cmdline(int ix,int iy,int il,int icol);
  virtual FtpVisual* new_FtpVisual(int ix,int iy,int il,int ih);
  virtual Switcher*  new_Switcher(int ix,int iy,int il,int ih);
  virtual EXWin*     new_EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl=0);

  virtual int        post_install();
  virtual const char*  get_ini_filename();

};

#endif
/* ------------ End of file -------------- */

