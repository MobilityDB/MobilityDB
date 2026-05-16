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
 * @brief Index-inspection functions for `th3index`.
 *
 * All entries are `unary_scalar` lifts of the corresponding h3-pg
 * functions. Each returns a Temporal of the base scalar type,
 * preserving the time axis of the input.
 */

/******************************************************************************
 * h3_get_resolution
 ******************************************************************************/

CREATE FUNCTION h3_get_resolution(th3index)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Th3index_get_resolution'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_get_base_cell_number
 ******************************************************************************/

CREATE FUNCTION h3_get_base_cell_number(th3index)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Th3index_get_base_cell_number'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_valid_cell
 ******************************************************************************/

CREATE FUNCTION h3_is_valid_cell(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_res_class_iii
 ******************************************************************************/

CREATE FUNCTION h3_is_res_class_iii(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_res_class_iii'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_is_pentagon
 ******************************************************************************/

CREATE FUNCTION h3_is_pentagon(th3index)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Th3index_is_pentagon'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
