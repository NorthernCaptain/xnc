#ifndef SKIN_H
#define SKIN_H

struct Skin
{
        Pixmap     skin;        /* Skin image */
        int        l,h;        /* Skin sizes lxh*/
};

struct Sprite
{
        Skin       *im;
        int        x,y,l,h;        /* Position and size in 'im' skin */
        int        tox, toy;        /* Position on target window */
};


#define LEFT_SIDE 1
#define RIGHT_SIDE 2
#define TOP_SIDE 4
#define BOTTOM_SIDE 8
#define LAST_RECAREA 128

struct RecArea
{
  int x,y,l,h;
  int sides;
  void *data;
};

#endif
/* ------------ End of file -------------- */

