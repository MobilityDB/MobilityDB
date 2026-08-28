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
 * @brief Datum-convention wrappers for the S2 static kernel and the
 * `s2_cellops` descriptor that plugs S2 into the shared temporal cell-index
 * machinery (meos/src/temporal/tcellindex.c).
 *
 * An S2 cell is a uint64 carried in a Datum with the int8/bigint convention
 * (Int64GetDatum / DatumGetInt64), holding the face, the position along the
 * Hilbert curve and the level of a cell of the sphere decomposed through a
 * circumscribed cube (https://s2geometry.io/). The cell is defined on the
 * sphere, so its centre and its boundary are geodetic (SRID 4326), and its
 * four edges are geodesics rather than the straight segments a planar grid
 * carries.
 */

#include "s2cell/s2cell.h"

/* C */
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_s2cell.h>
#include <pgtypes.h>
#include "temporal/meos_catalog.h"
#include "temporal/tcellindex.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Datum-convention static-cell wrappers
 *****************************************************************************/

/**
 * @brief Return the level of an S2 cell
 */
static Datum
datum_s2_get_resolution(Datum d)
{
  return Int32GetDatum((int32) s2cell_get_resolution(DatumGetS2Cell(d)));
}

/**
 * @brief Return true if a value encodes a valid S2 cell
 */
static Datum
datum_s2_is_valid_cell(Datum d)
{
  return BoolGetDatum(s2cell_is_valid_cell(DatumGetS2Cell(d)));
}

/**
 * @brief Return the ancestor of an S2 cell at a level
 */
static Datum
datum_s2_cell_to_parent(Datum cell_d, Datum res_d)
{
  S2CellId parent = s2cell_cell_to_parent(DatumGetS2Cell(cell_d),
    (uint32_t) DatumGetInt32(res_d));
  return S2CellGetDatum(parent);
}

/**
 * @brief Return the geodetic centre of an S2 cell
 */
static Datum
datum_s2_cell_to_point(Datum d)
{
  return PointerGetDatum(s2cell_cell_to_geogpoint(DatumGetS2Cell(d)));
}

/**
 * @brief Return the geodetic boundary of an S2 cell
 * @details The four vertices are the corners of the cell on the sphere, and
 * the ring closes on the first of them.
 */
static Datum
datum_s2_cell_to_boundary(Datum d)
{
  return PointerGetDatum(s2cell_cell_to_geog(DatumGetS2Cell(d)));
}

/**
 * @brief Return the area in square metres of an S2 cell
 */
static Datum
datum_s2_cell_area(Datum d)
{
  return Float8GetDatum(s2cell_cell_area(DatumGetS2Cell(d)));
}

/**
 * @brief Return the canonical token of an S2 cell
 */
static Datum
datum_s2_cell_to_token(Datum d)
{
  char *str = s2cell_cell_to_token(DatumGetS2Cell(d));
  text *result = cstring_to_text(str);
  pfree(str);
  return PointerGetDatum(result);
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief S2 operations descriptor consumed by `dggs_cellops()`.
 */
const DggsCellOps s2_cellops =
{
  .celltype        = T_S2CELL,
  .settype         = T_S2CELLSET,
  .temptype        = T_TS2CELL,
  .min_resolution  = S2_MIN_LEVEL,
  .max_resolution  = S2_MAX_LEVEL,
  .point_temptype  = T_TGEOGPOINT,
  .point_srid      = SRID_DEFAULT,
  .get_resolution  = &datum_s2_get_resolution,
  .is_valid_cell   = &datum_s2_is_valid_cell,
  .cell_to_parent  = &datum_s2_cell_to_parent,
  .cell_to_point   = &datum_s2_cell_to_point,
  .cell_to_boundary = &datum_s2_cell_to_boundary,
  .cell_area       = &datum_s2_cell_area
};

/*****************************************************************************
 * S2-unique temporal op: cell -> token (ttext)
 *
 * The token has no H3 or QUADBIN analogue, so it is a typed ts2cell function
 * rather than a generic DggsCellOps entry: the descriptor exposes only the
 * operations shared by every DGGS.
 *****************************************************************************/

/**
 * @ingroup meos_s2
 * @brief Return the canonical token of each cell in a temporal S2 value
 * @param[in] temp Temporal value
 * @csqlfn #Ts2cell_cell_to_token()
 */
Temporal *
ts2cell_cell_to_token(const Temporal *temp)
{
  VALIDATE_TS2CELL(temp, NULL);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_s2_cell_to_token;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = T_TTEXT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
