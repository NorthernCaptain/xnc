#ifndef GEOMFUNCS_H
#define GEOMFUNCS_H

#include "skin.h"

struct GEOM_TBL
{
        short int guitype;
        char iname[20];
        int x, y, l, h;
        void  *data1;
        void  *data2;
        void  *data3;
        void  *data4;
};
#define S_GEOM_TBL sizeof(GEOM_TBL)

extern int mlook;
extern Skin skin;

extern GEOM_TBL*  geom_get_transtbl(int *maxentries);
extern void geom_set_transtbl(GEOM_TBL* toset,int max_entries);
extern void geom_translate_by_iname(int guitype,char *iname, int *ix, int *iy, int *il, int *ih);
extern void geom_translate_by_guitype(int guitype, int *ix, int *iy, int *il, int *ih);
extern GEOM_TBL* geom_get_data_by_iname(int guitype, char *iname);

#define        LOOKFOUR        1        /*  Old XNC 4.x style look  */
#define        LOOKFIVE        2        /* New XNC 5.x style look */
#define        LOOKMOTIF        4        /* Motif look alike */


#endif


/* ------------ End of file -------------- */

