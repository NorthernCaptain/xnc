#ifndef AUSUP
#define AUSUP
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "au_lex.h"

#define SUP_COMLEN        256

class SUP
{
private:
  int scancoun;
  int new_line;
  static char scanstr[1002];
  char sup_tmp[256];

public:
  char name[10];
  char *ext[10];
  int maxext;
  int headln;
  int total;
  int dname, dsize, dattr, ddate, dtime, dmonth, dday, dyear;
  int name_cr;
  char sep[50];
  // arc[0] - place number for archieve name
  // arc[1] - place number for file name
  char list[SUP_COMLEN];
  char larc[2];
  char unpack[SUP_COMLEN];
  char uarc[2];
  char sunpack[SUP_COMLEN];
  char sarc[2];
  char sdelpack[SUP_COMLEN];
  char darc[2];
  char spack[SUP_COMLEN];
  char parc[2];
  char sdpack[SUP_COMLEN];
  char pdarc[2];
  char lunpack[SUP_COMLEN];
  char luarc[2];
  char lpack[SUP_COMLEN];
  char lparc[2];
  char create_com[SUP_COMLEN];
  char create_arc[2];
  char create_many_com[SUP_COMLEN];
  char create_many_arc[2];

  SUP *next;
  SUP() {
    next = NULL;
    name[0] = 0;
    maxext = headln = 0;
    dname = dsize = dattr = ddate = dtime = dmonth = dday = dyear = -1;
    name_cr = 0;
    sep[0] = 0;
    sdelpack[0]
      = sunpack[0]
      = unpack[0]
      = spack[0]
      = sdpack[0]
      = lunpack[0]
      = lpack[0]
      = create_com[0]
      = create_many_com[0]
      = 0;
    uarc[0] = uarc[1]
      = sarc[0] = sarc[1]
      = larc[0] = larc[1]
      = darc[0] = darc[1]
      = parc[0] = parc[1]
      = pdarc[0] = pdarc[1]
      = luarc[0] = luarc[1]
      = lparc[0] = lparc[1]
      = create_arc[0] = create_arc[1]
      = create_many_arc[0] = create_many_arc[1]
      = (char)-1;
    scancoun = 0;
    new_line = 1;
  };
  ~SUP() {
    for (int i=0; i < maxext; i++) 
      delete ext[i];
  };
  int    is_yours(char *ar);
  void skip_header(FILE *fp);
  char* getword(FILE *fp, int& co);
  void addext(char *);
  void addseparator(char *);
  void show();
  void flush_line();
  char *single_extract_command(char *dest, const char *archive, const char *file);
  char *single_listextract_command(char *dest, const char *archive, const char *file);
  char *single_pack_file_command(char *dest, const char *archive, const char *file);
  char *single_pack_dir_command(char *dest, const char *archive, const char *file);
  char *single_listpack_command(char *dest, const char *archive, const char *file);
  char *single_delete_command(char *dest, const char *archive, const char *file);
  char *single_create_command(char *dest, const char *archive, const char *file);
  char *single_create_many(char *dest, const char *archive, const char *file);
  int   is_create_supported() { return create_com[0]!=0; };
  int   is_many_create_supported() { return create_many_com[0]!=0; };
};

extern SUP *sup; 

extern int comp_sup(char *sname);
extern void compile_AFS_supports(char *supname);
extern void deinit_AFS_supports();
extern int is_it_afs_file(char *ar);
extern SUP* find_support(char *ar);
extern char *quote_path(char *dest, const char *src);
extern char *strong_quote_path(char *dest, const char *src);
extern char *quote_first_path(char *dest, const char *src);

#endif
/* ------------ End of file -------------- */

