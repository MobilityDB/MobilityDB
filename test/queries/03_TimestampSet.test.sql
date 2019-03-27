-------------------------------------------------------------------------------
-- Tests for timestamp set data type.
-- File TimestampSet.c
-------------------------------------------------------------------------------

select memSize(timestampset '{2000-01-01}');
select memSize(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select timespan(timestampset '{2000-01-01}');
select timespan(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select numTimestamps(timestampset '{2000-01-01}');
select numTimestamps(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select startTimestamp(timestampset '{2000-01-01}');
select startTimestamp(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select endTimestamp(timestampset '{2000-01-01}');
select endTimestamp(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select timestampN(timestampset '{2000-01-01}', 1);
select timestampN(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', 1);
select timestampN(timestampset '{2000-01-01}', 2);
select timestampN(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', 4);

select timestamps(timestampset '{2000-01-01}');
select timestamps(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}');

select shift(timestampset '{2000-01-01}', '5 min');
select shift(timestampset '{2000-01-01, 2000-01-02, 2000-01-03}', '5 min');

select timestampset_cmp(timestampset '{2000-01-01}', timestampset '{2000-01-01, 2000-01-02, 2000-01-03}') = -1;
select timestampset '{2000-01-01}' = timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
select timestampset '{2000-01-01}' <> timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
select timestampset '{2000-01-01}' < timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
select timestampset '{2000-01-01}' <= timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
select timestampset '{2000-01-01}' > timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';
select timestampset '{2000-01-01}' >= timestampset '{2000-01-01, 2000-01-02, 2000-01-03}';

-------------------------------------------------------------------------------
