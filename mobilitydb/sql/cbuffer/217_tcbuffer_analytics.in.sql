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
 * @brief Analytic functions for temporal circular buffers
 *
 * All functions simplify the center-point trajectory (cast to tgeompoint),
 * extract the surviving timestamps, and restrict the original tcbuffer to
 * those timestamps — preserving the radius channel at each surviving instant.
 */

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tcbuffer, float)
  RETURNS tcbuffer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.atTime(
      $1,
      @extschema@.set(@extschema@.timestamps(
        @extschema@.minDistSimplify($1::@extschema@.tgeompoint, $2))))
  $$;

CREATE FUNCTION minTimeDeltaSimplify(tcbuffer, interval)
  RETURNS tcbuffer
  LANGUAGE SQL IMMUTABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.atTime(
      $1,
      @extschema@.set(@extschema@.timestamps(
        @extschema@.minTimeDeltaSimplify($1::@extschema@.tgeompoint, $2))))
  $$;

CREATE FUNCTION maxDistSimplify(tcbuffer, float, boolean DEFAULT TRUE)
  RETURNS tcbuffer
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE AS $$
    SELECT @extschema@.atTime(
      $1,
      @extschema@.set(@extschema@.timestamps(
        @extschema@.maxDistSimplify($1::@extschema@.tgeompoint, $2, $3))))
  $$;

CREATE FUNCTION douglasPeuckerSimplify(tcbuffer, float, boolean DEFAULT TRUE)
  RETURNS tcbuffer
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE AS $$
    SELECT @extschema@.atTime(
      $1,
      @extschema@.set(@extschema@.timestamps(
        @extschema@.douglasPeuckerSimplify($1::@extschema@.tgeompoint, $2, $3))))
  $$;

/*****************************************************************************/
