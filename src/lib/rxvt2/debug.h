/*--------------------------------*-C-*---------------------------------*
 * File:	debug.h
 *
 * This module is all original work by Robert Nation
 * <nation@rocket.sanders.lockheed.com>
 *
 * Copyright 1994, Robert Nation, no rights reserved.
 *
 * You can do what you like with this source code provided you don't make
 * money from it and you include an unaltered copy of this message
 * (including the copyright).  As usual, the author accepts no
 * responsibility for anything, nor does he guarantee anything whatsoever.
 *
 * Modified by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions.
 *
 * Modifications by Raul Garcia Garcia <rgg@tid.es>
 *----------------------------------------------------------------------*/
#ifndef _DEBUG_H
#define _DEBUG_H
#include <X11/Xfuncproto.h>

/*{{{ prototypes */
_XFUNCPROTOBEGIN

extern void *
safe_malloc (size_t /* size */,
	     const char * /* id */);

extern void *
safe_calloc (size_t /* count */,
	     size_t /* size */,
	     const char * /* id */);

extern void *
safe_realloc (void * /* mem */,
	      size_t /* size */,
	      const char * /* id */);

extern void
safe_free (void * /* mem */,
	   const char * /* id1 */,
	   const char * /* id2 */);

extern void
safe_mem_check (const char * /* id1 */,
		const char * /* id2 */);

extern void *
my_malloc (size_t /* size */);

extern void *
my_calloc (size_t /* count */,
	   size_t /* size */);

extern void *
my_realloc (void * /* mem */,
	    size_t /* size */);

extern void
my_free (void * /* mem */);

_XFUNCPROTOEND
/*}}}*/

/*{{{ macros */
#ifdef DEBUG_MALLOC
# define MALLOC(sz,id)		safe_malloc ((sz),id)
# define CALLOC(n,sz,id)	safe_calloc ((n),(sz),id)
# define REALLOC(mem,sz,id)	safe_realloc ((mem),(sz),id)
# define FREE(mem,id,fn)	safe_free ((mem),id,fn)
# define MEM_CHECK(id1,id2)	safe_mem_check (id1,id2)
#else
# define MALLOC(sz,id)		my_malloc (sz)
# define CALLOC(n,sz,id)	my_calloc ((n),(sz))
# define REALLOC(mem,sz,id)	my_realloc ((mem),(sz))
# define FREE(ptr,id,fn)	my_free (ptr)
# define MEM_CHECK(id1,id2)	((void)0)
#endif
/*}}}*/
#endif	/* whole file */
/*----------------------- end-of-file (C header) -----------------------*/
/* ------------ End of file -------------- */

