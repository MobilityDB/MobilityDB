-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- tquadbin type: input/output, typmod, ASSIGNMENT casts to/from tbigint,
-- comparison operators, btree/hash indexing, binary IO round-trip.
--
-- The basetype is hex-encoded; quadbin '480fffffffffffff' is the z0 world
-- cell, '48427fffffffffff' is tile(3,5,4), '48a6227affffffff' is res 10.

-------------------------------------------------------------------------------
-- Input/output
-------------------------------------------------------------------------------
-- Temporal instant
SELECT tquadbin '480fffffffffffff@2012-01-01 08:00:00';
/* Errors */
SELECT tquadbin 'ABC@2012-01-01 08:00:00';
SELECT tquadbin '480fffffffffffff@2012-01-01 08:00:00,';

-------------------------------------------------------------------------------
-- Temporal discrete sequence

SELECT tquadbin ' { 480fffffffffffff@2001-01-01 08:00:00 , 48427fffffffffff@2001-01-01 08:05:00 , 48a6227affffffff@2001-01-01 08:06:00 } ';
SELECT tquadbin '{480fffffffffffff@2001-01-01 08:00:00,48427fffffffffff@2001-01-01 08:05:00}';
/* Errors */
SELECT tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02';

-------------------------------------------------------------------------------
-- Temporal continuous sequence (step interpolation inherited from int8)

SELECT tquadbin '[480fffffffffffff@2001-01-01 08:00:00,48427fffffffffff@2001-01-01 08:05:00]';
SELECT tquadbin 'Interp=Step;[480fffffffffffff@2001-01-01 08:00:00,48427fffffffffff@2001-01-01 08:05:00]';

-------------------------------------------------------------------------------
-- Temporal sequence set

SELECT tquadbin '{[480fffffffffffff@2001-01-01 08:00:00,48427fffffffffff@2001-01-01 08:05:00],[48a6227affffffff@2001-01-01 08:10:00]}';

-------------------------------------------------------------------------------
-- Typmod
-------------------------------------------------------------------------------

SELECT tquadbin(Instant) '480fffffffffffff@2012-01-01 08:00:00';
SELECT tquadbin(Sequence) '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02]';
SELECT tquadbin(SequenceSet) '{[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02]}';
/* Errors */
SELECT tquadbin(Instant) '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}';
SELECT tquadbin(Garbage) '480fffffffffffff@2012-01-01 08:00:00';

-------------------------------------------------------------------------------
-- Assignment casts to/from tbigint (explicit AS ASSIGNMENT, bidirectional)
-------------------------------------------------------------------------------

SELECT (tquadbin '480fffffffffffff@2012-01-01 08:00:00')::tbigint;
SELECT (tbigint '5192650370358181887@2012-01-01 08:00:00')::tquadbin;

-- Round-trip preserves value
SELECT ((tquadbin '480fffffffffffff@2012-01-01 08:00:00')::tbigint)::tquadbin
  = tquadbin '480fffffffffffff@2012-01-01 08:00:00';

-- Assignment-context: cast accepted in INSERT without ::
DROP TABLE IF EXISTS tbl_tquadbin_assign;
CREATE TABLE tbl_tquadbin_assign(k int, temp tquadbin);
INSERT INTO tbl_tquadbin_assign VALUES (1, tbigint '5192650370358181887@2012-01-01 08:00:00');
SELECT * FROM tbl_tquadbin_assign;
DROP TABLE tbl_tquadbin_assign;

-- The cast is NOT implicit
/* Errors */
SELECT tbigint '5192650370358181887@2012-01-01 08:00:00' = tquadbin '480fffffffffffff@2012-01-01 08:00:00';

-------------------------------------------------------------------------------
-- Comparison operators + btree/hash
-------------------------------------------------------------------------------

SELECT tquadbin '480fffffffffffff@2012-01-01' = tquadbin '480fffffffffffff@2012-01-01';
SELECT tquadbin '480fffffffffffff@2012-01-01' <> tquadbin '48427fffffffffff@2012-01-01';
SELECT temporal_cmp(tquadbin '480fffffffffffff@2012-01-01', tquadbin '480fffffffffffff@2012-01-01');

-------------------------------------------------------------------------------
-- Send/receive (binary IO round-trip)
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_tquadbin_binio;
CREATE TABLE tbl_tquadbin_binio(k int, temp tquadbin);
INSERT INTO tbl_tquadbin_binio VALUES
  (1, tquadbin '480fffffffffffff@2012-01-01 08:00:00'),
  (2, tquadbin '{480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02}'),
  (3, tquadbin '[480fffffffffffff@2001-01-01, 48427fffffffffff@2001-01-02]');

COPY tbl_tquadbin_binio TO '/tmp/tbl_tquadbin_binio' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_tquadbin_binio_tmp;
CREATE TABLE tbl_tquadbin_binio_tmp AS TABLE tbl_tquadbin_binio WITH NO DATA;
COPY tbl_tquadbin_binio_tmp FROM '/tmp/tbl_tquadbin_binio' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_tquadbin_binio t1, tbl_tquadbin_binio_tmp t2
  WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_tquadbin_binio;
DROP TABLE tbl_tquadbin_binio_tmp;

-------------------------------------------------------------------------------
