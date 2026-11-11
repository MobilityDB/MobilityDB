-------------------------------------------------------------------------------

SELECT round(stbox(npoint 'NPoint(1,0.5)'), 6);
SELECT round(stbox(nsegment 'NSegment(1,0.5,0.7)'), 6);
SELECT round(stbox(npoint 'NPoint(1,0.5)', timestamptz '2001-01-01'), 6);
SELECT round(stbox(npoint 'NPoint(1,0.5)', tstzspan '[2001-01-01, 2001-01-02]'), 6);

SELECT round(stbox(tnpoint 'NPoint(1,0.5)@2001-01-01'), 6);
SELECT round(stbox(tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}'), 6);
SELECT round(stbox(tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]'), 6);
SELECT round(stbox(tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}'), 6);

-------------------------------------------------------------------------------
/* Errors */
SELECT stbox 'STBOX X((1.0,2.0),(1.0,2.0))' && tnpoint 'NPoint(1,0.5)@2001-01-01';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && tstzspan '[2001-01-01,2001-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' && tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> tstzspan '[2001-01-01,2001-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' @> tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ tstzspan '[2001-01-01,2001-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' <@ tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= tstzspan '[2001-01-01,2001-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' ~= tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- tstzspan '[2001-01-01,2001-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- tnpoint 'NPoint(1,0.5)@2001-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2001-01-01' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2001-01-01, NPoint(2,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03}' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03]' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}' -|- tnpoint '{[NPoint(1,0.4)@2001-01-01, NPoint(1,0.5)@2001-01-02, NPoint(1,0.7)@2001-01-03],[Npoint(3,0.5)@2001-01-04, NPoint(3,0.5)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Selectivity tests
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tnpoint WHERE temp && stbox 'SRID=5676;STBOX X((0,0),(50,50))';
SELECT COUNT(*) FROM tbl_tnpoint WHERE temp && tnpoint '[Npoint(1,0.1)@2001-06-01, Npoint(1,0.9)@2001-07-01]';

-------------------------------------------------------------------------------
