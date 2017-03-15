/* $Id: rng.h 55 2005-09-13 22:29:52Z asminer $ */
/* -------------------------------------------------------------------------  
 * Name            : rng.h  (header file for the library file rng.c) 
 * Author          : Steve Park & Dave Geyer  
 * Language        : ANSI C 
 * Latest Revision : 09-11-98
 * ------------------------------------------------------------------------- 
 */

#if !defined( _RNG_ )
#define _RNG_

double Random(void);
void   GetSeed(long *x);
void   PutSeed(long x);
void   TestRandom(void);

#endif
