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
 * @brief Hierarchical-grid functions for `th3index`.
 *
 * The two-arg forms (parent / center_child / child_pos with an
 * explicit target resolution) lift via `lift_with_const` — the
 * integer resolution is constant across the time axis. The no-arg
 * forms drop to the next-coarser / next-finer resolution and are
 * plain `unary_scalar` lifts.
 *
 * `h3_child_pos_to_cell` takes TWO temporal operands (the child
 * position and the parent cell) — both axes are synchronised.
 */

/******************************************************************************
 * h3_cell_to_parent
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_parent(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_parent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_cell_to_parent(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_parent_next'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_center_child
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_center_child(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_center_child'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3_cell_to_center_child(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_center_child_next'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_cell_to_child_pos
 ******************************************************************************/

CREATE FUNCTION h3_cell_to_child_pos(th3index, integer)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Th3index_cell_to_child_pos'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * h3_child_pos_to_cell
 *
 * Two temporal operands (child_pos : tbigint, parent : th3index) are
 * synchronised over their shared time axis. The childRes argument
 * stays constant.
 ******************************************************************************/

CREATE FUNCTION h3_child_pos_to_cell(tbigint, th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Th3index_child_pos_to_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
