/****************************************************************************
*  Copyright (C) 1997 by Leo Khramov
*  email:   leo@xnc.dubna.su
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
#include "au_lex.h"
#include "x_actions.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
#include "key.h"
#include "internal_keys.h"

extern char home_files_dir[];
const u_int k_mask = ShiftMask | ControlMask | Mod1Mask;

static struct lexdata_s klextab[] = {
  { "MoveUp",                   AMoveUp        },
  { "MoveDown",                 AMoveDown      },
  { "MoveLeft",                 AMoveLeft      },
  { "MoveRight",                AMoveRight     },
  { "Execute",                  AExecute       },
  { "ManualSelectFile",         AInsert        },
  { "ManPage",                  AMan           },
  { "UserMenu",                 AMenu          },
  { "ViewFile",                 AFView         },
  { "EditFile",                 AFEdit         },
  { "CopyFile",                 AFCopy         },
  { "MoveFile",                 AFMove         },
  { "MakeDir",                  AMakeDir       },
  { "DelFile",                  AFDelete       },
  { "AttributeFile",            AAttributes    },
  { "Quit",                     AQuit          },
  { "KillMenu",                 AKill          },
  { "SwitchPanels",             ASwitchTab     },
  { "SwitchToTerm",             ASwitchtoTerm  },
  { "RereadDir",                ARereadDir     },
  { "InvertMaskSelection",      AInvertMask    },
  { "AddMaskSelection",         ASelectMask    },
  { "DelMaskSelection",         ADeselectMask  },
  { "CmdLineCommandUp",         ACmdUp         },
  { "CmdLineCommandDown",       ACmdDown       },
  { "CmdLineCommandRight",      ACmdRight      },
  { "CmdLineCommandLeft",       ACmdLeft       },
  { "FindFile",                 AFind          },
  { "QuickDir",                 AQuickView     },
  { "MemoryInfo",               AMemory        },
  { "Print",                    APrint         },
  { "RaiseWindow",              ARaise         },
  { "SimpleView",               AFSimpleView   },
  { "RemountList",              ARemountList   },
  { "RemountCurrent",           ARemount       },
  { "NewFileEdit",              AFNewEdit      },
  { "DiskInfo",                 ADiskInfo      },
  { "AddtoBookMark",            ABookAdd       },
  { "SwitchToBook1",            ABook1         },
  { "SwitchToBook2",            ABook2         },
  { "SwitchToBook3",            ABook3         },
  { "SwitchToBook4",            ABook4         },
  { "SwitchToBook5",            ABook5         },
  { "SwitchToBook6",            ABook6         },
  { "SwitchToBook7",            ABook7         },
  { "SwitchToBook8",            ABook8         },
  { "SwitchToBook9",            ABook9         },
  { "DelCurrentBook",           ADelBook       },
  { "ChangeOwner",              AChown         },
  { "CleanAFScache",            ACleanCache    },
  { "ActivateFtpLink",          AFtpLink       },
  { "InsertLeftDir",            AInsertLeftDir },
  { "InsertRightDir",           AInsertRightDir },
  { "SelectFileMask",           ASelectFileMask },
  { "DeselectFileMask",         ADeselectFileMask },
  { "MoveCursorHome",           AMoveHome         },
  { "MoveCursorEnd" ,           AMoveEnd          },
  { "MoveCursorPageUp",         AMovePageUp       },
  { "MoveCursorPageDown" ,      AMovePageDn       },
  { "InodeInfo" ,               AInodeInfo        },
  { "QuickFileInfo" ,           AQuickFInfo       },
  { "SwapPanels",               ASwapPanels    },
  { "SetPanelToCurrent",        ASetPanelToCurrent },
  { "CreateArchive",            ACreateArchive },
  { "BranchView",               ABranchView    },
  { "RenameFile",               ARenameFile    },
  { "ShowHiddenFiles",          AShowHiddenFiles},
  { "SetBriefMode",             ABriefMode},
  { "SetFullMode",              AFullMode},
  { "SetCustomMode",            ACustomMode},
  { "SortByName",               ASortByName},
  { "SortByExtension",          ASortByExt},
  { "SortBySize",               ASortBySize},
  { "SortByTime",               ASortByTime},
  { "SortByUnsort",             ASortByUnsort},
  { "ReverseSort",              AReverseSort},
  { "ChangeDirectory",          AChangeDir},
  { "MakeSymLink",              ASymLink},
  { ">LSF-keys",                START_LSF      },
  { ">endLSF",                  END_LSF        },
  { ":",                        ENTRY_SEP      },
  { "+",                        ENTRY_PLUS     },
  { ";",                        ENTRY_NEXT     },
  { "Alt",                      Alt            },
  { "Ctrl",                     Ctrl           },
  { "Shift",                    Shift          },
  { NULL, 0 }
};
LexDemon *keydm;
char *keysbuf;
int verbose=0;

ActionKey *acts[80];
int maxacts=0;
extern Display *disp;
int rmaxacts;
int keyerr=0;

void key_error(char *er)
{
  fprintf(stderr,"\nError: %s\n",er);
  keyerr++;
}

char* action_name_by_code(int code)
{
  for (int i=0; klextab[i].string; i++) {
    if (klextab[i].lexem == code)
      return (char*)klextab[i].string;
  }
  return NULL;
}

int init_keysupport(char *fname)
{
  int fp = open(fname,O_RDONLY);
  long fsiz;
  char c;

  if (fp == -1)
    return 0;
  
  fsiz = lseek(fp,0l,SEEK_END);
  keysbuf = new char[fsiz+1];
  lseek(fp, 0l, SEEK_SET);
  read(fp, keysbuf, fsiz);
  keysbuf[fsiz] = 0;
  close(fp);
  keydm = new LexDemon(klextab);
  keydm->set_input_stream(keysbuf);
  keydm->set_terminators(";:+");
  keydm->disable_const_detection();
  c = keydm->process_lex();
  if (c != START_LSF) {
    fprintf(stderr,"Error: not a keys support file!\n");
    delete keydm;
    delete keysbuf;
    return 0;
  }
  fprintf(stderr,"Compiling Key Support.");
  for(int i = 0; i < 80; i++)
    acts[i] = NULL;
  maxacts = 0;
  return 1;
}

int init_replace_keysupport(char *fname)
{
  int fp = open(fname,O_RDONLY);
  long fsiz;
  char c;

  if (fp == -1)
    return 0;
  
  fsiz = lseek(fp,0l,SEEK_END);
  keysbuf = new char[fsiz+1];
  lseek(fp, 0l, SEEK_SET);
  read(fp, keysbuf, fsiz);
  keysbuf[fsiz] = 0;
  close(fp);
  keydm=new LexDemon(klextab);
  keydm->set_input_stream(keysbuf);
  keydm->set_terminators(";:+");
  keydm->disable_const_detection();
  c = keydm->process_lex();
  rmaxacts=0;
  if (c != START_LSF) {
    fprintf(stderr,"Error: not a keys support file!\n");
    delete keydm;
    delete keysbuf;
    return 0;
  }
  fprintf(stderr,"Recompiling Key Support.");
  return 1;
}

int init_internal_keysupport()
{
  char c;

  keydm = new LexDemon(klextab);
  keydm->set_input_stream(internal_keysbuf);
  keydm->set_terminators(";:+");
  keydm->disable_const_detection();
  c = keydm->process_lex();
  if (c != START_LSF) {
    fprintf(stderr,"Error: not a keys support file!\n");
    delete keydm;
    return 0;
  }
  fprintf(stderr,"Compiling Internal Key Support.");
  for(int i=0; i < 80; i++)
    acts[i] = NULL;
  maxacts = 0;
  return 1;
}

int waitfor(u_char c)
{
  if(keydm->process_lex() != c) {
    key_error("Unsupported symbol!");
    return 0;
  }
  return 1;
}

int process_str()
{
  ActionKey *ak;
  int i;
  KeySym keysym;
  u_char c = keydm->process_lex();

  if (c==0 || c>=AEmpty) 
    key_error("Not an action!");
  else {
    ak = new ActionKey;
    ak->action = (int)c;
    fprintf(stderr, ".");
    c = keydm->process_lex();
    if (c!=ENTRY_SEP) {
      key_error("Missing ':'");
      return -1;
    }
    i=0;
    ak->mods[0] = ak->mods[1] = ak->mods[2] = 0;
    while (1) {
      if (i > 2)
        break;
      c = keydm->process_lex();
      if (c == END_LSF)
        break;
      if (c < AEmpty) {
        keydm->unlex(c);
        break;
      }
      switch (c) {
      case Alt: 
        ak->mods[i] |= Mod1Mask;
        waitfor(ENTRY_PLUS);
        continue;
      case Ctrl:
        ak->mods[i] |= ControlMask;
        waitfor(ENTRY_PLUS);
        continue;
      case Shift:
        ak->mods[i] |= ShiftMask;
        waitfor(ENTRY_PLUS);
        continue;
      default:
        keysym = XStringToKeysym(keydm->wlast);
        if (keysym == NoSymbol)
          fprintf(stderr, "Can't detect key '%s'\n", keydm->wlast);
        else
          ak->keys[i] = XKeysymToKeycode(disp, keysym);
        c = keydm->process_lex();
        i++;
        break;
      }
      if (c!=ENTRY_NEXT) {
        key_error("Missing ';'");
        return -1;
      }
    }
  }
  acts[maxacts++] = ak;
  if (c == END_LSF)
    return 0;
  return 1;
}

int parse_str()
{
  ActionKey *ak;
  int i;
  KeySym keysym;
  u_char c = keydm->process_lex();

  if (c==0 || c >= AEmpty) 
   key_error("Not an action!");
  else {
    ak = action_by_name(c);
    fprintf(stderr, ".");
    c = keydm->process_lex();
    if (c != ENTRY_SEP) {
      key_error("Missing ':'");
      return -1;
    }
    i = 0;
    ak->mods[0] = ak->mods[1] = ak->mods[2] = 0;
    ak->keys[0] = ak->keys[1] = ak->keys[2] = 0;
    while (1) {
      if ( i > 2)
        break;
      c = keydm->process_lex();
      if (c==END_LSF)
        break;
      if (c < AEmpty) {
        keydm->unlex(c);
        break;
      }
      switch (c) {
      case Alt:
        ak->mods[i] |= Mod1Mask;
        waitfor(ENTRY_PLUS);
        continue;
      case Ctrl:
        ak->mods[i] |= ControlMask;
        waitfor(ENTRY_PLUS);
        continue;
      case Shift:
        ak->mods[i] |= ShiftMask;
        waitfor(ENTRY_PLUS);
        continue;
      default:
        keysym = XStringToKeysym(keydm->wlast);
        if (keysym == NoSymbol)
          fprintf(stderr, "Can't detect key '%s'\n", keydm->wlast);
        else
          ak->keys[i] = XKeysymToKeycode(disp, keysym);
        c = keydm->process_lex();
        i++;
        break;
      }
      if (c != ENTRY_NEXT) {
        key_error("Missing ';'");
        return -1;
      }
    }
    rmaxacts++;
  }
  if (c == END_LSF)
    return 0;
  return 1;
}

int is_it_k(u_int key, u_int mod, u_int key2, u_int mod2)
{
  return key == key2 && (mod & k_mask) == mod2;
}

int look_for_key(XEvent* ev)
{
  u_int mod=ev->xkey.state;
  u_int key=ev->xkey.keycode;

  if(ev->type!=KeyPress)
    return AEmpty;

  for( int i=0; i < maxacts; i++)
    if(is_it_k(key,mod,acts[i]->keys[0], acts[i]->mods[0])
       || is_it_k(key,mod,acts[i]->keys[1], acts[i]->mods[1])
       || is_it_k(key,mod,acts[i]->keys[2], acts[i]->mods[2]))
      return acts[i]->action;
  return AEmpty;
}

ActionKey* action_by_name(int n)
{
  for(int i=0; i < maxacts; i++)
    if (acts[i]->action == n)
      return acts[i];
  return NULL;
}


int action_key_by_id(char* to, int id, int idx)
{
    ActionKey* act=action_by_name(id);
    if(!act)
	return 0;
    *to=0;
    if(act->mods[idx] & ShiftMask)
    {
	if(*to!=0)
	    strcat(to, "+");
	strcat(to, "Shift");
    }
    if(act->mods[idx] & ControlMask)
    {
	if(*to!=0)
	    strcat(to, "+");
	strcat(to, "Ctrl");
    }
    if(act->mods[idx] & Mod1Mask)
    {
	if(*to!=0)
	    strcat(to, "+");
	strcat(to, "Alt");
    }

    if(*to!=0)
	strcat(to, "+");
    strcat(to, XKeysymToString(XKeycodeToKeysym(disp, act->keys[idx], 0)));
    return 1;
}

 
void deinit_keysupport()
{
  delete keydm;
  delete keysbuf;
  for(int i=0; i < maxacts; i++)
    delete acts[i];
}

void compile_key_support()
{
  int r;
  char tmp[L_MAXPATH];
  keyerr=0;

  strcpy(tmp, home_files_dir);
  strcat(tmp, "/xnc.keys");
  if (init_keysupport(tmp)) {
    while ((r=process_str()) == 1) ;
    if (r==0)
      fprintf(stderr, "OK\n");
    else
      fprintf(stderr, "Compilation terminated!\n");
    fprintf(stderr,"     (0) warnings, (%d) errors\n", keyerr);
    fprintf(stderr,"Total actions defined: %d\n", maxacts);
  } else {
    fprintf(stderr,"!!!!!!!!!!XNC PANIC!!!!!!!!!: Key Support not loaded\n");
    keyerr=0;
    init_internal_keysupport();
    while ((r=process_str()) == 1);
    if (r == 0)
      fprintf(stderr,"OK\n");
    else
      fprintf(stderr,"Compilation terminated!\n");
    fprintf(stderr,"     (0) warnings, (%d) errors\n", keyerr);
    fprintf(stderr, "Total actions defined: %d\n", maxacts);
  }
}

void compile_internal_key_support()
{
  int r;                  

  keyerr=0;
  init_internal_keysupport();
  while ((r=process_str()) == 1 );
  if (r == 0)
    fprintf(stderr, "OK\n");
  else
    fprintf(stderr, "Compilation terminated!\n");
  fprintf(stderr,"     (0) warnings, (%d) errors\n", keyerr);
  fprintf(stderr, "Total actions defined: %d\n", maxacts);
}

void parse_file_in_replace_mode(char *name)
{
  char tmp[L_MAXPATH];
  int r;

  keyerr=0;
  strcpy(tmp,home_files_dir);
  strcat(tmp,name);
  if (init_replace_keysupport(tmp)) {
    while((r=parse_str()) == 1) ;
    if (r == 0)
      fprintf(stderr, "OK\n");
    else
      fprintf(stderr, "Compilation terminated!\n");
    fprintf(stderr,"     (0) warnings, (%d) errors\n", keyerr);
    fprintf(stderr, "Total actions redefined: %d\n", rmaxacts);
 }
}

