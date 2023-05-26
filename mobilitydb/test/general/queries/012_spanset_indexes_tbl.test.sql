-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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
-- File span_ops.c
-- Tests of operators for span types.
-------------------------------------------------------------------------------

ANALYZE tbl_intspanset;
ANALYZE tbl_floatspanset;

DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_rtree_idx ON tbl_intspanset USING GIST(i);
CREATE INDEX tbl_floatspanset_rtree_idx ON tbl_floatspanset USING GIST(f);

SELECT COUNT(*) FROM tbl_intspanset WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i << 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> 85;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> 85;

SELECT COUNT(*) FROM tbl_intspanset WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i << intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> intspan '[85, 95]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> intspan '[85, 95]';

SELECT i <-> 101 FROM tbl_intspanset ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspanset ORDER BY 1 LIMIT 3;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> 85.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> 85.0;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> floatspan '[85, 95]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> floatspan '[85, 95]';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspanset_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspanset_quadtree_idx ON tbl_intspanset USING SPGIST(i);
CREATE INDEX tbl_floatspanset_quadtree_idx ON tbl_floatspanset USING SPGIST(f);

SELECT COUNT(*) FROM tbl_intspanset WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- 50;
SELECT COUNT(*) FROM tbl_intspanset WHERE i << 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< 15;
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> 85;
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> 85;

SELECT COUNT(*) FROM tbl_intspanset WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i -|- intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i << intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &< intspan '[15, 25]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i >> intspan '[85, 95]';
SELECT COUNT(*) FROM tbl_intspanset WHERE i &> intspan '[85, 95]';

SELECT i <-> 101 FROM tbl_intspanset ORDER BY 1 LIMIT 3;
SELECT i <-> intspan '[101,105]' FROM tbl_intspanset ORDER BY 1 LIMIT 3;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< 15.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> 85.0;
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> 85.0;

SELECT COUNT(*) FROM tbl_floatspanset WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f << floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &< floatspan '[15, 25]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f >> floatspan '[85, 95]';
SELECT COUNT(*) FROM tbl_floatspanset WHERE f &> floatspan '[85, 95]';

SELECT round((f <-> 101.0)::numeric, 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;
SELECT round((f <-> floatspan '[101,105]')::numeric, 6) FROM tbl_floatspanset ORDER BY 1 LIMIT 3;

DROP INDEX IF EXISTS tbl_intspanset_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspanset_quadtree_idx;

-------------------------------------------------------------------------------
