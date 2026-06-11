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

-- MF-JSON output: every row returns a non-empty JSON string with the
-- expected "type" key.

SELECT bool_and(asMFJSON(temp) LIKE '%"type":"MovingPCPoint"%')
FROM tbl_tpcpoint;

SELECT bool_and(asMFJSON(temp) LIKE '%"type":"MovingPCPatch"%')
FROM tbl_tpcpatch;

-- Bbox embedding: with options=1 the output includes a "bbox" key and
-- the matching pcid (= 1 for our datagen schema).
SELECT bool_and(asMFJSON(temp, 1) LIKE '%"pcid":1%' AND
                asMFJSON(temp, 1) LIKE '%"bbox":%')
FROM tbl_tpcpoint;

-- Hand-roll a JSON parse via regex extraction: pull the first
-- coordinate triple out and verify it's three numbers.
SELECT bool_and(
  substring(asMFJSON(inst) FROM '"coordinates":\[\[[^\]]+\]') ~
    '^"coordinates":\[\[-?[0-9.]+,-?[0-9.]+(,-?[0-9.]+)?\]$')
FROM tbl_tpcpoint_inst;

-- tpcpatch instants emit pcid + npoints + bounds.
SELECT bool_and(asMFJSON(inst) LIKE '%"pcid":1%' AND
                asMFJSON(inst) LIKE '%"npoints":%' AND
                asMFJSON(inst) LIKE '%"bounds":[%')
FROM tbl_tpcpatch_inst;

-------------------------------------------------------------------------------
