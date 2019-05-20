-------------------------------------------------------------------------------
/* Errors */

select astext(tcentroid(temp)) from (values 
(tgeompoint 'Point(0 0)@2000-01-01'),(tgeompoint 'srid=5676;Point(1 1)@2000-01-01)'),('Point(2 2)@2000-01-01)')) v(temp);

select astext(tcentroid(temp)) from (values 
(tgeompoint 'Point(0 0)@2000-01-01'),(tgeompoint 'Point(1 1)@2000-01-01)'),('Point(2 2 2)@2000-01-01)')) v(temp);

select astext(tcentroid(temp)) from (values 
(tgeompoint 'Point(0 0)@2000-01-01'),(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02]'),('Point(2 2 2)@2000-01-01)')) v(temp);

-------------------------------------------------------------------------------
