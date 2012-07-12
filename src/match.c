/************************************************************************
 *   IRC - Internet Relay Chat, src/match.c
 *   Copyright (C) 1990 Jarkko Oikarinen
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Changes:
 * Gerhard Mack<gmack@innerfire.net> Feb 18, 2002
 * Ported to acidblood
 *
 * Thomas Helvey <tomh@inxpress.net> June 23, 1999
 * Const correctness changes
 * Cleanup of collapse and match
 * Moved static calls variable to match
 * Added asserts for null pointers
 * $Id: match.c,v 1.2 2007/01/12 03:47:17 gmack Exp $
 *
 */
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "acidblood.h"
#include "acidfuncs.h"
/*
**  Compare if a given string (name) matches the given
**  mask (which can contain wild cards: '*' - match any
**  number of chars, '?' - match any single character.
**
**      return  1, if match
**              0, if no match
*/
/*
** match()
** Iterative matching function, rather than recursive.
** Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
*/
/* behavior change - (Thomas Helvey <tomh@inxpress.net>)
 * removed escape handling, none of the masks used with this
 * function should contain an escape '\\' unless you are searching
 * for one, it is no longer possible to escape * and ?. 
 *
 * - This is no longer the case. We can use match on gecos which we could
 * legitimately want to use an escape. So I re-enabled escaping - A1kmm.
 *
 * Moved calls rollup to function body, since match isn't recursive
 * there isn't any reason to have it exposed to the file, this change
 * also has the added benefit of making match reentrant. :)
 * Added asserts, mask and name cannot be null.
 * Changed ma and na to unsigned to get rid of casting.
 *
 * NOTICE: match is now a boolean operation, not a lexical comparison
 * if a line matches a mask, true (1) is returned, otherwise false (0)
 * is returned.
 */
#define MATCH_MAX_CALLS 512  /* ACK! This dies when it's less that this
                                and we have long lines to parse */
int match(const char *mask, const char *name)
{
  const unsigned char* m = (const unsigned char*)  mask;
  const unsigned char* n = (const unsigned char*)  name;
  const unsigned char* ma = (const unsigned char*) mask;
  const unsigned char* na = (const unsigned char*) name;
  int   wild  = 0;
  int   calls = 0;
  int   quote = 0;
  assert(0 != mask);
  assert(0 != name);
  if (!mask || !name)
    return 0;
  while (calls++ < MATCH_MAX_CALLS) {
    if (quote)
      quote++;
    if (quote == 3)
      quote = 0;
    if (*m == '\\' && !quote)
      {
       m++;
       quote = 1;
       continue;
      }
    if (!quote && *m == '*') {
      /*
       * XXX - shouldn't need to spin here, the mask should have been
       * collapsed before match is called
       */
      while (*m == '*')
        m++;
      if (*m == '\\')
        {
          m++;
          /* This means it is an invalid mask -A1kmm. */
          if (!*m)
            return 0;
          quote = 2;
        }
      wild = 1;
      ma = m;
      na = n;
    }

    if (!*m) {
      if (!*n)
        return 1;
      if (quote)
        return 0;
      for (m--; (m > (const unsigned char*) mask) && (*m == '?'); m--)
        ;
      if (*m == '*' && (m > (const unsigned char*) mask))
        return 1;
      if (!wild)
        return 0;
      m = ma;
      n = ++na;
    }
    else if (!*n) {
      /*
       * XXX - shouldn't need to spin here, the mask should have been
       * collapsed before match is called
       */
      if (quote)
        return 0;
      while (*m == '*')
        m++;
      return (*m == 0);
    }
    if (tolower(*m) != tolower(*n) && !(!quote && *m == '?')) {
      if (!wild)
        return 0;
      m = ma;
      n = ++na;
    }
    else {
      if (*m)
        m++;
      if (*n)
        n++;
    }
  }
  return 0;
}
