  /*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @file
 * @brief R-tree GiST index and Quad-tree SP-GiST index for temporal types
 * where only the time dimension is taken into account for indexing, currently,
 * `tbool` and `ttext`
 */

/* PostgreSQL */
#include <postgres.h>
#include <access/gist.h>
/* MEOS */
#include <meos.h>
#include "general/span.h"
/* MobilityDB */
#include "pg_general/temporal.h" /* For temporal_bbox_slice */

/*****************************************************************************
 * GiST compress method for temporal values
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_gist_compress);
/**
 * @brief GiST compress method for temporal values
 */
Datum
Temporal_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    Span *s = palloc(sizeof(Span));
    temporal_bbox_slice(entry->key, s);
    gistentryinit(*retval, PointerGetDatum(s), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_spgist_compress);
/**
 * @brief SP-GiST compress function for temporal values
 */
Datum
Temporal_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/
