-------------------------------------------------------------------------------
-- contains
-------------------------------------------------------------------------------

SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT contains(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------
-- disjoint
-------------------------------------------------------------------------------

SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-------------------------------------------------------------------------------
-- intersects
-------------------------------------------------------------------------------

SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-------------------------------------------------------------------------------
-- touches
-------------------------------------------------------------------------------

SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

-------------------------------------------------------------------------------
-- dwithin
-------------------------------------------------------------------------------

SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)', 2);

SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 2);
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
SELECT dwithin(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 2);
-- NULL
SELECT dwithin(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02', 2);

-------------------------------------------------------------------------------
