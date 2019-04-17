--------------------------------------------------

SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);


SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wcount(temp, NULL) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wcount(temp, NULL) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wsum(temp, NULL) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wsum(temp, NULL) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wavg(temp, NULL) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wavg(temp, NULL) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

--------------------------------------------------

SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wcount(temp, NULL) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wcount(temp, NULL) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wsum(temp, NULL) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wsum(temp, NULL) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wavg(temp, NULL) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wavg(temp, NULL) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

--------------------------------------------------


