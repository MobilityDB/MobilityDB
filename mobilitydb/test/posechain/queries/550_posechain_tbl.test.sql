-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Send and receive: a binary round trip changes no value
-------------------------------------------------------------------------------

COPY tbl_posechain TO '/tmp/tbl_posechain' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_posechain_tmp;
CREATE TABLE tbl_posechain_tmp AS TABLE tbl_posechain WITH NO DATA;
COPY tbl_posechain_tmp FROM '/tmp/tbl_posechain' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain_tmp t2 WHERE t1.k = t2.k AND t1.pc <> t2.pc;
DROP TABLE tbl_posechain_tmp;

-------------------------------------------------------------------------------
-- Text and binary round trips
-------------------------------------------------------------------------------

-- The E forms carry the frame and the plain forms do not, so a value read back
-- from a plain form is the same chain in SRID 0 and equals the original once
-- the frame is named again.
-- 17 decimal digits is what a double takes to be written and read back exactly;
-- at the default of 15 the text forms round 41 of these chains.
SELECT COUNT(*) FROM tbl_posechain WHERE posechainFromEWKT(asEWKT(pc, 17)) <> pc;
SELECT COUNT(*) FROM tbl_posechain WHERE setSRID(posechainFromText(asText(pc, 17)), 3812) <> pc;
SELECT COUNT(*) FROM tbl_posechain WHERE SRID(posechainFromText(asText(pc, 17))) <> 0;
SELECT COUNT(*) FROM tbl_posechain WHERE posechainFromHexEWKB(asHexEWKB(pc)) <> pc;
SELECT COUNT(*) FROM tbl_posechain WHERE setSRID(posechainFromHexEWKB(asHexWKB(pc)), 3812) <> pc;
SELECT COUNT(*) FROM tbl_posechain WHERE posechainFromBinary(asEWKB(pc)) <> pc;

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------

SELECT MIN(numPoses(pc)), MAX(numPoses(pc)) FROM tbl_posechain;
SELECT COUNT(*) FROM tbl_posechain WHERE poseN(pc, 1) IS NOT NULL;
-- A one-link chain composes to the link it holds
SELECT COUNT(*) FROM tbl_posechain WHERE pose(pc) IS NOT NULL;
SELECT COUNT(*) FROM tbl_posechain WHERE numPoses(pc) = 1 AND pose(pc) <> poseN(pc, 1);

-------------------------------------------------------------------------------
-- Conversions
-------------------------------------------------------------------------------

-- A chain reaches a geometry through the pose it composes to, there being no
-- cast of its own: one topocentric frame sits at the outside of a chain, so
-- what a chain names in that frame is where its last link lands
SELECT COUNT(*) FROM tbl_posechain WHERE pose(pc)::geometry IS NOT NULL;
SELECT COUNT(*) FROM tbl_posechain WHERE pc::posechainset IS NOT NULL;
SELECT COUNT(*) FROM tbl_posechain WHERE stbox(pc) IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons: the order is total
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain t2 WHERE t1.pc = t2.pc;
SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain t2 WHERE t1.pc <> t2.pc;
SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain t2 WHERE t1.pc < t2.pc;
SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain t2 WHERE t1.pc <= t2.pc;
SELECT COUNT(*) FROM tbl_posechain t1, tbl_posechain t2
WHERE (t1.pc < t2.pc)::int + (t1.pc = t2.pc)::int + (t1.pc > t2.pc)::int <> 1;

-------------------------------------------------------------------------------
