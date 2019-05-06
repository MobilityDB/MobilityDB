-----------------------------------------------------------------------
-- Boolean functions and operators
-- N.B. The names and, or, not are reserved words 
-----------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE TRUE & temp IS NOT NULL;
SELECT count(*) FROM tbl_tbool WHERE temp & TRUE IS NOT NULL;
SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp & t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE TRUE | temp IS NOT NULL;
SELECT count(*) FROM tbl_tbool WHERE temp | TRUE IS NOT NULL;
SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp | t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE ~ temp IS NOT NULL;

/*****************************************************************************/
