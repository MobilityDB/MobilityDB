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

-- The two index classes of a ts2cell. The key is the stbox, as it is for every
-- cell index, so a table indexed either way answers a bounding-box query with
-- the rows a sequential scan gives.

CREATE TABLE tbl_ts2cell_idx AS
SELECT k, ts2cell(geoToS2Cell(geography(ST_SetSRID(ST_MakePoint(
    4.0 + k * 0.01, 50.0 + k * 0.01), 4326)), 12),
  timestamptz '2001-01-01' + k * interval '1 hour') AS temp
FROM generate_series(1, 200) k;

SELECT COUNT(*) FROM tbl_ts2cell_idx
WHERE temp && stbox 'SRID=4326;GEODSTBOX X((4.0,50.0),(4.5,50.5))';

CREATE INDEX tbl_ts2cell_rtree_idx ON tbl_ts2cell_idx USING gist(temp);
SELECT COUNT(*) FROM tbl_ts2cell_idx
WHERE temp && stbox 'SRID=4326;GEODSTBOX X((4.0,50.0),(4.5,50.5))';
DROP INDEX tbl_ts2cell_rtree_idx;

CREATE INDEX tbl_ts2cell_quadtree_idx ON tbl_ts2cell_idx USING spgist(temp);
SELECT COUNT(*) FROM tbl_ts2cell_idx
WHERE temp && stbox 'SRID=4326;GEODSTBOX X((4.0,50.0),(4.5,50.5))';
DROP INDEX tbl_ts2cell_quadtree_idx;

DROP TABLE tbl_ts2cell_idx;

-------------------------------------------------------------------------------
