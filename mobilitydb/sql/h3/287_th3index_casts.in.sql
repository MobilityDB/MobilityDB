/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief Convenience casts for `th3index`.
 *
 * `h3-pg` exposes plain-SQL `h3index :: point`, `h3index :: geometry`
 * and `h3index :: geography`. The temporal counterparts reuse the
 * lifted `h3_cell_to_latlng` conversion defined in
 * `282_th3index_latlng.in.sql` — these casts are pure sugar for the
 * explicit call.
 *
 * Casts are EXPLICIT (not IMPLICIT nor ASSIGNMENT): typing a cell as
 * a point should not happen by accident. Matches h3-pg's own cast
 * direction.
 *
 * The `th3index :: tbigint` and `tbigint :: th3index` casts are
 * already declared in `270_th3index.in.sql` as ASSIGNMENT and are
 * not re-declared here.
 */

/******************************************************************************
 * th3index :: tgeogpoint
 ******************************************************************************/

CREATE CAST (th3index AS tgeogpoint) WITH FUNCTION h3_cell_to_latlng(th3index);

/******************************************************************************
 * th3index :: tgeompoint
 ******************************************************************************/

CREATE CAST (th3index AS tgeompoint)
  WITH FUNCTION h3_cell_to_latlng_tgeompoint(th3index);

/******************************************************************************/
