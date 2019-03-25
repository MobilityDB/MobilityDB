-------------------------------------------------------------------------------
-- Tests for period data type.
-- File Period.c
-------------------------------------------------------------------------------

select tstzrange(period '[2000-01-01,2000-01-01]');
select tstzrange(period '[2000-01-01,2000-01-02]');
select tstzrange(period '(2000-01-01,2000-01-02]');
select tstzrange(period '[2000-01-01,2000-01-02)');
select tstzrange(period '(2000-01-01,2000-01-02)');

select period(tstzrange '[2000-01-01,2000-01-01]');
select period(tstzrange '[2000-01-01,2000-01-02]');
select period(tstzrange '(2000-01-01,2000-01-02]');
select period(tstzrange '[2000-01-01,2000-01-02)');
select period(tstzrange'(2000-01-01,2000-01-02)');

select lower(period '[2000-01-01,2000-01-01]');
select lower(period '[2000-01-01,2000-01-02]');
select lower(period '(2000-01-01,2000-01-02]');
select lower(period '[2000-01-01,2000-01-02)');
select lower(period '(2000-01-01,2000-01-02)');

select upper(period '[2000-01-01,2000-01-01]');
select upper(period '[2000-01-01,2000-01-02]');
select upper(period '(2000-01-01,2000-01-02]');
select upper(period '[2000-01-01,2000-01-02)');
select upper(period '(2000-01-01,2000-01-02)');

select lower_inc(period '[2000-01-01,2000-01-01]');
select lower_inc(period '[2000-01-01,2000-01-02]');
select lower_inc(period '(2000-01-01,2000-01-02]');
select lower_inc(period '[2000-01-01,2000-01-02)');
select lower_inc(period '(2000-01-01,2000-01-02)');

select upper_inc(period '[2000-01-01,2000-01-01]');
select upper_inc(period '[2000-01-01,2000-01-02]');
select upper_inc(period '(2000-01-01,2000-01-02]');
select upper_inc(period '[2000-01-01,2000-01-02)');
select upper_inc(period '(2000-01-01,2000-01-02)');

select duration(period '[2000-01-01,2000-01-01]');
select duration(period '[2000-01-01,2000-01-02]');
select duration(period '(2000-01-01,2000-01-02]');
select duration(period '[2000-01-01,2000-01-02)');
select duration(period '(2000-01-01,2000-01-02)');

select shift(period '[2000-01-01,2000-01-01]', '5 min');
select shift(period '[2000-01-01,2000-01-02]', '5 min');
select shift(period '(2000-01-01,2000-01-02]', '5 min');
select shift(period '[2000-01-01,2000-01-02)', '5 min');
select shift(period '(2000-01-01,2000-01-02)', '5 min');

select period_cmp('[2000-01-01,2000-01-01]', '(2000-01-01,2000-01-02)');
select period '[2000-01-01,2000-01-01]' = period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' <> period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' < period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' <= period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' > period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' >= period '(2000-01-01,2000-01-02)';
select period '[2000-01-01,2000-01-01]' = period '(2000-01-01,2000-01-02)';

-------------------------------------------------------------------------------
