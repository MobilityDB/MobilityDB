-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The R-tree GiST, quad-tree and k-d tree SP-GiST indexes of a ts2cell, keyed
-- by the stbox of the value. Under each index every operator of the class, with
-- each right argument it declares, answers the rows a sequential scan gives,
-- from a plan that probes the index.
-------------------------------------------------------------------------------

CREATE FUNCTION test_probes_index(query text)
RETURNS boolean LANGUAGE plpgsql AS $$
DECLARE
  line text;
BEGIN
  FOR line IN EXECUTE 'EXPLAIN (COSTS OFF) ' || query LOOP
    IF line LIKE '%Index Cond%' OR line LIKE '%Order By%' THEN
      RETURN true;
    END IF;
  END LOOP;
  RETURN false;
END;
$$;

DROP TABLE IF EXISTS tbl_ts2cell_src;
CREATE TABLE tbl_ts2cell_src AS
SELECT k, 4.0 + (k % 20) * 0.05 AS x, 50.0 + (k / 20) * 0.05 AS y,
  timestamptz '2001-01-01' + k * interval '1 hour' AS t
FROM generate_series(1, 240) k;

-- Instants, moving points, and runs of one cell whose periods meet
DROP TABLE IF EXISTS tbl_ts2cell_idx;
CREATE TABLE tbl_ts2cell_idx AS
SELECT k, ts2cell(geoToS2Cell(geography(ST_SetSRID(ST_MakePoint(x, y), 4326)), 12), t) AS temp
FROM tbl_ts2cell_src WHERE k <= 100
UNION ALL
SELECT k, ts2cell(tgeogpoint(format('[Point(%s %s)@%s, Point(%s %s)@%s)',
    x, y, t, x + 0.04, y + 0.04, t + interval '3 hours')), 12)
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200
UNION ALL
SELECT k, ts2cell(geoToS2Cell(geography(ST_SetSRID(ST_MakePoint(4.5, 51.5), 4326)), 12),
    tstzspan(format('[%s, %s)', t, t + interval '1 hour')))
FROM tbl_ts2cell_src WHERE k > 200;
ANALYZE tbl_ts2cell_idx;

-- The box of a value, a box around it, and a box inside the moving points
DROP TABLE IF EXISTS tbl_ts2cell_box;
CREATE TABLE tbl_ts2cell_box AS
SELECT temp::stbox AS b FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT expandTime(expandSpace(temp::stbox, 0.1), interval '1 day')
FROM tbl_ts2cell_idx WHERE k % 4 = 1
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX XT(((%s,%s),(%s,%s)),[%s,%s])',
    x + 0.02, y + 0.02, x + 0.021, y + 0.021, t + interval '1 hour',
    t + interval '2 hours'))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2;
ANALYZE tbl_ts2cell_box;

-- A box of space alone, which the operators on time refuse: one around a
-- value, one inside a moving point, the space of a value, and one that meets it
DROP TABLE IF EXISTS tbl_ts2cell_xbox;
CREATE TABLE tbl_ts2cell_xbox AS
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))', x, y, x + 0.1, y + 0.1)) AS b
FROM tbl_ts2cell_src WHERE k % 4 = 3
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    x + 0.02, y + 0.02, x + 0.021, y + 0.021))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmin(temp::stbox), ymin(temp::stbox), xmax(temp::stbox), ymax(temp::stbox)))
FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT stbox(format('SRID=4326;GEODSTBOX X((%s,%s),(%s,%s))',
    xmax(temp::stbox), ymin(temp::stbox), xmax(temp::stbox) + 0.1, ymax(temp::stbox)))
FROM tbl_ts2cell_idx WHERE k % 4 = 1;
ANALYZE tbl_ts2cell_xbox;

-- The period of a value, one around it, one inside it, and one that meets it
DROP TABLE IF EXISTS tbl_ts2cell_span;
CREATE TABLE tbl_ts2cell_span AS
SELECT temp::tstzspan AS t FROM tbl_ts2cell_idx WHERE k % 4 = 0
UNION ALL
SELECT tstzspan(format('[%s, %s]', t - interval '1 day', t + interval '1 day'))
FROM tbl_ts2cell_src WHERE k % 4 = 1
UNION ALL
SELECT tstzspan(format('[%s, %s]', t + interval '1 hour', t + interval '2 hours'))
FROM tbl_ts2cell_src WHERE k > 100 AND k <= 200 AND k % 4 = 2
UNION ALL
SELECT tstzspan(format('[%s, %s)', t - interval '1 hour', t))
FROM tbl_ts2cell_src WHERE k % 4 = 3;
ANALYZE tbl_ts2cell_span;

DROP TABLE IF EXISTS test_ts2cell_idx;
CREATE TABLE test_ts2cell_idx(op text, no_idx text, rtree_idx text, quadtree_idx text,
  kdtree_idx text, rtree_probe boolean, quadtree_probe boolean,
  kdtree_probe boolean);

INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> ts2cell', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> stbox', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<# tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<# tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#>> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('#&> tstzspan', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<< stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&< stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&& stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('>> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('~= stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('@> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<@ stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('&<| stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('<<| stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|>> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('|&> stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text);
INSERT INTO test_ts2cell_idx(op, no_idx) VALUES ('-|- stbox x', (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text);

-- With no index a plan cannot probe one, whatever the planner settings
SET enable_seqscan = off;
SELECT test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b') AS no_index_probe;
RESET enable_seqscan;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_rtree ON tbl_ts2cell_idx USING gist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET rtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  rtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_rtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_quadtree ON tbl_ts2cell_idx USING spgist(temp);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET quadtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  quadtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_quadtree;

-------------------------------------------------------------------------------

CREATE INDEX tbl_ts2cell_idx_kdtree ON tbl_ts2cell_idx USING spgist(temp ts2cell_kdtree_ops);
SET enable_seqscan = off;
SET enable_hashjoin = off;
SET enable_mergejoin = off;
SET enable_sort = off;
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp << t2.temp')
WHERE op = '<< ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &< t2.temp')
WHERE op = '&< ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp && t2.temp')
WHERE op = '&& ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &> t2.temp')
WHERE op = '&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp >> t2.temp')
WHERE op = '>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp ~= t2.temp')
WHERE op = '~= ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp @> t2.temp')
WHERE op = '@> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <@ t2.temp')
WHERE op = '<@ ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<| t2.temp')
WHERE op = '&<| ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<| t2.temp')
WHERE op = '<<| ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |>> t2.temp')
WHERE op = '|>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp |&> t2.temp')
WHERE op = '|&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp -|- t2.temp')
WHERE op = '-|- ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp &<# t2.temp')
WHERE op = '&<# ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp <<# t2.temp')
WHERE op = '<<# ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #>> t2.temp')
WHERE op = '#>> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_idx t2 WHERE t1.temp #&> t2.temp')
WHERE op = '#&> ts2cell';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp &<# t2.b')
WHERE op = '&<# stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp <<# t2.b')
WHERE op = '<<# stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #>> t2.b')
WHERE op = '#>> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_box t2 WHERE t1.temp #&> t2.b')
WHERE op = '#&> stbox';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp && t2.t')
WHERE op = '&& tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp ~= t2.t')
WHERE op = '~= tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp @> t2.t')
WHERE op = '@> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <@ t2.t')
WHERE op = '<@ tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp -|- t2.t')
WHERE op = '-|- tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp &<# t2.t')
WHERE op = '&<# tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp <<# t2.t')
WHERE op = '<<# tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #>> t2.t')
WHERE op = '#>> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_span t2 WHERE t1.temp #&> t2.t')
WHERE op = '#&> tstzspan';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp << t2.b')
WHERE op = '<< stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &< t2.b')
WHERE op = '&< stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp && t2.b')
WHERE op = '&& stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &> t2.b')
WHERE op = '&> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp >> t2.b')
WHERE op = '>> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp ~= t2.b')
WHERE op = '~= stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp @> t2.b')
WHERE op = '@> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <@ t2.b')
WHERE op = '<@ stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp &<| t2.b')
WHERE op = '&<| stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp <<| t2.b')
WHERE op = '<<| stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |>> t2.b')
WHERE op = '|>> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp |&> t2.b')
WHERE op = '|&> stbox x';
UPDATE test_ts2cell_idx SET kdtree_idx = (SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b)::text,
  kdtree_probe = test_probes_index('SELECT COUNT(*) FROM tbl_ts2cell_idx t1, tbl_ts2cell_xbox t2 WHERE t1.temp -|- t2.b')
WHERE op = '-|- stbox x';
RESET enable_seqscan;
RESET enable_hashjoin;
RESET enable_mergejoin;
RESET enable_sort;
DROP INDEX tbl_ts2cell_idx_kdtree;

-------------------------------------------------------------------------------

SELECT op, split_part(no_idx, ':', 1) AS no_idx FROM test_ts2cell_idx ORDER BY op;
SELECT * FROM test_ts2cell_idx
WHERE no_idx IS DISTINCT FROM rtree_idx OR no_idx IS DISTINCT FROM quadtree_idx OR
  no_idx IS DISTINCT FROM kdtree_idx OR NOT rtree_probe OR NOT quadtree_probe OR
  NOT kdtree_probe
ORDER BY op;

DROP TABLE test_ts2cell_idx;
DROP TABLE tbl_ts2cell_span;
DROP TABLE tbl_ts2cell_xbox;
DROP TABLE tbl_ts2cell_box;
DROP TABLE tbl_ts2cell_idx;
DROP TABLE tbl_ts2cell_src;
DROP FUNCTION test_probes_index(text);

-------------------------------------------------------------------------------
