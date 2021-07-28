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
 * @file tnumber_gist.c
 * R-tree GiST index for temporal integers and temporal floats
 */

#ifndef __TNUMBER_GIST_H__
#define __TNUMBER_GIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum tbox_gist_union(PG_FUNCTION_ARGS);
extern Datum tbox_gist_penalty(PG_FUNCTION_ARGS);
extern Datum tbox_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum tnumber_gist_consistent(PG_FUNCTION_ARGS);
extern Datum tnumber_gist_compress(PG_FUNCTION_ARGS);
extern Datum tbox_gist_same(PG_FUNCTION_ARGS);

/* The following functions are also called by tpoint_gist.c */
extern int interval_cmp_lower(const void *i1, const void *i2);
extern int interval_cmp_upper(const void *i1, const void *i2);
extern float non_negative(float val);

/* The following functions are also called by tnumber_spgist.c */
extern bool tbox_index_consistent_leaf(const TBOX *key, const TBOX *query,
  StrategyNumber strategy);

/*****************************************************************************/

#endif
