/*
 *  Header file for history.cxx - History subroutines.
 */
#ifndef HISTORY_H
#define HISTORY_H

extern void hist_addstring(char *history_id, char *string);
extern void hist_saveall(char *fname);
extern void hist_loadall(char *fname,int progr=0);
extern void init_history();
extern void hist_add_to_top(char *hist_id, char *string);

struct History
{
        char id[32];
        char *string;
        History *next,*prev,*neibour;
        History() {next=prev=neibour=NULL;id[0]=0;};
        History(char *hid, char *str)
        {
                next=prev=neibour=NULL;
                strncpy(id,hid,31);
		id[31]=0;
                if(str)
                {
                  string=new char[strlen(str)+1];
                  strcpy(string,str);
                }
        };
};

extern History *hist_gethistory(char *hist_id);

#endif

/* ------------ End of file -------------- */

