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

DROP TABLE IF EXISTS mobilitydb_typcache;
CREATE TABLE mobilitydb_typcache (
  temptypid Oid PRIMARY KEY,
  basetypid Oid
);

ALTER TABLE mobilitydb_typcache SET SCHEMA pg_catalog;

CREATE FUNCTION register_temporal(temporal CHAR(24), base CHAR(24))
RETURNS void AS $$
BEGIN
  WITH valueid AS (SELECT oid, typname FROM pg_type WHERE typname=base),
  tempid AS (SELECT oid, typname FROM pg_type WHERE typname=temporal)
  INSERT INTO mobilitydb_typcache (temptypid, basetypid)
  SELECT te.oid, v.oid FROM valueid v, tempid te;
END;
$$ LANGUAGE plpgsql;

/******************************************************************************/
