--------------------------------------------------

SELECT tand(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tand(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

SELECT tor(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tor(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tbool),('true@2000-01-01'::tbool)) t(temp);

--------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tsum(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tsum(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT tavg(temp) FROM (VALUES
(NULL::tint),(NULL::tint)) t(temp);
SELECT tavg(temp) FROM (VALUES
(NULL::tint),('1@2000-01-01'::tint)) t(temp);

--------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tsum(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT tavg(temp) FROM (VALUES
(NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT tavg(temp) FROM (VALUES
(NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

--------------------------------------------------

SELECT tmin(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tmin(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

SELECT tmax(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tmax(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

SELECT tcount(temp) FROM (VALUES
(NULL::ttext),(NULL::ttext)) t(temp);
SELECT tcount(temp) FROM (VALUES
(NULL::ttext),('AAAA@2000-01-01'::ttext)) t(temp);

--------------------------------------------------





