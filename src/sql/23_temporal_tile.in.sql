/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Bucket and tile functions for temporal types.
 * The time bucket function are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

/*****************************************************************************
 * Bucket functions
 *****************************************************************************/

CREATE OR REPLACE FUNCTION valueBucket("value" integer, width integer,
  origin integer DEFAULT '0')
  RETURNS integer
  AS 'MODULE_PATHNAME', 'number_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION valueBucket("value" float, width float,
  origin float DEFAULT '0.0')
  RETURNS float
  AS 'MODULE_PATHNAME', 'number_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

-- bucketing of timestamptz happens at UTC time
CREATE OR REPLACE FUNCTION timeBucket(ts timestamptz, width interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'timestamptz_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

-- If an interval is given as the third argument, the bucket alignment is offset by the interval.
CREATE OR REPLACE FUNCTION timeBucket(ts timestamptz, width interval, "offset" interval)
  RETURNS timestamptz
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT AS
$BODY$
    SELECT @extschema@.timeBucket(ts-"offset", width)+"offset";
$BODY$;

/*****************************************************************************
 * Bucketing
 *****************************************************************************/

CREATE TYPE index_intrange AS (
  index integer,
  range intrange
);
CREATE TYPE index_floatrange AS (
  index integer,
  range floatrange
);

CREATE OR REPLACE FUNCTION bucketList(intrange, int,
  int DEFAULT 0)
  RETURNS SETOF index_intrange
  AS 'MODULE_PATHNAME', 'range_bucket_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION bucketList(floatrange, float,
  float DEFAULT 0.0)
  RETURNS SETOF index_floatrange
  AS 'MODULE_PATHNAME', 'range_bucket_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION bucketIntRange(integer, int,
  int DEFAULT 0)
  RETURNS intrange
  AS 'MODULE_PATHNAME', 'range_bucket'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION bucketFloatRange(integer, float,
  float DEFAULT 0.0)
  RETURNS floatrange
  AS 'MODULE_PATHNAME', 'range_bucket'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Bucketing
 *****************************************************************************/

CREATE TYPE index_period AS (
  index integer,
  period period
);

CREATE OR REPLACE FUNCTION bucketList(period, interval,
  TimestampTz DEFAULT '2000-01-03')
  RETURNS SETOF index_period
  AS 'MODULE_PATHNAME', 'period_bucket_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION bucketPeriod(integer, interval,
  TimestampTz DEFAULT '2000-01-03')
  RETURNS period
  AS 'MODULE_PATHNAME', 'period_bucket'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Multidimensional tiling
 *****************************************************************************/

CREATE TYPE indices_tbox AS (
  indices integer[],
  box tbox
);

CREATE OR REPLACE FUNCTION multidimGrid(tbox, float, interval,
  float DEFAULT 0.0, TimestampTz DEFAULT '2000-01-03')
  RETURNS SETOF indices_tbox
  AS 'MODULE_PATHNAME', 'tbox_multidim_grid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION multidimTileTbox(int[], float, interval,
  float DEFAULT 0.0, TimestampTz DEFAULT '2000-01-03')
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'tbox_multidim_tile'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Splitting
/*****************************************************************************/

CREATE TYPE int_tint AS (
  number integer,
  tnumber tint
);
CREATE TYPE float_tfloat AS (
  number float,
  tnumber tfloat
);

CREATE OR REPLACE FUNCTION valueSplit(tint, width integer,
    origin integer DEFAULT 0)
  RETURNS setof int_tint
  AS 'MODULE_PATHNAME', 'tnumber_value_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION valueSplit(tfloat, width float,
    origin float DEFAULT 0.0)
  RETURNS setof float_tfloat
  AS 'MODULE_PATHNAME', 'tnumber_value_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

CREATE OR REPLACE FUNCTION valueSplitNew(tint, width integer,
    origin integer DEFAULT 0)
  RETURNS setof int_tint
  AS 'MODULE_PATHNAME', 'tnumber_value_split_new'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION valueSplitNew(tfloat, width float,
    origin float DEFAULT 0.0)
  RETURNS setof float_tfloat
  AS 'MODULE_PATHNAME', 'tnumber_value_split_new'
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

CREATE OR REPLACE FUNCTION timeSplit(tbool, width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tbool
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(tint, width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tint
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(tfloat, width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tfloat
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(ttext, width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_ttext
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE int_time_tint AS (
  number integer,
  time timestamptz,
  tnumber tint
);
CREATE TYPE float_time_tfloat AS (
  number float,
  time timestamptz,
  tnumber tfloat
);

CREATE OR REPLACE FUNCTION valueTimeSplit(tint, integer, interval,
    vorigin integer DEFAULT 0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS setof int_time_tint
  AS 'MODULE_PATHNAME', 'tnumber_value_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION valueTimeSplit(tfloat, float, interval,
    vorigin float DEFAULT 0.0, torigin timestamptz DEFAULT '2000-01-03')
  RETURNS setof float_time_tfloat
  AS 'MODULE_PATHNAME', 'tnumber_value_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/


