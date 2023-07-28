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
-- File timeops.c
-- Tests of operators that do involve indexes for time types.
-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tstzspanset_big_quadtree_idx;
DROP INDEX IF EXISTS tbl_tstzspan_big_quadtree_idx;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tstzspan_big_quadtree_idx ON tbl_tstzspan_big USING SPGIST(p);
CREATE INDEX tbl_tstzspanset_big_quadtree_idx ON tbl_tstzspanset_big USING SPGIST(ps);

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p -|- timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p <<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p &<# timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #>> timestamptz '2001-01-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #&> timestamptz '2001-01-01';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p <@ tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p -|- tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #>> tstzspan '[2001-11-01, 2001-12-01]';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #&> tstzspan '[2001-11-01, 2001-12-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p <@ tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p -|- tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p <<# tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p &<# tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #>> tstzspanset '{[2001-01-01, 2001-02-01]}';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p #&> tstzspanset '{[2001-01-01, 2001-02-01]}';

SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps && tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps <@ tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps -|- tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps <<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps &<# tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps #>> tstzspan '[2001-01-01, 2001-02-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps #&> tstzspan '[2001-01-01, 2001-02-01]';

DROP INDEX tbl_tstzspan_big_quadtree_idx;
DROP INDEX tbl_tstzspanset_big_quadtree_idx;

-------------------------------------------------------------------------------
