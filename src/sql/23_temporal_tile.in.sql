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

CREATE OR REPLACE FUNCTION numberBucket("value" integer, bucket_width integer,
  origin integer DEFAULT '0')
  RETURNS integer
  AS 'MODULE_PATHNAME', 'number_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION numberBucket("value" float, bucket_width float,
  origin float DEFAULT '0.0')
  RETURNS float
  AS 'MODULE_PATHNAME', 'number_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

-- bucketing of timestamptz happens at UTC time
CREATE OR REPLACE FUNCTION timeBucket(ts timestamptz, bucket_width interval,
  origin timestamptz DEFAULT '2000-01-03')
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'timestamptz_bucket'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

-- If an interval is given as the third argument, the bucket alignment is offset by the interval.
CREATE OR REPLACE FUNCTION timeBucket(ts timestamptz, bucket_width interval, "offset" interval)
  RETURNS timestamptz
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE STRICT AS
$BODY$
    SELECT @extschema@.timeBucket(ts-"offset", bucket_width)+"offset";
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
  RETURNS SETOF intrange_bucket
  AS 'MODULE_PATHNAME', 'range_bucket_list'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION bucketList(floatrange, float,
  float DEFAULT 0.0)
  RETURNS SETOF floatrange_bucket
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

CREATE OR REPLACE FUNCTION timeSplit(tbool, bucket_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tbool
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(tint, bucket_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tint
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(tfloat, bucket_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_tfloat
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION timeSplit(ttext, bucket_width interval,
    origin timestamptz DEFAULT '2000-01-03')
  RETURNS setof time_ttext
  AS 'MODULE_PATHNAME', 'temporal_time_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;

/*****************************************************************************/

CREATE TYPE int_tint AS (
  number integer,
  tnumber tint
);
CREATE TYPE float_tfloat AS (
  number float,
  tnumber tfloat
);

CREATE OR REPLACE FUNCTION valueSplit(tint, bucket_width integer,
    origin integer DEFAULT 0)
  RETURNS setof int_tint
  AS 'MODULE_PATHNAME', 'tnumber_value_split'
  LANGUAGE C IMMUTABLE PARALLEL SAFE STRICT;
CREATE OR REPLACE FUNCTION valueSplit(tfloat, bucket_width float,
    origin float DEFAULT 0.0)
  RETURNS setof float_tfloat
  AS 'MODULE_PATHNAME', 'tnumber_value_split'
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


