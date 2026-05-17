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

-- Smoke tests for the |=| nearest-approach-distance operator and KNN
-- ordering via GiST.

-------------------------------------------------------------------------------
-- Self-distance is zero (every value is at distance 0 from its own bbox).
-------------------------------------------------------------------------------

SELECT bool_and((temp |=| temp) = 0) FROM tbl_tpcpoint;
SELECT bool_and((temp |=| temp) = 0) FROM tbl_tpcpatch;
SELECT bool_and((b |=| b) = 0) FROM tbl_tpcbox;

-------------------------------------------------------------------------------
-- KNN ordering — index scan should return the top-N rows in the same
-- order as a sequential scan.
-------------------------------------------------------------------------------

CREATE INDEX tbl_tpcpoint_gist_dist_idx ON tbl_tpcpoint USING gist(temp);

SET enable_seqscan = on;
SET enable_indexscan = off;
SET enable_bitmapscan = off;
SELECT k, ((temp |=| tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1, 0)) > 0)::text AS finite
FROM tbl_tpcpoint
ORDER BY temp |=| tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1, 0)
LIMIT 5;

SET enable_seqscan = off;
SET enable_indexscan = on;
SET enable_bitmapscan = on;
SELECT k, ((temp |=| tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1, 0)) > 0)::text AS finite
FROM tbl_tpcpoint
ORDER BY temp |=| tpcbox_zt(0, 0, 0, 0, 0, 0,
  tstzspan '[2001-06-01, 2001-06-30]', 1, 0)
LIMIT 5;

DROP INDEX tbl_tpcpoint_gist_dist_idx;
RESET enable_seqscan;
RESET enable_indexscan;
RESET enable_bitmapscan;

-------------------------------------------------------------------------------
