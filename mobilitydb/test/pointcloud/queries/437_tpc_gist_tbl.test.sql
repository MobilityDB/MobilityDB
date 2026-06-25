-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
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

-- GiST index test: build the index, run a few box-overlap queries
-- and verify the answers match a sequential-scan plan.

-------------------------------------------------------------------------------
-- tpcpoint
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_idx ON tbl_tpcpoint USING gist(temp);

-- Index plan should match sequential scan for the same query.
SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpoint
WHERE temp && tpcbox_zt(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1, 0);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpoint
WHERE temp && tpcbox_zt(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1, 0);

DROP INDEX tbl_tpcpoint_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- tpcpatch
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_count FROM tbl_tpcpatch
WHERE temp && tpcbox_zt(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1, 0);

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_count FROM tbl_tpcpatch
WHERE temp && tpcbox_zt(0, 0, 0, 50, 50, 50,
  tstzspan '[2001-06-01, 2001-12-31]', 1, 0);

DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
-- Operator-family coverage. For each operator the seqscan and indexed
-- counts must match. This locks index correctness across the kinds of
-- queries a real drone-LiDAR workload pivots on.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpatch_gist_idx ON tbl_tpcpatch USING gist(temp);

-- &&  : bbox overlap (already covered above; repeated for symmetry).
-- <@  : value's bbox contained in the query bbox.
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_contained FROM tbl_tpcpatch
WHERE temp <@ tpcbox_zt(-1000, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1, 0);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_contained FROM tbl_tpcpatch
WHERE temp <@ tpcbox_zt(-1000, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1, 0);

-- <<# : strictly before (time)
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_before FROM tbl_tpcpatch
WHERE temp <<# tpcbox_zt(0, 0, 0, 100, 100, 100,
  tstzspan '[2001-12-15, 2002-01-01]', 1, 0);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_before FROM tbl_tpcpatch
WHERE temp <<# tpcbox_zt(0, 0, 0, 100, 100, 100,
  tstzspan '[2001-12-15, 2002-01-01]', 1, 0);

-- << : strictly left (X)
SET enable_seqscan = on;  SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT COUNT(*) AS seq_left FROM tbl_tpcpatch
WHERE temp << tpcbox_zt(50, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1, 0);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT COUNT(*) AS idx_left FROM tbl_tpcpatch
WHERE temp << tpcbox_zt(50, -1000, -1000, 1000, 1000, 1000,
  tstzspan '[2001-01-01, 2002-01-01]', 1, 0);

DROP INDEX tbl_tpcpatch_gist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
