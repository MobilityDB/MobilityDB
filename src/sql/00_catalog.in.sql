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
 * catalog.sql
 * Routines for the temporal catalog.
 */
DROP TABLE IF EXISTS mobdb_temptype;
CREATE TABLE mobdb_temptype (
  temptypid Oid PRIMARY KEY,
  basetypid Oid NOT NULL,
  basetyplen smallint NOT NULL,
  basebyval boolean NOT NULL,
  basecont boolean NOT NULL,
  boxtypid Oid,
  boxtyplen smallint
);
ALTER TABLE mobdb_temptype SET SCHEMA pg_catalog;

CREATE FUNCTION register_temporal_type(temporal CHAR(24), base CHAR(24),
  contbase boolean, box CHAR(24))
RETURNS void AS $$
BEGIN
  IF box IS NULL OR box = '' THEN
    WITH tempid AS (SELECT oid FROM pg_type WHERE typname = temporal),
      baseid AS (SELECT oid, typlen, typbyval FROM pg_type WHERE typname = base),
      boxid(oid, typlen) AS (SELECT 0::Oid, 0::smallint)
    INSERT INTO mobdb_temptype (temptypid, basetypid, basetyplen, basebyval,
       basecont, boxtypid, boxtyplen)
    SELECT t.oid, v.oid, v.typlen, v.typbyval, contbase, b.oid, b.typlen
    FROM tempid t, baseid v, boxid b;
  ELSE
    WITH tempid AS (SELECT oid FROM pg_type WHERE typname = temporal),
      baseid AS (SELECT oid, typlen, typbyval FROM pg_type WHERE typname = base),
      boxid AS (SELECT oid, typlen FROM pg_type WHERE typname = box)
    INSERT INTO mobdb_temptype (temptypid, basetypid, basetyplen, basebyval,
       basecont, boxtypid, boxtyplen)
    SELECT t.oid, v.oid, v.typlen, v.typbyval, contbase, b.oid, b.typlen
    FROM tempid t, baseid v, boxid b;
  END IF;
END;
$$ LANGUAGE plpgsql;

/******************************************************************************/
