#ifndef AU_STACK
#define AU_STACK
#include "au_glob.h"

template <class T>
class Stack
{
 public:
  T o;
  Stack *next;
  Stack() {next=NULL;};
  void push(T t)
  {
   Stack<T> *s=new Stack<T>;
   s->o=t;
   s->next=next;
   next=s;
  };

 T pop()
  {
   T o;
   Stack<T> *s=next;
   if(s)
   {
    o=next->o;
    next=next->next;
    delete s;
    return o;
   }
   printf("\nStack Error: Pop from empty stack!\n");
   exit(-1);
   return o;
  };
 int is_empty() {return (next==NULL) ? 1:0;};
 void print(void (*f)(void *))
   {if(next) {f(&next->o);next->print(f);};};
 T last() {if(next) return next->o;};
};

#endif
/* ------------ End of file -------------- */

