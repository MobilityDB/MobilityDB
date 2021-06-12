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
 * @file temporal_spgist.c
 * Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * These functions are based on those in the file `rangetypes_spgist.c`.
 */

#if MOBDB_PGSQL_VERSION >= 110000

#include "temporal_spgist.h"

#include <assert.h>
#include <access/spgist.h>
#include <utils/builtins.h>

#include "timetypes.h"
#include "tempcache.h"
#include "period.h"
#include "time_gist.h"
#include "time_spgist.h"
#include "temporaltypes.h"

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(sptemporal_gist_compress);
/**
 * SP-GiST compress function for temporal values
 */
PGDLLEXPORT Datum
sptemporal_gist_compress(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Period *period = palloc(sizeof(Period));

  temporal_bbox(period, temp);

  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_PERIOD(period);
}
#endif

/*****************************************************************************/
