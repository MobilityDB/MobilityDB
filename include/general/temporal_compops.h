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
 * @file temporal_compops.h
 * Temporal comparison operators (=, <>, <, >, <=, >=).
 */

#ifndef __TEMPORAL_COMPOPS_H__
#define __TEMPORAL_COMPOPS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

#include "temporal.h"

/*****************************************************************************/

extern Datum teq_base_temporal(PG_FUNCTION_ARGS);
extern Datum teq_temporal_base(PG_FUNCTION_ARGS);
extern Datum teq_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tne_base_temporal(PG_FUNCTION_ARGS);
extern Datum tne_temporal_base(PG_FUNCTION_ARGS);
extern Datum tne_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tlt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tle_base_temporal(PG_FUNCTION_ARGS);
extern Datum tle_temporal_base(PG_FUNCTION_ARGS);
extern Datum tle_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tgt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tge_base_temporal(PG_FUNCTION_ARGS);
extern Datum tge_temporal_base(PG_FUNCTION_ARGS);
extern Datum tge_temporal_temporal(PG_FUNCTION_ARGS);

extern Temporal * tcomp_temporal_base1(const Temporal *temp, Datum value,
  Oid datumtypid, Datum (*func)(Datum, Datum, Oid, Oid), bool invert);

/*****************************************************************************/

#endif
