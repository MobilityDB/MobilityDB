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

/*
 * random_th3index.sql
 * Basic synthetic data generator functions for the static H3 cell index type
 * and the temporal H3 cell index type. Cells are always VALID: each is built
 * from a random WGS84 (SRID 4326) point via geoToH3Cell, so the h3-semantic
 * operators (lat/lng, hierarchy, traversal, metrics) exercise real cells.
 * th3index has step interpolation only, so there is no continuous-sequence
 * generator.
 */

------------------------------------------------------------------------------
-- Static H3 cell index type
------------------------------------------------------------------------------

/**
 * @brief Generate a random valid H3 cell at a random resolution
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution (0..15)
 */
DROP FUNCTION IF EXISTS random_h3index;
CREATE FUNCTION random_h3index(lowres int DEFAULT 0, highres int DEFAULT 10)
  RETURNS h3index AS $$
BEGIN
  IF lowres < 0 OR highres > 15 OR lowres > highres THEN
    RAISE EXCEPTION 'lowres/highres must satisfy 0 <= lowres <= highres <= 15: %, %',
      lowres, highres;
  END IF;
  RETURN geoToH3Cell(ST_SetSRID(random_geom_point(-180, 180, -90, 90), 4326),
    random_int(lowres, highres));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_h3index(0, 10) AS cell
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate an array of random H3 cells
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the array
 */
DROP FUNCTION IF EXISTS random_h3index_array;
CREATE FUNCTION random_h3index_array(lowres int, highres int, mincard int,
    maxcard int)
  RETURNS h3index[] AS $$
DECLARE
  result h3index[];
  card int;
BEGIN
  IF mincard > maxcard THEN
    RAISE EXCEPTION 'mincard must be less than or equal to maxcard: %, %',
      mincard, maxcard;
  END IF;
  card = random_int(mincard, maxcard);
  FOR i IN 1..card
  LOOP
    result[i] = random_h3index(lowres, highres);
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_h3index_array(0, 10, 5, 10) AS arr
FROM generate_series(1, 10) AS k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random set of H3 cells
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the set
 */
DROP FUNCTION IF EXISTS random_h3index_set;
CREATE FUNCTION random_h3index_set(lowres int, highres int, mincard int,
    maxcard int)
  RETURNS h3indexset AS $$
BEGIN
  RETURN set(random_h3index_array(lowres, highres, mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_h3index_set(0, 10, 5, 10) AS s
FROM generate_series(1, 10) AS k;
*/

------------------------------------------------------------------------------
-- Temporal H3 cell index type (step interpolation only)
------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal H3 cell index of instant subtype
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_th3index_inst;
CREATE FUNCTION random_th3index_inst(lowres int, highres int,
  lowtime timestamptz, hightime timestamptz)
  RETURNS th3index AS $$
BEGIN
  RETURN th3index(random_h3index(lowres, highres),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_inst(0, 10, '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal H3 cell index of discrete sequence subtype
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 */
DROP FUNCTION IF EXISTS random_th3index_discseq;
CREATE FUNCTION random_th3index_discseq(lowres int, highres int,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int)
  RETURNS th3index AS $$
DECLARE
  result th3index[];
  card int;
  t timestamptz;
BEGIN
  card = random_int(mincard, maxcard);
  t = random_timestamptz(lowtime, hightime);
  FOR i IN 1..card
  LOOP
    result[i] = th3index(random_h3index(lowres, highres), t);
    t = t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN th3indexSeq(result, 'discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_discseq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS ti
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal H3 cell index of sequence subtype (step)
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the number of instants
 * @param[in] fixstart True when the sequence starts at lowtime
 */
DROP FUNCTION IF EXISTS random_th3index_seq;
CREATE FUNCTION random_th3index_seq(lowres int, highres int,
  lowtime timestamptz, hightime timestamptz, maxminutes int, mincard int,
  maxcard int, fixstart bool DEFAULT false)
  RETURNS th3index AS $$
DECLARE
  tsarr timestamptz[];
  result th3index[];
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
    result[i] = th3index(random_h3index(lowres, highres), tsarr[i]);
  END LOOP;
  -- Step interpolation with exclusive upper bound must repeat the previous
  -- value in the last two instants
  IF card <> 1 AND NOT upper_inc THEN
    result[card] = th3index(getValue(result[card - 1]), tsarr[card]);
  ELSE
    result[card] = th3index(random_h3index(lowres, highres), tsarr[card]);
  END IF;
  RETURN th3indexSeq(result, 'step', lower_inc, upper_inc);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_seq(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10) AS seq
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------

/**
 * @brief Generate a random temporal H3 cell index of sequence set subtype
 * @param[in] lowres, highres Inclusive bounds of the H3 resolution
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the number of instants
 * in a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the number of sequences
 */
DROP FUNCTION IF EXISTS random_th3index_seqset;
CREATE FUNCTION random_th3index_seqset(lowres int, highres int,
  lowtime timestamptz, hightime timestamptz, maxminutes int,
  mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS th3index AS $$
DECLARE
  result th3index[];
  card int;
  seq th3index;
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
    SELECT random_th3index_seq(lowres, highres, t1, t2, maxminutes, mincardseq,
      maxcardseq, i > 1) INTO seq;
    result[i] = seq;
    t1 = endTimestamp(seq) + random_minutes(1, maxminutes);
    t2 = t2 + interval '1 minute' * maxminutes * (1 + maxcardseq - mincardseq);
  END LOOP;
  RETURN th3indexSeqSet(result);
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_seqset(0, 10, '2001-01-01', '2001-12-31', 10, 5, 10, 5, 10) AS ss
FROM generate_series(1, 10) k;
*/

-------------------------------------------------------------------------------
