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
 * @brief Cell-metric functions for `th3index`.
 *
 * All three functions take a textual `unit` argument (`km`, `km2`,
 * `m`, `m2`, `rads`, `rads2` as in h3-pg). The unit string is
 * validated at bind time by the MEOS implementation; an invalid
 * value raises an error before any per-instant work happens.
 *
 * `h3_great_circle_distance(tgeogpoint, tgeogpoint, text)` is the
 * `binary_synced` form of the scalar h3-pg helper — both geodetic
 * points are synchronised over their shared time axis.
 */

/******************************************************************************
 * h3_cell_area
 ******************************************************************************/

CREATE FUNCTION h3_cell_area(th3index, text DEFAULT 'km2')
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Th3index_cell_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_edge_length
 ******************************************************************************/

CREATE FUNCTION h3_edge_length(th3index, text DEFAULT 'km')
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Th3index_edge_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_great_circle_distance
 ******************************************************************************/

CREATE FUNCTION h3_great_circle_distance(tgeogpoint, tgeogpoint,
  text DEFAULT 'km')
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tgeogpoint_great_circle_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
