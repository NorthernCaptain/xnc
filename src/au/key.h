#ifndef XNCKEY_H
#define XNCKEY_H


struct ActionKey
{
 unsigned int keys[3];
 unsigned int mods[3];
 int action;
 ActionKey() {keys[0]=keys[1]=keys[2]=0;mods[0]=mods[1]=mods[2]=0;};
};


void parse_file_in_replace_mode(char *name);
void compile_key_support();
void compile_internal_key_support();
char* action_name_by_code(int code);
int look_for_key(XEvent* ev);
void deinit_keysupport();
ActionKey* action_by_name(int n);
int action_key_by_id(char* to, int id, int idx=0);


#endif
/* ------------ End of file -------------- */

