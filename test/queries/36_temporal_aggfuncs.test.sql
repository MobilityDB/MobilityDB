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

SELECT extent(temp) FROM (VALUES
(NULL::tbool),(NULL::tbool)) t(temp);
SELECT tcount(temp) FROM (VALUES
('true@2000-01-01'::tbool),(NULL::tbool)) t(temp);

SELECT extent(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool), 
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool), 
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tand(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool), 
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

SELECT tor(temp) FROM (VALUES
('[true@2000-01-01, false@2000-01-03, true@2000-01-05, false@2000-01-07]'::tbool), 
('[true@2000-01-02, false@2000-01-06]'::tbool)) t(temp);

--------------------------------------------------

SELECT extent(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tmin(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tmax(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tsum(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

SELECT tavg(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tint), 
('[3@2000-01-02, 4@2000-01-06]'::tint)) t(temp);

--------------------------------------------------

SELECT extent(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tmin(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tmax(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tavg(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('Interp=Stepwise;[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

--------------------------------------------------

SELECT extent(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tcount(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tmin(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tmax(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT tsum(temp) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

SELECT round(tavg(temp), 6) FROM (VALUES
('[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

--------------------------------------------------

/* Errors */
SELECT tsum(temp) FROM ( VALUES
(tfloat '[1@2000-01-01, 2@2000-01-02]'), 
(tfloat '{3@2000-01-03, 4@2000-01-04}')) t(temp);
SELECT tsum(temp) FROM ( VALUES
(tfloat '{1@2000-01-01, 2@2000-01-02}'), 
(tfloat '[3@2000-01-03, 4@2000-01-04]')) t(temp);
SELECT tsum(temp) FROM (VALUES
('Interp=Stepwise;[1@2000-01-01, 2@2000-01-03, 1@2000-01-05, 2@2000-01-07]'::tfloat), 
('[3@2000-01-02, 4@2000-01-06]'::tfloat)) t(temp);

--------------------------------------------------
