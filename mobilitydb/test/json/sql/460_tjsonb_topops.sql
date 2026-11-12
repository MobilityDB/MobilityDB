-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' && tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' && tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' && tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' && tstzspan '[2001-01-01,2001-01-02]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' && tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' && tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' && tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' && tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' && tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' && tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' && tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' && tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' && tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' && tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' && tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' && tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' && tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' && tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' && tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' && tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' #@> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' #@> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' #@> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' #@> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #@> tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #@> tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #@> tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' #@> tstzspan '[2001-01-01,2001-01-02]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #@> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #@> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #@> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' #@> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #@> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #@> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #@> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' #@> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #@> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #@> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #@> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' #@> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #@> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' #@> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' #@> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' #@> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' <@# tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@# tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@# tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' <@# tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' <@# tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' <@# tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' <@# tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' <@# tstzspan '[2001-01-01,2001-01-02]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' <@# tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' <@# tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' <@# tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' <@# tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' <@# tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' <@# tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' <@# tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' <@# tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' <@# tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' <@# tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' <@# tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' <@# tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' <@# tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' <@# tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' <@# tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' <@# tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' ~= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' ~= tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' ~= tstzspan '[2001-01-01,2001-01-02]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' ~= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' ~= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' ~= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' ~= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' ~= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' ~= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' ~= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' ~= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' ~= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' ~= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' ~= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' ~= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' ~= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' ~= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' ~= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' ~= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tstzspan '[2001-01-01,2001-01-02]' -|- tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' -|- tstzspan '[2001-01-01,2001-01-02]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' -|- tstzspan '[2001-01-01,2001-01-02]';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' -|- tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' -|- tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' -|- tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' -|- tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' -|- tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' -|- tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' -|- tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' -|- tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' -|- tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' -|- tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' -|- tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' -|- tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' -|- tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(2 2)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03}' -|- tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03]' -|- tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}' -|- tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1)"}@2001-01-03],[{"geom": "Point(3 3)"}@2001-01-04, {"geom": "Point(3 3)"}@2001-01-05]}';

-------------------------------------------------------------------------------
-- Selectivity tests
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tjsonb WHERE temp && tstzspan '[2001-01-01,2001-01-02]';
SELECT COUNT(*) FROM tbl_tjsonb WHERE temp && tjsonb '[{"geom": "Point(1 1)"}@2001-06-01, {"geom": "Point(1 1)"}@2001-07-01]';

-------------------------------------------------------------------------------
