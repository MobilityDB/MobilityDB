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

SELECT contains(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT contains(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT contains(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT contains(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT contains(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- containsproperly
-------------------------------------------------------------------------------

SELECT containsproperly(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT containsproperly(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT containsproperly(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT containsproperly(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT containsproperly(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT containsproperly(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT containsproperly(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- covers
-------------------------------------------------------------------------------

SELECT covers(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT covers(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT covers(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT covers(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT covers(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT covers(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT covers(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- coveredby
-------------------------------------------------------------------------------

SELECT coveredby(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT coveredby(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT coveredby(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT coveredby(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT coveredby(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT coveredby(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT coveredby(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- crosses
-------------------------------------------------------------------------------

SELECT crosses(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT crosses(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT crosses(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT crosses(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT crosses(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT crosses(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT crosses(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

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

SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT disjoint(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT disjoint(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT disjoint(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT disjoint(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- equals
-------------------------------------------------------------------------------

SELECT equals(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT equals(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT equals(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT equals(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT equals(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT equals(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT equals(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

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

SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT intersects(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT intersects(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT intersects(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT intersects(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- overlaps
-------------------------------------------------------------------------------

SELECT overlaps(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT overlaps(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT overlaps(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT overlaps(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT overlaps(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT overlaps(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT overlaps(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

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

SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT touches(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT touches(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT touches(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT touches(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- within
-------------------------------------------------------------------------------

SELECT within(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT within(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT within(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT within(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT within(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT within(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT within(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

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
-- relate (2 arguments returns text)
-------------------------------------------------------------------------------

SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT relate(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}');
-- NULL
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02');

-------------------------------------------------------------------------------
-- relate (3 arguments returns boolean)
-------------------------------------------------------------------------------

SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point(1 1)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');

SELECT relate(geometry 'SRID=5676;Point empty', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(geometry 'SRID=5676;Point empty', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');

SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(npoint 'NPoint(1, 0.5)', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point(1 1)', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point(1 1)', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point(1 1)', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point(1 1)', 'T*****FF*');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  geometry 'SRID=5676;Point empty', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  geometry 'SRID=5676;Point empty', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  geometry 'SRID=5676;Point empty', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  geometry 'SRID=5676;Point empty', 'T*****FF*');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01',  npoint 'NPoint(1, 0.5)', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}',  npoint 'NPoint(1, 0.5)', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]',  npoint 'NPoint(1, 0.5)', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}',  npoint 'NPoint(1, 0.5)', 'T*****FF*');

SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint 'Npoint(1, 0.5)@2000-01-01', 'T*****FF*');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', 'T*****FF*');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', 'T*****FF*');
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');
SELECT relate(tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');
SELECT relate(tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');
SELECT relate(tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.5)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}', 'T*****FF*');
-- NULL
SELECT relate(tnpoint 'Npoint(1, 0.5)@2000-01-01', tnpoint 'Npoint(1, 0.5)@2000-01-02', 'T*****FF*');

-------------------------------------------------------------------------------
