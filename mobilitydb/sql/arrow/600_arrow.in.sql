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
 * @brief Arrow C Data Interface round-trip self-check functions for the core and geometry types
 */

CREATE FUNCTION arrowRoundtrip(tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(geomset)
  RETURNS geomset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(geogset)
  RETURNS geogset
  AS 'MODULE_PATHNAME', 'Set_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Span_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(datespanset)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Spanset_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tstzspanset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tbool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tint)
  RETURNS tint
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tbigint)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tfloat)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(ttext)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tgeogpoint)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION arrowRoundtrip(tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
