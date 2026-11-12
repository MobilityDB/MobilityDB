-------------------------------------------------------------------------------
-- Temporal text concatenation
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE t || temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext, tbl_text WHERE temp || t IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp || t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal uppercase, lowercase or initcap
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_ttext WHERE upper(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext WHERE lower(temp) IS NOT NULL;
SELECT COUNT(*) FROM tbl_ttext WHERE initcap(temp) IS NOT NULL;

-------------------------------------------------------------------------------

