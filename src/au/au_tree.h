#ifndef AU_TREE
#define AU_TREE
#include "au_glob.h"


//////////////////////////////Tree classes///////////////////////////////////
///Base class for organizing binary tree.
///Use name as key of nodes.
class Tree
{
public:
  char *name;             // key of the node
  int   name_allocated;   // flag: true if name was dynamically allocated
  int   data;             // additional (user) data
  
  Tree() {
     left = right = NULL;
     name = NULL;
     data = 0;
     name_allocated = 0;
  };
  Tree(const char *o, int dat = 0) {
     left = right = NULL;
     name = new char[strlen(o)+1];
     name_allocated = 1;
     strcpy(name, o);
     data = dat;
  };

  virtual int add(Tree *o);
  virtual Tree* find(char *key);
  virtual void ls_node();

  ~Tree() {
    if (left)
      delete left;
    if (right)
      delete right;
    if (name_allocated && name)
      delete[] name;
  };
   
protected:
  Tree *left, *right;
};
 
////WTree class
////This class is extension of Tree class
////Need for building table of variables
class WTree : public Tree
{
public:
  float val;
  char  type;
  char  chg;

  WTree() : Tree() {
     val = 0.0;
     type = 0;
     chg = 0;
  };
  WTree(const char *n) : Tree(n) {
     val = 0.0;
     type = 1;
     chg = 0;
  };
  virtual void ls_node();
 };
#endif
//////////////////////////End of file////////////////////////////////

/* ------------ End of file -------------- */

