/*--------------------------------*-C-*---------------------------------*
 * File:	debug.c
 *
 * This module is all original work by Robert Nation
 * <nation@rocket.sanders.lockheed.com>
 *
 * Copyright 1994, Robert Nation, no rights reserved.
 * The author guarantees absolutely nothing about anything, anywhere, anytime
 * Use this code at your own risk for anything at all.
 *
 * Additional modifications by mj olesen <olesen@me.QueensU.CA>
 * No additional restrictions.
 *
 * Modifications by Raul Garcia Garcia <rgg@tid.es>
 *----------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "feature.h"
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

#ifndef DEBUG_MALLOC

/*
 * malloc and check for NULL return
 */
void *
my_malloc (size_t size)
{
   void * ptr;
   if ((ptr = malloc (size)) == NULL)
     {
	printf ("malloc error: size 0x%X\n",
		(int) size);
	abort ();
     }
   return ptr;
}

/*
 * calloc and check for NULL return
 */
void *
my_calloc (size_t count, size_t size)
{
   void	* ptr;

   if ((ptr = calloc (count, size)) == NULL)
     {
	printf ("calloc error: count * size 0x%X\n",
		(int) (count * size));
	abort ();
     }
   return ptr;
}

/*
 * realloc and check for NULL return
 */
void *
my_realloc (void * mem, size_t size)
{
   void	* ptr;
   /*
    * do this since some implementations (Sun for one) gags on realloc with
    * a NULL pointer
    */
   if ((ptr = (mem ? realloc (mem, size) : malloc (size))) == NULL)
     {
	printf ("realloc error: size 0x%X\n", (int) size);
	abort ();
     }
   return ptr;
}

void
my_free (void * mem)
{
   if (mem != NULL)		/* in case free() is non-compilant */
     free (mem);
}

#else	/* DEBUG_MALLOC */

#define MAX_MALLOCS	32
/* Number of pad characters to use when checking for out-of-bounds */
#define NPAD		4
#define PADCHAR1	0x3E
#define PADCHAR2	0x3F

static short num_alloc = 0;
static struct {
   void * base;
   const char * name;
} Pointer [MAX_MALLOCS];

/*
 * realloc that checks for NULL return, and adds out-of-bounds checking if
 * DEBUG_MALLOC is set
 */
void *
safe_malloc (size_t size, const char * id)
{
   void * ptr;
#ifdef DEBUG_MALLOC
   char * tmp;
   int i;

   if ((ptr = malloc (size + 2 * NPAD * sizeof(int))) == NULL)
     {
	abort ();
     }

   tmp = ptr;
   for (i = 0; i < NPAD * sizeof(int); i++) tmp [i] = PADCHAR1;
   tmp += size + NPAD * sizeof(int);
   for (i = 0; i < NPAD * sizeof(int); i++) tmp [i] = PADCHAR2;
   *((int *) ptr + NPAD - 1) = size;	/* save the size */

   fprintf (stderr, "malloc: 0x%08X [%u bytes]: \"%s\"\n",
	    (unsigned int) ptr, (unsigned int) size, id);

   for (i = 0; i < MAX_MALLOCS; i++)
     {
	if (Pointer[i].base == NULL)
	  {
	     num_alloc++;
	     Pointer[i].base = ptr;
	     Pointer[i].name = id;
	     safe_mem_check ("malloc", id);
	     break;
	  }
     }

   if (i >= MAX_MALLOCS)
     fprintf (stderr, "MAX_MALLOCS (%d) exceeded\n", MAX_MALLOCS);

   return ((void *) ((int *)ptr + NPAD));
#else
   /*
    * do this since some implementations (Sun for one) gags on realloc with
    * a NULL pointer
    */
   if ((ptr = malloc (size)) == NULL)
     {
	printf ("malloc error: [%u bytes]\n", (unsigned int) size);
	abort ();
     }
   return ptr;
#endif
}

/*
 * Calloc that checks for NULL return, and adds out-of-bounds
 * checking if DEBUG_MALLOC is set.
 */
void *
safe_calloc (size_t count, size_t size, const char * id)
{
   void	* ptr;

   ptr = safe_malloc (count * size, id);
   memset (ptr, 0, size);
   return ptr;
}

/*
 * Realloc that checks for NULL return, and adds out-of-bounds
 * checking if DEBUG_MALLOC is set.
 */
void *
safe_realloc (void * mem, size_t size, const char * id)
{
   void	* ptr;

   ptr = safe_malloc (size, id);
   if (mem != NULL)
     {
	int oldsz = *((int *)mem - 1);

	if (oldsz > size)
	  oldsz = size;

	memcpy (ptr, mem, oldsz);
	safe_free (mem, "realloc", id);
     }
   return ptr;
}

/*
 * Free command good for use with above malloc, checks for out-of-bounds
 * before freeing.
 */
void
safe_free (void * ptr, const char * id1, const char * id2)
{
   if (ptr != NULL)
     {
#ifdef DEBUG_MALLOC
	/* Check each memory region */

	unsigned int i;
	ptr = (void *)((int *)ptr - NPAD);
	i = *((unsigned int *) ptr + NPAD - 1);

	fprintf (stderr, "free: 0x%08X [%u bytes]",
		 (unsigned int) ptr, i);

	for (i = 0; i < MAX_MALLOCS; i++)
	  {
	     if (Pointer[i].base == ptr)
	       {
		  fprintf (stderr,": \"%s\"\n", Pointer[i].name);
		  safe_mem_check (id1, id2);
		  Pointer[i].base = NULL;
		  num_alloc--;
		  break;
	       }
	  }
#endif
	free (ptr);
     }
}

/*
 * Check all allocated memory for out-of-bounds memory usage
 */
void
safe_mem_check (const char * id1, const char * id2)
{
#ifdef DEBUG_MALLOC
   int i, fail = 0;
   static const char * prev_id1 = NULL, * prev_id2 = NULL;

   char * msg_fmt = "Ouch%d: at 0x%08X (size %d) [idx %d]: \"%s\" (%s/%s)\n";
   char * last_chk = "Last successful check (%s/%s)\n";

   if (num_alloc <= 0)
     return;

   for (i = 0; i < MAX_MALLOCS; i++)
     {
	/* Check each memory region */
	char * ptr = Pointer[i].base;
	int j, size;

	if (ptr == NULL)
	  continue;
	size = *((int *) ptr + NPAD - 1);

	for (j = 0; j < ((NPAD-1) * sizeof(int)); j++)
	  {
	     if (ptr [j] == PADCHAR1)
	       continue;
	     fail = 1;
	     fprintf (stderr, msg_fmt,
		      fail,
		      (unsigned int) Pointer[i].base, size, j,
		      Pointer[i].name,
		      id1, id2);
	     if (prev_id1 != NULL)
	       fprintf (stderr, last_chk, prev_id1, prev_id2);
	  }

	ptr += size + NPAD * sizeof(int);
	for (j = 0; j < NPAD * sizeof(int); j++)
	  {
	     if (ptr [j] == PADCHAR2)
	       continue;
	     fail = 2;
	     fprintf (stderr, msg_fmt,
		      fail,
		      (unsigned int) Pointer[i].base, size, j,
		      Pointer[i].name,
		      id1, id2);
	     if (prev_id1 != NULL)
	       fprintf (stderr, last_chk, prev_id1, prev_id2);
	  }
     }

   if (!fail)
     {
	prev_id1 = id1;
	prev_id2 = id2;
     }
#endif
}

#endif	/* DEBUG_MALLOC */
/*----------------------- end-of-file (C source) -----------------------*/
