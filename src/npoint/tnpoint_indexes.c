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
 * @file tnpoint_indexes.c
 * @brief R-tree GiST and quad-tree SP-GiST indexes for temporal network
 * points.
 */

#include "npoint/tnpoint_indexes.h"

/* PostgreSQL */
#include <access/gist.h>
/* MobilityDB */
#include "point/tpoint.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_gist_compress);
/**
 * GiST compress function for temporal network points
 */
PGDLLEXPORT Datum
Tnpoint_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY* entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
    STBOX *box = (STBOX *) palloc(sizeof(STBOX));
    temporal_bbox_slice(entry->key, box);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * SP-GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnpoint_spgist_compress);
/**
 * SP-GiST compress function for temporal network points
 */
PGDLLEXPORT Datum
Tnpoint_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  STBOX *result = (STBOX *) palloc(sizeof(STBOX));
  temporal_bbox_slice(tempdatum, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
