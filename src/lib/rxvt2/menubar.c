/*--------------------------------*-C-*---------------------------------*
 * File:	menubar.c
 *
 * Copyright 1996
 * mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *
 * menuBar syntax:
 *
 *	= title				set menuBar title
 * 	+/				access menuBar top level
 * 	-/				remove menuBar top level
 *	NUL				remove menuBar top level
 *
 *	+ [/menu/path/]submenu		add/access menu
 *	- [/menu/path/]submenu		remove menu
 *
 *	+ [/menu/path/]{item}[{rtext}] [action]	  add/alter item
 *	- [/menu/path/][item}		remove item
 *
 *	+ [/menu/path/]{-}		add separator
 *	- [/menu/path/]{-}		remove separator
 *
 *	+ ../				access parent menu (1 level)
 *	+ ../../			access parent menu (multiple levels)
 *
 *	<b>Begin<r>Right<l>Left<u>Up<d>Down<e>End
 *----------------------------------------------------------------------*/
/*{{{ includes */
#include "main.h"
#include <string.h>

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#include "menubar.h"
#include "command.h"
#include "debug.h"
#include "misc.h"
/*}}}*/

#ifndef NO_MENUBAR
#define HSPACE		1		/* one space */
#define MENU_MARGIN	2
#define menu_height()	(TermWin.fheight + 2 * MENU_MARGIN)

#define MENU_DELAY_USEC	250000	/* 1/4 sec */

#define SEPARATOR_HALFHEIGHT	(SHADOW + 1)
#define SEPARATOR_HEIGHT	(2 * SEPARATOR_HALFHEIGHT)
#define isSeparator(name)	((name)[0] == '\0')

#define SEPARATOR_NAME		"-"
#define MENUITEM_BEG		'{'
#define MENUITEM_END		'}'

#define DOTS	".."

#define Menu_PixelWidth(menu)	(2 * SHADOW + \
Width2Pixel ((menu)->width + 3 * HSPACE))

menuBar_t menuBar;
static GC topShadowGC, botShadowGC, neutralGC, menubarGC;

struct menu_t;

/*{{{ typedefs, local variables */
typedef struct menuitem_t {
   struct menuitem_t * prev;	/* prev menu-item */
   struct menuitem_t * next;	/* next menu-item */
   char * name;	/* character string displayed */
   char * name2;	/* character string displayed (right) */
   short len;			/* strlen (name) */
   short len2;			/* strlen (name) */
   union {
      short type;			/* must not be changed; first element */
      struct {
	 short type;
	 short len;		/* strlen (str) */
	 unsigned char * str;	/* action to take */
      } action;
      struct {
	 short type;
	 struct menu_t * menu;	/* sub-menu */
      } submenu;
#define MenuLabel		0
#define MenuAction		1
#define MenuTerminalAction	2
#define MenuSubMenu		3
   } entry;
} menuitem_t;

typedef struct menu_t {
   struct menu_t * parent;	/* parent menu */
   struct menu_t * prev;	/* prev menu */
   struct menu_t * next;	/* next menu */
   menuitem_t * head;		/* double-linked list */
   menuitem_t * tail;		/* double-linked list */
   menuitem_t * item;		/* current item */
   char * name;			/* menu name */
   short len;			/* strlen (name) */
   short width;			/* maximum menu width [chars] */
   Window win;		/* window of the menu */
   short x;		/* x location [pixels] (chars if parent == NULL) */
   short y;		/* y location [pixels] */
   short w, h;		/* window width, height [pixels] */
} menu_t;

static struct {
   menu_t * head, * tail;	/* double-linked list of menus */
   char * title;		/* title to put in the empty menuBar */
} RootBar = { NULL, NULL, NULL };
static menu_t * ActiveMenu = NULL;	/* currently active menu */

static int menuarrows_x = 0;
typedef struct {
   char name;
   char key;
   short type;
   short len;		/* strlen (str) */
   unsigned char * str;	/* action to take */
} menuarrow_t;
static menuarrow_t menuarrows [] = {
     {'l', 'D', 0, 0, NULL},
     {'u', 'A', 0, 0, NULL},
     /* {'c', 0, 0, 0, NULL}, */
     {'d', 'B', 0, 0, NULL},
     {'r', 'C', 0, 0, NULL}
};
#define NARROWS	(sizeof(menuarrows)/sizeof(menuarrows [0]))

/*}}}*/

/*{{{ prototypes: */
static menu_t *
menu_delete (menu_t * menu);
/*}}}*/

/*
 * find an item called NAME in MENU
 */
static menuitem_t *
menuitem_find (menu_t * menu, char * name)
{
   menuitem_t * item;

   assert (name != NULL);
   assert (menu != NULL);

   /* find the last item in the menu, this is good for separators */
   for (item = menu->tail; item != NULL; item = item->prev)
     {
	if (item->entry.type == MenuSubMenu)
	  {
	     if (!strcmp (name, (item->entry.submenu.menu)->name))
	       break;
	  }
	else
	  if ((isSeparator (name) && isSeparator (item->name)) ||
		 !strcmp (name, item->name))
	  break;
     }
   return item;
}

/*
 * unlink ITEM from its MENU and free its memory
 */
static void
menuitem_free (menu_t * menu, menuitem_t * item)
{
   /* disconnect */
   menuitem_t * prev, * next;

   assert (menu != NULL);

   prev = item->prev;
   next = item->next;
   if (prev != NULL) prev->next = next;
   if (next != NULL) next->prev = prev;

   /* new head, tail */
   if (menu->tail == item) menu->tail = prev;
   if (menu->head == item) menu->head = next;

   switch (item->entry.type) {
    case MenuAction:
    case MenuTerminalAction:
      FREE (item->entry.action.str, "free", "free");
      break;
    case MenuSubMenu:
      (void) menu_delete (item->entry.submenu.menu);
      break;
   }
   if (item->name != NULL)	FREE (item->name, "free", "free");
   if (item->name2 != NULL)	FREE (item->name2, "free", "free");
   FREE (item, "free", "free");
}

static menuarrow_t *
menuarrow_find (int name)
{
   int i;
   for (i = 0; i < NARROWS; i++)
     if (name == menuarrows [i].name)
       return (menuarrows+i);
   return NULL;
}

static void
menuarrow_free (int name)
{
   menuarrow_t * arr;

   if (!name)
     {
	int i;
	for (i = 0; i < NARROWS; i++)
	  menuarrow_free (menuarrows[i].name);
     }
   else if ((arr = menuarrow_find (name)) != NULL)
     {
	switch (arr->type) {
	 case MenuAction:
	 case MenuTerminalAction:
	   FREE (arr->str, "free", "free");
	   arr->str = NULL;
	   break;
	}
	arr->type = MenuLabel;
     }
}

static void
menuarrow_add (char * string)
{
   int i;
   unsigned xtra_len;
   char * p;

   struct {
      char * str;
      int len;
   }
   beg = {NULL, 0}, end = {NULL, 0}, * cur, parse [NARROWS];

   memset (parse, 0, sizeof(parse));

   /* printf ("add arrows = `%s'\n", string); */
   for (p = string; p != NULL && *p; string = p)
     {
	p = (string + 3);
	/* printf ("parsing at %s\n", string); */
	switch (string [1]) {
	 case 'b': cur = &beg; break;
	 case 'e': cur = &end; break;

	 default:
	   for (i = 0; i < NARROWS; i++)
	     {
		if (string [1] == menuarrows[i].name)
		  {
		     cur = &(parse [i]);
		     break;
		  }
	     }
	   if (i >= NARROWS) continue;	/* not found */
	   break;
	}

	string = p;
	cur->str = string;
	cur->len = 0;

	if (cur == &end)
	  {
	     p = strchr (string, '\0');
	  }
	else
	  {
	     char * next = string;

	     do
	       {
		  p = strchr (next, '<');
		  if (p != NULL)
		    {
		       if (p [1] && p [2] == '>')
			 break;
		       /* parsed */
		    }
		  else
		    {
		       if (beg.str == NULL)	/* no end needed */
			 p = strchr (next, '\0');
		       break;
		    }
		  next = (p+1);
	       }
	     while (1);
	  }

	if (p == NULL) return;
	cur->len = (p - string);
     }

#ifdef DEBUG_MENUARROWS
   cur = &beg;
   printf ("<b>(len %d) = %.*s\n",
	   cur->len, cur->len, (cur->str ? cur->str : ""));
   for (i = 0; i < NARROWS; i++)
     {
	cur = &(parse [i]);
	printf ("<%c>(len %d) = %.*s\n",
		menuarrows[i].name,
		cur->len,
		cur->len,
		(cur->str ? cur->str : ""));
     }
   cur = &end;
   printf ("<e>(len %d) = %.*s\n",
	   cur->len, cur->len, (cur->str ? cur->str : ""));
#endif

   xtra_len = (beg.len + end.len);
   for (i = 0; i < NARROWS; i++)
     {
	if (xtra_len || parse [i].len)
	  menuarrow_free (menuarrows [i].name);
     }

   for (i = 0; i < NARROWS; i++)
     {
	menuarrow_t * arr = (menuarrows+i);
	unsigned char * str;
	unsigned int len;

	if (!parse [i].len) continue;

	str = MALLOC ((parse [i].len + xtra_len+1), "add");
	if (str == NULL) continue;

	len = 0;
	if (beg.len)
	  {
	     strncpy (str+len, beg.str, beg.len);
	     len += beg.len;
	  }

	strncpy (str+len, parse [i].str, parse [i].len);
	len += parse [i].len;

	if (end.len)
	  {
	     strncpy (str+len, end.str, end.len);
	     len += end.len;
	  }

	str [len] = '\0';

#ifdef DEBUG_MENUARROWS
	printf ("<%c>(len %d) = %s\n", menuarrows[i].name, len, str);
#else
	len = escaped_string (str);
#endif
	if (len)
	  {
	     arr->type = MenuAction;
	     /* sort command vs. terminal actions */
	     if (str [0] == '\0')
	       {
		  if (len > 1) memmove (str, str+1, len--);
		  if (str [0] != '\0')
		    arr->type = MenuTerminalAction;
	       }

	     arr->str = str;
	     arr->len = len;
	  }
	else
	  {
	     FREE (str, "add", "add");
	  }
     }
}

static menuitem_t *
menuitem_add (menu_t * menu, char * name, char * name2, char * action)
{
   menuitem_t * item;
   unsigned int len;

   assert (name != NULL);
   assert (action != NULL);

   if (menu == NULL) return NULL;

   if (isSeparator (name))
     {
	/* add separator, no action */
	name = "";
	action = "";
     }
   else
     {
	/*
	 * add/replace existing menu item
	 */
	item = menuitem_find (menu, name);
	if (item != NULL)
	  {
	     if (item->name2 != NULL && name2 != NULL)
	       {
		  FREE (item->name2, "add", "add");
		  item->len2 = 0;
		  item->name2 = NULL;
	       }
	     switch (item->entry.type) {
	      case MenuAction:
	      case MenuTerminalAction:
		FREE (item->entry.action.str, "add", "add");
		item->entry.action.str = NULL;
		break;
	     }
	     goto Item_Found;
	  }
     }
   /* allocate a new itemect */
   if ((item = MALLOC (sizeof(menuitem_t), "add")) == NULL)
     return NULL;

   item->len2 = 0;
   item->name2 = NULL;

   len = strlen (name);
   item->name = MALLOC ((len+1), "add");
   if (item->name != NULL)
     {
	strcpy (item->name, name);
	if (name [0] == '.' && name [1] != '.')
	  len = 0;		/* hidden menu name */
     }
   else
     {
	FREE (item, "add", "add");
	return NULL;
     }
   item->len = len;

   /* add to tail of list */
   item->prev = menu->tail;
   item->next = NULL;

   if (menu->tail != NULL) (menu->tail)->next = item;
   menu->tail = item;
   /* fix head */
   if (menu->head == NULL) menu->head = item;

   /*
    * add action
    */
   Item_Found:
   if (name2 != NULL && item->name2 == NULL)
     {
	len = strlen (name2);
	if (len == 0 || (item->name2 = MALLOC ((len+1), "add")) == NULL)
	  {
	     len = 0;
	     item->name2 = NULL;
	  }
	else
	  {
	     strcpy (item->name2, name2);
	  }
	item->len2 = len;
     }

   item->entry.type = MenuLabel;
   len = strlen (action);

   if (len == 0 && item->name2 != NULL)
     {
	action = item->name2;
	len = item->len2;
     }

   if (len)
     {
	unsigned char * str = MALLOC ((len+1), "add");
	if (str == NULL)
	  {
	     menuitem_free (menu, item);
	     return NULL;
	  }
	strcpy (str, action);
#ifndef DEBUG_MENU
	len = escaped_string (str);
#endif
	if (len)
	  {
	     item->entry.type = MenuAction;
	     /* sort command vs. terminal actions */
	     if (str [0] == '\0')
	       {
		  if (len > 1) memmove (str, str+1, len--);
		  if (str [0] != '\0')
		    item->entry.type = MenuTerminalAction;
	       }

	     item->entry.action.str = str;
	     item->entry.action.len = len;
	  }
	else
	  {
	     FREE (str, "add", "add");
	  }
     }

   /* new item and a possible increase in width */
   if (menu->width < (item->len + item->len2))
     menu->width = (item->len + item->len2);

   return item;
}

/*
 * search for the base starting menu for NAME.
 * return a pointer to the portion of NAME that remains
 */
static char *
menu_find_base (menu_t ** menu, char * path)
{
   menu_t * m = NULL;
   menuitem_t * item;

   assert (menu != NULL);

   if (path [0] == '\0') return path;

   if (strchr (path, '/') != NULL)
     {
	register char * p = path;

	while ((p = strchr (p, '/')) != NULL)
	  {
	     p++;
	     if (*p == '/') path = p;
	  }
	if (path [0] == '/')
	  {
	     path++;
	     *menu = NULL;
	  }
	while ((p = strchr (path, '/')) != NULL)
	  {
	     p [0] = '\0';
	     if (path [0] == '\0') return NULL;
	     if (!strcmp (path, DOTS))
	       {
		  if (*menu != NULL) *menu = (*menu)->parent;
	       }
	     else
	       {
		  path = menu_find_base (menu, path);
		  if (path [0] != '\0')   /* not found */
		    {
		       p [0] = '/';   /* fix-up name again */
		       return path;
		    }
	       }

	     path = (p+1);
	  }
     }
   if (!strcmp (path, DOTS))
     {
	path += strlen (DOTS);
	if (*menu != NULL) *menu = (*menu)->parent;
	return path;
     }

   /* find this menu */
   if (*menu == NULL)
     {
	for (m = RootBar.tail; m != NULL; m = m->prev)
	  {
	     if (!strcmp (path, m->name))
	       break;
	  }
     }
   else
     {
	/* find this menu */
	for (item = (*menu)->tail; item != NULL; item = item->prev)
	  {
	     if (item->entry.type == MenuSubMenu &&
		 !strcmp (path, (item->entry.submenu.menu)->name))
	       {
		  m = (item->entry.submenu.menu);
		  break;
	       }
	  }
     }
   if (m != NULL)
     {
	*menu = m;
	path += strlen (path);
     }
   return path;
}

/*
 * delete this entire menu
 */
static menu_t *
menu_delete (menu_t * menu)
{
   menu_t * parent = NULL, * prev, * next;
   menuitem_t * item;

   /* delete the entire menu */
   if (menu == NULL)
     return NULL;

   parent = menu->parent;

   /* unlink MENU */
   prev = menu->prev;
   next = menu->next;
   if (prev != NULL) prev->next = next;
   if (next != NULL) next->prev = prev;

   /* fix the index */
   if (parent == NULL)
     {
	const int len = (menu->len + HSPACE);

	if (RootBar.tail == menu) RootBar.tail = prev;
	if (RootBar.head == menu) RootBar.head = next;

	for (next = menu->next; next != NULL; next = next->next)
	  next->x -= len;
     }
   else
     {
	for (item = parent->tail; item != NULL; item = item->prev)
	  {
	     if (item->entry.type == MenuSubMenu &&
		 item->entry.submenu.menu == menu)
	       {
		  item->entry.submenu.menu = NULL;
		  menuitem_free (menu->parent, item);
		  break;
	       }
	  }
     }

   item = menu->tail;
   while (item != NULL)
     {
	menuitem_t * p = item->prev;
	menuitem_free (menu, item);
	item = p;
     }

   if (menu->name != NULL)	FREE (menu->name, "free", "free");
   FREE (menu, "free", "free");

   return parent;
}

static menu_t *
menu_add (menu_t * parent, char * path)
{
   menu_t * menu;

   if (strchr (path, '/') != NULL)
     {
	register char * p;
	if (path [0] == '/')
	  {
	     /* shouldn't happen */
	     path++;
	     parent = NULL;
	  }
	while ((p = strchr (path, '/')) != NULL)
	  {
	     p [0] = '\0';
	     if (path [0] == '\0') return NULL;

	     parent = menu_add (parent, path);
	     path = (p+1);
	  }
     }
   if (!strcmp (path, DOTS))
     return (parent != NULL ? parent->parent : parent);

   if (path [0] == '\0') return parent;

   /* allocate a new menu */
   if ((menu = MALLOC (sizeof(menu_t), "add")) == NULL)
     return parent;

   menu->width = 0;
   menu->parent = parent;
   menu->len = strlen (path);
   menu->name = MALLOC ((menu->len+1), "add");
   if (menu->name == NULL)
     {
	FREE (menu, "add", "add");
	return parent;
     }
   strcpy (menu->name, path);

   /* initialize head/tail */
   menu->head = menu->tail = NULL;
   menu->prev = menu->next = NULL;

   menu->win = None;
   menu->x = menu->y = menu->w = menu->h = 0;
   menu->item = NULL;

   /* add to tail of list */
   if (parent == NULL)
     {
	menu->prev = RootBar.tail;
	if (RootBar.tail != NULL) RootBar.tail->next = menu;
	RootBar.tail = menu;
	if (RootBar.head == NULL) RootBar.head = menu;	/* fix head */
	if (menu->prev)
	  menu->x = (menu->prev->x + menu->prev->len + HSPACE);
     }
   else
     {
	menuitem_t * item;
	item = menuitem_add (parent, path, "", "");
	if (item == NULL)
	  {
	     FREE (menu, "add", "add");
	     return parent;
	  }
	assert (item->entry.type == MenuLabel);
	item->entry.type = MenuSubMenu;
	item->entry.submenu.menu = menu;
     }

   return menu;
}

static void
drawbox_menubar (int x, int len, int state)
{
   GC top, bot;

   x = Width2Pixel (x);
   len = Width2Pixel (len + HSPACE);
   if (x >= TermWin.width)
     return;
   else if (x + len >= TermWin.width)
     len = (TermWin_TotalWidth() - x);

#ifdef MENUBAR_SHADOW_IN
   state = -state;
#endif
   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break; /* SHADOW_OUT */
    case -1: top = botShadowGC; bot = topShadowGC; break; /* SHADOW_IN */
    case 0: top = bot = neutralGC; break;	/* neutral */
   }

   Draw_Shadow (menuBar.win, top, bot,
		x, 0, len, menuBar_TotalHeight ());
}

static void
drawtriangle (int x, int y, int state)
{
   GC top, bot;
   int w;
#ifdef MENU_SHADOW_IN
   state = -state;
#endif
   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break; /* SHADOW_OUT */
    case -1: top = botShadowGC; bot = topShadowGC; break; /* SHADOW_IN */
    case 0: top = bot = neutralGC; break;	/* neutral */
   }

   w = menu_height () / 2;

   x -= (SHADOW + MENU_MARGIN) + (3 * w / 2);
   y += (SHADOW + MENU_MARGIN) + (w / 2);

   Draw_Triangle (ActiveMenu->win, top, bot, x, y, w, 'r');
}

static void
drawbox_menuitem (int y, int state)
{
   GC top, bot;

#ifdef MENU_SHADOW_IN
   state = -state;
#endif
   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break; /* SHADOW_OUT */
    case -1: top = botShadowGC; bot = topShadowGC; break; /* SHADOW_IN */
    case 0: top = bot = neutralGC; break;	/* neutral */
   }

   Draw_Shadow (ActiveMenu->win, top, bot,
		SHADOW + 0,
		SHADOW + y,
		ActiveMenu->w - 2 * (SHADOW),
		menu_height () + 2 * MENU_MARGIN);
   XFlush (Xdisplay);
}

#ifdef DEBUG_MENU
static void
print_menu_ancestors (menu_t * menu)
{
   if (menu == NULL)
     {
	printf ("Top Level menu\n");
	return;
     }

   printf ("menu %s ", menu->name);
   if (menu->parent != NULL)
     {
	menuitem_t * item;
	for (item = menu->parent->head; item != NULL; item = item->next)
	  {
	     if (item->entry.type == MenuSubMenu &&
		 item->entry.submenu.menu == menu)
	       {
		  break;
	       }
	  }
	if (item == NULL)
	  {
	     printf ("is an orphan!\n");
	     return;
	  }
     }
   printf ("\n");
   print_menu_ancestors (menu->parent);
}

static void
print_menu_descendants (menu_t * menu)
{
   menuitem_t * item;
   menu_t * parent;
   int i, level = 0;

   parent = menu;
   do
     {
	level++;
	parent = parent->parent;
     }
   while (parent != NULL);

   for (i = 0; i < level; i++) printf (">");
   printf ("%s\n", menu->name);

   for (item = menu->head; item != NULL; item = item->next)
     {
	if (item->entry.type == MenuSubMenu)
	  {
	     if (item->entry.submenu.menu == NULL)
	       printf ("> %s == NULL\n", item->name);
	     else
	       print_menu_descendants (item->entry.submenu.menu);
	  }
	else
	  {
	     for (i = 0; i < level; i++) printf ("+");
	     if (item->entry.type == MenuLabel)
	       printf ("label: ");
	     printf ("%s\n", item->name);
 	  }
     }

   for (i = 0; i < level; i++) printf ("<");
   printf ("\n");
}
#endif

/* pop up/down the current menu and redraw the menuBar button */
static void
menu_show (void)
{
   int x, y, xright;
   menuitem_t * item;

   if (ActiveMenu == NULL) return;

   x = ActiveMenu->x;
   if (ActiveMenu->parent == NULL)
     {
	register int h;
	drawbox_menubar (x, ActiveMenu->len, -1);
	x = Width2Pixel (x);

	ActiveMenu->y = 1;
	ActiveMenu->w = Menu_PixelWidth (ActiveMenu);

	if ((x + ActiveMenu->w) >= TermWin.width)
	  x = (TermWin_TotalWidth() - ActiveMenu->w);

	/* find the height */
	for (h = 0, item = ActiveMenu->head; item != NULL; item = item->next)
	  {
	     if (isSeparator (item->name))
	       h += SEPARATOR_HEIGHT;
	     else
	       h += menu_height ();
	  }
	ActiveMenu->h = h + 2 * (SHADOW + MENU_MARGIN);
     }

   if (ActiveMenu->win == None)
     {
	ActiveMenu->win = XCreateSimpleWindow (Xdisplay, TermWin.vt,
					       x,
					       ActiveMenu->y,
					       ActiveMenu->w,
					       ActiveMenu->h,
					       0,
					       PixColors [fgColor],
					       PixColors [scrollColor]);
	XMapWindow (Xdisplay, ActiveMenu->win);
     }

   Draw_Shadow (ActiveMenu->win,
		topShadowGC, botShadowGC,
		0, 0,
		ActiveMenu->w, ActiveMenu->h);

   /* determine the correct right-alignment */
   for (xright = 0, item = ActiveMenu->head; item != NULL; item = item->next)
     if (item->len2 > xright) xright = item->len2;

   for (y = 0, item = ActiveMenu->head;
	item != NULL;
	item = item->next)
     {
	const int xoff = (SHADOW + Width2Pixel (HSPACE) / 2);
	const int yoff = (SHADOW + MENU_MARGIN);
	register int h;
	GC gc = menubarGC;

	if (isSeparator (item->name))
	  {
	     Draw_Shadow (ActiveMenu->win,
			  topShadowGC, botShadowGC,
			  xoff,
			  yoff + y + SEPARATOR_HALFHEIGHT,
			  ActiveMenu->w - (2 * xoff),
			  0);
	     h = SEPARATOR_HEIGHT;
	  }
	else
	  {
	     char * name = item->name;
	     int len = item->len;

	     if (item->entry.type == MenuLabel)
	       {
		  gc = botShadowGC;
	       }
	     else if (item->entry.type == MenuSubMenu)
	       {
		  int x1, y1;
		  menuitem_t * it;
		  menu_t * menu = item->entry.submenu.menu;

		  drawtriangle (ActiveMenu->w, y, +1);

		  name = menu->name;
		  len =  menu->len;

		  y1 = ActiveMenu->y + y;

		  /* place sub-menu at midpoint of parent menu */
		  menu->w = Menu_PixelWidth (menu);
		  x1 = ActiveMenu->w / 2;

		  /* right-flush menu if it's too small */
		  if (x1 > menu->w)
		    x1 += (x1 - menu->w);
		  x1 += x;

		  /* find the height of this submenu */
		  for (h = 0, it = menu->head; it != NULL; it = it->next)
		    {
		       if (isSeparator (it->name))
			 h += SEPARATOR_HEIGHT;
		       else
			 h += menu_height ();
		    }
		  menu->h = h + 2 * (SHADOW + MENU_MARGIN);

		  /* ensure menu is in window limits */
		  if ((x1 + menu->w) >= TermWin.width)
		    x1 = (TermWin_TotalWidth() - menu->w);

		  if ((y1 + menu->h) >= TermWin.height)
		    y1 = (TermWin_TotalHeight() - menu->h);

		  menu->x = (x1 < 0 ? 0 : x1);
		  menu->y = (y1 < 0 ? 0 : y1);
	       }
	     else if (item->name2 && !strcmp (name, item->name2))
	       name = NULL;

	     if (len && name)
	       XDrawString (Xdisplay,
			    ActiveMenu->win, gc,
			    xoff,
			    yoff + y + menu_height() - (2 * MENU_MARGIN),
			    name, len);

	     len = item->len2;
	     name = item->name2;
	     if (len && name)
	       XDrawString (Xdisplay,
			    ActiveMenu->win, gc,
			    ActiveMenu->w - (xoff + Width2Pixel (xright)),
			    yoff + y + menu_height() - (2 * MENU_MARGIN),
			    name, len);

	     h = menu_height();
	  }
	y += h;
     }
}

static void
menu_display (void (*update)(void))
{
   if (ActiveMenu == NULL) return;
   if (ActiveMenu->win != None)
     XDestroyWindow (Xdisplay, ActiveMenu->win);
   ActiveMenu->win = None;
   ActiveMenu->item = NULL;

   if (ActiveMenu->parent == NULL)
     drawbox_menubar (ActiveMenu->x, ActiveMenu->len, +1);
   ActiveMenu = ActiveMenu->parent;
   update ();
}

static void
menu_hide_all (void)
{
   menu_display (menu_hide_all);
}

static void
menu_hide (void)
{
   menu_display (menu_show);
}

static void
delete_entire_menu (void)
{
   menu_t * menu;
   menu = RootBar.tail;
   while (menu != NULL)
     {
	menu_t * prev = menu->prev;
	menu_delete (menu);
	menu = prev;
     }
   RootBar.head = RootBar.tail = ActiveMenu = NULL;
   menuarrow_free (0);		/* remove all arrow functions */
}

/*
 * user interface for building/deleting and otherwise managing menus
 */
void
menubar_dispatch (char * str)
{
   static menu_t * menu = NULL;		/* the menu currently being built */
   const char * const id = "dispatch";
   int cmd;
   char * path, * name, * name2;

   if (menubar_visible () && ActiveMenu != NULL)
     menubar_expose ();
   else
     ActiveMenu = NULL;

   /* skip leading spaces */
   while (isspace (*str)) str++;
   cmd = *str++;

   switch (cmd) {
    case '\0':			/* delete entire menu */
      delete_entire_menu ();
      menu = NULL;
      return;
      break;

    case '=':		/* add/change the menuBar title */
      while (isspace (*str)) str++;
      name = REALLOC (RootBar.title, (strlen (str)+1), id);
      if (name != NULL)
	{
	   strcpy (name, str);
	   RootBar.title = name;
	}
      menubar_expose ();
      return;
      break;

    case '<':
      str--;
      if (str [1] && str [2] == '>') menuarrow_add (str);
      return;
      break;

    case '+':
    case '-':
      while (isspace (*str)) str++;
      path = name = str;

      name2 = NULL;
      /* parse STR, allow spaces inside (name)  */
      if (path [0] != '\0')
	{
	   name = strchr (path, MENUITEM_BEG);
	   str = strchr (path, MENUITEM_END);
	   if (name != NULL || str != NULL)
	     {
		if (name == NULL || str == NULL || str <= (name+1)
		    || (name > path && name [-1] != '/'))
		  {
		     print_error ("menu error <%s>\n", path);
		     return;
		  }

		if (str [1] == MENUITEM_BEG)
		  {
		     name2 = (str+2);
		     str = strchr (name2, MENUITEM_END);

		     if (str == NULL)
		       {
			  print_error ("menu error <%s>\n", path);
			  return;
		       }
		     name2 [-2] = '\0';	/* remove prev MENUITEM_END */
		  }

		if (name > path && name [-1] == '/')
		  name [-1] = '\0';

		*name++ = '\0';			/* delimit */
		*str++ = '\0';			/* delimit */

		while (isspace (*str)) str++;   /* skip space */
	     }

#if 0
	   printf ("path = <%s>, name = <%s>, name2 = <%s>, action = <%s>\n",
		   path,
		   (name ? name : ""),
		   (name2 ? name2 : ""),
		   (str ? str : "")
		   );
#endif
	}
      break;

    default:			/* unknown command */
      return;
      break;
   }

   /* process the different commands */
   switch (cmd) {
    case '+':			/* add/replace existing menu or menuitem */
      if (path [0] != '\0')
	{
	   path = menu_find_base (&menu, path);
	   if (path [0] != '\0')
	     menu = menu_add (menu, path);
	}
      if (name [0] != '\0')
	{
	   if (!strcmp (name, SEPARATOR_NAME)) name = "";
	   menuitem_add (menu, name, name2, str);
	}
      break;

    case '-':			/* delete menu entry */
      if (!strcmp (path, "/") && name [0] == '\0')
	{
	   delete_entire_menu ();
	   menu = NULL;
	   menubar_expose ();
	}
      else if (path [0] != '\0')
	{
	   path = menu_find_base (&menu, path);
	   if (path [0] != '\0')
	     menu = NULL;
	}

      if (menu != NULL)
	{
	   if (name [0] == '\0')
	     {
		menu = menu_delete (menu);
	     }
	   else
	     {
		menuitem_t * item;
		if (!strcmp (name, SEPARATOR_NAME)) name = "";
		item = menuitem_find (menu, name);

		if (item != NULL && item->entry.type != MenuSubMenu)
		  {
		     menuitem_free (menu, item);

		     /* fix up the width */
		     menu->width = 0;
		     for (item = menu->head; item != NULL; item = item->next)
		       {
			  if (menu->width < (item->len + item->len2))
			    menu->width = (item->len + item->len2);
		       }
		  }
	     }
	   menubar_expose ();
	}
      break;
   }
}

static void
draw_menuarrows (int name, int state)
{
   GC top, bot;

   int i;
#ifdef MENU_SHADOW_IN
   state = -state;
#endif
   switch (state) {
    case +1: top = topShadowGC; bot = botShadowGC; break; /* SHADOW_OUT */
    case -1: top = botShadowGC; bot = topShadowGC; break; /* SHADOW_IN */
    case 0: top = bot = neutralGC; break;	/* neutral */
   }

   if (!menuarrows_x) return;

   for (i = 0; i < NARROWS; i++)
     {
	const int w = Width2Pixel (1);
	const int y = (menuBar_TotalHeight () - w) / 2;
	int x = menuarrows_x + (5 * Width2Pixel (i)) / 4;

	if (!name || name == menuarrows[i].name)
	  Draw_Triangle (menuBar.win, top, bot, x, y, w, menuarrows[i].name);
     }
   XFlush (Xdisplay);
}

void
menubar_expose (void)
{
   menu_t * menu;
   int x;

   if (!menubar_visible ())
     return;

   if (menubarGC == None)
     {
	/* Create the graphics context */
	XGCValues gcvalue;
	gcvalue.font = TermWin.font->fid;

	gcvalue.foreground = (Xdepth <= 2 ?
			      PixColors [fgColor] :
			      PixColors [blackColor]);
	menubarGC = XCreateGC (Xdisplay, menuBar.win,
				GCForeground|GCFont,
				&gcvalue);

	gcvalue.foreground = PixColors [scrollColor];
	neutralGC = XCreateGC (Xdisplay, menuBar.win,
			       GCForeground,
			       &gcvalue);

	gcvalue.foreground = PixColors [bottomShadowColor];
	botShadowGC = XCreateGC (Xdisplay, menuBar.win,
				 GCForeground|GCFont,
				 &gcvalue);

	gcvalue.foreground = PixColors [topShadowColor];
	topShadowGC = XCreateGC (Xdisplay, menuBar.win,
				 GCForeground,
				 &gcvalue);
     }

   /* make sure the font is correct */
   XSetFont (Xdisplay, menubarGC, TermWin.font->fid);
   XSetFont (Xdisplay, botShadowGC, TermWin.font->fid);
   XClearWindow (Xdisplay, menuBar.win);

   menu_hide_all ();

   x = 0;
   for (menu = RootBar.head; menu != NULL; menu = menu->next)
     {
	int len = menu->len;
	x = (menu->x + menu->len + HSPACE);

#ifdef DEBUG_MENU
	print_menu_descendants (menu);
#endif

	if (x >= TermWin.ncol)
	  len = (TermWin.ncol - (menu->x + HSPACE));

	drawbox_menubar (menu->x, len, +1);

	XDrawString (Xdisplay,
		     menuBar.win, menubarGC,
		     (Width2Pixel (menu->x) + Width2Pixel (HSPACE)/2),
		     menuBar_height (),
		     menu->name, len);

	if (x >= TermWin.ncol)
	  break;
     }
   drawbox_menubar (x, TermWin.ncol, (x ? +1 : -1));

   /* add the menuBar title, if it exists and there's plenty of room */
   menuarrows_x = 0;
   if (x < TermWin.ncol)
     {
	char * str, title [256];
	int len, ncol = TermWin.ncol;

	if (x < (ncol - (NARROWS+1)))
	  {
	     ncol -= (NARROWS+1);
	     menuarrows_x = Width2Pixel (ncol);
	  }
	draw_menuarrows (0, +1);

	str = (RootBar.title ? RootBar.title : APL_NAME "-%v");
	for (len = 0; *str && len < sizeof(title)-1; str++)
	  {
	     char * s;
	     switch (*str) {
	      case '%':
		str++;
		switch (str[0]) {
		 case 'v': s = VERSION; break;
		 case '%': s = "%"; break;
		 default:  s = NULL;
		}
		if (s != NULL)
		  while (*s && len < sizeof(title)-1)
		    title [len++] = *s++;
		break;

	      default:
		title [len++] = str[0];
		break;
	     }
	  }
	title [len] = '\0';

	ncol -= (x + len + HSPACE);
	if (len > 0 && ncol >= 0)
	  XDrawString (Xdisplay,
		       menuBar.win, menubarGC,
		       Width2Pixel (x) + Width2Pixel (ncol + HSPACE)/2,
		       menuBar_height (),
		       title, len);
     }
}

int
menubar_mapping (int map)
{
   int change = 0;
   if (map && !menubar_visible ())
     {
	menuBar.state = 1;
	XMapWindow (Xdisplay, menuBar.win);
	change = 1;
     }
   else if (!map && menubar_visible ())
     {
	menubar_expose ();
	menuBar.state = 0;
	XUnmapWindow (Xdisplay, menuBar.win);
	change = 1;
     }
   else
     menubar_expose ();

   return change;
}

static int
menu_select (XButtonEvent * ev)
{
   menuitem_t * thisitem, * item = NULL;
   int this_y, y;

   Window unused_root, unused_child;
   int unused_root_x, unused_root_y;
   unsigned int unused_mask;

   if (ActiveMenu == NULL) return 0;

   XQueryPointer (Xdisplay, ActiveMenu->win,
		  &unused_root, &unused_child,
		  &unused_root_x, &unused_root_y,
		  &(ev->x), &(ev->y),
		  &unused_mask);

   if (ActiveMenu->parent != NULL && (ev->x < 0 || ev->y < 0))
     {
	menu_hide ();
	return 1;
     }

   /* determine the menu item corresponding to the Y index */
   if (ev->x >= 0 &&
       ev->x <= (ActiveMenu->w - SHADOW))
     {
	for (y = 0, item = ActiveMenu->head;
	     item != NULL;
	     item = item->next)
	  {
	     int h = menu_height ();
	     if (isSeparator (item->name))
	       {
		  h = SEPARATOR_HEIGHT;
	       }
	     else if (ev->y >= y && ev->y < (y + h))
	       {
		  break;
	       }
	     y += h;
	  }
     }

   if (item == NULL && ev->type == ButtonRelease)
     {
	menu_hide_all ();
	return 0;
     }

   thisitem = item;
   this_y = y;

   /* erase the last item */
   if (ActiveMenu->item != NULL)
     {
	if (ActiveMenu->item != thisitem)
	  {
	     for (y = 0, item = ActiveMenu->head;
		  item != NULL;
		  item = item->next)
	       {
		  int h = menu_height ();
		  if (isSeparator (item->name))
		    {
		       h = SEPARATOR_HEIGHT;
		    }
		  else if (item == ActiveMenu->item)
		    {
		       /* erase old menuitem */
		       drawbox_menuitem (y, 0);   /* No Shadow */
		       if (item->entry.type == MenuSubMenu)
			 drawtriangle (ActiveMenu->w, y, +1);
		       break;
		    }
		  y += h;
	       }
	  }
	else
	  {
	     switch (ev->type) {
	      case ButtonRelease:
		switch (item->entry.type) {
		 case MenuLabel:
		 case MenuSubMenu:
		   menu_hide_all ();
		   break;

		 case MenuAction:
		 case MenuTerminalAction:
		   drawbox_menuitem (this_y, -1);
		   /*
		    * use select for timing
		    * remove menu before sending keys to the application
		    */
		     {
			struct itimerval tv;
			tv.it_value.tv_sec = 0;
			tv.it_value.tv_usec = MENU_DELAY_USEC;

			select (0, NULL, NULL, NULL, &tv.it_value);
		     }
		   menu_hide_all ();
#ifndef DEBUG_MENU
		   switch (item->entry.type) {
		    case MenuTerminalAction:
		      cmd_write (item->entry.action.str,
				 item->entry.action.len);
		      break;

		    default:
		      tt_write (item->entry.action.str,
				item->entry.action.len);
		      break;
		   }
#else	/* DEBUG_MENU */
		   printf ("%s: %s\n", item->name, item->entry.action.str);
#endif	/* DEBUG_MENU */
		   break;
		}
		break;

	      default:
		if (item->entry.type == MenuSubMenu)
		  goto DoMenu;
		break;
	     }
	     return 0;
	  }
     }

   DoMenu:
   ActiveMenu->item = thisitem;
   y = this_y;
   if (thisitem != NULL)
     {
	item = ActiveMenu->item;
	if (item->entry.type != MenuLabel)
	  drawbox_menuitem (y, +1);
	if (item->entry.type == MenuSubMenu)
	  {
	     int x;
	     drawtriangle (ActiveMenu->w, y, -1);

	     x = ev->x + (ActiveMenu->parent ?
			  ActiveMenu->x :
			  Width2Pixel (ActiveMenu->x));

	     if (x >= item->entry.submenu.menu->x)
	       {
		  ActiveMenu = item->entry.submenu.menu;
		  menu_show ();
		  return 1;
	       }
	  }
     }
   return 0;
}

static void
menubar_select (XButtonEvent * ev)
{
   menu_t * menu = NULL;

   /* determine the menu corresponding to the X index */
   if (ev->y >= 0 && ev->y <= menuBar_height ())
     {
	for (menu = RootBar.head; menu != NULL; menu = menu->next)
	  {
	     int x = Width2Pixel (menu->x);
	     int w = Width2Pixel (menu->len + HSPACE);

	     if ((ev->x >= x && ev->x < x + w))
	       break;
	  }
     }

   switch (ev->type) {
    case ButtonRelease:
      menu_hide_all ();
      break;

    case ButtonPress:
      if (menu == NULL && menuarrows_x && ev->x >= menuarrows_x)
	{
	   int i;
	   for (i = 0; i < NARROWS; i++)
	     {
		if (ev->x >= (menuarrows_x + (Width2Pixel (4 * i + i)) / 4) &&
		    ev->x < (menuarrows_x + (Width2Pixel (4* i + i + 4)) / 4))
		  {
		     menuarrow_t * arr = (menuarrows+i);
		     char def[] = "\033[?";

		     draw_menuarrows (menuarrows[i].name, -1);
		     /*
		      * use select for timing
		      */
		       {
			  struct itimerval tv;
			  tv.it_value.tv_sec = 0;
			  tv.it_value.tv_usec = MENU_DELAY_USEC;

			  select (0, NULL, NULL, NULL, &tv.it_value);
		       }
		     draw_menuarrows (menuarrows[i].name, +1);
#ifndef DEBUG_MENUARROWS
		     switch (arr->type) {
		      case MenuAction:
			tt_write (arr->str, arr->len);
			break;

		      case MenuTerminalAction:
			cmd_write (arr->str, arr->len);
			break;

		      default:
			if ((def [2] = menuarrows[i].key) != 0)
			  tt_write (def, 3);
			break;
		     }
#else	/* DEBUG_MENUARROWS */
		     printf ("'%c': ", menuarrows[i].name);
		     switch (arr->type) {
		      case MenuAction:
		      case MenuTerminalAction:
			printf ("%s\n", arr->str);
			break;

		      default:
			printf ("\\033[%c (default)\n", menuarrows[i].key);
			break;
		     }
#endif	/* DEBUG_MENUARROWS */
		     return;
		  }
	     }
	}
      /*drop*/

    default:
      /*
       * press menubar or move to a new entry
       */
      if (menu != NULL && menu != ActiveMenu)
	{
	   menu_hide_all ();		/* pop down old menu */
	   ActiveMenu = menu;
	   menu_show ();		/* pop up new menu */
	}
      break;
   }
}

void
menubar_control (XButtonEvent * ev)
{
   switch (ev->type) {
    case ButtonPress:
      if (ev->button == Button1)
	menubar_select (ev);
      break;

    case ButtonRelease:
      if (ev->button == Button1)
	menu_select (ev);
      break;

    case MotionNotify:
      while (XCheckTypedWindowEvent (Xdisplay, TermWin.parent,
				     MotionNotify, ev));

      if (ActiveMenu)
	while (menu_select (ev));
      else
	ev->y = -1;
      if (ev->y < 0)
	{
	   Window unused_root, unused_child;
	   int unused_root_x, unused_root_y;
	   unsigned int unused_mask;

	   XQueryPointer (Xdisplay, menuBar.win,
			  &unused_root, &unused_child,
			  &unused_root_x, &unused_root_y,
			  &(ev->x), &(ev->y),
			  &unused_mask);
	   menubar_select (ev);
	}
      break;
   }
}
#endif	/* NO_MENUBAR */
/*----------------------- end-of-file (C source) -----------------------*/
