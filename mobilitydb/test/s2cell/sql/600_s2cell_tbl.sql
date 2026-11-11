-- Table-based tests for the static s2cell type, exercised over tbl_s2cell.

-------------------------------------------------------------------------------
-- Send / receive
-------------------------------------------------------------------------------

\copy tbl_s2cell TO 'tbl_s2cell' (FORMAT BINARY);
DROP TABLE IF EXISTS tbl_s2cell_tmp;
CREATE TABLE tbl_s2cell_tmp AS TABLE tbl_s2cell WITH NO DATA;
\copy tbl_s2cell_tmp FROM 'tbl_s2cell' (FORMAT BINARY);
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell_tmp t2 WHERE t1.k = t2.k AND t1.cell <> t2.cell;
DROP TABLE tbl_s2cell_tmp;

-------------------------------------------------------------------------------
-- Validity predicates
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cell WHERE NOT isValidCell(cell);

-------------------------------------------------------------------------------
-- Casts to / from bigint (assignment)
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cell WHERE (cell::bigint)::s2cell <> cell;

-------------------------------------------------------------------------------
-- Comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell = t2.cell;
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell <> t2.cell;
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell < t2.cell;
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell <= t2.cell;
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell > t2.cell;
SELECT COUNT(*) FROM tbl_s2cell t1, tbl_s2cell t2 WHERE t1.cell >= t2.cell;

-------------------------------------------------------------------------------
