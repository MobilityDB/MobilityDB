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
 * @file time_aggfuncs.h
 * Aggregate functions for time types
 */

#ifndef __TIME_AGGFUNCS_H__
#define __TIME_AGGFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "temporal.h"
#include "temporal_util.h"

/*****************************************************************************/

/**
 * Structure to represent elements in the skiplists
 */
 
typedef union ValueElem
{
  Period *p;
  TimestampTz t;
} ValueElem;

typedef struct
{
  ValueElem value;
  int height;
  int next[SKIPLIST_MAXLEVEL];
} TimeElem;

typedef enum
{
  TIMESTAMPTZ,
  TIMESTAMPSET,
  PERIOD,
  PERIODSET
} TimeType;

/**
 * Structure to represent skiplists that keep the current state of an aggregation
 */
typedef struct
{
  TimeType timetype;
  int capacity;
  int next;
  int length;
  int *freed;
  int freecount;
  int freecap;
  int tail;
  TimeElem *elems;
} TimeSkipList;

/*****************************************************************************/

extern Datum time_agg_serialize(PG_FUNCTION_ARGS);
extern Datum time_agg_deserialize(PG_FUNCTION_ARGS);

extern Datum timestampset_tunion_transfn(PG_FUNCTION_ARGS);
extern Datum period_tunion_transfn(PG_FUNCTION_ARGS);
extern Datum periodset_tunion_transfn(PG_FUNCTION_ARGS);

extern Datum time_tunion_combinefn(PG_FUNCTION_ARGS);

extern Datum timestamp_tunion_finalfn(PG_FUNCTION_ARGS);
extern Datum period_tunion_finalfn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
