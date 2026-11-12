-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= geometry 'Polygon((1 1,2 2,3 1,1 1))';

SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #<> geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> geometry 'Polygon((1 1,2 2,3 1,1 1))';

SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT geometry 'Polygon((1 1,2 2,3 1,1 1))' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

-- An empty geometry has no point, so it equals no value
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= geometry 'Polygon empty';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> geometry 'Polygon empty';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' ?= geometry 'Polygon empty';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' %<> geometry 'Polygon empty';

/* Errors */
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01' #= trgeometry 'SRID=3812;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box

SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' && stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 04:00, 2001-01-01 06:00])';
SELECT stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 08:00, 2001-01-01 10:00])' ?&& trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]';
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(11,11))';
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(9,9))';
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));[Pose(Point(0 0), 0)@2001-01-01, Pose(Point(10 10), 0)@2001-01-02]' ?&& stbox 'STBOX T([2001-01-03, 2001-01-04])';
/* Errors */
SELECT trgeometry 'Polygon((0 0,1 0,0 1,0 0));Pose(Point(0 0), 0)@2001-01-01' ?&& stbox 'SRID=4326;STBOX X((0,0),(1,1))';
