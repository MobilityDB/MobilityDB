/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Lat/Lng-conversion functions of temporal H3 cell indices built only
 * in MEOS
 * @details The functions take and return typed values and reach the internal
 * functions of @p th3index_latlng.c, which carry the cells as datums.
 */

/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "h3/h3index.h"
#include "h3/th3index_internal.h"

/*****************************************************************************
 * Split
 *****************************************************************************/

/**
 * @brief Return the H3 cells of an array of datums, freeing the array
 */
static H3Index *
h3index_cells(Datum *datums, int count)
{
  H3Index *result = palloc(sizeof(H3Index) * count);
  for (int i = 0; i < count; i++)
    result[i] = DatumGetH3Index(datums[i]);
  pfree(datums);
  return result;
}

/**
 * @ingroup meos_h3_latlng
 * @brief Return the fragments of a temporal planar point split by the H3
 * cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution H3 resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeompoint_h3index_split()
 */
Temporal **
tgeompoint_h3index_split(const Temporal *temp, int32 resolution,
  bool border_inc, H3Index **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_h3index_split(temp, resolution, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = h3index_cells(datum_cells, *count);
  return result;
}

/**
 * @ingroup meos_h3_latlng
 * @brief Return the fragments of a temporal geodetic point split by the H3
 * cells it crosses at a resolution, and the cell of each
 * @param[in] temp Temporal point
 * @param[in] resolution H3 resolution
 * @param[in] border_inc True when the grid contains the upper border of the
 * extent
 * @param[out] cells Cell of each fragment
 * @param[out] count Number of fragments
 * @csqlfn #Tgeogpoint_h3index_split()
 */
Temporal **
tgeogpoint_h3index_split(const Temporal *temp, int32 resolution,
  bool border_inc, H3Index **cells, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOGPOINT(temp, NULL); VALIDATE_NOT_NULL(cells, NULL);

  Datum *datum_cells;
  Temporal **result = tpoint_h3index_split(temp, resolution, border_inc,
    &datum_cells, count);
  if (! result)
    return NULL;
  *cells = h3index_cells(datum_cells, *count);
  return result;
}

/*****************************************************************************/
