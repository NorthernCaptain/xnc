#ifndef GUIPLUGIN_H
#define GUIPLUGIN_H
#include "baseguiplugin.h"
#include "xh.h"
#include "gui.h"
#include "bookmark.h"
#include "panel.h"
#include "ftpvisuals.h"
#include "pager.h"
#include "infowin.h"
#include "exwin.h"


//This class is some kind of wrapper or interface between main program and plugin
class GuiPlugin:public BaseGuiPlugin
{
 public:
  GuiPlugin(char* pl_name, char* pl_ver):BaseGuiPlugin(pl_name,pl_ver) {};
  virtual InfoWin*   new_InfoWin(int ix,int iy,int il,int ih,char *iname,int icol,int ikey=0);
  virtual EXWin*     new_EXWin(int ix,int iy,int il,int ih,char *iname,void (*ifunc)(Window,GC&),int hfl=0);

  virtual BookMark*  new_BookMark(int il,int ih);
  virtual Lister*    new_Lister(int ix,int iy,int il,int ih,int icolumns=2);
  virtual Cmdline*   new_Cmdline(int ix,int iy,int il,int icol);
  virtual FtpVisual* new_FtpVisual(int ix,int iy,int il,int ih);
  virtual Switcher*  new_Switcher(int ix,int iy,int il,int ih);
  virtual unsigned long get_exwin_foreground() { return cols[1]; };

  virtual const char*  get_ini_filename();
};

#endif
/* ------------ End of file -------------- */

