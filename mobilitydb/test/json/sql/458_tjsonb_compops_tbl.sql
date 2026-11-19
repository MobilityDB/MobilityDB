-------------------------------------------------------------------------------
-- Ever comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb ?= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp ?= t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ?= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb ?<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp ?<> t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ?<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Always comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb %= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp %= t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp %= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb %<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp %<> t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp %<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp #= t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_jsonb t1, tbl_tjsonb t2 WHERE t1.jb #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_jsonb t2 WHERE t1.temp #<> t2.jb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Ever and always overlaps with a period: a value ever lies in a period where
-- its restriction to the period is not empty, and always where its
-- restriction to the complement is empty
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tstzspan t2
WHERE t1.temp ?&& t2.t <> (atTime(t1.temp, t2.t) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tstzspan t2
WHERE t1.temp %&& t2.t <> (minusTime(t1.temp, t2.t) IS NULL);
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tstzspan t2
WHERE t2.t ?&& t1.temp <> t1.temp ?&& t2.t;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tstzspan t2 WHERE t1.temp ?&& t2.t;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tstzspan t2 WHERE t1.temp %&& t2.t;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tjsonb t1, tbl_tjsonb t2 WHERE t1.temp %&& t2.temp;

-------------------------------------------------------------------------------
