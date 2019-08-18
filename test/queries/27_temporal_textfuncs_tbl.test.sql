/*****************************************************************************
 * Temporal text concatenation
 *****************************************************************************/

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t || temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp || t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp || t2.temp IS NOT NULL;

/*****************************************************************************
 * Temporal upper/lower case
 *****************************************************************************/

SELECT count(*) FROM tbl_ttext WHERE upper(temp) IS NOT NULL;
SELECT count(*) FROM tbl_ttext WHERE lower(temp) IS NOT NULL;

/*****************************************************************************/

