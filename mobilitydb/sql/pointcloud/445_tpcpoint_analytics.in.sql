/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Analytic functions for temporal pcpoint
 *
 * All functions simplify the XY trajectory (cast to tgeompoint) and then
 * delete from the original tpcpoint the instants the simplification dropped,
 * preserving all per-point sensor channels.
 *
 * The instants are DELETED rather than the survivors selected: restricting to
 * a set of timestamps yields a discrete value, which would silently strip the
 * interpolation and the sequence segmentation, whereas deleting with connect
 * set to TRUE rejoins the survivors and keeps both. When the simplification
 * drops nothing the difference is empty, and an empty set is NULL, so the
 * COALESCE returns the value unchanged.
 */

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tpcpoint, float)
  RETURNS tpcpoint
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT COALESCE(
      @extschema@.deleteTime($1,
        @extschema@.set(@extschema@.timestamps($1)) -
        @extschema@.set(@extschema@.timestamps(
          @extschema@.minDistSimplify($1::@extschema@.tgeompoint, $2))),
        TRUE),
      $1)
  $$;

CREATE FUNCTION minTimeDeltaSimplify(tpcpoint, interval)
  RETURNS tpcpoint
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT COALESCE(
      @extschema@.deleteTime($1,
        @extschema@.set(@extschema@.timestamps($1)) -
        @extschema@.set(@extschema@.timestamps(
          @extschema@.minTimeDeltaSimplify($1::@extschema@.tgeompoint, $2))),
        TRUE),
      $1)
  $$;

CREATE FUNCTION maxDistSimplify(tpcpoint, float, boolean DEFAULT TRUE)
  RETURNS tpcpoint
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE AS $$
    SELECT COALESCE(
      @extschema@.deleteTime($1,
        @extschema@.set(@extschema@.timestamps($1)) -
        @extschema@.set(@extschema@.timestamps(
          @extschema@.maxDistSimplify($1::@extschema@.tgeompoint, $2, $3))),
        TRUE),
      $1)
  $$;

CREATE FUNCTION douglasPeuckerSimplify(tpcpoint, float, boolean DEFAULT TRUE)
  RETURNS tpcpoint
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE AS $$
    SELECT COALESCE(
      @extschema@.deleteTime($1,
        @extschema@.set(@extschema@.timestamps($1)) -
        @extschema@.set(@extschema@.timestamps(
          @extschema@.douglasPeuckerSimplify($1::@extschema@.tgeompoint, $2, $3))),
        TRUE),
      $1)
  $$;

/*****************************************************************************/
