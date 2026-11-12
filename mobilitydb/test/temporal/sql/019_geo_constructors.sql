-------------------------------------------------------------------------------
-- Tests for constructors of PostgreSQL geometry types.
-- File geo_constructors.c
-------------------------------------------------------------------------------

SELECT point(1.5, 2.5);
SELECT line(1.5, 2.5, 3.5);
SELECT lseg(point(1.5, 2.5), point(3.5, 4.5));
SELECT box(point(1.5, 2.5), point(3.5, 4.5));
SELECT circle(point(1.5, 2.5), 2.5);
SELECT path(ARRAY[point(1.5, 1.5), point(2.5, 2.5)]);
SELECT path(ARRAY[point(1.5, 1.5), point(2.5, 2.5), point(1.5, 1.5)]);
SELECT polygon(ARRAY[point(1.5, 1.5), point(2.5, 2.5), point(1.5, 1.5)]);
SELECT polygon(ARRAY[point(2.5, 2.5), point(3.5, 1.5), point(1.5, 1.5), point(2.5, 2.5)]);

-------------------------------------------------------------------------------
