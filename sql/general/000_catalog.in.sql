/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
  temptypname text NOT NULL,
  basetypid Oid NOT NULL,
  basetypname text NOT NULL,
  basetyplen smallint NOT NULL,
  basebyval boolean NOT NULL,
  basecont boolean NOT NULL,
  boxtypid Oid,
  boxtypname text,
  boxtyplen smallint
);
ALTER TABLE mobdb_temptype SET SCHEMA pg_catalog;

CREATE FUNCTION register_temporal_type(temporal CHAR(24), base CHAR(24),
  contbase boolean, box CHAR(24))
RETURNS void AS $$
BEGIN
  IF box IS NULL OR box = '' THEN
    WITH tempinfo AS (
        SELECT oid, typname FROM pg_type WHERE typname = temporal),
      baseinfo AS (
        SELECT oid, typname, typlen, typbyval FROM pg_type WHERE typname = base),
      boxinfo(oid, typname, typlen) AS (
        SELECT 0::Oid, '', 0::smallint)
    INSERT INTO mobdb_temptype (temptypid, temptypname, basetypid, basetypname,
      basetyplen, basebyval, basecont, boxtypid, boxtypname, boxtyplen)
    SELECT t.oid, t.typname, v.oid, v.typname, v.typlen, v.typbyval, contbase,
      b.oid, b.typname, b.typlen
    FROM tempinfo t, baseinfo v, boxinfo b;
  ELSE
    WITH tempinfo AS (
        SELECT oid, typname FROM pg_type WHERE typname = temporal),
      baseinfo AS (
        SELECT oid, typname, typlen, typbyval FROM pg_type WHERE typname = base),
      boxinfo AS (
        SELECT oid, typname, typlen FROM pg_type WHERE typname = box)
    INSERT INTO mobdb_temptype (temptypid, temptypname, basetypid, basetypname,
      basetyplen, basebyval, basecont, boxtypid, boxtypname, boxtyplen)
    SELECT t.oid, t.typname, v.oid, v.typname, v.typlen, v.typbyval, contbase,
      b.oid, b.typname, b.typlen
    FROM tempinfo t, baseinfo v, boxinfo b;
  END IF;
END;
$$ LANGUAGE plpgsql;

/******************************************************************************/
