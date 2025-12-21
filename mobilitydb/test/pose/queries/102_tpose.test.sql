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
-- documentation for any purpose, without fee, and without a written
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
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input
-------------------------------------------------------------------------------

SELECT asText(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT asText(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT asText(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }');

SELECT asText(tpose '  Pose (  Point(1 1)  ,   0.5  )  @  2000-01-01  ');
SELECT asText(tpose '  {  Pose( Point(1 1) , 0.3 ) @ 2000-01-01  , Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5) @  2000-01-03   }   ');
SELECT asText(tpose '  [  Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tpose '  {  [  Pose(Point(1 1), 0.2)@2000-01-01 ,  Pose  (  Point(1 1) , 0.4 ) @2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

-- Normalization
SELECT asText(tpose '[Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tpose '{[Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05, Pose(Point(2 2), 0.6)@2000-01-06]}');

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT asText(tpose 'Pose(Point(1 1), 0.123456789)@2000-01-01', 6);
SELECT asText(tpose '{Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03}', 6);
SELECT asText(tpose '[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(tpose '{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);
SELECT asText(tpose 'Interp=Step;[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(tpose 'Interp=Step;{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }')));

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }', 'NDR')));

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }', 'XDR')));

-------------------------------------------------------------------------------

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }')));

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }', 'NDR')));

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }', 'XDR')));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(tpose('Pose(Point(1 1),0)'::pose, '2012-01-01'::timestamp));

SELECT asText(tposeSeq(ARRAY['Pose(Point(1 1),0)@2012-01-01'::tpose, 'Pose(Point(2 2),1)@2012-02-01'::tpose], 'discrete'));

SELECT asText(tposeSeq(ARRAY['Pose(Point(1 1),0)@2012-01-01'::tpose, 'Pose(Point(1 1),1)@2012-02-01'::tpose], 'linear', true, false));

SELECT asText(tposeSeqSet(ARRAY[tpose '[Pose(Point(1 1),0)@2012-01-01, Pose(Point(1 1),1)@2012-02-01]', '[Pose(Point(2 2),0)@2012-03-01, Pose(Point(2 2),1)@2012-04-01]']));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asText(tposeInst(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(setInterp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 'discrete'));
SELECT asText(tposeSeq(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(tposeSeqSet(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(tposeInst(tpose '{Pose(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(setInterp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'discrete'));
SELECT asText(tposeSeq(tpose '{Pose(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(tposeSeqSet(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));

SELECT asText(tposeInst(tpose '[Pose(Point(1 1), 0.3)@2000-01-01]'));
SELECT asText(setInterp(tpose '[Pose(Point(1 1), 0.3)@2000-01-01]', 'discrete'));
SELECT asText(tposeSeq(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(tposeSeqSet(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));

SELECT asText(tposeInst(tpose '{[Pose(Point(1 1), 0.3)@2000-01-01]}'));
SELECT asText(setInterp(tpose '{[Pose(Point(1 1), 0.3)@2000-01-01], [Pose(Point(2 2), 0.6)@2000-01-04]}', 'discrete'));
SELECT asText(tposeSeq(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]}'));
SELECT asText(tposeSeqSet(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(setInterp(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'linear'));
SELECT asText(setInterp(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'linear'));

SELECT asText(round(tpose '{[Pose(Point(1 1), 0.123456789)@2012-01-01, Pose(Point(1 1), 0.5)@2012-01-02)}', 6));

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT asText(appendInstant(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tpose 'Pose(Point(1 1), 0.7)@2000-01-02'));
SELECT asText(appendInstant(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tpose 'Pose(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tpose 'Pose(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }', tpose 'Pose(Point(2 2), 0.7)@2000-01-06'));

-------------------------------------------------------------------------------

SELECT asText(appendSequence(tpose '{Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(2 2), 0.4)@2000-01-02}', tpose '{Pose(Point(3 3), 0.6)@2000-01-03}'));
SELECT asText(appendSequence(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02]', tpose '[Pose(Point(1 1), 0.6)@2000-01-03]'));

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT asText(round(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'::tgeompoint, 6));
SELECT asText(round(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::tgeompoint, 6));
SELECT asText(round(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::tgeompoint, 6));
SELECT asText(round(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }'::tgeompoint, 6));

SELECT round(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'::tfloat, 6);
SELECT round(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::tfloat, 6);
SELECT round(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::tfloat, 6);
SELECT round(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }'::tfloat, 6);

-- SELECT asText(round((tpose 'Pose(Point(1 1), 0.5)@2000-01-01'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05] }'::tgeompoint)::tpose, 6));
-- NULL
-- SELECT asText(tgeompoint 'SRID=5676;Point(-1 -1)@2000-01-01'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;{POINT(48.7186629128278 77.7640705101509)@2000-01-01, POINT(48.71 77.76)@2000-01-02}'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;[POINT(48.7186629128278 77.7640705101509)@2000-01-01, POINT(48.71 77.76)@2000-01-02]'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;{[POINT(62.7866330839742 80.1435561997142)@2000-01-01, POINT(62.7866330839742 80.1435561997142)@2000-01-02],[POINT(48.7186629128278 77.7640705101509)@2000-01-03, POINT(48.71 77.76)@2000-01-04]}'::tpose;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT tempSubtype(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT tempSubtype(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT tempSubtype(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT memSize(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT memSize(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT memSize(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT memSize(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(getValue(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(getValues(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(getValues(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(getValues(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(getValues(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT getTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT getTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT getTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT getTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timeSpan(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timeSpan(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timeSpan(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timeSpan(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT duration(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', true);
SELECT duration(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', true);
SELECT duration(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', true);
SELECT duration(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', true);

SELECT duration(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT duration(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT duration(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT duration(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT getTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT asText(shiftTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', '5 min'));
SELECT asText(shiftTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '5 min'));
SELECT asText(shiftTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '5 min'));
SELECT asText(shiftTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '5 min'));

SELECT asText(scaleTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', '1 day'));
SELECT asText(scaleTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '1 day'));
SELECT asText(scaleTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '1 day'));
SELECT asText(scaleTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' ?= pose 'Pose(Point(1 1), 0.5)';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' %= pose 'Pose(Point(1 1), 0.5)';

SELECT asText(shiftTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', '1 year'::interval));
SELECT asText(shiftTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '1 year'::interval));
SELECT asText(shiftTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '1 year'::interval));
SELECT asText(shiftTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '1 year'::interval));

SELECT asText(startValue(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(startValue(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(startValue(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(startValue(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(endValue(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(endValue(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(endValue(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(endValue(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(valueN(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 1));
SELECT asText(valueN(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT asText(valueN(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT asText(valueN(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT numInstants(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT numInstants(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT numInstants(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT numInstants(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(startInstant(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(startInstant(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(startInstant(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(startInstant(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(endInstant(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(endInstant(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(endInstant(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(endInstant(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(instantN(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 1));
SELECT asText(instantN(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT asText(instantN(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT asText(instantN(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT asText(instants(tpose 'Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(instants(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(instants(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(instants(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT numTimestamps(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT numTimestamps(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT numTimestamps(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT numTimestamps(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT startTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT numSequences(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(startSequence(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(endSequence(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(sequenceN(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));
SELECT asText(sequences(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT startTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(tpose 'Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

SELECT asText(atValues(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValues(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValues(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValues(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', pose 'Pose(Point(1 1), 0.5)'));

SELECT asText(minusValues(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValues(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValues(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValues(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', pose 'Pose(Point(1 1), 0.5)'));

SELECT asText(atValues(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', poseset '{"Pose(Point(1 1), 0.5)"}'));

SELECT asText(minusValues(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', poseset '{"Pose(Point(1 1), 0.5)"}'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(valueAtTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', '2000-01-01'));
SELECT asText(valueAtTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '2000-01-01'));
SELECT asText(valueAtTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '2000-01-01'));
SELECT asText(valueAtTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '2000-01-01'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------

SELECT asText(beforeTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(beforeTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01', false));

SELECT asText(afterTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(afterTimestamp(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01', false));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' = tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' = tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' = tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' = tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' != tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' != tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' != tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' != tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' < tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' < tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' < tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' < tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' <= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' <= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' <= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' <= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' > tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' > tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' > tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' > tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' >= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' >= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' >= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2000-01-01' >= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= tpose 'Pose(Point(1 1), 0.5)@2000-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= tpose '{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= tpose '[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= tpose '{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

-------------------------------------------------------------------------------/
