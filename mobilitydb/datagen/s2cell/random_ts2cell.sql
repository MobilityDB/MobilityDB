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
 * @brief Basic synthetic data generator functions for the static S2 cell
 * index type and the temporal S2 cell index type
 * @details Cells are always VALID: each
 * is built from a random WGS84 (SRID 4326) point via geoToS2Cell, so the
 * bounding-box operators exercise real cells. ts2cell has step interpolation
 * only, so there is no continuous-sequence generator.
 */

------------------------------------------------------------------------------
-- Static S2 cell index type
------------------------------------------------------------------------------

/**
 * @brief Generate a random valid S2 cell at a random level
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level (0..30)
 */
DROP FUNCTION IF EXISTS random_s2cell;
CREATE FUNCTION random_s2cell(lowlevel int DEFAULT 0, highlevel int DEFAULT 10)
  RETURNS s2cell AS $$
BEGIN
  IF lowlevel < 0 OR highlevel > 30 OR lowlevel > highlevel THEN
    RAISE EXCEPTION 'lowlevel/highlevel must satisfy 0 <= lowlevel <= highlevel <= 30: %, %',
      lowlevel, highlevel;
  END IF;
  RETURN geoToS2Cell(random_geog_point(-180, 180, -90, 90),
    random_int(lowlevel, highlevel));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_s2cell(0, 10) AS cell
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random S2 cells
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_s2cell_array;
CREATE FUNCTION random_s2cell_array(lowlevel int, highlevel int, mincard int,
    maxcard int)
  RETURNS s2cell[] AS $$
DECLARE
  result s2cell[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_s2cell(lowlevel, highlevel);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_s2cell_array(0, 10, 5, 10) AS arr
FROM generate_series(1, 10) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random set of S2 cells
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 */
DROP FUNCTION IF EXISTS random_s2cell_set;
CREATE FUNCTION random_s2cell_set(lowlevel int, highlevel int, mincard int,
    maxcard int)
  RETURNS s2cellset AS $$
BEGIN
  RETURN set(random_s2cell_array(lowlevel, highlevel, mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_s2cell_set(0, 10, 5, 10) AS s
FROM generate_series(1, 10) AS k;
*/

------------------------------------------------------------------------------
-- Temporal S2 cell index type (step interpolation only)
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal S2 cell index of instant subtype
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_ts2cell_inst;
CREATE FUNCTION random_ts2cell_inst(lowlevel int, highlevel int,
  lowtime timestamptz, hightime timestamptz)
  RETURNS ts2cell AS $$
BEGIN
  RETURN ts2cell(random_s2cell(lowlevel, highlevel),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ts2cell_inst(0, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal S2 cell index of discrete sequence
 * subtype
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 */
DROP FUNCTION IF EXISTS random_ts2cell_discseq;
CREATE FUNCTION random_ts2cell_discseq(lowlevel int, highlevel int,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int)
  RETURNS ts2cell AS $$
DECLARE
  result ts2cell[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(mincard, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = ts2cell(random_s2cell(lowlevel, highlevel), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN ts2cellSeq(result, 'discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ts2cell_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS ti
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal S2 cell index of sequence subtype
 * (step)
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] fixstart True when the sequence starts at lowtime
 */
DROP FUNCTION IF EXISTS random_ts2cell_seq;
CREATE FUNCTION random_ts2cell_seq(lowlevel int, highlevel int,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int, fixstart bool DEFAULT false)
  RETURNS ts2cell AS $$
DECLARE
  tsarr timestamptz[];
  result ts2cell[];
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
    result[i] = ts2cell(random_s2cell(lowlevel, highlevel), tsarr[i]);
  END LOOP;
  -- Step interpolation with exclusive upper bound must repeat the previous
  -- value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = ts2cell(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = ts2cell(random_s2cell(lowlevel, highlevel), tsarr[card]);
  END IF;
  RETURN ts2cellSeq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ts2cell_seq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS seq
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal S2 cell index of sequence set subtype
 * @param[in] lowlevel, highlevel Inclusive bounds of the S2 level
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants
 * in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 */
DROP FUNCTION IF EXISTS random_ts2cell_seqset;
CREATE FUNCTION random_ts2cell_seqset(lowlevel int, highlevel int,
  lowtime timestamptz, hightime timestamptz, maxminutes int,
  mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS ts2cell AS $$
DECLARE
  result ts2cell[];
  card int;
  seq ts2cell;
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
    SELECT random_ts2cell_seq(lowlevel, highlevel, t1, t2, maxminutes, mincardseq,
      maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN ts2cellSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_ts2cell_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10, 5, 10) AS ss
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------
