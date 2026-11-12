-------------------------------------------------------------------------------

SELECT round(stbox(pose 'Pose(Point(1 1),0.5)'), 6);
SELECT round(stbox(pose 'Pose(Point(1 1),0.5)', timestamptz '2001-01-01'), 6);
SELECT round(stbox(pose 'Pose(Point(1 1),0.5)', tstzspan '[2001-01-01, 2001-01-02]'), 6);

SELECT round(stbox(tpose 'Pose(Point(1 1),0.5)@2001-01-01'), 6);
SELECT round(stbox(tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}'), 6);
SELECT round(stbox(tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]'), 6);
SELECT round(stbox(tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}'), 6);

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' && tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && tstzspan '[2001-01-01,2001-01-02]';

SELECT tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' && tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' && tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' && tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' && tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' && tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' && tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' && tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' @> tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' @> tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> tstzspan '[2001-01-01,2001-01-02]';

SELECT tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' @> tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' @> tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' @> tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' @> tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' @> tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' @> tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' @> tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@ tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ tstzspan '[2001-01-01,2001-01-02]';

SELECT tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' <@ tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' <@ tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' <@ tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' <@ tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' <@ tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' <@ tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' <@ tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= tstzspan '[2001-01-01,2001-01-02]';

SELECT tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' ~= tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' ~= tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' ~= tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' ~= tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' ~= tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' ~= tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' ~= tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- tstzspan '[2001-01-01,2001-01-02]';

SELECT tpose 'SRID=5676;Pose(Point(1 1),0.5)@2001-01-01' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tpose 'SRID=5676;{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' -|- tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- tpose 'Pose(Point(1 1),0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' -|- tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' -|- tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1),0.5)@2001-01-01' -|- tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{Pose(Point(1 1),0.5)@2001-01-01, Pose(Point(2 2),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03}' -|- tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03]' -|- tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';
SELECT tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}' -|- tpose '{[Pose(Point(1 1),0.4)@2001-01-01, Pose(Point(1 1),0.5)@2001-01-02, Pose(Point(1 1),0.7)@2001-01-03],[Pose(Point(3 3),0.5)@2001-01-04, Pose(Point(3 3),0.5)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Selectivity tests
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tpose2d WHERE temp && stbox 'SRID=3812;STBOX X((0,0),(50,50))';
SELECT COUNT(*) FROM tbl_tpose2d WHERE temp && tpose 'SRID=3812;[Pose(Point(1 1),0.1)@2001-06-01, Pose(Point(1 1),0.9)@2001-07-01]';

-------------------------------------------------------------------------------
