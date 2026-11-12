-------------------------------------------------------------------------------

SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)'), 6);
SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)', timestamptz '2001-01-01'), 6);
SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)', tstzspan '[2001-01-01, 2001-01-02]'), 6);

SELECT round(stbox(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01'), 6);
SELECT round(stbox(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}'), 6);
SELECT round(stbox(tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]'), 6);
SELECT round(stbox(tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}'), 6);

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- tstzspan '[2001-01-01,2001-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2001-01-01' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03}' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03]' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2001-01-01, Cbuffer(Point(1 1),0.5)@2001-01-02, Cbuffer(Point(1 1),0.7)@2001-01-03],[Cbuffer(Point(3 3),0.5)@2001-01-04, Cbuffer(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Selectivity tests
-------------------------------------------------------------------------------

  SELECT COUNT(*) FROM tbl_tcbuffer WHERE temp && stbox 'SRID=3812;STBOX X((0,0),(50,50))';
SELECT COUNT(*) FROM tbl_tcbuffer WHERE temp && tcbuffer 'SRID=3812;[Cbuffer(Point(1 1),0.1)@2001-06-01, Cbuffer(Point(1 1),0.9)@2001-07-01]';

-------------------------------------------------------------------------------
