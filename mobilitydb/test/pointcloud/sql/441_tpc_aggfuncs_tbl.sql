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

-- Table-level tests for the extent() aggregate over the pgPointCloud
-- temporal types.  Each test reduces a whole table to a single scalar
-- so the expected output stays human-readable.

-------------------------------------------------------------------------------
-- extent(tpcbox) — folding the per-row tpcbox bboxes is idempotent
-- against re-running the same aggregation on the result row.
-------------------------------------------------------------------------------

SELECT pcid(extent(b)) FROM tbl_tpcbox;

-- Aggregating the aggregate result yields the same value back.
SELECT extent(b) ~= (SELECT extent(b) FROM tbl_tpcbox) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- extent(tpcpoint) / extent(tpcpatch) — pcid must propagate from the
-- rows to the aggregate result, and the box must contain every row's
-- start time.
-------------------------------------------------------------------------------

SELECT pcid(extent(temp)) FROM tbl_tpcpoint;
SELECT pcid(extent(temp)) FROM tbl_tpcpatch;

SELECT bool_and(startTimestamp(temp) >= tmin(ext) AND endTimestamp(temp) <= tmax(ext))
FROM tbl_tpcpoint, (SELECT extent(temp) AS ext FROM tbl_tpcpoint) e;
SELECT bool_and(startTimestamp(temp) >= tmin(ext) AND endTimestamp(temp) <= tmax(ext))
FROM tbl_tpcpatch, (SELECT extent(temp) AS ext FROM tbl_tpcpatch) e;

-------------------------------------------------------------------------------
-- tcount / wcount — count of overlapping rows at each timestamp.
-- Aggregates require uniform subtype per group, so we sample on the
-- per-subtype tables rather than the merged tbl_tpcpoint / tbl_tpcpatch.
-------------------------------------------------------------------------------

SELECT numInstants(tcount(inst)) > 0 FROM tbl_tpcpoint_inst;
SELECT numInstants(tcount(seq))  > 0 FROM tbl_tpcpoint_seq;
SELECT numInstants(tcount(inst)) > 0 FROM tbl_tpcpatch_inst;
SELECT numInstants(tcount(seq))  > 0 FROM tbl_tpcpatch_seq;

SELECT numInstants(wcount(inst, '1 day'::interval)) > 0 FROM tbl_tpcpoint_inst;
SELECT numInstants(wcount(seq,  '1 day'::interval)) > 0 FROM tbl_tpcpoint_seq;
SELECT numInstants(wcount(inst, '1 day'::interval)) > 0 FROM tbl_tpcpatch_inst;
SELECT numInstants(wcount(seq,  '1 day'::interval)) > 0 FROM tbl_tpcpatch_seq;

-------------------------------------------------------------------------------
-- appendInstant — the forms every type without a distance takes: the
-- interpolation, and the maximum time interval between consecutive instants
-------------------------------------------------------------------------------

SELECT numInstants(appendInstant(inst ORDER BY getTimestamp(inst))) =
    count(DISTINCT getTimestamp(inst)),
  numInstants(appendInstant(inst, 'step' ORDER BY getTimestamp(inst))) =
    count(DISTINCT getTimestamp(inst)),
  numSequences(appendInstant(inst, 'step', interval '1 microsecond'
    ORDER BY getTimestamp(inst))) = count(DISTINCT getTimestamp(inst))
FROM (SELECT DISTINCT ON (getTimestamp(inst)) inst FROM tbl_tpcpatch_inst) t;

-------------------------------------------------------------------------------
-- merge — combines instants across rows into one temporal value.
-- pcid must propagate to the result.
-------------------------------------------------------------------------------

SELECT pcid(merge(inst)) FROM tbl_tpcpoint_inst;
SELECT pcid(merge(inst)) FROM tbl_tpcpatch_inst;

-------------------------------------------------------------------------------
-- tnpoints / tdensity — per-instant point counts and density.
-- Both walk every instant of every tpcpatch row: tnpoints sums the
-- pcpatch's npoints, tdensity returns npoints / xy-bbox-area. Verify
-- the result is a non-empty tint / tfloat. A patch whose XY bounds have no
-- area, one point or points on a line, has no density: the division by
-- zero raises an error, so the density reads the rows whose every patch has
-- an area.
-------------------------------------------------------------------------------

SELECT numInstants(tnpoints(inst)) > 0 FROM tbl_tpcpatch_inst;
SELECT numInstants(tnpoints(seq))  > 0 FROM tbl_tpcpatch_seq;

SELECT numInstants(tdensity(inst)) > 0 FROM tbl_tpcpatch_inst
WHERE (SELECT bool_and(area(getValue(i)::tpcbox::stbox) > 0)
  FROM unnest(instants(inst)) i);
SELECT numInstants(tdensity(seq))  > 0 FROM tbl_tpcpatch_seq
WHERE (SELECT bool_and(area(getValue(i)::tpcbox::stbox) > 0)
  FROM unnest(instants(seq)) i);

-- Two points over a 1x1 XY box, then one point: the second patch has no area
SELECT tdensity(v) FROM (VALUES (tpcpatch(pcpatch(pcpoint(1, 1, 1, 1),
  pcpoint(1, 2, 2, 2)), '2001-01-01'::timestamptz))) t(v);
SELECT tdensity(v) FROM (VALUES (tpcpatch(pcpatch(pcpoint(1, 1, 1, 1),
  pcpoint(1, 2, 2, 2)), '2001-01-01'::timestamptz)),
  (tpcpatch(pcpatch(pcpoint(1, 3, 3, 3)), '2001-01-02'::timestamptz))) t(v);

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- The combine function answers what the aggregate answers
--
-- A COMBINEFUNC is reached only by a parallel plan, and which rows a worker
-- receives is not controllable, so a parallel query cannot state a
-- deterministic expected output. The function is therefore called directly,
-- which is what makes these the first cases to reach it at all.
-------------------------------------------------------------------------------

-- extent(tpcbox) names its transition function as its own combine, so the one
-- function answers in both roles
SELECT extentTransition(tpcboxX(0, 0, 5, 5, 1), tpcboxX(3, 3, 10, 10, 1));
-- The schema states what a coordinate means, so two of them are a refusal
SELECT extentTransition(tpcboxX(0, 0, 5, 5, 1), tpcboxX(3, 3, 10, 10, 0));
-- A null state on either side leaves the other untouched
SELECT extentTransition(NULL::tpcbox, tpcboxX(3, 3, 10, 10, 1));
SELECT extentTransition(tpcboxX(0, 0, 5, 5, 1), NULL::tpcbox);
SELECT extentTransition(NULL::tpcbox, NULL::tpcbox);

-- The combine function of the three extent aggregates
SELECT extentCombine(tpcboxX(0, 0, 5, 5, 1), tpcboxX(3, 3, 10, 10, 1));
SELECT extentCombine(tpcboxX(0, 0, 5, 5, 1), tpcboxX(3, 3, 10, 10, 0));
SELECT extentCombine(NULL::tpcbox, tpcboxX(3, 3, 10, 10, 1));
SELECT extentCombine(tpcboxX(0, 0, 5, 5, 1), NULL::tpcbox);
SELECT extentCombine(NULL::tpcbox, NULL::tpcbox);

-------------------------------------------------------------------------------
-- A direct call expands a copy of the box it is given
DROP TABLE IF EXISTS tbl_extent_state;
CREATE TABLE tbl_extent_state AS
SELECT extent(temp) AS b, extent(temp) AS b0
FROM (SELECT temp FROM tbl_tpcpoint WHERE temp IS NOT NULL ORDER BY k LIMIT 1) t;
SELECT extentTransition(b, (SELECT temp FROM tbl_tpcpoint
  WHERE temp IS NOT NULL ORDER BY k DESC LIMIT 1)) ~= b0
FROM tbl_extent_state;
SELECT extentTransition(b, (SELECT extent(temp) FROM tbl_tpcpoint)) ~= b0,
  extentCombine(b, (SELECT extent(temp) FROM tbl_tpcpoint)) ~= b0
FROM tbl_extent_state;
SELECT b ~= b0 FROM tbl_extent_state;
DROP TABLE tbl_extent_state;

-------------------------------------------------------------------------------
