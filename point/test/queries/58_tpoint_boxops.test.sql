﻿-------------------------------------------------------------------------------

SELECT geometry 'Point empty'::stbox;
SELECT geometry 'Point(1 1)'::stbox;
SELECT geometry 'Point(1 1 1)'::stbox;
SELECT setprecision(geography 'Point empty'::stbox, 13);
SELECT setprecision(geography 'Point(1 1)'::stbox, 13);
SELECT setprecision(geography 'Point(1 1 1)'::stbox, 13);
SELECT timestamptz '2000-01-01'::stbox;
SELECT timestampset '{2000-01-01, 2000-01-02}'::stbox;
SELECT period '[2000-01-01, 2000-01-02]'::stbox;
SELECT periodset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}'::stbox;
SELECT stbox(geometry 'Point empty', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point(1 1)', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point(1 1 1)', timestamptz '2000-01-01');
SELECT stbox(geometry 'Point empty', period '[2000-01-01, 2000-01-02]');
SELECT stbox(geometry 'Point(1 1)', period '[2000-01-01, 2000-01-02]');
SELECT stbox(geometry 'Point(1 1 1)', period '[2000-01-01, 2000-01-02]');
SELECT setprecision(stbox(geography 'Point empty', timestamptz '2000-01-01'), 13);
SELECT setprecision(stbox(geography 'Point(1 1)', timestamptz '2000-01-01'), 13);
SELECT setprecision(stbox(geography 'Point(1 1 1)', timestamptz '2000-01-01'), 13);
SELECT setprecision(stbox(geography 'Point empty', period '[2000-01-01, 2000-01-02]'), 13);
SELECT setprecision(stbox(geography 'Point(1 1)', period '[2000-01-01, 2000-01-02]'), 13);
SELECT setprecision(stbox(geography 'Point(1 1 1)', period '[2000-01-01, 2000-01-02]'), 13);

SELECT tgeompoint 'Point(1 1)@2000-01-01'::stbox;
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::stbox;
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::stbox;
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::stbox;

SELECT setprecision(tgeogpoint 'Point(1 1)@2000-01-01'::stbox, 13);
SELECT setprecision(tgeogpoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}'::stbox, 13);
SELECT setprecision(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]'::stbox, 13);
SELECT setprecision(tgeogpoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}'::stbox, 13);

-------------------------------------------------------------------------------

SELECT stboxes(tgeompoint '[Point(1 1)@2000-01-01]');
SELECT stboxes(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT stboxes(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}');
-- NULL
SELECT stboxes(tgeompoint 'Point(1 1)@2000-01-01');
SELECT stboxes(tgeompoint '{Point(1 1)@2000-01-01}');

-------------------------------------------------------------------------------

SELECT count(*) FROM tbl_tgeompoint WHERE temp::stbox IS NOT NULL;
SELECT count(*) FROM tbl_tgeogpoint WHERE temp::stbox IS NOT NULL;

-------------------------------------------------------------------------------

SELECT expandSpatial(stbox 'STBOX((1.0, 2.0), (1.0, 2.0))', 0.5);
SELECT expandSpatial(stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))', 0.5);
SELECT expandSpatial(stbox 'STBOX T((1.0, 2.0, 2000-01-03), (1.0, 2.0, 2000-01-03))', 0.5);
SELECT expandSpatial(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-04), (1.0, 2.0, 3.0, 2000-01-03))', 0.5);
SELECT expandSpatial(stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))', 0.5);

SELECT expandTemporal(stbox 'STBOX T((1.0, 2.0, 2000-01-03), (1.0, 2.0, 2000-01-03))', '1 day');
SELECT expandTemporal(stbox 'STBOX ZT((1.0, 2.0, 3.0, 2000-01-04), (1.0, 2.0, 3.0, 2000-01-04))', '1 day');
SELECT expandTemporal(stbox 'GEODSTBOX T((1.0, 2.0, 3.0, 2000-01-04), (1.0, 2.0, 3.0, 2000-01-04))', '1 day');
/* Errors */
SELECT expandTemporal(stbox 'STBOX((1.0, 2.0), (1.0, 2.0))', '1 day');
SELECT expandTemporal(stbox 'STBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))', '1 day');
SELECT expandTemporal(stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))', '1 day');

-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1)' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point(1 1)' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point(1 1)' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point empty' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point empty' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point empty' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point Z empty' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point Z empty' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point Z empty' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1)' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1)' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1)' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point empty' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point empty' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point empty' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' && tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' && tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' && tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' && tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' && tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point Z empty' && tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point Z empty' && tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point Z empty' && tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestamptz '2000-01-01' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestamptz '2000-01-01' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestamptz '2000-01-01' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';
/* Errors */
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeompoint 'Point(1 1 1)@2000-01-01';

SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' && tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && geography 'Point (1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' && geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' && geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' && geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' && geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' && geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' && geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' && geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' && geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' && stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' && stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' && stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' && stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' && stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' && tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' && tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

/* Errors */
SELECT geometry 'SRID=5676;Point(1 1)' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' && geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01'&& geometry 'Point(1 1 1)';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' && tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1,1,2001-01-01),(2,2,2001-01-02))' && stbox 'STBOX T((1,1,2001-01-01),(2,2,2001-01-02))';
SELECT stbox 'GEODSTBOX T((1,1,1,2001-01-01),(2,2,2,2001-01-02))' && stbox 'STBOX T((1,1,2001-01-01),(2,2,2001-01-02))';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' && stbox 'STBOX T((1,1,2001-01-01),(2,2,2001-01-02))';

-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1)' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point(1 1)' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point(1 1)' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point empty' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point empty' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point empty' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point Z empty' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point Z empty' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point Z empty' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1)' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1)' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1)' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point empty' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point empty' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point empty' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' @> tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' @> tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' @> tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' @> tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' @> tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point Z empty' @> tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point Z empty' @> tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point Z empty' @> tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestamptz '2000-01-01' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestamptz '2000-01-01' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestamptz '2000-01-01' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' @> tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' @> geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' @> geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' @> geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' @> geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' @> geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' @> geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' @> geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' @> geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' @> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' @> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' @> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' @> stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' @> stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' @> tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' @> tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

/* Errors */
SELECT geometry 'SRID=5676;Point(1 1)' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' @> geometry 'Point(1 1 1)';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' @> tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1)' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point(1 1)' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point(1 1)' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point empty' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point empty' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point empty' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point Z empty' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point Z empty' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point Z empty' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1)' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1)' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1)' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point empty' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point empty' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point empty' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' <@ tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' <@ tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' <@ tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' <@ tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' <@ tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point Z empty' <@ tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point Z empty' <@ tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point Z empty' <@ tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' <@ tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <@ geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <@ geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <@ geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <@ geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' <@ geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' <@ geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' <@ geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' <@ geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' <@ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' <@ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' <@ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' <@ stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' <@ stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' <@ tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' <@ tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

/* Errors */
SELECT geometry 'SRID=5676;Point(1 1)' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' <@ geometry 'Point(1 1 1)';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' <@ tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1)' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point(1 1)' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point(1 1)' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point empty' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point empty' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point empty' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point Z empty' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point Z empty' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point Z empty' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1)' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1)' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1)' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point empty' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point empty' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point empty' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' -|- tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' -|- tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' -|- tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' -|- tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' -|- tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point Z empty' -|- tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point Z empty' -|- tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point Z empty' -|- tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' -|- tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' -|- geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' -|- geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' -|- geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' -|- geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' -|- geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' -|- geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' -|- geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' -|- geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' -|- stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' -|- stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' -|- stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' -|- stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' -|- stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' -|- tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' -|- tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

/* Errors */
SELECT geometry 'SRID=5676;Point(1 1)' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' -|- geometry 'Point(1 1 1)';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' -|- tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------

SELECT geometry 'Point(1 1)' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1)' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point(1 1)' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point(1 1)' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point empty' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point empty' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT geometry 'Point empty' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT geometry 'Point empty' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT geometry 'Point(1 1 1)' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point(1 1 1)' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point(1 1 1)' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geometry 'Point Z empty' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT geometry 'Point Z empty' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT geometry 'Point Z empty' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT geometry 'Point Z empty' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT geography 'Point(1 1)' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1)' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1)' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1)' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point empty' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT geography 'Point empty' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT geography 'Point empty' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT geography 'Point empty' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT geography 'Point(1 1 1)' ~= tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point(1 1 1)' ~= tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point(1 1 1)' ~= tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point(1 1 1)' ~= tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT geography 'Point Z empty' ~= tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01';
SELECT geography 'Point Z empty' ~= tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}';
SELECT geography 'Point Z empty' ~= tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]';
SELECT geography 'Point Z empty' ~= tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestamptz '2000-01-01' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestamptz '2000-01-01' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestamptz '2000-01-01' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT stbox 'STBOX((1.0, 2.0), (1.0, 2.0))' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint 'Point(1.5 1.5)@2000-01-01';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]';
SELECT stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}';

SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint 'Point(1 1 1)@2000-01-01';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))' ~= tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= geometry 'Point(1 1)';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= geometry 'Point(1 1)';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= geometry 'Point(1 1)';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= geometry 'Point(1 1)';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= geometry 'Point empty';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= geometry 'Point empty';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= geometry 'Point empty';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= geometry 'Point empty';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= geometry 'Point(1 1 1)';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= geometry 'Point(1 1 1)';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= geometry 'Point(1 1 1)';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= geometry 'Point(1 1 1)';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= geometry 'Point Z empty';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= geometry 'Point Z empty';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= geometry 'Point Z empty';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= geometry 'Point Z empty';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= geography 'Point(1 1)';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= geography 'Point(1 1)';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= geography 'Point(1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= geography 'Point(1 1)';

SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= geography 'Point empty';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= geography 'Point empty';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= geography 'Point empty';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= geography 'Point empty';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' ~= geography 'Point(1 1 1)';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' ~= geography 'Point(1 1 1)';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' ~= geography 'Point(1 1 1)';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' ~= geography 'Point(1 1 1)';

SELECT tgeogpoint 'Point(1.5 1.5 1.5)@2000-01-01' ~= geography 'Point Z empty';
SELECT tgeogpoint '{Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03}' ~= geography 'Point Z empty';
SELECT tgeogpoint '[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03]' ~= geography 'Point Z empty';
SELECT tgeogpoint '{[Point(1.5 1.5 1.5)@2000-01-01, Point(2.5 2.5 2.5)@2000-01-02, Point(1.5 1.5 1.5)@2000-01-03],[Point(3.5 3.5 3.5)@2000-01-04, Point(3.5 3.5 3.5)@2000-01-05]}' ~= geography 'Point Z empty';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= timestamptz '2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= timestamptz '2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= timestamptz '2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= timestamptz '2000-01-01';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= timestamptz '2000-01-01';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= timestamptz '2000-01-01';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= timestampset '{2000-01-01, 2000-01-03}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= timestampset '{2000-01-01, 2000-01-03}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= period '[2000-01-01,2000-01-02]';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= period '[2000-01-01,2000-01-02]';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= stbox 'STBOX((1.0, 2.0), (1.0, 2.0))';
SELECT tgeogpoint 'Point(1.5 1.5)@2000-01-01' ~= stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03}' ~= stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03]' ~= stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1.5 1.5)@2000-01-01, Point(2.5 2.5)@2000-01-02, Point(1.5 1.5)@2000-01-03],[Point(3.5 3.5)@2000-01-04, Point(3.5 3.5)@2000-01-05]}' ~= stbox 'GEODSTBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= stbox 'STBOX Z((1.0, 2.0, 2.0), (1.0, 2.0, 2.0))';
SELECT tgeogpoint 'Point(1 1 1)@2000-01-01' ~= stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';
SELECT tgeogpoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= stbox 'GEODSTBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))';

SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}' ~= tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03],[Point(3 3)@2000-01-04, Point(3 3)@2000-01-05]}';

SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint 'Point(1 1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03]' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';
SELECT tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}' ~= tgeompoint '{[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02, Point(1 1 1)@2000-01-03],[Point(3 3 3)@2000-01-04, Point(3 3 3)@2000-01-05]}';

/* Errors */
SELECT geometry 'SRID=5676;Point(1 1)' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT geometry 'Point(1 1 1)' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= geometry 'SRID=5676;Point(1 1)';
SELECT tgeompoint 'Point(1 1)@2000-01-01' ~= geometry 'Point(1 1 1)';
SELECT tgeompoint 'SRID=5676;Point(1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';
SELECT tgeompoint 'Point(1 1 1)@2000-01-01' ~= tgeompoint 'Point(1 1)@2000-01-01';

-------------------------------------------------------------------------------
