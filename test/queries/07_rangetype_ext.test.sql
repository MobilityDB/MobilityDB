-------------------------------------------------------------------------------
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

select intrange 'empty' << 5;
select intrange '[3,5)' << 5;
select 5 << intrange 'empty';
select 5 << intrange '[3,5)';

select intrange 'empty' >> 5;
select intrange '[3,5)' >> 5;
select 5 >> intrange 'empty';
select 5 >> intrange '[3,5)';

select intrange 'empty' &< 5;
select intrange '[3,5)' &< 5;
select 5 &< intrange 'empty';
select 5 &< intrange '[3,5)';

select intrange 'empty' &> 5;
select intrange '[3,5)' &> 5;
select 5 &> intrange 'empty';
select 5 &> intrange '[3,5)';

select intrange 'empty' -|- 5;
select intrange '[3,5)' -|- 5;
select 5 -|- intrange 'empty';
select 5 -|- intrange '[3,5)';

-------------------------------------------------------------------------------

select floatrange 'empty' << 5.5;
select floatrange '[3.5, 5.5]' << 5.5;
select 5.5 << floatrange 'empty';
select 5.5 << floatrange '[3.5, 5.5]';

select floatrange 'empty' >> 5.5;
select floatrange '[3.5, 5.5]' >> 5.5;
select 5.5 >> floatrange 'empty';
select 5.5 >> floatrange '[3.5, 5.5]';

select floatrange 'empty' &< 5.5;
select floatrange '[3.5, 5.5]' &< 5.5;
select 5.5 &< floatrange 'empty';
select 5.5 &< floatrange '[3.5, 5.5]';

select floatrange 'empty' &> 5.5;
select floatrange '[3.5, 5.5]' &> 5.5;
select 5.5 &> floatrange 'empty';
select 5.5 &> floatrange '[3.5, 5.5]';

select floatrange 'empty' -|- 5.5;
select floatrange '[3.5, 5.5]' -|- 5.5;
select 5.5 -|- floatrange 'empty';
select 5.5 -|- floatrange '[3.5, 5.5]';

-------------------------------------------------------------------------------
