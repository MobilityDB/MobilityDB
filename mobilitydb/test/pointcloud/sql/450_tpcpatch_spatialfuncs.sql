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

-- Regression tests for the swath of a tpcpatch (430_tpcpatch.in.sql).
--
-- The footprint of an instant is the convex hull of the positions of the
-- points of its patch, two consecutive footprints are joined by the convex
-- hull of both, and the swath is the union of these strips. A row of points,
-- as a ping of a multibeam echosounder sounds, has a segment as footprint, and
-- two rows give the quadrilateral between them.
--
-------------------------------------------------------------------------------

\set row1 'pcpatch(pcpoint(1, 0, 0, 0), pcpoint(1, 1, 0, 0), pcpoint(1, 2, 0, 0))'
\set row2 'pcpatch(pcpoint(1, 0, 1, 0), pcpoint(1, 1, 1, 0), pcpoint(1, 2, 1, 0))'
\set row3 'pcpatch(pcpoint(1, 0, 2, 0), pcpoint(1, 1, 2, 0), pcpoint(1, 2, 2, 0))'
\set row4 'pcpatch(pcpoint(1, 0, 5, 0), pcpoint(1, 1, 5, 0), pcpoint(1, 2, 5, 0))'
\set tri 'pcpatch(pcpoint(1, 0, 0, 0), pcpoint(1, 4, 0, 0), pcpoint(1, 0, 3, 0))'

-- Test for NULL inputs since the function is not strict
SELECT swath(NULL::tpcpatch);
SELECT swath(NULL::tpcpatch, interval '1 day');

-- The footprint of a single instant
SELECT ST_AsText(swath(tpcpatch(:tri, '2001-01-01'::timestamptz)));
SELECT ST_AsText(swath(tpcpatch(:row1, '2001-01-01'::timestamptz)));
SELECT ST_AsText(swath(tpcpatch(pcpatch(pcpoint(1, 1, 1, 1)),
  '2001-01-01'::timestamptz)));

-- Two consecutive rows give the quadrilateral between them, and three rows
-- the union of the two strips
SELECT ST_AsText(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz)])));
SELECT ST_AsText(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz),
  tpcpatch(:row3, '2001-01-03'::timestamptz)])));

-- A discrete sequence and a sequence set join their consecutive instants all
-- the same
SELECT ST_Area(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz),
  tpcpatch(:row3, '2001-01-03'::timestamptz)], 'discrete')));
SELECT ST_Area(swath(tpcpatchSeqSet(ARRAY[
  tpcpatchSeq(ARRAY[tpcpatch(:row1, '2001-01-01'::timestamptz),
    tpcpatch(:row2, '2001-01-02'::timestamptz)]),
  tpcpatchSeq(ARRAY[tpcpatch(:row3, '2001-01-03'::timestamptz),
    tpcpatch(:row4, '2001-01-04'::timestamptz)])])));

-- The largest gap joined splits the swath at a longer pause, and an instant
-- joined to neither neighbour contributes its own footprint
SELECT ST_AsText(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz),
  tpcpatch(:row4, '2001-01-05'::timestamptz)]), interval '1 day'));
SELECT ST_AsText(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row3, '2001-01-03'::timestamptz),
  tpcpatch(:row4, '2001-01-05'::timestamptz)]), interval '1 day'));
-- The pieces of a split swath
SELECT ST_NumGeometries(swath(tpcpatchSeq(ARRAY[
  tpcpatch(pcpatch(pcpoint(1, 0, 0, 0), pcpoint(1, 2, 0, 0)), '2001-01-01'::timestamptz),
  tpcpatch(pcpatch(pcpoint(1, 0, 1, 0), pcpoint(1, 2, 1, 0)), '2001-01-02'::timestamptz),
  tpcpatch(pcpatch(pcpoint(1, 0, 5, 0), pcpoint(1, 2, 5, 0)), '2001-01-05'::timestamptz)]),
  interval '1 day'));
-- A gap equal to the largest one is joined
SELECT ST_Area(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz)]), interval '1 day'));
-- A NULL interval joins every gap
SELECT ST_Area(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz),
  tpcpatch(:row4, '2001-01-05'::timestamptz)]), NULL));

-- The soundings themselves have no area, the swath has the area between them
SELECT ST_Area(traversedArea(tgeometry(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz)], 'discrete')))),
  ST_Area(swath(tpcpatchSeq(ARRAY[
  tpcpatch(:row1, '2001-01-01'::timestamptz),
  tpcpatch(:row2, '2001-01-02'::timestamptz)], 'discrete')));

-------------------------------------------------------------------------------
