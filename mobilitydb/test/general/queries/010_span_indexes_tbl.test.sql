-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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

ANALYZE tbl_intspan;
ANALYZE tbl_floatspan;

DROP INDEX IF EXISTS tbl_intspan_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_rtree_idx;

DROP INDEX IF EXISTS tbl_intspan_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspan_rtree_idx ON tbl_intspan USING GIST(i);
CREATE INDEX tbl_floatspan_rtree_idx ON tbl_floatspan USING GIST(f);

SELECT COUNT(*) FROM tbl_intspan WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i -|- 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i << 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i &< 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i >> 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i &> 50;

SELECT COUNT(*) FROM tbl_intspan WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i -|- intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i << intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i &< intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i >> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i &> intspan '[45, 55]';

SELECT COUNT(*) FROM tbl_floatspan WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f << 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f &< 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f >> 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f &> 50.0;

SELECT COUNT(*) FROM tbl_floatspan WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f << floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f &< floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f >> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f &> floatspan '[45, 55]';

DROP INDEX IF EXISTS tbl_intspan_rtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_rtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_intspan_quadtree_idx ON tbl_intspan USING SPGIST(i);
CREATE INDEX tbl_floatspan_quadtree_idx ON tbl_floatspan USING SPGIST(f);

SELECT COUNT(*) FROM tbl_intspan WHERE i @> 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i -|- 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i << 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i &< 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i >> 50;
SELECT COUNT(*) FROM tbl_intspan WHERE i &> 50;

SELECT COUNT(*) FROM tbl_intspan WHERE i && intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i @> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i <@ intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i -|- intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i << intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i &< intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i >> intspan '[45, 55]';
SELECT COUNT(*) FROM tbl_intspan WHERE i &> intspan '[45, 55]';

SELECT COUNT(*) FROM tbl_floatspan WHERE f @> 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f -|- 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f << 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f &< 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f >> 50.0;
SELECT COUNT(*) FROM tbl_floatspan WHERE f &> 50.0;

SELECT COUNT(*) FROM tbl_floatspan WHERE f && floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f @> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f <@ floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f << floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f &< floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f >> floatspan '[45, 55]';
SELECT COUNT(*) FROM tbl_floatspan WHERE f &> floatspan '[45, 55]';

DROP INDEX IF EXISTS tbl_intspan_quadtree_idx;
DROP INDEX IF EXISTS tbl_floatspan_quadtree_idx;

-------------------------------------------------------------------------------
