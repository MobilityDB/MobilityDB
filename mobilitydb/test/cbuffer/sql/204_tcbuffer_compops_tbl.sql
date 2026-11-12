-------------------------------------------------------------------------------
-- Ever comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb ?= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp ?= t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp ?= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb ?<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp ?<> t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp ?<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Always comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb %= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp %= t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp %= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb %<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp %<> t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp %<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------
-- Temporal comparisons
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb #= t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp #= t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #= t2.temp IS NOT NULL;

SELECT COUNT(*) FROM tbl_cbuffer t1, tbl_tcbuffer t2 WHERE t1.cb #<> t2.temp IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_cbuffer t2 WHERE t1.temp #<> t2.cb IS NOT NULL;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.temp #<> t2.temp IS NOT NULL;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box

SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year') <> (atStbox(t1.temp, expandTime(expandSpace(stbox(t2.temp), 20), '1 year')) IS NOT NULL);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year') <> (minusStbox(t1.temp, expandTime(expandSpace(stbox(t2.temp), 20), '1 year')) IS NULL);
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND expandTime(expandSpace(stbox(t2.temp), 20), '1 year') ?&& t1.temp <> t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp ?&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp %&& expandTime(expandSpace(stbox(t2.temp), 20), '1 year');
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2 WHERE t1.k <= 30 AND t2.k <= 30
AND t1.temp && expandTime(expandSpace(stbox(t2.temp), 20), '1 year');

-- Two values ever overlap only where their boxes overlap, and where they
-- always overlap they ever do
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
WHERE t1.temp ?&& t2.temp AND NOT t1.temp && t2.temp;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
WHERE t1.temp %&& t2.temp AND NOT t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
WHERE t1.temp ?&& t2.temp;
SELECT COUNT(*) FROM tbl_tcbuffer t1, tbl_tcbuffer t2
WHERE t1.temp %&& t2.temp;
