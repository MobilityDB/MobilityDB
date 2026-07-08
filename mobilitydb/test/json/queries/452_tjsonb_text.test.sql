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
-- documentation for any purtext, without fee, and without a written
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
-- AND FITNESS FOR A PARTICULAR PURtext. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext ' "  {  \"geom\":  \"Point(1 1)\"  }  "  @  2000-01-01';
SELECT ttext '  {  {  "geom":   "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '  [  {  "geom":  "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '  {  [  {  "geom":   "Point(1 1)"  }  @2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-- Normalization
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05, {"geom": "Point(2 2)"}@2000-01-06]}';

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03],[{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT ttextFromBinary(asBinary(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'));
SELECT ttextFromBinary(asBinary(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'));
SELECT ttextFromBinary(asBinary(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'));
SELECT ttextFromBinary(asBinary(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}'));

SELECT ttextFromBinary(asBinary(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'NDR'));
SELECT ttextFromBinary(asBinary(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'NDR'));
SELECT ttextFromBinary(asBinary(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'NDR'));
SELECT ttextFromBinary(asBinary(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'NDR'));

SELECT ttextFromBinary(asBinary(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'XDR'));
SELECT ttextFromBinary(asBinary(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'XDR'));
SELECT ttextFromBinary(asBinary(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'XDR'));
SELECT ttextFromBinary(asBinary(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'XDR'));

-------------------------------------------------------------------------------

SELECT ttextFromHexWKB(asHexWKB(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'));
SELECT ttextFromHexWKB(asHexWKB(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}'));

SELECT ttextFromHexWKB(asHexWKB(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'NDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'NDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'NDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'NDR'));

SELECT ttextFromHexWKB(asHexWKB(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'XDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'XDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 'XDR'));
SELECT ttextFromHexWKB(asHexWKB(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 'XDR'));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT ttext('{"geom": "Point(1 1)"}'::text, '2012-01-01'::timestamp);

SELECT ttextSeq(ARRAY['"{\"geom\": \"Point(1 1)\"}"@2012-01-01'::ttext, '"{\"geom\": \"Point(2 2)\"}"@2012-02-01'::ttext], 'discrete');

SELECT ttextSeq(ARRAY['"{\"geom\": \"Point(1 1)\"}"@2012-01-01'::ttext, '"{\"geom\": \"Point(2 2)\"}"@2012-02-01'::ttext], 'step', false, true);

SELECT ttextSeqSet(ARRAY[ttext '[{"geom": "Point(1 1)"}@2012-01-01, {"geom": "Point(1 1)"}@2012-02-01]', '[{"geom": "Point(2 2)"}@2012-03-01, {"geom": "Point(2 2)"}@2012-04-01]']);

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT ttextInst(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT setInterp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 'discrete');
SELECT ttextSeq(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT ttextSeqSet(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

SELECT ttextInst(ttext '{{"geom": "Point(1 1)"}@2000-01-01}');
SELECT setInterp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 'discrete');
SELECT ttextSeq(ttext '{{"geom": "Point(1 1)"}@2000-01-01}');
SELECT ttextSeqSet(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');

SELECT ttextInst(ttext '[{"geom": "Point(1 1)"}@2000-01-01]');
SELECT setInterp(ttext '[{"geom": "Point(1 1)"}@2000-01-01]', 'discrete');
SELECT ttextSeq(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT ttextSeqSet(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');

SELECT ttextInst(ttext '{[{"geom": "Point(1 1)"}@2000-01-01]}');
SELECT setInterp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01], [{"geom": "Point(2 2)"}@2000-01-04]}', 'discrete');
SELECT ttextSeq(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]}');
SELECT ttextSeqSet(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT appendInstant(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-02');
SELECT appendInstant(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-04');
SELECT appendInstant(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-04');
SELECT appendInstant(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }', ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-06');

SELECT appendSequence(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(2 2)"}@2000-01-02}', ttext '{{"geom": "Point(3 3)"}@2000-01-03}');
SELECT appendSequence(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02]', ttext '[{"geom": "Point(1 1)"}@2000-01-03]');

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'::text;
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'::text;
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'::text;
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }'::text;

SELECT (ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01'::text)::ttext;
SELECT (ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}'::text)::ttext;
SELECT (ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]'::text)::ttext;
SELECT (ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05] }'::text)::ttext;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT tempSubtype(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT tempSubtype(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT tempSubtype(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT memSize(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT memSize(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT memSize(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT memSize(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getValue(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

SELECT getValues(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT getValues(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT getValues(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT getValues(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT getTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT getTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT getTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timeSpan(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timeSpan(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timeSpan(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timeSpan(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT duration(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', true);
SELECT duration(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', true);
SELECT duration(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', true);
SELECT duration(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', true);

SELECT duration(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT duration(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT duration(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT duration(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT getTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT shiftTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '5 min');
SELECT shiftTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '5 min');
SELECT shiftTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '5 min');
SELECT shiftTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '5 min');

SELECT scaleTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '1 day');
SELECT scaleTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '1 day');
SELECT scaleTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '1 day');
SELECT scaleTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '1 day');

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' ?= text '{"geom": "Point(1 1)"}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' ?= text '{"geom": "Point(1 1)"}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' ?= text '{"geom": "Point(1 1)"}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' ?= text '{"geom": "Point(1 1)"}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' %= text '{"geom": "Point(1 1)"}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' %= text '{"geom": "Point(1 1)"}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' %= text '{"geom": "Point(1 1)"}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' %= text '{"geom": "Point(1 1)"}';

SELECT shiftTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '1 year'::interval);
SELECT shiftTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '1 year'::interval);
SELECT shiftTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '1 year'::interval);
SELECT shiftTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '1 year'::interval);

SELECT startValue(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startValue(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startValue(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startValue(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endValue(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endValue(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endValue(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endValue(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT valueN(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT valueN(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT valueN(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT valueN(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT numInstants(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT numInstants(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT numInstants(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT numInstants(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startInstant(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startInstant(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startInstant(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startInstant(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endInstant(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endInstant(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endInstant(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endInstant(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT instantN(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT instantN(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT instantN(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT instantN(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT instants(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT instants(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT instants(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT instants(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT numTimestamps(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT numTimestamps(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT numTimestamps(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT numTimestamps(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startTimestamp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startTimestamp(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startTimestamp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endTimestamp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endTimestamp(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endTimestamp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timestampN(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT timestampN(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT timestampN(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT timestampN(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT timestamps(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timestamps(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timestamps(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timestamps(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT numSequences(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT startSequence(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT endSequence(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');
SELECT sequenceN(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);
SELECT sequences(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT startTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT startTimestamp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT startTimestamp(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT startTimestamp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT endTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT endTimestamp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT endTimestamp(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT endTimestamp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

SELECT timestampN(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', 1);
SELECT timestampN(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', 1);
SELECT timestampN(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', 1);
SELECT timestampN(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', 1);

SELECT timestamps(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01');
SELECT timestamps(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}');
SELECT timestamps(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]');
SELECT timestamps(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

SELECT atValues(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', text '{"geom": "Point(1 1)"}');
SELECT atValues(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', text '{"geom": "Point(1 1)"}');
SELECT atValues(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', text '{"geom": "Point(1 1)"}');
SELECT atValues(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', text '{"geom": "Point(1 1)"}');

SELECT minusValues(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', text '{"geom": "Point(1 1)"}');
SELECT minusValues(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', text '{"geom": "Point(1 1)"}');
SELECT minusValues(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', text '{"geom": "Point(1 1)"}');
SELECT minusValues(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', text '{"geom": "Point(1 1)"}');

SELECT atValues(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT atValues(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', textset '{"{\"geom\": \"Point(1 1)\"}"}');

SELECT minusValues(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', textset '{"{\"geom\": \"Point(1 1)\"}"}');
SELECT minusValues(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', textset '{"{\"geom\": \"Point(1 1)\"}"}');

SELECT atTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT atTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT atTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT atTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT valueAtTimestamp(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', '2000-01-01');
SELECT valueAtTimestamp(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', '2000-01-01');
SELECT valueAtTimestamp(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', '2000-01-01');
SELECT valueAtTimestamp(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', '2000-01-01');

SELECT minusTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT minusTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT atTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT atTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT atTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT atTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT minusTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT minusTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT minusTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT minusTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT atTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT atTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT minusTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT minusTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT atTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT atTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

SELECT minusTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT minusTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT deleteTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', timestamptz '2000-01-01');
SELECT deleteTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', timestamptz '2000-01-01');
SELECT deleteTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', timestamptz '2000-01-01');
SELECT deleteTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', timestamptz '2000-01-01');

SELECT deleteTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzset '{2000-01-01}');
SELECT deleteTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzset '{2000-01-01}');
SELECT deleteTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzset '{2000-01-01}');
SELECT deleteTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzset '{2000-01-01}');

SELECT deleteTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]');
SELECT deleteTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]');

SELECT deleteTime(ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}');
SELECT deleteTime(ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}');

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' = ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' = ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' = ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' = ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' != ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' != ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' != ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' != ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' < ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' < ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' < ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' < ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' <= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' <= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' <= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' <= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' > ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' > ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' > ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' > ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01' >= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}' >= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]' >= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= ttext '"{\"geom\": \"Point(1 1)\"}"@2000-01-01';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= ttext '{{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03}';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= ttext '[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03]';
SELECT ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}' >= ttext '{[{"geom": "Point(1 1)"}@2000-01-01, {"geom": "Point(1 1)"}@2000-01-02, {"geom": "Point(1 1)"}@2000-01-03], [{"geom": "Point(2 2)"}@2000-01-04, {"geom": "Point(2 2)"}@2000-01-05]}';

-------------------------------------------------------------------------------/
