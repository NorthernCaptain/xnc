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
#include "au_tree.h"

/// Add new leaf of the tree.
/// Return 1 if success, 0 otherwise.
int Tree::add(Tree* o)
{
  int res;

  res = strcmp(o->name, name);
  if (res < 0) {
    if (left)
      left->add(o);
    else {
      left = o;
    }
    return 1;
  } else if (res > 0) {
    if (right)
      right->add(o);
    else {
      right = o;
    }
    return 1;
  }
  return 0;
}

///Look trought the tree and find o.
///Return address if success, NULL if object not found.
Tree* Tree::find(char *o)
{
  int res;

  res = strcmp(o, name);
  if (res < 0) {
    if (left)
      return left->find(o);
    else
      return NULL;
  } else if (res > 0) {
    if (right)
      return right->find(o);
    else
      return NULL;
  }
  return this;
}

///List 'name' field and walk thought the tree
void Tree::ls_node()
{
  if (left)
    left->ls_node();
  printf("%-30s %d\n", name, data);
  if (right)
    right->ls_node();
}

////////////End of Tree...
///List 'name','type' and 'val' fields and walk thought the tree
void WTree::ls_node()
{
  if (left)
    left->ls_node();
  if (verbose) {
    printf("%-30sreal%12.3f",name,val);
    if (chg)
      printf("\n");
    else
      printf("    Not used...\n");
  }
  if (right)
    right->ls_node();
}
////////////End of WTree...

