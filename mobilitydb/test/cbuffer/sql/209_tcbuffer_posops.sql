-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' << tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' << tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' << tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' << tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' << tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' << tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' << tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' << tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' << tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' << tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' << tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' << tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' << tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' << tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' << tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' << tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

/* Errors */
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << stbox 'SRID=4326;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' >> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' >> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' >> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' >> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' >> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' >> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' >> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' >> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' >> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' >> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' >> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' >> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' >> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' >> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' >> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' >> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' >> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' >> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' >> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' >> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &< tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &< tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &< tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &< tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &< tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &< tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &< tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &< tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &< tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &< tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &< tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &< tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &< tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &< tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &< tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &< tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &< tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &< tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &< tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &< tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<| tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<| tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<| tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<| tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |>> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |>> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |>> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |>> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<| tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<| tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<| tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<| tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<| tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<| tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<| tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |&> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |&> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |&> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' |&> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' |&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' |&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' |&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<# tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<# tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<# tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' <<# tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #>> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #>> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #>> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #>> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #>> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' &<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<# tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<# tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<# tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' &<# tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' &<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' &<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' &<# tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #&> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #&> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #&> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])' #&> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' #&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' #&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' #&> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------
/* Errors */

SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' << tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' &< tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' >> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' &> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' <<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' &<| tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' |>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX T([2001-01-01,2001-01-02])' |&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';

SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' <<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &<# tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #>> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #&> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' << stbox 'STBOX T([2001-01-01,2001-01-02])';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &< stbox 'STBOX T([2001-01-01,2001-01-02])';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' >>  stbox 'STBOX T([2001-01-01,2001-01-02])' ;
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &>  stbox 'STBOX T([2001-01-01,2001-01-02])';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<| stbox 'STBOX T([2001-01-01,2001-01-02])';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<| stbox 'STBOX T([2001-01-01,2001-01-02])';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01'|>>  stbox 'STBOX T([2001-01-01,2001-01-02])' ;
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' |&>  stbox 'STBOX T([2001-01-01,2001-01-02])';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' &<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #>> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' #&> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------

