-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purjsonb, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURjsonb. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb ' "  {  \"geom\":  \"Point(1 1)\"  }  "  @  2000-01-01';
SELECT tjsonb '  {  {  "geom":   "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '  [  {  "geom":  "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '  {  [  {  "geom":   "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-- Normalization
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05, {"geom": "Point(2 2)"}@2000-01-06]}';

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT tjsonbFromBinary(asBinary(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'));
SELECT tjsonbFromBinary(asBinary(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'));
SELECT tjsonbFromBinary(asBinary(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'));
SELECT tjsonbFromBinary(asBinary(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}'));

SELECT tjsonbFromBinary(asBinary(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'NDR'));
SELECT tjsonbFromBinary(asBinary(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'NDR'));
SELECT tjsonbFromBinary(asBinary(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'NDR'));
SELECT tjsonbFromBinary(asBinary(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'NDR'));

-- SELECT tjsonbFromBinary(asBinary(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'XDR'));
-- SELECT tjsonbFromBinary(asBinary(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'XDR'));
-- SELECT tjsonbFromBinary(asBinary(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'XDR'));
-- SELECT tjsonbFromBinary(asBinary(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'XDR'));

-------------------------------------------------------------------------------

SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}'));

SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'NDR'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'NDR'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'NDR'));
SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'NDR'));

-- SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'XDR'));
-- SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'XDR'));
-- SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'XDR'));
-- SELECT tjsonbFromHexWKB(asHexWKB(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'XDR'));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT tjsonb('{"geom": "Point(1 1)"}'::jsonb, '2012-01-01'::timestamp);

SELECT tjsonbSeq(ARRAY['"{\"geom\": \"Point(1 1)\"}"@2012-01-01'::tjsonb, '"{\"geom\": \"Point(2 2)\"}"@2012-02-01'::tjsonb], 'discrete');

SELECT tjsonbSeq(ARRAY['"{\"geom\": \"Point(1 1)\"}"@2012-01-01'::tjsonb, '"{\"geom\": \"Point(2 2)\"}"@2012-02-01'::tjsonb], 'step', false, true);

SELECT tjsonbSeqSet(ARRAY[tjsonb '[{"geom": "Point(1 1)"}@2012-01-01, {"geom": "Point(1 1)"}@2012-02-01]', '[{"geom": "Point(2 2)"}@2012-03-01, {"geom": "Point(2 2)"}@2012-04-01]']);

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT tjsonbInst(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT setInterp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'discrete');
SELECT tjsonbSeq(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT tjsonbSeqSet(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

SELECT tjsonbInst(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01}');
SELECT setInterp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'discrete');
SELECT tjsonbSeq(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01}');
SELECT tjsonbSeqSet(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');

SELECT tjsonbInst(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01]');
SELECT setInterp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01]', 'discrete');
SELECT tjsonbSeq(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT tjsonbSeqSet(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');

SELECT tjsonbInst(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01]}');
SELECT setInterp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01], [{"geom": "Point(2 2)"}@2000-01-04]}', 'discrete');
SELECT tjsonbSeq(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]}');
SELECT tjsonbSeqSet(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT appendInstant(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-02');
SELECT appendInstant(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-04');
SELECT appendInstant(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-04');
SELECT appendInstant(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }', tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-06');

SELECT appendSequence(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02}', tjsonb '{{"geom": "Point(3 3)"}@2000-01-03}');
SELECT appendSequence(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02]', tjsonb '[{"geom": "Point(1 1)"}@2000-01-03]');

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'::text;
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'::text;
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'::text;
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }'::text;

SELECT (tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'::text)::tjsonb;
SELECT (tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'::text)::tjsonb;
SELECT (tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'::text)::tjsonb;
SELECT (tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }'::text)::tjsonb;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT tempSubtype(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT tempSubtype(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT tempSubtype(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT memSize(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT memSize(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT memSize(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT memSize(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getValue(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

SELECT getValues(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT getValues(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT getValues(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT getValues(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT getTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT getTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT getTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timeSpan(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timeSpan(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timeSpan(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timeSpan(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT duration(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', true);
SELECT duration(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', true);
SELECT duration(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', true);
SELECT duration(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', true);

SELECT duration(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT duration(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT duration(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT duration(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT shiftTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '5 min');
SELECT shiftTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '5 min');
SELECT shiftTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '5 min');
SELECT shiftTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '5 min');

SELECT scaleTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '1 day');
SELECT scaleTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '1 day');
SELECT scaleTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '1 day');
SELECT scaleTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '1 day');

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' ?= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' ?= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' ?= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' ?= jsonb '{"geom": "Point(1 1)"}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' %= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' %= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' %= jsonb '{"geom": "Point(1 1)"}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' %= jsonb '{"geom": "Point(1 1)"}';

SELECT shiftTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '1 year'::interval);
SELECT shiftTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '1 year'::interval);
SELECT shiftTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '1 year'::interval);
SELECT shiftTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '1 year'::interval);

SELECT startValue(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startValue(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startValue(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startValue(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endValue(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endValue(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endValue(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endValue(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT valueN(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT valueN(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT valueN(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT valueN(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT numInstants(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT numInstants(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT numInstants(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT numInstants(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startInstant(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startInstant(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startInstant(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startInstant(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endInstant(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endInstant(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endInstant(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endInstant(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT instantN(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT instantN(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT instantN(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT instantN(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT instants(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT instants(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT instants(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT instants(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT numTimestamps(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT numTimestamps(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT numTimestamps(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT numTimestamps(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startTimestamp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startTimestamp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startTimestamp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endTimestamp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endTimestamp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endTimestamp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timestampN(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT timestampN(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT timestampN(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT timestampN(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT timestamps(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timestamps(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timestamps(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timestamps(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT numSequences(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT startSequence(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT endSequence(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT sequenceN(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);
SELECT sequences(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startTimestamp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startTimestamp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startTimestamp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endTimestamp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endTimestamp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endTimestamp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timestampN(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT timestampN(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT timestampN(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT timestampN(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT timestamps(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timestamps(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timestamps(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timestamps(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

SELECT atValues(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', jsonb '{"geom": "Point(1 1)"}');
SELECT atValues(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', jsonb '{"geom": "Point(1 1)"}');
SELECT atValues(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', jsonb '{"geom": "Point(1 1)"}');
SELECT atValues(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', jsonb '{"geom": "Point(1 1)"}');

SELECT minusValues(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', jsonb '{"geom": "Point(1 1)"}');
SELECT minusValues(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', jsonb '{"geom": "Point(1 1)"}');
SELECT minusValues(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', jsonb '{"geom": "Point(1 1)"}');
SELECT minusValues(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', jsonb '{"geom": "Point(1 1)"}');

SELECT atValues(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');

SELECT minusValues(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', jsonbset '{"{\"geom\": \"Point(1 1)\"}"}');

SELECT atTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT atTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT atTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT atTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT valueAtTimestamp(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '2000-01-01');
SELECT valueAtTimestamp(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '2000-01-01');
SELECT valueAtTimestamp(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '2000-01-01');
SELECT valueAtTimestamp(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '2000-01-01');

SELECT minusTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT minusTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT atTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT atTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT atTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT atTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT minusTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT minusTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT minusTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT minusTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT atTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT minusTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT atTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

SELECT minusTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT deleteTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT deleteTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT deleteTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT deleteTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT deleteTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT deleteTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT deleteTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT deleteTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT deleteTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT deleteTime(tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= tjsonb '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= tjsonb '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= tjsonb '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= tjsonb '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------
