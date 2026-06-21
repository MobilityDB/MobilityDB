/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Datum-convention wrappers for the quadbin static kernel and the
 * `quadbin_cellops` descriptor that plugs quadbin into the shared temporal
 * cell-index machinery (meos/src/temporal/tcellindex.c).
 *
 * A quadbin cell is a uint64 carried in a Datum with the int8/bigint
 * convention (Int64GetDatum / DatumGetInt64). The cell centroid is emitted as
 * a planar tgeompoint in the cell's lon/lat (SRID 4326); a Web-Mercator
 * (SRID 3857) emission would set point_srid = 3857 and reproject here.
 */

#include "quadbin/quadbin.h"
#include "quadbin/tquadbin.h"

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_quadbin.h>
#include "geo/tgeo_spatialfuncs.h"
#include "temporal/meos_catalog.h"
#include <pgtypes.h>
#include "temporal/tcellindex.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"

/*****************************************************************************
 * Datum-convention static-cell wrappers
 *****************************************************************************/

static Datum
datum_quadbin_get_resolution(Datum d)
{
  return Int32GetDatum((int32) quadbin_get_resolution((Quadbin) DatumGetInt64(d)));
}

static Datum
datum_quadbin_is_valid_cell(Datum d)
{
  return BoolGetDatum(quadbin_is_valid_cell((Quadbin) DatumGetInt64(d)));
}

static Datum
datum_quadbin_cell_to_parent(Datum cell_d, Datum res_d)
{
  Quadbin parent = quadbin_cell_to_parent((Quadbin) DatumGetInt64(cell_d),
    (uint32_t) DatumGetInt32(res_d));
  return Int64GetDatum((int64) parent);
}

static Datum
datum_quadbin_cell_to_point(Datum d)
{
  double lon, lat;
  quadbin_cell_to_point((Quadbin) DatumGetInt64(d), &lon, &lat);
  /* Planar (non-geodetic) lon/lat point, SRID 4326. geopoint_make is
   * available in both the MEOS and the MEOS=OFF extension build, unlike the
   * MEOS-only geompoint_make2d. */
  GSERIALIZED *gs = geopoint_make(lon, lat, 0.0, false, false, 4326);
  return PointerGetDatum(gs);
}

static Datum
datum_quadbin_cell_to_boundary(Datum d)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_to_bounding_box((Quadbin) DatumGetInt64(d), &xmin, &ymin,
    &xmax, &ymax);
  POINTARRAY *pa = ptarray_construct_empty(LW_FALSE, LW_FALSE, 5);
  POINT4D pt;
  pt.z = 0.0; pt.m = 0.0;
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE); /* close */
  LWPOLY *poly = lwpoly_construct_empty(4326, LW_FALSE, LW_FALSE);
  lwpoly_add_ring(poly, pa);
  GSERIALIZED *gs = geo_serialize(lwpoly_as_lwgeom(poly));
  lwpoly_free(poly);
  return PointerGetDatum(gs);
}

static Datum
datum_quadbin_cell_area(Datum d)
{
  return Float8GetDatum(quadbin_cell_area((Quadbin) DatumGetInt64(d)));
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief Quadbin operations descriptor consumed by `dggs_cellops()`.
 */
const DggsCellOps quadbin_cellops =
{
  .celltype        = T_QUADBIN,
  .settype         = T_QUADBINSET,
  .temptype        = T_TQUADBIN,
  .min_resolution  = 0,
  .max_resolution  = 26,
  .point_temptype  = T_TGEOMPOINT,
  .point_srid      = 4326,
  .get_resolution  = &datum_quadbin_get_resolution,
  .is_valid_cell   = &datum_quadbin_is_valid_cell,
  .cell_to_parent  = &datum_quadbin_cell_to_parent,
  .cell_to_point   = &datum_quadbin_cell_to_point,
  .cell_to_boundary = &datum_quadbin_cell_to_boundary,
  .cell_area       = &datum_quadbin_cell_area
};

/*****************************************************************************
 * Quadbin-unique temporal op: cell -> quadkey (ttext)
 *
 * The quadkey (base-4 slippy-tile string) has no H3 analogue, so it is a typed
 * tquadbin function rather than a generic DggsCellOps entry: the descriptor
 * exposes only the operations shared by every DGGS.
 *****************************************************************************/

static Datum
datum_quadbin_cell_to_quadkey(Datum d)
{
  char *str = quadbin_cell_to_quadkey((Quadbin) DatumGetInt64(d));
  text *result = cstring_to_text(str);
  pfree(str);
  return PointerGetDatum(result);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal quadkey (ttext) of a temporal quadbin cell.
 * @csqlfn #Tquadbin_cell_to_quadkey()
 */
Temporal *
tquadbin_cell_to_quadkey(const Temporal *temp)
{
  VALIDATE_TQUADBIN(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_quadbin_cell_to_quadkey;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TQUADBIN;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
