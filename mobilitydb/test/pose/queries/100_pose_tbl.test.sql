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

-------------------------------------------------------------------------------
-- Send/receive functions
-------------------------------------------------------------------------------

COPY tbl_pose2d TO '/tmp/tbl_pose2d' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_pose2d_tmp;
CREATE TABLE tbl_pose2d_tmp AS TABLE tbl_pose2d WITH NO DATA;
COPY tbl_pose2d_tmp FROM '/tmp/tbl_pose2d' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d_tmp t2 WHERE t1.k = t2.k AND t1.pose <> t2.pose;
DROP TABLE tbl_pose2d_tmp;

-------------------------------------------------------------------------------
-- Accessing values
-------------------------------------------------------------------------------

SELECT ST_AsEWKT(MAX(point(pose)), 6) FROM tbl_pose2d;
SELECT MAX(rotation(pose)) FROM tbl_pose2d;

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d WHERE pose::geometry IS NOT NULL;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose = t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose != t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose < t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose <= t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose > t2.pose;
SELECT COUNT(*) FROM tbl_pose2d t1, tbl_pose2d t2 WHERE t1.pose >= t2.pose;

-------------------------------------------------------------------------------/
