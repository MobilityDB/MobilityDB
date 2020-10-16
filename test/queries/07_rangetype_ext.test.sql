-------------------------------------------------------------------------------
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

SELECT intrange 'empty' << 5;
SELECT intrange '[3,5)' << 5;
SELECT 5 << intrange 'empty';
SELECT 5 << intrange '[3,5)';

SELECT intrange 'empty' >> 5;
SELECT intrange '[3,5)' >> 5;
SELECT 5 >> intrange 'empty';
SELECT 5 >> intrange '[3,5)';

SELECT intrange 'empty' &< 5;
SELECT intrange '[3,5)' &< 5;
SELECT 5 &< intrange 'empty';
SELECT 5 &< intrange '[3,5)';

SELECT intrange 'empty' &> 5;
SELECT intrange '[3,5)' &> 5;
SELECT 5 &> intrange 'empty';
SELECT 5 &> intrange '[3,5)';

SELECT intrange 'empty' -|- 5;
SELECT intrange '[3,5)' -|- 5;
SELECT 5 -|- intrange 'empty';
SELECT 5 -|- intrange '[3,5)';

-------------------------------------------------------------------------------

SELECT floatrange 'empty' << 5.5;
SELECT floatrange '[3.5, 5.5]' << 5.5;
SELECT 5.5 << floatrange 'empty';
SELECT 5.5 << floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' >> 5.5;
SELECT floatrange '[3.5, 5.5]' >> 5.5;
SELECT 5.5 >> floatrange 'empty';
SELECT 5.5 >> floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' &< 5.5;
SELECT floatrange '[3.5, 5.5]' &< 5.5;
SELECT 5.5 &< floatrange 'empty';
SELECT 5.5 &< floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' &> 5.5;
SELECT floatrange '[3.5, 5.5]' &> 5.5;
SELECT 5.5 &> floatrange 'empty';
SELECT 5.5 &> floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' -|- 5.5;
SELECT floatrange '[3.5, 5.5]' -|- 5.5;
SELECT 5.5 -|- floatrange 'empty';
SELECT 5.5 -|- floatrange '[3.5, 5.5]';

-------------------------------------------------------------------------------
