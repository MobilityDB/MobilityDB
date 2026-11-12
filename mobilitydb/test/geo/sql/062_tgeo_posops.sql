-------------------------------------------------------------------------------

/* Errors*/
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' << tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' &< tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' >> tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' &> tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' <<| tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' &<| tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' |>> tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' |&> tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' <</ tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' &</ tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' />> tstzspan '[2001-01-01,2001-01-02]'::stbox;
SELECT stbox 'STBOX ZT(((1.0,1.0,1.0),(2.0,2.0,2.0)),[2002-01-02,2002-01-02])' /&> tstzspan '[2001-01-01,2001-01-02]'::stbox;

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' << tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' << tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' << tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' << tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' << tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' << tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' << tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' << tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' << tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' << tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' << tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' << tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' << tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' << tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' << tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' << tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' << tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' << tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' << tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' << tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' << tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' << tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' << tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' << tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' << tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' << tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' << stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' << tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' >> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' >> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' >> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' >> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' >> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' >> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' >> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' >> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' >> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' >> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' >> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' >> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' >> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' >> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' >> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' >> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' >> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' >> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' >> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' >> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' >> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' >> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' >> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' >> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' >> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' >> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' >> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' >> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' &< tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &< tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &< tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &< tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' &< tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &< tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &< tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &< tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' &< tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &< tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &< tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &< tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' &< tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &< tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &< tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &< tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &< tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &< tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &< tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &< tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &< tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &< tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &< tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &< tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &< tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' &< tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &< stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' &< tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' &> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' &> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' &> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' &> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' &> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' &> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' <<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' <<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<| stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' <<| tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' |>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' |>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' |>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' |>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' |>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |>> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' |>> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' &<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<| tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<| tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<| tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<| tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<| tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<| tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<| tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' &<| tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<| stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' &<| tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tgeometry 'Point(1 1)@2001-01-01' |&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' |&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' |&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' |&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' |&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' |&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' |&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' |&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' |&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' |&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

-- Mixed 2D/3D
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> tgeometry 'Point(1 1)@2001-01-01';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' |&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' |&> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' |&> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' <</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' <</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' <</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' <</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' <</ tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <</ tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' />> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' />> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' />> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' />> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' />> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' />> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' />> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' />> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' />> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' />> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' />> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' />> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' />> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' />> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' />> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' />> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' />> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' />> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' &</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' &</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' &</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &</ stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &</ tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &</ tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &</ tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &</ tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' &</ tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &</ tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' /&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' /&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))' /&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' /&> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' /&> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' /&> stbox 'STBOX Z((1.0,1.0,1.0),(2.0,2.0,2.0))';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' /&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' /&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' /&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' /&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' /&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' /&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' /&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' /&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' /&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' /&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> stbox 'SRID=5676;STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tgeometry 'SRID=5676;Point(1 1)@2001-01-01' /&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' /&> tgeometry 'Point(1 1)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography 'Point(1.5 1.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';

SELECT tgeometry 'Point(1 1)@2001-01-01' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1.5 1.5)@2001-01-01' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' <<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1 1 1)@2001-01-01' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1)@2001-01-01' <<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' <<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' <<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' <<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' <<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' <<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' <<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' <<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography 'Point(1.5 1.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';

SELECT tgeometry 'Point(1 1)@2001-01-01' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1.5 1.5)@2001-01-01' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #>> tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1 1 1)@2001-01-01' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1)@2001-01-01' #>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #>> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' #>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #>> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' #>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #>> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' #>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #>> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #>> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #>> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #>> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography 'Point(1.5 1.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';

SELECT tgeometry 'Point(1 1)@2001-01-01' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1.5 1.5)@2001-01-01' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' &<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1 1 1)@2001-01-01' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1)@2001-01-01' &<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<# tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<# tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<# tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' &<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' &<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' &<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' &<# tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' &<# tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &<# tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' &<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography 'Point(1.5 1.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}';

SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography 'Point(1 1 1)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> stbox 'STBOX XT(((1.0,1.0),(2.0,2.0)),[2002-01-01,2002-01-02])';

SELECT tgeometry 'Point(1 1)@2001-01-01' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1.5 1.5)@2001-01-01' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03]' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1.5 1.5)@2001-01-01, Point(2.5 2.5)@2001-01-02, Point(1.5 1.5)@2001-01-03],[Point(3.5 3.5)@2001-01-04, Point(3.5 3.5)@2001-01-05]}' #&> tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography 'Point(1 1 1)@2001-01-01' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tgeography '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tstzspan '[2001-01-01,2001-01-02]';

SELECT tgeometry 'Point(1 1)@2001-01-01' #&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #&> tgeometry 'Point(1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1)@2001-01-01' #&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #&> tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1)@2001-01-01' #&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #&> tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1)@2001-01-01' #&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03}' #&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]' #&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}' #&> tgeometry '{[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02, Point(1 1)@2001-01-03],[Point(3 3)@2001-01-04, Point(3 3)@2001-01-05]}';

SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03}' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03]' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';
SELECT tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}' #&> tgeometry '{[Point(1 1 1)@2001-01-01, Point(2 2 2)@2001-01-02, Point(1 1 1)@2001-01-03],[Point(3 3 3)@2001-01-04, Point(3 3 3)@2001-01-05]}';

/* Errors */
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #&> tgeometry 'Point(1 1 1)@2001-01-01';
SELECT tgeometry 'Point(1 1 1)@2001-01-01' #&> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------
