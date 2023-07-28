/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * These functions are based on those in the file `rangetypes_spgist.c`.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/spgist.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"
#include "general/span.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/time_gist.h"

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
