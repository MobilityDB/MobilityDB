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
 * documentation FOR any purpose, without fee, and without a written
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
 * random_tnpoint.sql
 * Basic synthetic data generator functions FOR network point types
 * and temporal network point types.
 */

------------------------------------------------------------------------------
-- Static Network Types
------------------------------------------------------------------------------

/**
 * Generate a random fraction between in the range [0,1]
 */
DROP FUNCTION IF EXISTS random_fraction;
CREATE FUNCTION random_fraction()
  RETURNS float AS $$
BEGIN
  RETURN random();
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_fraction() AS f
FROM generate_series(1,10) k;
*/

/**
 * Generate a random network point
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 */
DROP FUNCTION IF EXISTS random_npoint;
CREATE FUNCTION random_npoint(lown integer, highn integer)
  RETURNS npoint AS $$
BEGIN
  RETURN npoint(random_int(lown, highn), random_fraction());
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_npoint(1, 100) AS g
FROM generate_series(1,10) k;
*/

/**
 * Generate a random network segment
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 */
DROP FUNCTION IF EXISTS random_nsegment;
CREATE FUNCTION random_nsegment(lown integer, highn integer)
  RETURNS nsegment AS $$
DECLARE
  random1 float;
  random2 float;
  tmp float;
BEGIN
  random1 = random_fraction();
  random2 = random_fraction();
  IF random1 > random2 THEN
    tmp = random1;
    random1 = random2;
    random2 = tmp;
  END IF;
  RETURN nsegment(random_int(lown, highn), random1, random2);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_nsegment(1, 100) AS g
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random network points
 *
 * @param[in] lown, highn Inclusive bounds of the range for the rid
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_npoint_array;
CREATE FUNCTION random_npoint_array(lown integer, highn integer, mincard int,
    maxcard int)
  RETURNS npoint[] AS $$
DECLARE
  result npoint[];
  card int;
BEGIN
  IF lown > highn THEN
    RAISE EXCEPTION 'lown must be less than or equal to highn: %, %',
      lowy, highy;
  END IF;
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_npoint(lown, highn);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_npoint_array(1, 100, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate an array of random network points
 *
 * @param[in] lown, highn Inclusive bounds of the range for the rid
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_npoint_set;
CREATE FUNCTION random_npoint_set(lown integer, highn integer, mincard int,
    maxcard int)
  RETURNS npointset AS $$
DECLARE
  nparr npoint[];
BEGIN
  RETURN set(random_npoint_array(lown, highn, mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, asewkt(random_npoint_set(1, 100, 5, 10)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, asewkt(random_npoint_set(1, 100, 5, 10, 3812)) AS g
FROM generate_series(1, 15) AS k;

SELECT k, random_npoint_set(1, 100, 5, 10) AS g
FROM generate_series(1, 15) AS k;
*/

------------------------------------------------------------------------------
-- Temporal Network Types
------------------------------------------------------------------------------

/**
 * Generate a random temporal network point of instant subtype
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_tnpoint_inst;
CREATE FUNCTION random_tnpoint_inst(lown integer, highn integer,
  lowtime timestamptz, hightime timestamptz)
  RETURNS tnpoint AS $$
BEGIN
  RETURN tnpoint_inst(random_npoint(lown, highn),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tnpoint_inst(0, 100, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random temporal network point of discrete sequence subtype
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 */
DROP FUNCTION IF EXISTS random_tnpoint_discseq;
CREATE FUNCTION random_tnpoint_discseq(lown integer, highn integer,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int)
  RETURNS tnpoint AS $$
DECLARE
  result tnpoint[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(1, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = tnpoint_inst(random_npoint(lown, highn), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tnpoint_seq(result, 'Discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tnpoint_discseq(0, 100, '2001-01-01', '2001-12-31', 10, 10, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random temporal network point of sequence subtype
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] linear True when the sequence has linear interpolation
 * @param[in] fixstart True when this function is called for generating a
 *    sequence set value and in this case the start timestamp is already fixed
 */
DROP FUNCTION IF EXISTS random_tnpoint_contseq;
CREATE FUNCTION random_tnpoint_contseq(lown integer, highn integer,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int, linear bool DEFAULT true, fixstart bool DEFAULT false)
  RETURNS tnpoint AS $$
DECLARE
  tsarr timestamptz[];
  result tnpoint[];
  card int;
  rid int;
  t1 timestamptz;
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
  rid = random_int(lown, highn);
  FOR i IN 1..card - 1
  LOOP
    result[i] = tnpoint_inst(npoint(rid, random()), tsarr[i]);
  END LOOP;
  -- Sequences with step interpolation and exclusive upper bound must have
  -- the same value in the last two instants
  IF card <> 1 AND NOT upper_inc AND NOT linear THEN
    result[card] = tnpoint_inst(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = tnpoint_inst(npoint(rid, random()), tsarr[card]);
  END IF;
  IF linear THEN
    interp = 'Linear';
  ELSE
    interp = 'Step';
  END IF;
  RETURN tnpoint_seq(result, interp, lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tnpoint_contseq(0, 100, '2001-01-01', '2001-12-31', 10, 10, 10)
FROM generate_series (1, 15) AS k;

SELECT k, random_tnpoint_contseq(0, 100, '2001-01-01', '2001-12-31', 10, 10, 10, false)
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------

/**
 * Generate a random temporal network point of sequence set subtype
 *
 * @param[in] lown, highn Inclusive bounds of the range for the identifier of
 * the network point
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants in a
 * sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 * @param[in] linear True when the sequence set has linear interpolation
 */
DROP FUNCTION IF EXISTS random_tnpoint_seqset;
CREATE FUNCTION random_tnpoint_seqset(lown integer, highn integer,
  lowtime timestamptz, hightime timestamptz, maxminutes int,
  mincardseq int, maxcardseq int, mincard int, maxcard int,
  linear bool DEFAULT true)
  RETURNS tnpoint AS $$
DECLARE
  result tnpoint[];
  card int;
  seq tnpoint;
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
    SELECT random_tnpoint_contseq(lown, highn, t1, t2, maxminutes, mincardseq,
      maxcardseq, linear, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN tnpoint_seqset(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_tnpoint_seqset(0, 100, '2001-01-01', '2001-12-31', 10, 10, 10, 10, 10) AS ts
FROM generate_series (1, 15) AS k;

SELECT k, random_tnpoint_seqset(0, 100, '2001-01-01', '2001-12-31', 10, 10, 10, 10, 10, false) AS ts
FROM generate_series (1, 15) AS k;
*/

-------------------------------------------------------------------------------
