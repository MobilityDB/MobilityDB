/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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

/*
 * Bin and tile functions for temporal types.
 * The time bin function are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

/*****************************************************************************
 * Bin functions
 *****************************************************************************/

CREATE TYPE index_intspan AS (
  index integer,
  span intspan
);
CREATE TYPE index_bigintspan AS (
  index integer,
  span bigintspan
);
CREATE TYPE index_floatspan AS (
  index integer,
  span floatspan
);

CREATE FUNCTION bins(intspan, size integer,
  origin integer DEFAULT 0)
  RETURNS SETOF index_intspan
  AS 'MODULE_PATHNAME', 'Span_bins'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bins(bigintspan, size bigint,
  origin bigint DEFAULT 0)
  RETURNS SETOF index_bigintspan
  AS 'MODULE_PATHNAME', 'Span_bins'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bins(floatspan, size float,
  origin float DEFAULT 0.0)
  RETURNS SETOF index_floatspan
  AS 'MODULE_PATHNAME', 'Span_bins'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getBin("value" integer, size integer,
  origin integer DEFAULT 0)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Value_bin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getBin("value" bigint, size bigint,
  origin bigint DEFAULT 0)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Value_bin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getBin("value" float, size float,
  origin float DEFAULT 0.0)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Value_bin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE TYPE index_datespan AS (
  index integer,
  span datespan
);

CREATE FUNCTION bins(datespan, interval, date DEFAULT '2000-01-03')
  RETURNS SETOF index_datespan
  AS 'MODULE_PATHNAME', 'Span_bins'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getBin(date, interval, date DEFAULT '2000-01-03')
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Date_bin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE TYPE index_tstzspan AS (
  index integer,
  span tstzspan
);

CREATE FUNCTION bins(tstzspan, interval, timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_tstzspan
  AS 'MODULE_PATHNAME', 'Span_bins'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getBin(timestamptz, interval, timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Timestamptz_bin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Multidimensional tiling
 *****************************************************************************/

CREATE TYPE index_tbox AS (
  index integer,
  tile tbox
);

CREATE FUNCTION valueTiles(tbox, vsize float, vorigin float DEFAULT 0.0)
  RETURNS SETOF index_tbox
  AS 'MODULE_PATHNAME', 'Tbox_value_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timeTiles(tbox, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_tbox
  AS 'MODULE_PATHNAME', 'Tbox_time_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueTimeTiles(tbox, vsize float, duration interval,
  vorigin float DEFAULT 0.0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF index_tbox
  AS 'MODULE_PATHNAME', 'Tbox_value_time_tiles'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION getValueTile(v float, vsize float, vorigin float DEFAULT 0.0)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_get_value_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getTBoxTimeTile(t timestamptz, duration interval,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_get_time_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValueTimeTile(v float, t timestamptz, vsize float,
  duration interval, vorigin float DEFAULT 0.0,
  torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_get_value_time_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Boxes
 *****************************************************************************/

CREATE FUNCTION timeSpans(datespanset, tsize interval,
    torigin date DEFAULT '2000-01-01')
  RETURNS datespan[]
  AS 'MODULE_PATHNAME', 'Spanset_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION timeSpans(tstzspanset, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Spanset_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION valueSpans(intspanset, vsize int, vorigin int DEFAULT 0)
  RETURNS intspan[]
  AS 'MODULE_PATHNAME', 'Spanset_value_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueSpans(floatspanset, vsize float, vorigin float DEFAULT 0.0)
  RETURNS floatspan[]
  AS 'MODULE_PATHNAME', 'Spanset_value_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE FUNCTION timeSpans(tbool, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSpans(tint, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSpans(tfloat, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSpans(ttext, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Temporal_time_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION valueSpans(tint, vsize int, vorigin int DEFAULT 0)
  RETURNS intspan[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueSpans(tfloat, vsize float, vorigin float DEFAULT 0.0)
  RETURNS floatspan[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_spans'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE FUNCTION valueBoxes(tint, vsize int, vorigin int DEFAULT 0)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueBoxes(tfloat, vsize float, vorigin float DEFAULT 0.0)
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION timeBoxes(tint, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeBoxes(tfloat, tsize interval,
    torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE FUNCTION valueTimeBoxes(tint, vsize int, tsize interval,
    vorigin int DEFAULT 0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueTimeBoxes(tfloat, vsize float, tsize interval,
    vorigin float DEFAULT 0.0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS tbox[]
  AS 'MODULE_PATHNAME', 'Tnumber_value_time_boxes'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************
 * Splitting
 *****************************************************************************/

CREATE TYPE number_tint AS (
  number integer,
  tnumber tint
);
CREATE TYPE number_tfloat AS (
  number float,
  tnumber tfloat
);

CREATE FUNCTION valueSplit(tint, size integer, origin integer DEFAULT 0)
  RETURNS SETOF number_tint
  AS 'MODULE_PATHNAME', 'Tnumber_value_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueSplit(tfloat, size float, origin float DEFAULT 0.0)
  RETURNS SETOF number_tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_value_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE time_tbool AS (
  time timestamptz,
  temp tbool
);
CREATE TYPE time_tint AS (
  time timestamptz,
  temp tint
);
CREATE TYPE time_tfloat AS (
  time timestamptz,
  temp tfloat
);
CREATE TYPE time_ttext AS (
  time timestamptz,
  temp ttext
);

CREATE FUNCTION timeSplit(tbool, size interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF time_tbool
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSplit(tint, size interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF time_tint
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSplit(tfloat, size interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF time_tfloat
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION timeSplit(ttext, size interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF time_ttext
  AS 'MODULE_PATHNAME', 'Temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE number_time_tint AS (
  number integer,
  time timestamptz,
  tnumber tint
);
CREATE TYPE number_time_tfloat AS (
  number float,
  time timestamptz,
  tnumber tfloat
);

CREATE FUNCTION valueTimeSplit(tint, size integer, duration interval,
    vorigin integer DEFAULT 0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF number_time_tint
  AS 'MODULE_PATHNAME', 'Tnumber_value_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE FUNCTION valueTimeSplit(tfloat, size float, duration interval,
    vorigin float DEFAULT 0.0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS SETOF number_time_tfloat
  AS 'MODULE_PATHNAME', 'Tnumber_value_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/


