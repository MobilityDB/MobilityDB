-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT geometry 'SRID=5676;Point(1 1)' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point(1 1)' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point(1 1)' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point(1 1)' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT geometry 'SRID=5676;Point empty' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT geometry 'SRID=5676;Point empty' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT geometry 'SRID=5676;Point empty' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT geometry 'SRID=5676;Point empty' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT npoint 'NPoint(1,0.5)' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT npoint 'NPoint(1,0.5)' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT npoint 'NPoint(1,0.5)' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT npoint 'NPoint(1,0.5)' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> geometry 'SRID=5676;Point(1 1)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> geometry 'SRID=5676;Point(1 1)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> geometry 'SRID=5676;Point empty';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> geometry 'SRID=5676;Point empty';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> geometry 'SRID=5676;Point empty';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> npoint 'NPoint(1,0.5)';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> npoint 'NPoint(1,0.5)';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> npoint 'NPoint(1,0.5)';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestamptz '2000-01-01' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestamptz '2000-01-01' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestamptz '2000-01-01' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# timestamptz '2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# timestamptz '2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# timestamptz '2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# timestamptz '2000-01-01';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# timestampset '{2000-01-01, 2000-01-03}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# period '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestamptz '2000-01-01' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestamptz '2000-01-01' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestamptz '2000-01-01' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> timestamptz '2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> timestamptz '2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> timestamptz '2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> timestamptz '2000-01-01';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> timestampset '{2000-01-01, 2000-01-03}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> period '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestamptz '2000-01-01' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestamptz '2000-01-01' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestamptz '2000-01-01' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# timestamptz '2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# timestamptz '2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# timestamptz '2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# timestamptz '2000-01-01';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# timestampset '{2000-01-01, 2000-01-03}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# period '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestamptz '2000-01-01' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestamptz '2000-01-01' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestamptz '2000-01-01' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT timestampset '{2000-01-01, 2000-01-03}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT timestampset '{2000-01-01, 2000-01-03}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT timestampset '{2000-01-01, 2000-01-03}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT timestampset '{2000-01-01, 2000-01-03}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT period '[2000-01-01,2000-01-02]' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT period '[2000-01-01,2000-01-02]' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT period '[2000-01-01,2000-01-02]' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT period '[2000-01-01,2000-01-02]' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> timestamptz '2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> timestamptz '2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> timestamptz '2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> timestamptz '2000-01-01';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> timestampset '{2000-01-01, 2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> timestampset '{2000-01-01, 2000-01-03}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> period '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> period '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> periodset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> stbox 'SRID=5676;STBOX T((1.0, 1.0,2000-01-01), (2.0, 2.0,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------
-- NULL

SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T((,2000-01-01), (,2000-01-02))' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';

SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX((1.0, 1.0), (2.0, 2.0))' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';


SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << stbox 'STBOX T((,2000-01-01), (,2000-01-02))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< stbox 'STBOX T((,2000-01-01), (,2000-01-02))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >>  stbox 'STBOX T((,2000-01-01), (,2000-01-02))' ;
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &>  stbox 'STBOX T((,2000-01-01), (,2000-01-02))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| stbox 'STBOX T((,2000-01-01), (,2000-01-02))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| stbox 'STBOX T((,2000-01-01), (,2000-01-02))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01'|>>  stbox 'STBOX T((,2000-01-01), (,2000-01-02))' ;
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&>  stbox 'STBOX T((,2000-01-01), (,2000-01-02))';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# stbox 'STBOX((1.0, 1.0), (2.0, 2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# stbox 'STBOX((1.0, 1.0), (2.0, 2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> stbox 'STBOX((1.0, 1.0), (2.0, 2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> stbox 'STBOX((1.0, 1.0), (2.0, 2.0))';

-------------------------------------------------------------------------------

