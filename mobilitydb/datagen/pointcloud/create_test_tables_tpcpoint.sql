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

/**
 * @file
 * @brief Build the standard set of test tables for the pgpointcloud
 *   temporal types: pcpoint, pcpatch, pcpointset, pcpatchset, tpcbox,
 *   tpcpoint, tpcpatch.
 *
 * Mirrors the layout of @c create_test_tables_tcbuffer / @c create_test_tables_tnpoint.
 * All values share a single pcid, materialized on demand by
 * @c ensure_random_pcid() in @c random_tpcpoint.sql.
 */

DROP FUNCTION IF EXISTS create_test_tables_tpcpoint(int);
CREATE OR REPLACE FUNCTION create_test_tables_tpcpoint(size int DEFAULT 100)
RETURNS text AS $$
DECLARE
  perc int := size / 4;
BEGIN
  /* Materialize the synthetic pgpointcloud schema (pcid=1) used by every
     random function below.  ON CONFLICT DO NOTHING — leaves any existing
     pcid=1 intact. */
  PERFORM ensure_random_pcid();

------------------------------------------------------------------------------
-- Static pcpoint / pcpatch
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_pcpoint;
CREATE TABLE tbl_pcpoint AS
SELECT k, random_pcpoint(-100, 100, -100, 100, 0, 100) AS pt
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_pcpatch;
CREATE TABLE tbl_pcpatch AS
SELECT k,
  random_pcpatch(-100, 100, -100, 100, 0, 100, 1, 10) AS pa
FROM generate_series(1, size) k;

------------------------------------------------------------------------------
-- pcpointset / pcpatchset
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_pcpointset;
CREATE TABLE tbl_pcpointset AS
/* perc rows are NULL — exercises NULL-handling paths */
SELECT k, NULL::pcpointset AS s
FROM generate_series(1, perc) k UNION ALL
SELECT k,
  random_pcpointset(-100, 100, -100, 100, 0, 100, 1, 10) AS s
FROM generate_series(perc + 1, size) k;

DROP TABLE IF EXISTS tbl_pcpatchset;
CREATE TABLE tbl_pcpatchset AS
SELECT k, NULL::pcpatchset AS s
FROM generate_series(1, perc) k UNION ALL
SELECT k,
  random_pcpatchset(-100, 100, -100, 100, 0, 100, 1, 10, 1, 10) AS s
FROM generate_series(perc + 1, size) k;

------------------------------------------------------------------------------
-- TPCBox
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tpcbox;
CREATE TABLE tbl_tpcbox AS
SELECT k,
  random_tpcbox(-100, 100, -100, 100, 0, 100,
    '2001-01-01', '2001-12-31') AS b
FROM generate_series(1, size) k;

------------------------------------------------------------------------------
-- Temporal pcpoint
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tpcpoint_inst;
CREATE TABLE tbl_tpcpoint_inst AS
SELECT k, random_tpcpoint_inst(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31') AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpoint_discseq;
CREATE TABLE tbl_tpcpoint_discseq AS
SELECT k, random_tpcpoint_discseq(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpoint_seq;
CREATE TABLE tbl_tpcpoint_seq AS
SELECT k, random_tpcpoint_contseq(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpoint_seqset;
CREATE TABLE tbl_tpcpoint_seqset AS
SELECT k, random_tpcpoint_seqset(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpoint;
CREATE TABLE tbl_tpcpoint(k, temp) AS
(SELECT k, inst FROM tbl_tpcpoint_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4,     seq FROM tbl_tpcpoint_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2,     seq FROM tbl_tpcpoint_seq     LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss  FROM tbl_tpcpoint_seqset  LIMIT size / 4);

------------------------------------------------------------------------------
-- Temporal pcpatch
------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tpcpatch_inst;
CREATE TABLE tbl_tpcpatch_inst AS
SELECT k, random_tpcpatch_inst(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 1, 10) AS inst
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpatch_discseq;
CREATE TABLE tbl_tpcpatch_discseq AS
SELECT k, random_tpcpatch_discseq(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpatch_seq;
CREATE TABLE tbl_tpcpatch_seq AS
SELECT k, random_tpcpatch_contseq(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10) AS seq
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpatch_seqset;
CREATE TABLE tbl_tpcpatch_seqset AS
SELECT k, random_tpcpatch_seqset(-100, 100, -100, 100, 0, 100,
  '2001-01-01', '2001-12-31', 10, 1, 10, 1, 10, 1, 10) AS ss
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_tpcpatch;
CREATE TABLE tbl_tpcpatch(k, temp) AS
(SELECT k, inst FROM tbl_tpcpatch_inst LIMIT size / 4) UNION ALL
(SELECT k + size / 4,     seq FROM tbl_tpcpatch_discseq LIMIT size / 4) UNION ALL
(SELECT k + size / 2,     seq FROM tbl_tpcpatch_seq     LIMIT size / 4) UNION ALL
(SELECT k + size / 4 * 3, ss  FROM tbl_tpcpatch_seqset  LIMIT size / 4);

------------------------------------------------------------------------------
RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_tpcpoint(100);
