#ifndef BASEGUIPLUG_H
#define BASEGUIPLUG_H

#include "plugin.h"
#include "xh.h"
#include "gui.h"
#include "pager.h"
#include "scroll.h"

//Base plugin that can be used in any other program not only in xnc
class BaseGuiPlugin:public PluginObj
{
 public:
  BaseGuiPlugin(char *pl_name,char* pl_ver):PluginObj(GUI_Plugin,pl_name,pl_ver) {};
  virtual KEY*       new_KEY(int ix,int iy,int il,int ih,char *iname,ulong icol,void (*ifunc)());
  virtual BKey*      new_BKey(int ix,int iy,int il,int ih,Gui*,int);
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
  virtual ScrollBar* new_ScrollBar(int ix,int iy,int ih,Gui*);
};

extern BaseGuiPlugin *baseguiplugin;
extern void init_basegui_plugin();

#endif
/* ------------ End of file -------------- */

