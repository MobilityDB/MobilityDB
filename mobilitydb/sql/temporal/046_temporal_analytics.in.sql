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
 * @brief Analytic functions for temporal types
 */

/*****************************************************************************/

CREATE FUNCTION minDistSimplify(tfloat, float)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minTimeDeltaSimplify(tfloat, interval)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'Temporal_simplify_min_tdelta'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION maxDistSimplify(tfloat, float, boolean DEFAULT TRUE)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'Temporal_simplify_max_dist'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION douglasPeuckerSimplify(tfloat, float, boolean DEFAULT TRUE)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'Temporal_simplify_dp'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- Extended Kalman Filter (EKF) outlier filtering for tfloat

CREATE FUNCTION extendedKalmanFilter(tfloat, gates float, q float, variance float, boolean to_drop DEFAULT TRUE)
RETURNS tfloat
AS 'MODULE_PATHNAME', 'Temporal_ext_kalman_filter'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
