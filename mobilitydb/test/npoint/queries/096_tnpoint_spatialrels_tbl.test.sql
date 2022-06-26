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
set parallel_tuple_cost=0;
set parallel_setup_cost=0;
set force_parallel_mode=regress;
-------------------------------------------------------------------------------
-- Geometry rel tnpoint
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE contains(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE disjoint(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE intersects(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE touches(ST_SetSRID(t1.g, 5676), t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_geom_point t1, tbl_tnpoint t2 WHERE dwithin(ST_SetSRID(t1.g, 5676), t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- npoint rel tnpoint
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE disjoint(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE intersects(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE touches(t1.np, t2.temp) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_npoint t1, tbl_tnpoint t2 WHERE dwithin(t1.np, t2.temp, 0.01) AND t1.k < 10 AND t2.k%4 = 0 AND tempSubtype(temp) != 'SequenceSet';

-------------------------------------------------------------------------------
-- tnpoint rel <type>
 -------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE disjoint(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE intersects(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE touches(t1.temp, ST_SetSRID(t2.g, 5676)) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_geom_point t2 WHERE dwithin(t1.temp, ST_SetSRID(t2.g, 5676), 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE disjoint(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE intersects(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE touches(t1.temp, t2.np) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_npoint t2 WHERE dwithin(t1.temp, t2.np, 0.01) AND t1.k%4 = 0 AND t2.k < 10 AND tempSubtype(temp) != 'SequenceSet';

SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE disjoint(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE intersects(t1.temp, t2.temp) AND t1.k%4 = 0 AND t2.k%4 = 0;
SELECT COUNT(*) FROM tbl_tnpoint t1, tbl_tnpoint t2 WHERE dwithin(t1.temp, t2.temp, 0.01) AND t1.k%4 = 0 AND t2.k%4 = 0 AND tempSubtype(t1.temp) != 'SequenceSet';

-------------------------------------------------------------------------------
set parallel_tuple_cost=100;
set parallel_setup_cost=100;
set force_parallel_mode=off;
-------------------------------------------------------------------------------
