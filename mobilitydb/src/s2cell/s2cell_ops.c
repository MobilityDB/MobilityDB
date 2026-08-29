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
 * @brief PG V1 wrappers for the static `s2cell` cell operations.
 *
 * Each wrapper unpacks its arguments, delegates to the first-party S2
 * kernel declared in `meos_s2cell.h`, and returns the result. The
 * subset shared with every DGGS family (resolution, hierarchy,
 * point/boundary, area) is wrapped here under the names the
 * `DggsCellOps` descriptor in `meos/include/temporal/tcellindex.h`
 * fixes, together with the operations that are S2's own: the cube face
 * a cell sits on, the Hilbert range a cell spans, the level of the
 * deepest common ancestor of two cells, and the token.
 *
 * The centre and the boundary are geographies on the WGS84 sphere
 * (SRID 4326), since an S2 cell is a region of the sphere and not of a
 * projected plane, and the area is in square metres as the descriptor
 * states. There is no k-ring: the Hilbert curve gives four edge
 * neighbours and no ring of a given radius.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_s2cell.h>
#include <pgtypes.h>
#include "geo/stbox.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "s2cell/s2cell.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Accessors
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_get_resolution(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_get_resolution);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the resolution (level) of an S2 cell
 * @sqlfn getResolution()
 */
Datum
S2cell_get_resolution(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_INT32((int32) s2cell_get_resolution(cell));
}

PGDLLEXPORT Datum S2cell_get_face(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_get_face);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the cube face in `[0, 5]` an S2 cell sits on
 * @sqlfn s2GetFace()
 */
Datum
S2cell_get_face(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_INT32((int32) s2cell_get_face(cell));
}

PGDLLEXPORT Datum S2cell_cell_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_area);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the area in square metres of an S2 cell
 * @sqlfn cellArea()
 */
Datum
S2cell_cell_area(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_FLOAT8(s2cell_cell_area(cell));
}

PGDLLEXPORT Datum S2cell_edge_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_edge_length);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the length in metres of an edge of an S2 cell, from the
 * vertex of the given index to the next one counterclockwise
 * @sqlfn s2EdgeLength()
 */
Datum
S2cell_edge_length(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  int32 edge = PG_GETARG_INT32(1);
  PG_RETURN_FLOAT8(s2cell_edge_length(cell, (uint32_t) edge));
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_cell_to_parent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_parent);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the ancestor of an S2 cell at the given coarser level
 * @sqlfn cellToParent()
 */
Datum
S2cell_cell_to_parent(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  int32 level = PG_GETARG_INT32(1);
  PG_RETURN_S2CELL(s2cell_cell_to_parent(cell, (uint32_t) level));
}

PGDLLEXPORT Datum S2cell_cell_to_child(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_child);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the descendant of an S2 cell at the given finer level and
 * the given position along the Hilbert curve
 * @sqlfn s2CellToChild()
 */
Datum
S2cell_cell_to_child(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  int32 level = PG_GETARG_INT32(1);
  int32 position = PG_GETARG_INT32(2);
  PG_RETURN_S2CELL(s2cell_cell_to_child(cell, (uint32_t) level,
    (uint32_t) position));
}

PGDLLEXPORT Datum S2cell_cell_to_children(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_children);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the descendants of an S2 cell at the given finer level as
 * an s2cellset
 * @sqlfn cellToChildren()
 */
Datum
S2cell_cell_to_children(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  int32 level = PG_GETARG_INT32(1);
  Set *result = s2cell_cell_to_children_set(cell, level);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum S2cell_cell_contains(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_contains);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return true if the first S2 cell contains the second one
 * @sqlfn s2CellContains()
 */
Datum
S2cell_cell_contains(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  S2CellId other = PG_GETARG_S2CELL(1);
  PG_RETURN_BOOL(s2cell_cell_contains(cell, other));
}

PGDLLEXPORT Datum S2cell_common_ancestor_level(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_common_ancestor_level);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the level of the deepest cell containing two S2 cells, and
 * -1 if they lie on different cube faces
 * @sqlfn s2CommonAncestorLevel()
 */
Datum
S2cell_common_ancestor_level(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  S2CellId other = PG_GETARG_S2CELL(1);
  PG_RETURN_INT32(s2cell_common_ancestor_level(cell, other));
}

PGDLLEXPORT Datum S2cell_range_min(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_range_min);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the first leaf cell an S2 cell contains, which is the lower
 * bound of the contiguous Hilbert range its descendants occupy
 * @sqlfn s2RangeMin()
 */
Datum
S2cell_range_min(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_S2CELL(s2cell_range_min(cell));
}

PGDLLEXPORT Datum S2cell_range_max(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_range_max);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the last leaf cell an S2 cell contains, which is the upper
 * bound of the contiguous Hilbert range its descendants occupy
 * @sqlfn s2RangeMax()
 */
Datum
S2cell_range_max(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_S2CELL(s2cell_range_max(cell));
}

PGDLLEXPORT Datum S2cell_edge_neighbors(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_edge_neighbors);
/**
 * @ingroup mobilitydb_s2cell_base_hierarchy
 * @brief Return the four cells sharing an edge with an S2 cell as an
 * s2cellset
 * @sqlfn s2EdgeNeighbors()
 */
Datum
S2cell_edge_neighbors(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  Set *result = s2cell_edge_neighbors_set(cell);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Conversions
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_point_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_point_to_cell);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the S2 cell at the given level holding a point
 * @sqlfn geoToS2Cell()
 */
Datum
S2cell_point_to_cell(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  int32 level = PG_GETARG_INT32(1);
  S2CellId result = geo_to_s2cell_cell(gs, level);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_S2CELL(result);
}

PGDLLEXPORT Datum S2cell_cell_to_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_point);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the centre of an S2 cell
 * @sqlfn cellToPoint()
 */
Datum
S2cell_cell_to_point(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  GSERIALIZED *result = s2cell_cell_to_geogpoint(cell);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum S2cell_cell_to_boundary(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_boundary);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the boundary of an S2 cell as a polygon
 * @sqlfn cellToBoundary()
 */
Datum
S2cell_cell_to_boundary(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  GSERIALIZED *result = s2cell_cell_to_geog(cell);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum S2cell_cell_to_token(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cell_to_token);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the token of an S2 cell, which is its identifier in
 * hexadecimal with the trailing zeros removed
 * @sqlfn s2CellToToken()
 */
Datum
S2cell_cell_to_token(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  char *result = s2cell_cell_to_token(cell);
  if (! result)
    PG_RETURN_NULL();
  text *out = cstring_to_text(result);
  pfree(result);
  PG_RETURN_TEXT_P(out);
}

PGDLLEXPORT Datum S2cell_token_to_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_token_to_cell);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the S2 cell a token denotes
 * @sqlfn s2TokenToCell()
 */
Datum
S2cell_token_to_cell(PG_FUNCTION_ARGS)
{
  text *token = PG_GETARG_TEXT_P(0);
  char *str = text_to_cstring(token);
  S2CellId result = s2cell_token_to_cell(str);
  pfree(str);
  PG_FREE_IF_COPY(token, 0);
  PG_RETURN_S2CELL(result);
}

PGDLLEXPORT Datum S2cell_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_to_stbox);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the spatiotemporal bounding box of an S2 cell
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
S2cell_to_stbox(PG_FUNCTION_ARGS)
{
  PG_RETURN_STBOX_P(s2cell_to_stbox(PG_GETARG_S2CELL(0)));
}

PGDLLEXPORT Datum S2cell_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the spatiotemporal bounding box of an S2 cell and a
 * timestamptz
 * @sqlfn stbox()
 */
Datum
S2cell_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(s2cell_timestamptz_to_stbox(cell, t));
}

PGDLLEXPORT Datum S2cell_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_s2cell_base_conversion
 * @brief Return the spatiotemporal bounding box of an S2 cell and a
 * timestamptz span
 * @sqlfn stbox()
 */
Datum
S2cell_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  const Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(s2cell_tstzspan_to_stbox(cell, s));
}

/*****************************************************************************/
