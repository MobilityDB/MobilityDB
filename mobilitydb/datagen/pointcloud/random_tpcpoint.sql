/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * random_tpcpoint.sql
 * Synthetic data generator for the pgpointcloud temporal types
 * (pcpoint, pcpatch, pcpointset, pcpatchset, tpcbox, tpcpoint, tpcpatch).
 *
 * All randomly generated values share a single pcid.  The datagen
 * schema (pcid 1, SRID 0, three int32 dimensions X/Y/Z scaled by 0.01)
 * is materialized on demand via @ref ensure_random_pcid().
 */

------------------------------------------------------------------------------
-- Schema bootstrap
------------------------------------------------------------------------------

/**
 * @brief Ensure a default pgpointcloud schema (pcid 1) exists.
 * @details Inserts a minimal three-dimension XYZ schema if pointcloud_formats
 *   has no row with pcid 1. Idempotent — a real schema with that pcid is
 *   left in place. All random functions in this file use pcid 1.
 */
DROP FUNCTION IF EXISTS ensure_random_pcid();
CREATE FUNCTION ensure_random_pcid()
RETURNS integer AS $$
BEGIN
  INSERT INTO pointcloud_formats (pcid, srid, schema)
  VALUES (1, 0,
'<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <pc:dimension>
    <pc:position>1</pc:position>
    <pc:size>4</pc:size>
    <pc:description>X coordinate</pc:description>
    <pc:name>X</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>2</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Y coordinate</pc:description>
    <pc:name>Y</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:dimension>
    <pc:position>3</pc:position>
    <pc:size>4</pc:size>
    <pc:description>Z coordinate</pc:description>
    <pc:name>Z</pc:name>
    <pc:interpretation>int32_t</pc:interpretation>
    <pc:scale>0.01</pc:scale>
  </pc:dimension>
  <pc:metadata>
    <Metadata name="srid">0</Metadata>
  </pc:metadata>
</pc:PointCloudSchema>')
  ON CONFLICT (pcid) DO NOTHING;
  RETURN 1;
END;
$$ LANGUAGE PLPGSQL;

------------------------------------------------------------------------------
-- Static pcpoint
------------------------------------------------------------------------------

/**
 * @brief Generate a random pcpoint value
 * @param[in] lowx, highx, lowy, highy, lowz, highz Inclusive bounds for the
 *   X / Y / Z dimensions
 */
DROP FUNCTION IF EXISTS random_pcpoint;
CREATE FUNCTION random_pcpoint(lowx float, highx float, lowy float,
  highy float, lowz float, highz float)
  RETURNS pcpoint AS $$
BEGIN
  PERFORM ensure_random_pcid();
  IF lowx > highx OR lowy > highy OR lowz > highz THEN
    RAISE EXCEPTION 'Low bounds must be less than or equal to high bounds';
  END IF;
  RETURN PC_MakePoint(1, ARRAY[
    random_float(lowx, highx),
    random_float(lowy, highy),
    random_float(lowz, highz)]);
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate an array of distinct random pcpoint values
 */
DROP FUNCTION IF EXISTS random_pcpoint_array;
CREATE FUNCTION random_pcpoint_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int)
  RETURNS pcpoint[] AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  result pcpoint[];
BEGIN
  PERFORM ensure_random_pcid();
  FOR i IN 1 .. card LOOP
    result := array_append(result,
      random_pcpoint(lowx, highx, lowy, highy, lowz, highz));
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random pcpointset
 */
DROP FUNCTION IF EXISTS random_pcpointset;
CREATE FUNCTION random_pcpointset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int)
  RETURNS pcpointset AS $$
BEGIN
  RETURN set(random_pcpoint_array(lowx, highx, lowy, highy, lowz, highz,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

------------------------------------------------------------------------------
-- Static pcpatch
------------------------------------------------------------------------------

/**
 * @brief Generate a random pcpatch (compressed batch of N pcpoints)
 */
DROP FUNCTION IF EXISTS random_pcpatch;
CREATE FUNCTION random_pcpatch(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, mincard int, maxcard int)
  RETURNS pcpatch AS $$
BEGIN
  RETURN PC_Patch(random_pcpoint_array(lowx, highx, lowy, highy, lowz, highz,
    mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate an array of random pcpatch values
 */
DROP FUNCTION IF EXISTS random_pcpatch_array;
CREATE FUNCTION random_pcpatch_array(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, minpts int, maxpts int,
  mincard int, maxcard int)
  RETURNS pcpatch[] AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  result pcpatch[];
BEGIN
  FOR i IN 1 .. card LOOP
    result := array_append(result,
      random_pcpatch(lowx, highx, lowy, highy, lowz, highz, minpts, maxpts));
  END LOOP;
  RETURN result;
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random pcpatchset
 */
DROP FUNCTION IF EXISTS random_pcpatchset;
CREATE FUNCTION random_pcpatchset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, minpts int, maxpts int,
  mincard int, maxcard int)
  RETURNS pcpatchset AS $$
BEGIN
  RETURN set(random_pcpatch_array(lowx, highx, lowy, highy, lowz, highz,
    minpts, maxpts, mincard, maxcard));
END;
$$ LANGUAGE PLPGSQL STRICT;

------------------------------------------------------------------------------
-- TPCBox
------------------------------------------------------------------------------

/**
 * @brief Generate a random tpcbox with X, Y, Z and time dimensions
 */
DROP FUNCTION IF EXISTS random_tpcbox;
CREATE FUNCTION random_tpcbox(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tpcbox AS $$
DECLARE
  xmin float := random_float(lowx, highx);
  xmax float := random_float(xmin, highx);
  ymin float := random_float(lowy, highy);
  ymax float := random_float(ymin, highy);
  zmin float := random_float(lowz, highz);
  zmax float := random_float(zmin, highz);
  period tstzspan := random_tstzspan(lowtime, hightime, 30);
BEGIN
  PERFORM ensure_random_pcid();
  RETURN tpcbox_zt(xmin, ymin, zmin, xmax, ymax, zmax, period, 1, 0);
END;
$$ LANGUAGE PLPGSQL STRICT;

------------------------------------------------------------------------------
-- Temporal pcpoint
------------------------------------------------------------------------------

/**
 * @brief Generate a random tpcpoint of subtype Instant
 */
DROP FUNCTION IF EXISTS random_tpcpoint_inst;
CREATE FUNCTION random_tpcpoint_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz)
  RETURNS tpcpoint AS $$
BEGIN
  RETURN tpcpoint(
    random_pcpoint(lowx, highx, lowy, highy, lowz, highz),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpoint of subtype Sequence with discrete
 *   interpolation (a stream of unrelated instants)
 */
DROP FUNCTION IF EXISTS random_tpcpoint_discseq;
CREATE FUNCTION random_tpcpoint_discseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int)
  RETURNS tpcpoint AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t timestamptz := random_timestamptz(lowtime, hightime);
  result tpcpoint[];
BEGIN
  FOR i IN 1 .. card LOOP
    result := array_append(result, tpcpoint(
      random_pcpoint(lowx, highx, lowy, highy, lowz, highz), t));
    t := t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpointSeq(result, 'discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpoint of subtype Sequence with step
 *   interpolation (linear is not meaningful for opaque pcpoint values)
 */
DROP FUNCTION IF EXISTS random_tpcpoint_contseq;
CREATE FUNCTION random_tpcpoint_contseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincard int, maxcard int)
  RETURNS tpcpoint AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t timestamptz := random_timestamptz(lowtime, hightime);
  result tpcpoint[];
BEGIN
  FOR i IN 1 .. card LOOP
    result := array_append(result, tpcpoint(
      random_pcpoint(lowx, highx, lowy, highy, lowz, highz), t));
    t := t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpointSeq(result, 'step');
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpoint of subtype SequenceSet
 */
DROP FUNCTION IF EXISTS random_tpcpoint_seqset;
CREATE FUNCTION random_tpcpoint_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, mincardseq int, maxcardseq int,
  mincard int, maxcard int)
  RETURNS tpcpoint AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t1 timestamptz := lowtime;
  t2 timestamptz;
  seqs tpcpoint[];
BEGIN
  FOR i IN 1 .. card LOOP
    t2 := t1 + random_minutes(maxminutes, maxminutes * 2);
    seqs := array_append(seqs,
      random_tpcpoint_contseq(lowx, highx, lowy, highy, lowz, highz,
        t1, t2, maxminutes, mincardseq, maxcardseq));
    t1 := t2 + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpointSeqSet(seqs);
END;
$$ LANGUAGE PLPGSQL STRICT;

------------------------------------------------------------------------------
-- Temporal pcpatch
------------------------------------------------------------------------------

/**
 * @brief Generate a random tpcpatch of subtype Instant
 */
DROP FUNCTION IF EXISTS random_tpcpatch_inst;
CREATE FUNCTION random_tpcpatch_inst(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, minpts int, maxpts int)
  RETURNS tpcpatch AS $$
BEGIN
  RETURN tpcpatch(
    random_pcpatch(lowx, highx, lowy, highy, lowz, highz, minpts, maxpts),
    random_timestamptz(lowtime, hightime));
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpatch of subtype Sequence with discrete
 *   interpolation
 */
DROP FUNCTION IF EXISTS random_tpcpatch_discseq;
CREATE FUNCTION random_tpcpatch_discseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, minpts int, maxpts int,
  mincard int, maxcard int)
  RETURNS tpcpatch AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t timestamptz := random_timestamptz(lowtime, hightime);
  result tpcpatch[];
BEGIN
  FOR i IN 1 .. card LOOP
    result := array_append(result, tpcpatch(
      random_pcpatch(lowx, highx, lowy, highy, lowz, highz, minpts, maxpts),
      t));
    t := t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpatchSeq(result, 'discrete');
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpatch of subtype Sequence with step
 *   interpolation
 */
DROP FUNCTION IF EXISTS random_tpcpatch_contseq;
CREATE FUNCTION random_tpcpatch_contseq(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, minpts int, maxpts int,
  mincard int, maxcard int)
  RETURNS tpcpatch AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t timestamptz := random_timestamptz(lowtime, hightime);
  result tpcpatch[];
BEGIN
  FOR i IN 1 .. card LOOP
    result := array_append(result, tpcpatch(
      random_pcpatch(lowx, highx, lowy, highy, lowz, highz, minpts, maxpts),
      t));
    t := t + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpatchSeq(result, 'step');
END;
$$ LANGUAGE PLPGSQL STRICT;

/**
 * @brief Generate a random tpcpatch of subtype SequenceSet
 */
DROP FUNCTION IF EXISTS random_tpcpatch_seqset;
CREATE FUNCTION random_tpcpatch_seqset(lowx float, highx float, lowy float,
  highy float, lowz float, highz float, lowtime timestamptz,
  hightime timestamptz, maxminutes int, minpts int, maxpts int,
  mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS tpcpatch AS $$
DECLARE
  card int := random_int(mincard, maxcard);
  t1 timestamptz := lowtime;
  t2 timestamptz;
  seqs tpcpatch[];
BEGIN
  FOR i IN 1 .. card LOOP
    t2 := t1 + random_minutes(maxminutes, maxminutes * 2);
    seqs := array_append(seqs,
      random_tpcpatch_contseq(lowx, highx, lowy, highy, lowz, highz,
        t1, t2, maxminutes, minpts, maxpts, mincardseq, maxcardseq));
    t1 := t2 + random_minutes(1, maxminutes);
  END LOOP;
  RETURN tpcpatchSeqSet(seqs);
END;
$$ LANGUAGE PLPGSQL STRICT;

------------------------------------------------------------------------------
