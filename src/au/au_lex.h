#ifndef AU_LEX
#define AU_LEX
#include "au_glob.h"
#include "au_tree.h"
#include "au_stack.h"

/* Lexem Demon for converting input stream
   into a stream of lexems.
   Need translation table ('trs') and list of terms.
   Predefined lexems:
     c - constant
     s - string
   Before starting LexDemon generates tree of reserved words,
   which will be taken from translation table.
   So use 'add_reserved_word' for adding to this tree.
*/

struct lexdata_s 
{
  const char *string;
  int         lexem;
};

class LexDemon
{
public:
  char wlast[1024];      // last word
  int con;             // constant value
  char *lastinp;

  LexDemon(char *iterms, char **itrs);
  LexDemon(struct lexdata_s *iterms);
  
  void set_input_stream(char *inp) { input = inp; };
  char process_lex();
  void unlex(char ch) { lexch = ch; };
  void add_reserved_word(const char *w, int lexem);
  char *get_current() { return input; };
  void set_terminators(char *ss) { strcpy(internal_str, ss); };
  void disable_const_detection() { const_enabled = 0; };

  ~LexDemon() {
    if (res)
      delete res;
    if (allocated)
      delete[] terms;
  };
  
private:
  Tree  *res;
  struct lexdata_s *terms;
  int    allocated;
  char   internal_str[20];   // Up to 20 terminators may be defined.
  char   lexch;
  int    maxt;
  int    const_enabled;
  char  *input;              // input string

  int is_const(char *w);
  int is_t(char ch);
  char* comments(char *b);
  char *process_word(char *buf,char *w);
  char *process_words(char *buf,char *w);
};

#endif
/* ------------ End of file -------------- */

