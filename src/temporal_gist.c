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
 * @file temporal_gist.c
 * R-tree GiST index for temporal types where only the time dimension
 * is taken into account for indexing, currently, tbool and ttext.
 */

#include "temporal_gist.h"

#include <access/gist.h>

#include "timetypes.h"
#include "temporal.h"
#include "tempcache.h"
#include "time_gist.h"

/*****************************************************************************
 * GiST compress method for temporal values
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_gist_compress);
/**
 * GiST compress method for temporal values
 */
PGDLLEXPORT Datum
temporal_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);

  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    Temporal *temp = DatumGetTemporal(entry->key);
    Period *period = palloc0(sizeof(Period));
    temporal_bbox(period, temp);
    gistentryinit(*retval, PointerGetDatum(period),
      entry->rel, entry->page, entry->offset, false);
    PG_RETURN_POINTER(retval);
  }

  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST decompress method for temporal values
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION < 110000
PG_FUNCTION_INFO_V1(temporal_gist_decompress);
/**
 * GiST decompress method for temporal values (result in a period)
 */
PGDLLEXPORT Datum
temporal_gist_decompress(PG_FUNCTION_ARGS)
{
  GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(entry);
}
#endif

/*****************************************************************************/
