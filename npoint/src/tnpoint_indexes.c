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
 * @file tnpoint_indexes.c
 * R-tree GiST and SP-GiST indexes for temporal network points.
 */

#include "tnpoint_indexes.h"

#include <access/gist.h>
#include "tpoint.h"
#include "tnpoint.h"

/*****************************************************************************
 * GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_gist_compress);

PGDLLEXPORT Datum
tnpoint_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY* entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
      Temporal *temp = DatumGetTemporal(entry->key);
    STBOX *box = palloc0(sizeof(STBOX));
    temporal_bbox(box, temp);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * SP-GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_spgist_compress);

PGDLLEXPORT Datum
tnpoint_spgist_compress(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *result = palloc0(sizeof(STBOX));
  temporal_bbox(result, temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
