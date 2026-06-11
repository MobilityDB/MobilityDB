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
 * @brief Spatial functions for temporal pcpoint
 *
 * The spatial component of a tpcpoint is its XY (or XYZ) point trajectory,
 * accessible via the tgeompoint cast. Functions that return geometric
 * aggregates delegate to the corresponding tgeompoint overload.
 *
 * SRID uses the generic Tspatial_srid dispatcher which reads the SRID
 * stored in the first instant's value (same mechanism as tgeometry).
 */

/*****************************************************************************/

CREATE FUNCTION SRID(tpcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION centroid(tpcpoint)
  RETURNS tgeompoint
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT $1::@extschema@.tgeompoint
  $$;

CREATE FUNCTION trajectory(tpcpoint, bool DEFAULT FALSE)
  RETURNS geometry
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.trajectory($1::@extschema@.tgeompoint, $2)
  $$;

/*****************************************************************************/
