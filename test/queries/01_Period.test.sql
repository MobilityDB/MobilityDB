-------------------------------------------------------------------------------
-- Tests for period data type.
-- File Period.c
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT period ('2000-01-01','2000-01-02');
SELECT period ('2000-01-01','2000-01-01', true, true);
/* Errors */
SELECT period ('2000-01-01','2000-01-01');
SELECT period ('2000-01-02','2000-01-01');

-------------------------------------------------------------------------------
-- Casting
-------------------------------------------------------------------------------

SELECT tstzrange(period '[2000-01-01,2000-01-01]');
SELECT tstzrange(period '[2000-01-01,2000-01-02]');
SELECT tstzrange(period '(2000-01-01,2000-01-02]');
SELECT tstzrange(period '[2000-01-01,2000-01-02)');
SELECT tstzrange(period '(2000-01-01,2000-01-02)');

SELECT period(tstzrange '[2000-01-01,2000-01-01]');
SELECT period(tstzrange '[2000-01-01,2000-01-02]');
SELECT period(tstzrange '(2000-01-01,2000-01-02]');
SELECT period(tstzrange '[2000-01-01,2000-01-02)');
SELECT period(tstzrange'(2000-01-01,2000-01-02)');

SELECT period(timestamptz '2000-01-01');
SELECT timestamptz '2000-01-01'::period;
/* Errors */
select tstzrange '[2000-01-01,]'::period;
select tstzrange '[,2000-01-01]'::period;
select tstzrange 'empty'::period;

-------------------------------------------------------------------------------
-- Functions
-------------------------------------------------------------------------------

SELECT lower(period '[2000-01-01,2000-01-01]');
SELECT lower(period '[2000-01-01,2000-01-02]');
SELECT lower(period '(2000-01-01,2000-01-02]');
SELECT lower(period '[2000-01-01,2000-01-02)');
SELECT lower(period '(2000-01-01,2000-01-02)');

SELECT upper(period '[2000-01-01,2000-01-01]');
SELECT upper(period '[2000-01-01,2000-01-02]');
SELECT upper(period '(2000-01-01,2000-01-02]');
SELECT upper(period '[2000-01-01,2000-01-02)');
SELECT upper(period '(2000-01-01,2000-01-02)');

SELECT lower_inc(period '[2000-01-01,2000-01-01]');
SELECT lower_inc(period '[2000-01-01,2000-01-02]');
SELECT lower_inc(period '(2000-01-01,2000-01-02]');
SELECT lower_inc(period '[2000-01-01,2000-01-02)');
SELECT lower_inc(period '(2000-01-01,2000-01-02)');

SELECT upper_inc(period '[2000-01-01,2000-01-01]');
SELECT upper_inc(period '[2000-01-01,2000-01-02]');
SELECT upper_inc(period '(2000-01-01,2000-01-02]');
SELECT upper_inc(period '[2000-01-01,2000-01-02)');
SELECT upper_inc(period '(2000-01-01,2000-01-02)');

SELECT duration(period '[2000-01-01,2000-01-01]');
SELECT duration(period '[2000-01-01,2000-01-02]');
SELECT duration(period '(2000-01-01,2000-01-02]');
SELECT duration(period '[2000-01-01,2000-01-02)');
SELECT duration(period '(2000-01-01,2000-01-02)');

SELECT shift(period '[2000-01-01,2000-01-01]', '5 min');
SELECT shift(period '[2000-01-01,2000-01-02]', '5 min');
SELECT shift(period '(2000-01-01,2000-01-02]', '5 min');
SELECT shift(period '[2000-01-01,2000-01-02)', '5 min');
SELECT shift(period '(2000-01-01,2000-01-02)', '5 min');

SELECT period_cmp('[2000-01-01,2000-01-01]', '(2000-01-01,2000-01-02)');
SELECT period '[2000-01-01,2000-01-01]' = period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' <> period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' < period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' <= period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' > period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' >= period '(2000-01-01,2000-01-02)';
SELECT period '[2000-01-01,2000-01-01]' = period '(2000-01-01,2000-01-02)';

SELECT period_hash('[2000-01-01,2000-01-02]');
SELECT period_hash_extended('[2000-01-01,2000-01-02]');

-------------------------------------------------------------------------------
