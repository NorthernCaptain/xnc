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
#include "au_lex.h"
///////////////////Lexem Demon class functions///////////////////////////

LexDemon::LexDemon(struct lexdata_s *iterms)
{
  terms = iterms;
  internal_str[0] = 0;
  res = NULL;
  lexch = 0;
  const_enabled = 1;
  allocated = 0;

  for (maxt = 0; terms[maxt].string; maxt++) {
    if (terms[maxt].lexem != 'i' && terms[maxt].lexem != 'c')
      add_reserved_word(terms[maxt].string, terms[maxt].lexem);
  }
}

LexDemon::LexDemon(char *iterms, char **itrs)
{
  int i;
  
  internal_str[0] = 0;
  res = NULL;
  lexch = 0;
  const_enabled = 1;

  maxt = strlen(iterms);
  terms = new struct lexdata_s[maxt+1];
  allocated = 1;
  for (i = 0; iterms[i]; i++) {
    terms[i].string = itrs[i];
    terms[i].lexem = iterms[i];
    if (terms[i].lexem != 'i' && terms[i].lexem != 'c')
      add_reserved_word(terms[i].string, terms[i].lexem);
  }
  terms[i].lexem = 0;
  terms[i].string = NULL;
}

  

int LexDemon::is_const(char *w)
{
  while (*w != 0) {
    if (*w < '0' || *w > '9')
      return 0;
    w++;
  }
  return 1;
}

char *LexDemon::comments(char *b)
{
  while (*b && (*b==' ' || *b=='\n'))
    b++;
  if (*b == '#') {
    while (*b!='\n' && *b!=0)
      b++;
    if (*b=='\n')
      return b+1;
  }
  return b;
}

char LexDemon::process_lex()
{
  char w[80];
  Tree *res_found;

  if (lexch) {
    w[0] = lexch;
    lexch = 0;
    return w[0];
  }
  do {
    input = comments(input);
    while (*input == '\n')
      input++;
  } while (*input == '#');
  lastinp = input;
  input = process_word(input, w);
  if (*w == 0)
    return 0;
  if ((res_found = res->find(w))) {
    if (verbose)
      printf("LexDemon:  %10s -> %d\n", w, res_found->data);
    return res_found->data;
  }
  if(const_enabled && is_const(w)) {
    sscanf(w, "%d", &con);
    if (verbose)
      printf("LexDemon:  %10s -> c:%d\n", w, con);
    return 'c';
  }

  strcpy(wlast, w); 
  if (verbose)
    printf("LexDemon:  %10s -> s\n",w);
  return 's';
}

int LexDemon::is_t(char ch)
{
  return strchr((internal_str[0] ? internal_str : ";:"), ch) != NULL;
}

char* LexDemon::process_words(char *buf, char *w)
{
  do {
    *w++ = *buf++;
  } while (*buf && *buf!='\n' && *buf!='\'');
  *w=0;
  return buf+1;
}

char* LexDemon::process_word(char *buf, char *w)
{
  char *ww = w;

  *w = 0;
  while (*buf==' ' || *buf=='\n' || *buf=='\r')
    buf++;
  if (*buf=='\'')
    return process_words(buf+1, w);
  do {
    *w++ = *buf++;
  } while(*buf && *buf != ' ' && *buf !='\n' && *buf != '\r'
          && is_t(w[-1]) == 0);
  if (ww+1 != w && is_t(w[-1])) {
    *(--w) = 0;
    return buf-1;
  }
  if (ww+1==w && is_t(w[-1]) && *buf == '=') {
    *w = '=';
    *(++w) = 0; 
    return buf+1;
  }
  *w = 0;
  return buf;
}

void LexDemon::add_reserved_word(const char *w, int lexem)
{
  Tree *o = new Tree(w, lexem);
  if (res)
    res->add(o);
  else
    res = o;
}
