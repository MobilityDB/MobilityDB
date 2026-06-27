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

-- GiST and SP-GiST index support for tquadbin: build each opclass over a
-- small table and confirm the bounding-box operators return the same rows as
-- a sequential scan (the index is correct, not just creatable).

CREATE TABLE tbl_tquadbin(k int, temp tquadbin);
INSERT INTO tbl_tquadbin VALUES
  (1, tquadbin '480fffffffffffff@2001-01-01'),
  (2, tquadbin '48427fffffffffff@2001-01-02'),
  (3, tquadbin '48a6227affffffff@2001-01-03'),
  (4, tquadbin '[480fffffffffffff@2001-01-04, 48427fffffffffff@2001-01-05]'),
  (5, tquadbin '{48a6227affffffff@2001-01-06, 48427fffffffffff@2001-01-07}');

-- Reference counts via sequential scan (no index)
SET enable_seqscan = on; SET enable_indexscan = off; SET enable_bitmapscan = off;
SELECT count(*) FROM tbl_tquadbin WHERE temp && tquadbin '480fffffffffffff@2001-01-01';
SELECT count(*) FROM tbl_tquadbin WHERE temp && tstzspan '[2001-01-02, 2001-01-05]';
SELECT count(*) FROM tbl_tquadbin WHERE temp <@ (tquadbin '480fffffffffffff@2001-01-01')::stbox;

-- GiST: same operators must return the same counts
CREATE INDEX tbl_tquadbin_gist ON tbl_tquadbin USING gist(temp);
SET enable_seqscan = off; SET enable_indexscan = on; SET enable_bitmapscan = on;
SELECT count(*) FROM tbl_tquadbin WHERE temp && tquadbin '480fffffffffffff@2001-01-01';
SELECT count(*) FROM tbl_tquadbin WHERE temp && tstzspan '[2001-01-02, 2001-01-05]';
DROP INDEX tbl_tquadbin_gist;

-- SP-GiST (quadtree + kdtree)
CREATE INDEX tbl_tquadbin_spgist ON tbl_tquadbin USING spgist(temp tquadbin_quadtree_ops);
SELECT count(*) FROM tbl_tquadbin WHERE temp && tquadbin '480fffffffffffff@2001-01-01';
DROP INDEX tbl_tquadbin_spgist;
CREATE INDEX tbl_tquadbin_kdtree ON tbl_tquadbin USING spgist(temp tquadbin_kdtree_ops);
SELECT count(*) FROM tbl_tquadbin WHERE temp && tstzspan '[2001-01-02, 2001-01-05]';
DROP TABLE tbl_tquadbin;

SET enable_seqscan = on; SET enable_indexscan = on; SET enable_bitmapscan = on;
-------------------------------------------------------------------------------
