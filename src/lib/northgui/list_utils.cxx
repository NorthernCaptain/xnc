/****************************************************************************
*  Copyright (C) 1996-2002 by Leo Khramov
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
#include "gui_base.h"

//Lists for storing active Gui objects
List   el, l_exp;

Gui   *find(Window ww)
{
  List  *cur = el.next;
  while (cur != NULL)
    if (cur->w == ww)
      break;
    else
      cur = cur->next;
  if (cur)
    return cur->obj;
  return NULL;
}


Gui   *findexp(Window ww)
{
  List  *cur = l_exp.next;
  while (cur != NULL)
    if (cur->w == ww)
      break;
    else
      cur = cur->next;
  if (cur)
    return cur->obj;
  return NULL;
}

void   addto_exp(Gui * o, Window w)
{
  List  *cur = new List;
  cur->obj = o;
  cur->w = w;
  cur->next = l_exp.next;
  if (l_exp.next)
    l_exp.next->prev = cur;
  l_exp.next = cur;
  cur->prev = &l_exp;
}

void   addto_el(Gui * o, Window w)
{
  List  *cur = new List;
  cur->obj = o;
  cur->w = w;
  cur->next = el.next;
  if (el.next)
    el.next->prev = cur;
  el.next = cur;
  cur->prev = &el;
  addto_exp(o, w);
}

void   delfrom_exp(Gui * o)
{
  List  *cur = l_exp.next;
  while (cur != NULL)
    if (cur->obj == o)
      break;
    else
      cur = cur->next;
  if (cur != NULL)
    {
      cur->prev->next = cur->next;
      if (cur->next)
        cur->next->prev = cur->prev;
      delete cur;
    }
}

void   delfrom_el(Gui * o)
{
  List  *cur = el.next;
  while (cur != NULL)
    if (cur->obj == o)
      break;
    else
      cur = cur->next;
  if (cur != NULL)
    {
      cur->prev->next = cur->next;
      if (cur->next)
        cur->next->prev = cur->prev;
      delete cur;
    }
}

void   delall_el()
{
  List  *cur, *cur2;
  cur = el.next;
  while (cur != NULL)
    {
      cur2 = cur->next;
      delete cur;
      cur = cur2;
    }
}

void   delall_exp()
{
  List  *cur, *cur2;
  cur = l_exp.next;
  while (cur != NULL)
    {
      cur2 = cur->next;
      delete cur;
      cur = cur2;
    }
}

