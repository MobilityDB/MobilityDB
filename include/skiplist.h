/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file skiplist.h
 * Skiplist data structure used for performing aggregates
 */

#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "temporal.h"

/*****************************************************************************/

/* Constants defining the behaviour of skip lists which are internal types
   for computing aggregates */

#define SKIPLIST_MAXLEVEL 32  // maximum possible is 47 with current RNG
#define SKIPLIST_INITIAL_CAPACITY 1024
#define SKIPLIST_GROW 1       // double the capacity to expand the skiplist
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
} Elem;

typedef enum
{
  TIMESTAMPTZ,
  PERIOD,
  TEMPORAL
} ElemType;

/**
 * Structure to represent skiplists that keep the current state of an aggregation
 */
typedef struct
{
  ElemType elemtype;
  int capacity;
  int next;
  int length;
  int *freed;
  int freecount;
  int freecap;
  int tail;
  void *extra;
  size_t extrasize;
  Elem *elems;
} SkipList;

/*****************************************************************************/

extern Datum tagg_serialize(PG_FUNCTION_ARGS);
extern Datum tagg_deserialize(PG_FUNCTION_ARGS);

/*****************************************************************************/

extern SkipList *skiplist_make(FunctionCallInfo fcinfo, void **values,
  ElemType elemtype, int count);
extern void *skiplist_headval(SkipList *list);
extern void skiplist_splice(FunctionCallInfo fcinfo, SkipList *list,
  void **values, int count, Datum (*func)(Datum, Datum), bool crossings);
extern void **skiplist_values(SkipList *list);
extern void aggstate_set_extra(FunctionCallInfo fcinfo, SkipList *state,
  void *data, size_t size);

/*****************************************************************************/

/* Functions for splicing the skiplist */

TimestampTz *timestamp_agg(TimestampTz *times1, int count1, TimestampTz *times2,
  int count2, int *newcount);
Period **period_agg(Period **periods1, int count1, Period **periods2, 
  int count2, int *newcount);
TInstant **tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2,
  int count2, Datum (*func)(Datum, Datum), int *newcount);
TSequence **tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2,
  int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount);

/*****************************************************************************/

#endif
