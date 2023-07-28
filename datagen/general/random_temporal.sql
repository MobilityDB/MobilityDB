/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * random_temporal.sql
 * Basic synthetic data generator functions for some PostgreSQL data types
 * and for temporal data types.
 *
 * These functions use lower and upper bounds for the generated values:
 * lowvalue and highvalue for values, lowtime and hightime for timestamps.
 * When generating series of values, the maxdelta argument states the maximum
 * difference between two consecutive values, while maxminutes states the
 * the maximum number of minutes between two consecutive timestamps as well as
 * the maximum number of minutes for time gaps between two consecutive
 * components of temporal instant/sequence sets.
 */

-------------------------------------------------------------------------------
-- Basic types
-------------------------------------------------------------------------------

/**
 * Generate a random boolean
 */
DROP FUNCTION IF EXISTS random_bool;
CREATE FUNCTION random_bool()
  RETURNS boolean AS $$
BEGIN
  IF random() > 0.5 THEN RETURN TRUE; ELSE RETURN FALSE; END IF;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_bool() AS i
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random integer in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 */
DROP FUNCTION IF EXISTS random_int;
CREATE FUNCTION random_int(lowvalue int, highvalue int)
  RETURNS int AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  RETURN floor(random() * (highvalue - lowvalue + 1) + lowvalue);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT random_int(1,7), COUNT(*)
FROM generate_series(1, 1e3)
GROUP BY 1
ORDER BY 1;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random bigint in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 */
DROP FUNCTION IF EXISTS random_bigint;
CREATE FUNCTION random_bigint(lowvalue bigint, highvalue bigint)
  RETURNS bigint AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  RETURN floor(random() * (highvalue - lowvalue + 1) + lowvalue);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT random_bigint(1,7), COUNT(*)
FROM generate_series(1, 1e3)
GROUP BY 1
ORDER BY 1;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random integers in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between two consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_int_array;
CREATE FUNCTION random_int_array(lowvalue int, highvalue int, maxdelta int,
  mincard int, maxcard int)
  RETURNS int[] AS $$
DECLARE
  result int[];
  card int;
  delta int;
  v int;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  card = random_int(mincard, maxcard);
  v = random_int(lowvalue, highvalue);
  FOR i IN 1..card
  LOOP
    result[i] = v;
    IF i = card THEN EXIT; END IF;
    delta = random_int(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (v + delta >= lowvalue AND v + delta <= highvalue) THEN
      v = v + delta;
    ELSIF (v - delta >= lowvalue AND v - delta <= highvalue) THEN
      v = v - delta;
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_int_array(-100, 100, 10, 5, 10) AS iarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random integer range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 */
DROP FUNCTION IF EXISTS random_int4range;
CREATE FUNCTION random_int4range(lowvalue int, highvalue int, maxdelta int)
  RETURNS int4range AS $$
DECLARE
  v int;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  v = random_int(lowvalue, highvalue - maxdelta);
  RETURN int4range(v, v + random_int(1, maxdelta));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_int4range(-100, 100, 10) AS ir
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random integer span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 */
DROP FUNCTION IF EXISTS random_intspan;
CREATE FUNCTION random_intspan(lowvalue int, highvalue int, maxdelta int)
  RETURNS intspan AS $$
DECLARE
  v int;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  v = random_int(lowvalue, highvalue - maxdelta);
  RETURN span(v, v + random_int(1, maxdelta));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_intspan(-100, 100, 10) AS ir
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random bigint span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 */
DROP FUNCTION IF EXISTS random_bigintspan;
CREATE FUNCTION random_bigintspan(lowvalue bigint, highvalue bigint, maxdelta int)
  RETURNS bigintspan AS $$
DECLARE
  v int;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  v = random_bigint(lowvalue, highvalue - maxdelta);
  RETURN span(v, v + random_bigint(1, maxdelta));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_bigintspan(-100, 100, 10) AS s
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random intspans within a span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the maximal span
 * @param[in] maxdelta Maximum value difference between consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_intspan_array;
CREATE FUNCTION random_intspan_array(lowvalue int, highvalue int,
  maxdelta int, mincard int, maxcard int)
  RETURNS intspan[] AS $$
DECLARE
  result intspan[];
  card int;
  v1 int;
  v2 int;
BEGIN
  IF lowvalue > highvalue - maxdelta * 2 * (maxcard - mincard) THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - '
      'maxdelta * 2 * (maxcard - mincard): %, %, %, %, %',
      lowvalue, highvalue, maxdelta, mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  v1 = lowvalue;
  v2 = highvalue - maxdelta * (card - 1) * 2;
  FOR i IN 1..card
  LOOP
    result[i] = random_intspan(v1, v2, maxdelta);
    v1 = upper(result[i]) + random_int(1, maxdelta);
    v2 = v2 + maxdelta * 2;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_intspan_array(1, 1000, 10, 5, 10) AS iarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random bigintspans within a span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the maximal span
 * @param[in] maxdelta Maximum value difference between consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_bigintspan_array;
CREATE FUNCTION random_bigintspan_array(lowvalue bigint, highvalue bigint,
  maxdelta int, mincard int, maxcard int)
  RETURNS bigintspan[] AS $$
DECLARE
  result bigintspan[];
  card int;
  v1 bigint;
  v2 bigint;
BEGIN
  IF lowvalue > highvalue - maxdelta * 2 * (maxcard - mincard) THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - '
      'maxdelta * 2 * (maxcard - mincard): %, %, %, %, %',
      lowvalue, highvalue, maxdelta, mincard, maxcard;
  END IF;
  card = random_bigint(mincard, maxcard);
  v1 = lowvalue;
  v2 = highvalue - maxdelta * (card - 1) * 2;
  FOR i IN 1..card
  LOOP
    result[i] = random_bigintspan(v1, v2, maxdelta);
    v1 = upper(result[i]) + random_bigint(1, maxdelta);
    v2 = v2 + maxdelta * 2;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_bigintspan_array(1, 1000, 10, 5, 10) AS biarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random integer span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_intspanset;
CREATE FUNCTION random_intspanset(lowvalue int, highvalue int, maxdelta int,
  mincard int, maxcard int)
  RETURNS intspanset AS $$
BEGIN
  RETURN spanset(random_intspan_array(lowvalue, highvalue, maxdelta,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_intspanset(-100, 100, 10, 5, 10) AS ir
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random bigint span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_bigintspanset;
CREATE FUNCTION random_bigintspanset(lowvalue bigint, highvalue bigint,
  maxdelta int, mincard int, maxcard int)
  RETURNS bigintspanset AS $$
BEGIN
  RETURN spanset(random_bigintspan_array(lowvalue, highvalue, maxdelta,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_bigintspanset(-100, 100, 10, 5, 10) AS bs
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random float in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 */
DROP FUNCTION IF EXISTS random_float;
CREATE FUNCTION random_float(lowvalue float, highvalue float)
  RETURNS float AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  RETURN random() * (highvalue - lowvalue) + lowvalue;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_float(-100, 100) AS f
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random floats in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between two consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_float_array;
CREATE FUNCTION random_float_array(lowvalue float, highvalue float,
  maxdelta float, mincard int, maxcard int)
  RETURNS float[] AS $$
DECLARE
  result float[];
  card int;
  delta float;
  v float;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  card = random_int(mincard, maxcard);
  v = random_float(lowvalue, highvalue - maxdelta);
  FOR i IN 1..card
  LOOP
    result[i] = v;
    IF i = card THEN EXIT; END IF;
    delta = random_float(-1 * maxdelta, maxdelta);
    /* If neither of these conditions is satisfied the same value is kept */
    IF (v + delta >= lowvalue AND v + delta <= highvalue) THEN
      v = v + delta;
    ELSIF (v - delta >= lowvalue AND v - delta <= highvalue) THEN
      v = v - delta;
    END IF;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_float_array(-100, 100, 10, 5, 10) AS farr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random float span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the span
 * @param[in] maxdelta Maximum difference between two consecutive values
 */
DROP FUNCTION IF EXISTS random_floatspan;
CREATE FUNCTION random_floatspan(lowvalue float, highvalue float, maxdelta int)
  RETURNS floatspan AS $$
DECLARE
  v float;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  v = random_float(lowvalue, highvalue - maxdelta);
  RETURN span(v, v + random_float(1, maxdelta));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_floatspan(-100, 100, 10) AS fr
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random floatspans within a span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the maximal span
 * @param[in] maxdelta Maximum value difference between consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_floatspan_array;
CREATE FUNCTION random_floatspan_array(lowvalue float, highvalue float,
  maxdelta int, mincard float, maxcard float)
  RETURNS floatspan[] AS $$
DECLARE
  result floatspan[];
  card float;
  v1 float;
  v2 float;
BEGIN
  IF lowvalue > highvalue - maxdelta * 2 * (maxcard - mincard) THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - '
      'maxdelta * 2 * (maxcard - mincard): %, %, %, %, %',
      lowvalue, highvalue, maxdelta, mincard, maxcard;
  END IF;
  card = random_float(mincard, maxcard);
  v1 = lowvalue;
  v2 = highvalue - maxdelta * (card - 1) * 2;
  FOR i IN 1..card
  LOOP
    result[i] = random_floatspan(v1, v2, maxdelta);
    v1 = upper(result[i]) + random_float(1, maxdelta);
    v2 = v2 + maxdelta * 2;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_floatspan_array(1, 1000, 10, 5, 10) AS iarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random float span
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between the lower and upper bounds
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_floatspanset;
CREATE FUNCTION random_floatspanset(lowvalue float, highvalue float,
  maxdelta int, mincard float, maxcard float)
  RETURNS floatspanset AS $$
DECLARE
  v float;
BEGIN
  RETURN spanset(random_floatspan_array(lowvalue, highvalue, maxdelta,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_floatspanset(-100, 100, 10, 5, 10) AS ir
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random ASCII character
 */
DROP FUNCTION IF EXISTS random_ascii;
CREATE FUNCTION random_ascii()
  RETURNS char AS $$
BEGIN
  -- ascii('A') = 65, ascii('Z') = 90,
  RETURN chr(random_int(65, 90));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ascii() AS m
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random text value
 *
 * @param[in] maxlength Maximum length of the text value
 */
DROP FUNCTION IF EXISTS random_text;
CREATE FUNCTION random_text(maxlength int)
  RETURNS text AS $$
DECLARE
  result text;
BEGIN
  SELECT string_agg(random_ascii(),'') INTO result
  FROM generate_series(1, random_int(1, maxlength)) AS x;
  result = replace(result, '"', '\"');
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_text(20) AS text
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random text values
 *
 * @param[in] maxlength Maximum length of the text value
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_text_array;
CREATE FUNCTION random_text_array(maxlength int, mincard int, maxcard int)
  RETURNS text[] AS $$
DECLARE
  textarr text[];
BEGIN
  SELECT array_agg(random_text(maxlength)) INTO textarr
  FROM generate_series(mincard, mincard + random_int(mincard, maxcard)) AS t;
  RETURN textarr;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_text_array(20, 5, 10) AS text
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a set of random text values
 *
 * @param[in] maxlength Maximum length of the text value
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_textset;
CREATE FUNCTION random_textset(maxlength int, mincard int, maxcard int)
  RETURNS textset AS $$
DECLARE
  textarr text[];
BEGIN
  textarr := '{}'::text[];
  SELECT array_agg(DISTINCT random_text(maxlength)) INTO textarr
  FROM generate_series(mincard, mincard + random_int(mincard, maxcard)) AS t;
  RETURN set(textarr);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_textset(20, 5, 10) AS text
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Time Types
-------------------------------------------------------------------------------

/**
 * Generate a random timestamptz in a period
 *
 * @param[in] lowtime, hightime Inclusive bounds of the period
 */
DROP FUNCTION IF EXISTS random_timestamptz;
CREATE FUNCTION random_timestamptz(lowtime timestamptz, hightime timestamptz)
  RETURNS timestamptz AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN date_trunc('minute',
    (lowtime + random() * (hightime - lowtime)))::timestamptz(0);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_timestamptz('2001-01-01', '2002-01-01') AS t
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random interval of minutes
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the number of minutes
 */
DROP FUNCTION IF EXISTS random_minutes;
CREATE FUNCTION random_minutes(lowvalue int, highvalue int)
  RETURNS interval AS $$
BEGIN
  RETURN random_int(lowvalue, highvalue) * interval '1 minute';
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_minutes(1, 20) AS m
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an ordered array of random timestamptz in a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between two consecutive
 *    timestamps
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_timestamptz_array;
CREATE FUNCTION random_timestamptz_array(lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int,
  fixstart bool DEFAULT false)
  RETURNS timestamptz[] AS $$
DECLARE
  result timestamptz[];
  card int;
  t timestamptz;
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes * (maxcard - mincard) THEN
    RAISE EXCEPTION 'The duration between lowtime and hightime is not enough to generate the temporal value';
  END IF;
  card = random_int(mincard, maxcard);
  if fixstart THEN
    t = lowtime;
  ELSE
    t = random_timestamptz(lowtime, hightime - interval '1 minute' *
      maxminutes * card);
  END IF;
  FOR i IN 1..card
  LOOP
    result[i] = t;
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_timestamptz_array('2001-01-01', '2002-01-01', 10, 5, 10) AS tarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tstzspan within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between the timestamps
 * @param[in] fixstart True when this function is called for generating
 *   a tstzspan set and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tstzspan;
CREATE FUNCTION random_tstzspan(lowtime timestamptz, hightime timestamptz,
  maxminutes int, fixstart bool DEFAULT false)
  RETURNS tstzspan AS $$
DECLARE
  t timestamptz;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  if fixstart THEN
    t = lowtime;
  ELSE
    t = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  END IF;
  /* Generate instantaneous periods with 0.1 probability */
  IF random() < 0.1 THEN
    RETURN span(t, t, true, true);
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
    RETURN span(t, t + random_minutes(1, maxminutes), lower_inc, upper_inc);
  END IF;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzspan('2001-01-01', '2002-01-01', 10) AS p
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random periods within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between the timestamps
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_tstzspan_array;
CREATE FUNCTION random_tstzspan_array(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int)
  RETURNS tstzspan[] AS $$
DECLARE
  result tstzspan[];
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowtime > hightime - interval '1 minute' *
    maxminutes * 2 * (maxcard - mincard) THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - '
      'maxminutes * 2 * (maxcard - mincard) minutes: %, %, %, %, %',
      lowtime, hightime, maxminutes, mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' * maxminutes * (card - 1) * 2;
  FOR i IN 1..card
  LOOP
    result[i] = random_tstzspan(t1, t2, maxminutes, i > 1);
    t1 = upper(result[i]) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * 2;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzspan_array('2001-01-01', '2002-01-01', 10, 5, 10) AS parr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tstzrange within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between the timestamps
 */
DROP FUNCTION IF EXISTS random_tstzrange;
CREATE FUNCTION random_tstzrange(lowtime timestamptz, hightime timestamptz,
  maxminutes int)
  RETURNS tstzrange AS $$
BEGIN
  RETURN random_tstzspan(lowtime, hightime, maxminutes)::tstzrange;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzrange('2001-01-01', '2002-01-01', 10) AS r
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random tstzrange within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between the timestamps
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_tstzrange_array;
CREATE FUNCTION random_tstzrange_array(lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int)
  RETURNS tstzrange[] AS $$
DECLARE
  periodarr tstzspan[];
  result tstzrange[];
  card int;
BEGIN
  SELECT random_tstzspan_array(lowtime, hightime, maxminutes, mincard, maxcard)
  INTO periodarr;
  card = array_length(periodarr, 1);
  FOR i IN 1..card
  LOOP
    result[i] = periodarr[i]::tstzrange;
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzrange_array('2001-01-01', '2002-01-01', 10, 5, 10) AS rarr
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an ordered set random int in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between two consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 */
DROP FUNCTION IF EXISTS random_intset;
CREATE FUNCTION random_intset(lowvalue int, highvalue int, maxdelta int,
  mincard int, maxcard int)
  RETURNS intset AS $$
DECLARE
  iarr int[];
  v int;
  card int;
  i int;
BEGIN
  IF lowvalue >= highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  IF lowvalue > highvalue - maxdelta * (maxcard - mincard) THEN
    RAISE EXCEPTION 'The difference between lowvalue and highvalue is not enough to generate the intset';
  END IF;
  card = random_int(mincard, maxcard);
  v = random_int(lowvalue, highvalue - maxdelta * card);
  FOR i IN 1..card
  LOOP
    iarr[i] = v;
    v = v + random_int(1, maxdelta);
  END LOOP;
  RETURN set(iarr);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_intset(1, 100, 5, 5, 10) AS is
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an ordered set random bigint in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between two consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 */
DROP FUNCTION IF EXISTS random_bigintset;
CREATE FUNCTION random_bigintset(lowvalue bigint, highvalue bigint,
  maxdelta int, mincard int, maxcard int)
  RETURNS bigintset AS $$
DECLARE
  iarr bigint[];
  v bigint;
  card int;
  i int;
BEGIN
  IF lowvalue >= highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  IF lowvalue > highvalue - maxdelta * (maxcard - mincard) THEN
    RAISE EXCEPTION 'The difference between lowvalue and highvalue is not enough to generate the bigintset';
  END IF;
  card = random_int(mincard, maxcard);
  v = random_bigint(lowvalue, highvalue - maxdelta * card);
  FOR i IN 1..card
  LOOP
    iarr[i] = v;
    v = v + random_int(1, maxdelta);
  END LOOP;
  RETURN set(iarr);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_bigintset(1, 100, 5, 5, 10) AS is
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an ordered set random float in a range
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] maxdelta Maximum difference between two consecutive values
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 */
DROP FUNCTION IF EXISTS random_floatset;
CREATE FUNCTION random_floatset(lowvalue float, highvalue float,
  maxdelta int, mincard int, maxcard int)
  RETURNS floatset AS $$
DECLARE
  farr float[];
  v float;
  card int;
  i int;
BEGIN
  IF lowvalue >= highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  IF lowvalue > highvalue - maxdelta * (maxcard - mincard) THEN
    RAISE EXCEPTION 'The difference between lowvalue and highvalue is not enough to generate the floatset';
  END IF;
  card = random_int(mincard, maxcard);
  v = random_float(lowvalue, highvalue - maxdelta * card);
  FOR i IN 1..card
  LOOP
    farr[i] = v;
    v = v + random_float(1, maxdelta);
  END LOOP;
  RETURN set(farr);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_floatset(1, 100, 5, 5, 10) AS is
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tstzset within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between two consecutive timestamps
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_tstzset;
CREATE FUNCTION random_tstzset(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int)
  RETURNS tstzset AS $$
BEGIN
  RETURN set(random_timestamptz_array(lowtime, hightime, maxminutes,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzset('2001-01-01', '2002-01-01', 10, 5, 10) AS ps
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tstzspanset within a tstzspan
 *
 * @param[in] lowtime, hightime Inclusive bounds of the maximal tstzspan
 * @param[in] maxminutes Maximum number of minutes between two consecutive timestamps
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_tstzspanset;
CREATE FUNCTION random_tstzspanset(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int)
  RETURNS tstzspanset AS $$
BEGIN
  RETURN spanset(random_tstzspan_array(lowtime, hightime, maxminutes, mincard,
    maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tstzspanset('2001-01-01', '2002-01-01', 10, 5, 10) AS ps
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Tbox Type
-------------------------------------------------------------------------------

/**
 * Generate a random tbox within a range and a tstzspan
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value between the bounds
 * @param[in] maxminutes Maximum number of minutes between the bounds
 */
DROP FUNCTION IF EXISTS random_tbox;
CREATE FUNCTION random_tbox(lowvalue float, highvalue float,
   lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int)
  RETURNS tbox AS $$
DECLARE
  xmin float;
  tmin timestamptz;
BEGIN
  IF lowvalue > highvalue - maxdelta THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue - maxdelta: %, %, %',
      lowvalue, highvalue, maxdelta;
  END IF;
  IF lowtime > hightime - interval '1 minute' * maxminutes THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime - maxminutes minutes: %, %, %',
      lowtime, hightime, maxminutes;
  END IF;
  xmin = random_float(lowvalue, highvalue - maxdelta);
  tmin = random_timestamptz(lowtime, hightime - interval '1 minute' * maxminutes);
  RETURN tbox(span(xmin, xmin + random_float(1, maxdelta)),
    span(tmin, tmin + random_minutes(1, maxminutes)));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tbox(-100, 100, '2001-01-01', '2002-01-01', 10, 10) AS b
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Instant
-------------------------------------------------------------------------------

/**
 * Generate a random tbool instant
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_tbool_inst;
CREATE FUNCTION random_tbool_inst(lowtime timestamptz, hightime timestamptz)
  RETURNS tbool AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tbool_inst(random_bool(), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tbool_inst('2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tint instant
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range of values
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_tint_inst;
CREATE FUNCTION random_tint_inst(lowvalue int, highvalue int,
  lowtime timestamptz, hightime timestamptz)
  RETURNS tint AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tint_inst(random_int(lowvalue, highvalue),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tint_inst(1, 20, '2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tfloat instant
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range of values
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_tfloat_inst;
CREATE FUNCTION random_tfloat_inst(lowvalue float, highvalue float,
  lowtime timestamptz, hightime timestamptz)
  RETURNS tfloat AS $$
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN tfloat_inst(random_float(lowvalue, highvalue),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tfloat_inst(1, 20, '2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random ttext instant
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxlength Maximum length of the text value
 */
DROP FUNCTION IF EXISTS random_ttext_inst;
CREATE FUNCTION random_ttext_inst(lowtime timestamptz, hightime timestamptz,
  maxlength int)
  RETURNS ttext AS $$
BEGIN
  IF lowtime > hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  RETURN ttext_inst(random_text(maxlength), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ttext_inst('2001-01-01', '2002-01-01', 20) AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Discrete Sequence
-------------------------------------------------------------------------------

/**
 * Generate a random tbool discrete sequence
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 */
DROP FUNCTION IF EXISTS random_tbool_discseq;
CREATE FUNCTION random_tbool_discseq(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int)
  RETURNS tbool AS $$
DECLARE
  tsarr timestamptz[];
  result tbool[];
  card int;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard, maxcard)
  INTO tsarr;
  card = array_length(tsarr, 1);
  FOR i IN 1..card
  LOOP
    result[i] = tbool_inst(random_bool(), tsarr[i]);
  END LOOP;
  RETURN tbool_seq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tbool_discseq('2001-01-01', '2002-01-01', 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tint discrete sequence
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 */
DROP FUNCTION IF EXISTS random_tint_discseq;
CREATE FUNCTION random_tint_discseq(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, mincard int, maxcard int)
  RETURNS tint AS $$
DECLARE
  intarr int[];
  tsarr timestamptz[];
  result tint[];
  card int;
BEGIN
  SELECT random_int_array(lowvalue, highvalue, maxdelta, mincard, maxcard) INTO intarr;
  card = array_length(intarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tint_inst(intarr[i], tsarr[i]);
  END LOOP;
  RETURN tint_seq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tint_discseq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tfloat discrete sequence
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 */
DROP FUNCTION IF EXISTS random_tfloat_discseq;
CREATE FUNCTION random_tfloat_discseq(lowvalue float, highvalue float,
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int,
  mincard int, maxcard int)
  RETURNS tfloat AS $$
DECLARE
  floatarr float[];
  tsarr timestamptz[];
  result tfloat[];
  card int;
BEGIN
  SELECT random_float_array(lowvalue, highvalue, maxdelta, mincard, maxcard)
  INTO floatarr;
  card = array_length(floatarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card)
  INTO tsarr;
  FOR i IN 1..card
  LOOP
    result[i] = tfloat_inst(floatarr[i], tsarr[i]);
  END LOOP;
  RETURN tfloat_seq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tfloat_discseq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random ttext discrete sequence
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxlength Maximum length of the text value
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 */
DROP FUNCTION IF EXISTS random_ttext_discseq;
CREATE FUNCTION random_ttext_discseq(lowtime timestamptz, hightime timestamptz,
  maxlength int, maxminutes int, mincard int, maxcard int)
  RETURNS ttext AS $$
DECLARE
  tsarr timestamptz[];
  result ttext[];
  card int;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard, maxcard)
  INTO tsarr;
  card = array_length(tsarr, 1);
  FOR i IN 1..card
  LOOP
    result[i] = ttext_inst(random_text(maxlength), tsarr[i]);
  END LOOP;
  RETURN ttext_seq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ttext_discseq('2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- Temporal Continuous Sequence
-------------------------------------------------------------------------------

/**
 * Generate a random tbool sequence
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tbool_seq;
CREATE FUNCTION random_tbool_seq(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, fixstart bool DEFAULT false)
  RETURNS tbool AS $$
DECLARE
  tsarr timestamptz[];
  result tbool[];
  card int;
  v bool;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard, fixstart) INTO tsarr;
  card = array_length(tsarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  v = random_bool();
  FOR i IN 1..card - 1
  LOOP
    result[i] = tbool_inst(v, tsarr[i]);
    v = NOT v;
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    v = NOT v;
  END IF;
  result[card] = tbool_inst(v, tsarr[card]);
  RETURN tbool_seq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tbool_seq('2001-01-01', '2002-01-01', 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tint sequence
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tint_seq;
CREATE FUNCTION random_tint_seq(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, mincard int, maxcard int,
  fixstart bool DEFAULT false)
  RETURNS tint AS $$
DECLARE
  intarr int[];
  tsarr timestamptz[];
  result tint[];
  card int;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_int_array(lowvalue, highvalue, maxdelta, mincard, maxcard)
  INTO intarr;
  card = array_length(intarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    result[i] = tint_inst(intarr[i], tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tint_inst(intarr[card - 1], tsarr[card]);
  ELSE
    result[card] = tint_inst(intarr[card], tsarr[card]);
  END IF;
  RETURN tint_seq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tint_seq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tfloat sequence
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @parap[in] linear True for linear sequence, false for step sequence
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tfloat_seq;
CREATE FUNCTION random_tfloat_seq(lowvalue float, highvalue float,
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int,
  mincard int, maxcard int, linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS tfloat AS $$
DECLARE
  floatarr float[];
  tsarr timestamptz[];
  result tfloat[];
  card int;
  interp text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_float_array(lowvalue, highvalue, maxdelta, mincard, maxcard)
  INTO floatarr;
  card = array_length(floatarr, 1);
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, card, card,
    fixstart) INTO tsarr;
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    result[i] = tfloat_inst(floatarr[i], tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tfloat_inst(floatarr[card - 1], tsarr[card]);
  ELSE
    result[card] = tfloat_inst(floatarr[card], tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tfloat_seq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tfloat_seq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;

-- Step interpolation
SELECT k, random_tfloat_seq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, false) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random ttext sequence
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxlength Maximum length of the text value
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_ttext_seq;
CREATE FUNCTION random_ttext_seq(lowtime timestamptz, hightime timestamptz,
  maxlength int, maxminutes int, mincard int, maxcard int,
  fixstart bool DEFAULT false)
  RETURNS ttext AS $$
DECLARE
  tsarr timestamptz[];
  result ttext[];
  card int;
  v text;
  lower_inc boolean;
  upper_inc boolean;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard, fixstart) INTO tsarr;
  card = array_length(tsarr, 1);
  IF card = 1 THEN
    lower_inc = true;
    upper_inc = true;
  ELSE
    lower_inc = random() > 0.5;
    upper_inc = random() > 0.5;
  END IF;
  FOR i IN 1..card - 1
  LOOP
    v = random_text(maxlength);
    result[i] = ttext_inst(v, tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card = 1 OR upper_inc THEN
    v = random_text(maxlength);
  END IF;
  result[card] = ttext_inst(v, tsarr[card]);
  RETURN ttext_seq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ttext_seq('2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- Temporal Sequence Set
-------------------------------------------------------------------------------

/**
 * Function ensuring that the duration defined by hightime - lowtime is enough
 * for generating the sequence set given the other parameters
 * lowtime must be less than hightime -
 * ( (maxminutes * cardseq * card) + (card * maxminutes) ) minutes
 * where cardseq = (maxcardseq - mincardseq) and card = (maxcard - mincard)
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS tsequenceset_valid_duration;
CREATE FUNCTION tsequenceset_valid_duration(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS void AS $$
BEGIN
  IF lowtime >= hightime THEN
    RAISE EXCEPTION 'lowtime must be less than or equal to hightime: %, %',
      lowtime, hightime;
  END IF;
  IF mincardseq > maxcardseq THEN
    RAISE EXCEPTION 'mincardseq must be less than or equal to maxcardseq: %, %',
      mincardseq, maxcardseq;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  IF lowtime > hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) + ((maxcard - mincard) * maxminutes) ) THEN
    RAISE EXCEPTION
      'The duration between lowtime and hightime is not enough for generating the temporal sequence set';
  END IF;
END;
$$ LANGUAGE PLPGSQL STRICT;

-------------------------------------------------------------------------------

/**
 * Generate a random tbool sequence set
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS random_tbool_seqset;
CREATE FUNCTION random_tbool_seqset(lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS tbool AS $$
DECLARE
  result tbool[];
  seq tbool;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter is set to true for all i except 1
    SELECT random_tbool_seq(t1, t2, maxminutes, mincardseq, maxcardseq, i > 1)
    INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tbool_seqset(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tbool_seqset('2001-01-01', '2002-01-01', 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tint sequence set
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS random_tint_seqset;
CREATE FUNCTION random_tint_seqset(lowvalue int, highvalue int, lowtime timestamptz,
  hightime timestamptz, maxdelta int, maxminutes int, mincardseq int,
  maxcardseq int, mincard int, maxcard int)
  RETURNS tint AS $$
DECLARE
  result tint[];
  seq tint;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tint_seq(lowvalue, highvalue, t1, t2, maxdelta,
      maxminutes, mincardseq, maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tint_seqset(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tint_seqset(1, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random tfloat sequence set
 *
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS random_tfloat_seqset;
CREATE FUNCTION random_tfloat_seqset(lowvalue float, highvalue float,
  lowtime timestamptz, hightime timestamptz, maxdelta float, maxminutes int,
  mincardseq int, maxcardseq int, mincard int, maxcard int,
  linear bool DEFAULT true)
  RETURNS tfloat AS $$
DECLARE
  result tfloat[];
  seq tfloat;
  card float;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  IF lowvalue > highvalue THEN
    RAISE EXCEPTION 'lowvalue must be less than or equal to highvalue: %, %',
      lowvalue, highvalue;
  END IF;
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_tfloat_seq(lowvalue, highvalue, t1, t2, maxdelta,
      maxminutes, mincardseq, maxcardseq, linear, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tfloat_seqset(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tfloat_seqset(1, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;

-- Step interpolation
SELECT k, random_tfloat_seqset(1, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10, false) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random ttext sequence set
 *
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxlength Maximum length of the text value
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS random_ttext_seqset;
CREATE FUNCTION random_ttext_seqset(lowtime timestamptz, hightime timestamptz,
  maxlength int, maxminutes int, mincardseq int, maxcardseq int,
  mincard int, maxcard int)
  RETURNS ttext AS $$
DECLARE
  result ttext[];
  seq ttext;
  card int;
  t1 timestamptz;
  t2 timestamptz;
BEGIN
  PERFORM tsequenceset_valid_duration(lowtime, hightime, maxminutes, mincardseq,
    maxcardseq, mincard, maxcard);
  card = random_int(mincard, maxcard);
  t1 = lowtime;
  t2 = hightime - interval '1 minute' *
    ( (maxminutes * (maxcardseq - mincardseq) * (maxcard - mincard)) +
    ((maxcard - mincard) * maxminutes) );
  FOR i IN 1..card
  LOOP
    -- the last parameter (fixstart) is set to true for all i except 1
    SELECT random_ttext_seq(t1, t2, maxlength, maxminutes, mincardseq, maxcardseq, i > 1)
    INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN ttext_seqset(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ttext_seqset('2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
