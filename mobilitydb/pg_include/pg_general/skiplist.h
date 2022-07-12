/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @brief Skiplist data structure used for performing aggregates
 */

#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/palloc.h>
#include <fmgr.h>
/* MobilityDB */
#include "general/temporal.h"

/*****************************************************************************/

/* Constants defining the behaviour of skip lists which are internal types
   for computing aggregates */

#define SKIPLIST_MAXLEVEL 32  /**< maximum possible is 47 with current RNG */
#define SKIPLIST_INITIAL_CAPACITY 1024
#define SKIPLIST_GROW 1       /**< double the capacity to expand the skiplist */
#define SKIPLIST_INITIAL_FREELIST 32

/*****************************************************************************/

/**
 * Structure to represent elements in the skiplists
 */

typedef struct
{
  void *value;
  int height;
  int next[SKIPLIST_MAXLEVEL];
} SkipListElem;

typedef enum
{
  TIMESTAMPTZ,
  PERIOD,
  TEMPORAL
} SkipListElemType;

/**
 * Structure to represent skiplists that keep the current state of an aggregation
 */
typedef struct
{
  SkipListElemType elemtype;
  int capacity;
  int next;
  int length;
  int *freed;
  int freecount;
  int freecap;
  int tail;
  void *extra;
  size_t extrasize;
  SkipListElem *elems;
} SkipList;

/**
 * Helper macros to input the current aggregate state
 */
#define INPUT_AGG_TRANS_STATE(state)  \
  do {  \
    state = PG_ARGISNULL(0) ? NULL :  \
     (SkipList *) PG_GETARG_POINTER(0);  \
    if (PG_ARGISNULL(1))  \
    {  \
      if (state)  \
        PG_RETURN_POINTER(state);  \
      else  \
        PG_RETURN_NULL();  \
    }  \
  } while (0)

#define INPUT_AGG_COMB_STATE(state1, state2)  \
  do {  \
  state1 = PG_ARGISNULL(0) ? NULL :  \
    (SkipList *) PG_GETARG_POINTER(0);  \
  state2 = PG_ARGISNULL(1) ? NULL :  \
    (SkipList *) PG_GETARG_POINTER(1);  \
  if (state1 == NULL && state2 == NULL)  \
    PG_RETURN_NULL();  \
   } while (0) 

/*****************************************************************************/

extern SkipList *skiplist_make(FunctionCallInfo fcinfo, void **values,
  int count, SkipListElemType elemtype);
extern void *skiplist_headval(SkipList *list);
extern void skiplist_splice(FunctionCallInfo fcinfo, SkipList *list,
  void **values, int count, datum_func2 func, bool crossings);
extern void **skiplist_values(SkipList *list);
extern void aggstate_set_extra(FunctionCallInfo fcinfo, SkipList *state,
  void *data, size_t size);

/*****************************************************************************/

#endif
