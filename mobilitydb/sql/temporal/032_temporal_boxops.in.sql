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
 * @brief Bounding tbox operations for temporal types
 */

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

CREATE FUNCTION spans(tbool)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tint)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tbigint)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tfloat)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(ttext)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tboxes(tint)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxes(tbigint)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxes(tfloat)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNSpans(tbool, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tbigint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(tfloat, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNSpans(ttext, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNSpans(tbool, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tbigint, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(tfloat, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNSpans(ttext, integer)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_split_each_n_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitNTboxes(tint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNTboxes(tbigint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitNTboxes(tfloat, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION splitEachNTboxes(tint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_each_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNTboxes(tbigint, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_each_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION splitEachNTboxes(tfloat, integer)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_split_each_n_tboxes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
