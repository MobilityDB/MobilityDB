-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #= jsonb '{"geom": "Point(1 1)"}';

SELECT jsonb '{"geom": "Point(1 1)"}' #= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT jsonb '{"geom": "Point(1 1)"}' #= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT jsonb '{"geom": "Point(1 1)"}' #= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT jsonb '{"geom": "Point(1 1)"}' #= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #= tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #= tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #= tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #<> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #<> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #<> jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #<> jsonb '{"geom": "Point(1 1)"}';

SELECT jsonb '{"geom": "Point(1 1)"}' #<> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT jsonb '{"geom": "Point(1 1)"}' #<> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT jsonb '{"geom": "Point(1 1)"}' #<> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT jsonb '{"geom": "Point(1 1)"}' #<> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #<> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #<> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #<> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01' #<> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #<> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #<> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #<> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}' #<> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #<> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #<> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #<> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]' #<> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #<> tjsonb '"{\"geom\": \"Point(1 1)\"}"@2001-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #<> tjsonb '{{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #<> tjsonb '[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}' #<> tjsonb '{[{"geom": "Point(1 1)"}@2001-01-01, {"geom": "Point(1 1)"}@2001-01-02, {"geom": "Point(1 1"}@2001-01-03], [{"geom": "Point(2 2)"}@2001-01-04, {"geom": "Point(2 2)"}@2001-01-05]}';

-------------------------------------------------------------------------------/
