/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Basic synthetic data generator functions for the static JSONB type and
 * the temporal JSONB type
 * @details A value is the document of a vehicle at a location: an identifier, a
 * point as its hexadecimal EWKB and a timestamp. tjsonb has step interpolation
 * only, so a sequence does not take an interpolation.
 */

------------------------------------------------------------------------------
-- Static JSONB type
------------------------------------------------------------------------------

/**
 * @brief Generate a random JSONB document
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the timestamp
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_jsonb;
CREATE FUNCTION random_jsonb(lowx float, highx float, lowy float, highy float,
  lowtime timestamptz, hightime timestamptz, lowid int, highid int,
  srid int DEFAULT 0)
  RETURNS jsonb AS $$
BEGIN
  RETURN jsonb_build_object(
    'vehicleId', random_int(lowid, highid)::text,
    'location', jsonb_build_object(
      'geom', random_geom_point(lowx, highx, lowy, highy, srid)::text,
      'timestamp', random_timestamptz(lowtime, hightime)::text));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_jsonb(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 1, 100) AS jb
FROM generate_series(1, 10) k;

SELECT k, random_jsonb(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 1, 100, 3812) AS jb
FROM generate_series(1, 10) k;

-- Errors
SELECT k, random_jsonb(100, -100, -100, 100, '2001-01-01', '2001-12-31', 1, 100) AS jb
FROM generate_series(1, 10) k;
SELECT k, random_jsonb(-100, 100, 100, -100, '2001-01-01', '2001-12-31', 1, 100) AS jb
FROM generate_series(1, 10) k;
SELECT k, random_jsonb(-100, 100, -100, 100, '2001-12-31', '2001-01-01', 1, 100) AS jb
FROM generate_series(1, 10) k;
SELECT k, random_jsonb(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 100, 1) AS jb
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random JSONB documents
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the timestamp
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_jsonb_array;
CREATE FUNCTION random_jsonb_array(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, lowid int, highid int,
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS jsonb[] AS $$
DECLARE
  result jsonb[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_jsonb(lowx, highx, lowy, highy, lowtime, hightime, lowid,
      highid, srid);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_jsonb_array(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 1, 100, 1, 10) AS arr
FROM generate_series(1, 10) AS k;

-- Errors
SELECT k, random_jsonb_array(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 1, 100, 10, 1) AS arr
FROM generate_series(1, 10) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random set of JSONB documents
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowtime, hightime Inclusive bounds of the timestamp
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_jsonb_set;
CREATE FUNCTION random_jsonb_set(lowx float, highx float, lowy float,
  highy float, lowtime timestamptz, hightime timestamptz, lowid int, highid int,
  mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS jsonbset AS $$
BEGIN
  RETURN set(random_jsonb_array(lowx, highx, lowy, highy, lowtime, hightime,
    lowid, highid, mincard, maxcard, srid));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_jsonb_set(-100, 100, -100, 100, '2001-01-01', '2001-12-31', 1, 100, 1, 10) AS s
FROM generate_series(1, 10) AS k;
*/

------------------------------------------------------------------------------
-- Temporal JSONB type (step interpolation only)
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal JSONB of instant subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan, also those of
 * the timestamp of the document
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_tjsonb_inst;
CREATE FUNCTION random_tjsonb_inst(lowx float, highx float, lowy float,
  highy float, lowid int, highid int, lowtime timestamptz, hightime timestamptz,
  srid int DEFAULT 0)
  RETURNS tjsonb AS $$
BEGIN
  RETURN tjsonb(random_jsonb(lowx, highx, lowy, highy, lowtime, hightime, lowid,
    highid, srid), random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tjsonb_inst(-100, 100, -100, 100, 1, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal JSONB of discrete sequence subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan, also those of
 * the timestamp of the document
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_tjsonb_discseq;
CREATE FUNCTION random_tjsonb_discseq(lowx float, highx float, lowy float,
  highy float, lowid int, highid int, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, srid int DEFAULT 0)
  RETURNS tjsonb AS $$
DECLARE
  tsarr timestamptz[];
  result tjsonb[];
  card int;
BEGIN
  SELECT random_timestamptz_array(lowtime, hightime, maxminutes, mincard,
    maxcard) INTO tsarr;
  card = array_length(tsarr, 1);
  FOR i IN 1..card
  LOOP
    result[i] = tjsonb(random_jsonb(lowx, highx, lowy, highy, lowtime, hightime,
      lowid, highid, srid), tsarr[i]);
  END LOOP;
  RETURN tjsonbSeq(result, 'discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tjsonb_discseq(-100, 100, -100, 100, 1, 100, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal JSONB of sequence subtype (step)
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan, also those of
 * the timestamp of the document
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] srid Optional SRID of the point
 * @param[in] fixstart True when the sequence starts at lowtime
 */
DROP FUNCTION IF EXISTS random_tjsonb_seq;
CREATE FUNCTION random_tjsonb_seq(lowx float, highx float, lowy float,
  highy float, lowid int, highid int, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincard int, maxcard int, srid int DEFAULT 0,
  fixstart bool DEFAULT false)
  RETURNS tjsonb AS $$
DECLARE
  tsarr timestamptz[];
  result tjsonb[];
  card int;
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
    result[i] = tjsonb(random_jsonb(lowx, highx, lowy, highy, lowtime, hightime,
      lowid, highid, srid), tsarr[i]);
  END LOOP;
  -- Step interpolation with exclusive upper bound must repeat the previous
  -- value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = tjsonb(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tjsonb(random_jsonb(lowx, highx, lowy, highy, lowtime,
      hightime, lowid, highid, srid), tsarr[card]);
  END IF;
  RETURN tjsonbSeq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tjsonb_seq(-100, 100, -100, 100, 1, 100, '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal JSONB of sequence set subtype
 * @param[in] lowx, highx Inclusive bounds of the range for the x coordinates
 * @param[in] lowy, highy Inclusive bounds of the range for the y coordinates
 * @param[in] lowid, highid Inclusive bounds of the vehicle identifier
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan, also those of
 * the timestamp of the document
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants
 * in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] srid Optional SRID of the point
 */
DROP FUNCTION IF EXISTS random_tjsonb_seqset;
CREATE FUNCTION random_tjsonb_seqset(lowx float, highx float, lowy float,
  highy float, lowid int, highid int, lowtime timestamptz, hightime timestamptz,
  maxminutes int, mincardseq int, maxcardseq int, mincard int, maxcard int,
  srid int DEFAULT 0)
  RETURNS tjsonb AS $$
DECLARE
  result tjsonb[];
  card int;
  seq tjsonb;
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
    -- fixstart is true for all sequences except the first
    SELECT random_tjsonb_seq(lowx, highx, lowy, highy, lowid, highid, t1, t2,
      maxminutes, mincardseq, maxcardseq, srid, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tjsonbSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tjsonb_seqset(-100, 100, -100, 100, 1, 100, '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------
