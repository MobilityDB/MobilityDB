-------------------------------------------------------------------------------
-- Temporal eq
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE true #= temp IS NOT NULL;
SELECT count(*) FROM tbl_tbool WHERE temp #= true IS NOT NULL;
SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #= i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #= f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #= i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #= f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #= temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #= t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal ne
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tbool WHERE true #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_tbool WHERE temp #<> true IS NOT NULL;
SELECT count(*) FROM tbl_tbool t1, tbl_tbool t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #<> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #<> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #<> i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #<> f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #<> i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #<> f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #<> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #<> temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #<> t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal lt
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #< temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #< temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #< temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #< temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #< i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #< f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #< t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #< t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #< i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #< f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #< t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #< temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #< t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #< t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal gt
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #> temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #> temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #> i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #> f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #> t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #> i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #> f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #> t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #> temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #> t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal le
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #<= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #<= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #<= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #<= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #<= i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #<= f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #<= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #<= i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #<= f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #<= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #<= temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #<= t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #<= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal ge
-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tint, tbl_int WHERE i #>= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_int WHERE i #>= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_float WHERE f #>= temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE f #>= temp IS NOT NULL;

SELECT count(*) FROM tbl_tint, tbl_int WHERE temp #>= i IS NOT NULL;
SELECT count(*) FROM tbl_tint, tbl_float WHERE temp #>= f IS NOT NULL;

SELECT count(*) FROM tbl_tint t1, tbl_tint t2 WHERE t1.temp #>= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tint t1, tbl_tfloat t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

SELECT count(*) FROM tbl_tfloat, tbl_int WHERE temp #>= i IS NOT NULL;
SELECT count(*) FROM tbl_tfloat, tbl_float WHERE temp #>= f IS NOT NULL;

SELECT count(*) FROM tbl_tfloat t1, tbl_tint t2 WHERE t1.temp #>= t2.temp IS NOT NULL;
SELECT count(*) FROM tbl_tfloat t1, tbl_tfloat t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_ttext, tbl_text WHERE t #>= temp IS NOT NULL;
SELECT count(*) FROM tbl_ttext, tbl_text WHERE temp #>= t IS NOT NULL;
SELECT count(*) FROM tbl_ttext t1, tbl_ttext t2 WHERE t1.temp #>= t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
