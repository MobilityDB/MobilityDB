-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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

SELECT asText(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT asText(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT asText(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT asText(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');
SELECT asText(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT asText(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }');

SELECT asText(tpose '  Pose (  Point(1 1)  ,   0.5  )  @  2001-01-01  ');
SELECT asText(tpose '  {  Pose( Point(1 1) , 0.3 ) @ 2001-01-01  , Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5) @  2001-01-03   }   ');
SELECT asText(tpose '  [  Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT asText(tpose '  {  [  Pose(Point(1 1), 0.2)@2001-01-01 ,  Pose  (  Point(1 1) , 0.4 ) @2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

-- Normalization
SELECT asText(tpose '[Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT asText(tpose '{[Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05, Pose(Point(2 2), 0.6)@2001-01-06]}');

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT asText(tpose 'Pose(Point(1 1), 0.123456789)@2001-01-01', 6);
SELECT asText(tpose '{Pose(Point(1 1), 0.123456789)@2001-01-01, Pose(Point(1 1), 0.523456789)@2001-01-02, Pose(Point(1 1), 0.123456789)@2001-01-03}', 6);
SELECT asText(tpose '[Pose(Point(1 1), 0.123456789)@2001-01-01, Pose(Point(1 1), 0.523456789)@2001-01-02, Pose(Point(1 1), 0.123456789)@2001-01-03]', 6);
SELECT asText(tpose '{[Pose(Point(1 1), 0.123456789)@2001-01-01, Pose(Point(1 1), 0.523456789)@2001-01-02, Pose(Point(1 1), 0.123456789)@2001-01-03],[Pose(Point(2 2), 0.723456789)@2001-01-04, Pose(Point(2 2), 0.723456789)@2001-01-05]}', 6);
SELECT asText(tpose 'Interp=Step;[Pose(Point(1 1), 0.123456789)@2001-01-01, Pose(Point(1 1), 0.523456789)@2001-01-02, Pose(Point(1 1), 0.123456789)@2001-01-03]', 6);
SELECT asText(tpose 'Interp=Step;{[Pose(Point(1 1), 0.123456789)@2001-01-01, Pose(Point(1 1), 0.523456789)@2001-01-02, Pose(Point(1 1), 0.123456789)@2001-01-03],[Pose(Point(2 2), 0.723456789)@2001-01-04, Pose(Point(2 2), 0.723456789)@2001-01-05]}', 6);

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2001-01-01')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }')));

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'NDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }', 'NDR')));

SELECT asText(tposeFromBinary(asBinary(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'XDR')));
SELECT asText(tposeFromBinary(asBinary(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }', 'XDR')));

-------------------------------------------------------------------------------

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }')));

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }', 'NDR')));

SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'XDR')));
SELECT asText(tposeFromHexEWKB(asHexEWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }', 'XDR')));

SELECT asText(tposeFromHexEWKB(asHexWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }')));

SELECT asText(tposeFromHexEWKB(asHexWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'NDR')));
SELECT asText(tposeFromHexEWKB(asHexWKB(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'XDR')));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(tpose('Pose(Point(1 1),0)'::pose, '2012-01-01'::timestamp));

SELECT asText(tposeSeq(ARRAY['Pose(Point(1 1),0)@2012-01-01'::tpose, 'Pose(Point(2 2),1)@2012-02-01'::tpose], 'discrete'));

SELECT asText(tposeSeq(ARRAY['Pose(Point(1 1),0)@2012-01-01'::tpose, 'Pose(Point(1 1),1)@2012-02-01'::tpose], 'linear', true, false));

SELECT asText(tposeSeqSet(ARRAY[tpose '[Pose(Point(1 1),0)@2012-01-01, Pose(Point(1 1),1)@2012-02-01]', '[Pose(Point(2 2),0)@2012-03-01, Pose(Point(2 2),1)@2012-04-01]']));

-- From a temporal point and a temporal float: the result carries the point the
-- trajectory holds at each instant and the float as the rotation theta, over the
-- intersection of the two time frames.
SELECT asText(tpose(tgeompoint '[POINT(1 1)@2001-01-01, POINT(2 2)@2001-01-02)',
  tfloat '[1@2001-01-01, 2@2001-01-02)'));
SELECT asText(tpose(tgeompoint 'POINT(1 1)@2001-01-01', tfloat '1@2001-01-01'));

-- Time frames that do not intersect answer NULL
SELECT asText(tpose(tgeompoint '[POINT(1 1)@2001-01-01, POINT(2 2)@2001-01-02)',
  tfloat '[2@2001-01-03, 3@2001-01-04)')) IS NULL;

-- A partial overlap is restricted to the shared frame
SELECT asText(tpose(tgeompoint '[POINT(1 1)@2001-01-01, POINT(3 3)@2001-01-03]',
  tfloat '[1@2001-01-02, 3@2001-01-04]'));

-- The point must be 2D: a Z point resolves against the one declared overload
-- and meets the kernel's own guard. A tgeogpoint argument is refused one layer
-- lower, by PostgreSQL's overload resolution, whose message the suite does not
-- state anywhere because it moves between server versions.
SELECT asText(tpose(tgeompoint 'POINT Z(1 1 1)@2001-01-01', tfloat '1@2001-01-01'));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asText(tposeInst(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(setInterp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 'discrete'));
SELECT asText(tposeSeq(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(tposeSeqSet(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));

SELECT asText(tposeInst(tpose '{Pose(Point(1 1), 0.3)@2001-01-01}'));
SELECT asText(setInterp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 'discrete'));
SELECT asText(tposeSeq(tpose '{Pose(Point(1 1), 0.3)@2001-01-01}'));
SELECT asText(tposeSeqSet(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));

SELECT asText(tposeInst(tpose '[Pose(Point(1 1), 0.3)@2001-01-01]'));
SELECT asText(setInterp(tpose '[Pose(Point(1 1), 0.3)@2001-01-01]', 'discrete'));
SELECT asText(tposeSeq(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(tposeSeqSet(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));

SELECT asText(tposeInst(tpose '{[Pose(Point(1 1), 0.3)@2001-01-01]}'));
SELECT asText(setInterp(tpose '{[Pose(Point(1 1), 0.3)@2001-01-01], [Pose(Point(2 2), 0.6)@2001-01-04]}', 'discrete'));
SELECT asText(tposeSeq(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]}'));
SELECT asText(tposeSeqSet(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT asText(setInterp(tpose 'Interp=Step;[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 'linear'));
SELECT asText(setInterp(tpose 'Interp=Step;{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 'linear'));

SELECT asText(round(tpose '{[Pose(Point(1 1), 0.123456789)@2012-01-01, Pose(Point(1 1), 0.5)@2012-01-02)}', 6));

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT asText(appendInstant(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tpose 'Pose(Point(1 1), 0.7)@2001-01-02'));
SELECT asText(appendInstant(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tpose 'Pose(Point(1 1), 0.7)@2001-01-04'));
SELECT asText(appendInstant(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tpose 'Pose(Point(1 1), 0.7)@2001-01-04'));
SELECT asText(appendInstant(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }', tpose 'Pose(Point(2 2), 0.7)@2001-01-06'));

-------------------------------------------------------------------------------

SELECT asText(appendSequence(tpose '{Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(2 2), 0.4)@2001-01-02}', tpose '{Pose(Point(3 3), 0.6)@2001-01-03}'));
SELECT asText(appendSequence(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02]', tpose '[Pose(Point(1 1), 0.6)@2001-01-03]'));

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT asText(round(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'::tgeompoint, 6));
SELECT asText(round(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'::tgeompoint, 6));
SELECT asText(round(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'::tgeompoint, 6));
SELECT asText(round(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }'::tgeompoint, 6));

SELECT round(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'::tfloat, 6);
SELECT round(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'::tfloat, 6);
SELECT round(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'::tfloat, 6);
SELECT round(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }'::tfloat, 6);

-- SELECT asText(round((tpose 'Pose(Point(1 1), 0.5)@2001-01-01'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'::tgeompoint)::tpose, 6));
-- SELECT asText(round((tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05] }'::tgeompoint)::tpose, 6));
-- NULL
-- SELECT asText(tgeompoint 'SRID=5676;Point(-1 -1)@2001-01-01'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;{POINT(48.7186629128278 77.7640705101509)@2001-01-01, POINT(48.71 77.76)@2001-01-02}'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;[POINT(48.7186629128278 77.7640705101509)@2001-01-01, POINT(48.71 77.76)@2001-01-02]'::tpose;
-- SELECT asText(tgeompoint 'SRID=5676;{[POINT(62.7866330839742 80.1435561997142)@2001-01-01, POINT(62.7866330839742 80.1435561997142)@2001-01-02],[POINT(48.7186629128278 77.7640705101509)@2001-01-03, POINT(48.71 77.76)@2001-01-04]}'::tpose;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT tempBasetype(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT tempSubtype(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT tempSubtype(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT tempSubtype(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT memSize(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT memSize(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT memSize(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT memSize(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

-- The orientation channel of a temporal pose. A 2D pose interpolates its
-- stored angle linearly and that angle is its yaw, so a linear 2D temporal
-- pose projects to a linear temporal float. A 3D pose interpolates by SLERP,
-- whose Tait-Bryan decomposition is not linear in time, so it projects to a
-- step function.
SELECT asText(yaw(tpose '[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02]'));
SELECT asText(pitch(tpose '[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02]'));
SELECT asText(roll(tpose '[Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02]'));
SELECT asText(round(yaw(tpose '[Pose(Point Z(0 0 0), 1, 0, 0, 0)@2001-01-01, Pose(Point Z(1 1 1), 0.5, 0.5, 0.5, 0.5)@2001-01-02]'), 6));

SELECT asText(getValue(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));

SELECT asText(getValues(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(getValues(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(getValues(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(getValues(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT getTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT getTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT getTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT getTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT timeSpan(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT timeSpan(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT timeSpan(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT timeSpan(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT duration(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', true);
SELECT duration(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', true);
SELECT duration(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', true);
SELECT duration(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', true);

SELECT duration(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT duration(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT duration(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT duration(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT getTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT asText(shiftTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', '5 min'));
SELECT asText(shiftTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', '5 min'));
SELECT asText(shiftTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', '5 min'));
SELECT asText(shiftTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', '5 min'));

SELECT asText(scaleTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', '1 day'));
SELECT asText(scaleTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', '1 day'));
SELECT asText(scaleTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', '1 day'));
SELECT asText(scaleTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', '1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' ?= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' ?= pose 'Pose(Point(1 1), 0.5)';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' %= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' %= pose 'Pose(Point(1 1), 0.5)';

SELECT asText(shiftTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', '1 year'::interval));
SELECT asText(shiftTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', '1 year'::interval));
SELECT asText(shiftTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', '1 year'::interval));
SELECT asText(shiftTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', '1 year'::interval));

SELECT asText(startValue(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(startValue(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(startValue(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(startValue(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT asText(endValue(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(endValue(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(endValue(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(endValue(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT asText(valueN(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 1));
SELECT asText(valueN(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 1));
SELECT asText(valueN(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 1));
SELECT asText(valueN(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 1));

SELECT numInstants(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT numInstants(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT numInstants(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT numInstants(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT asText(startInstant(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(startInstant(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(startInstant(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(startInstant(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT asText(endInstant(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(endInstant(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(endInstant(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(endInstant(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT asText(instantN(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 1));
SELECT asText(instantN(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 1));
SELECT asText(instantN(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 1));
SELECT asText(instantN(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 1));

SELECT asText(instants(tpose 'Pose(Point(1 1), 0.5)@2001-01-01'));
SELECT asText(instants(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}'));
SELECT asText(instants(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]'));
SELECT asText(instants(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT numTimestamps(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT numTimestamps(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT numTimestamps(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT numTimestamps(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT startTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT startTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT startTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT startTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT endTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT endTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT endTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT endTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT timestampN(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 1);
SELECT timestampN(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 1);
SELECT timestampN(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 1);
SELECT timestampN(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 1);

SELECT timestamps(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT timestamps(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT timestamps(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT timestamps(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT numSequences(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');
SELECT asText(startSequence(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));
SELECT asText(endSequence(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));
SELECT asText(sequenceN(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 1));
SELECT asText(sequences(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}'));

SELECT startTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT startTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT startTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT startTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT endTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT endTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT endTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT endTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

SELECT timestampN(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', 1);
SELECT timestampN(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', 1);
SELECT timestampN(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 1);
SELECT timestampN(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', 1);

SELECT timestamps(tpose 'Pose(Point(1 1), 0.5)@2001-01-01');
SELECT timestamps(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}');
SELECT timestamps(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT timestamps(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

SELECT asText(atValue(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValue(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValue(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(atValue(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', pose 'Pose(Point(1 1), 0.5)'));

SELECT asText(minusValue(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValue(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValue(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', pose 'Pose(Point(1 1), 0.5)'));
SELECT asText(minusValue(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', pose 'Pose(Point(1 1), 0.5)'));

SELECT asText(atValues(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', poseset '{"Pose(Point(1 1), 0.5)"}'));

SELECT asText(minusValues(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', poseset '{"Pose(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', poseset '{"Pose(Point(1 1), 0.5)"}'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01'));

SELECT asText(valueAtTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', '2001-01-01'));
SELECT asText(valueAtTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', '2001-01-01'));
SELECT asText(valueAtTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', '2001-01-01'));
SELECT asText(valueAtTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', '2001-01-01'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzset '{2001-01-01}'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzset '{2001-01-01}'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzset '{2001-01-01}'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzset '{2001-01-01}'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzset '{2001-01-01}'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzset '{2001-01-01}'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzset '{2001-01-01}'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzset '{2001-01-01}'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspan '[2001-01-01, 2001-01-02]'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspan '[2001-01-01, 2001-01-02]'));

SELECT asText(atTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(atTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(atTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(atTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspanset '{[2001-01-01, 2001-01-02]}'));

SELECT asText(minusTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(minusTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(minusTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(minusTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspanset '{[2001-01-01, 2001-01-02]}'));

-------------------------------------------------------------------------------

SELECT asText(beforeTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01'));
SELECT asText(beforeTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01'));
SELECT asText(beforeTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01'));
SELECT asText(beforeTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01'));

SELECT asText(beforeTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01', false));
SELECT asText(beforeTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01', false));
SELECT asText(beforeTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01', false));
SELECT asText(beforeTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01', false));

SELECT asText(afterTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01'));
SELECT asText(afterTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01'));
SELECT asText(afterTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01'));
SELECT asText(afterTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01'));

SELECT asText(afterTimestamp(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01', false));
SELECT asText(afterTimestamp(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01', false));
SELECT asText(afterTimestamp(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01', false));
SELECT asText(afterTimestamp(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01', false));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(insert(
  tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02], [Pose(Point(1 1), 0.5)@2001-01-03, Pose(Point(1 1), 0.6)@2001-01-04]}',
  tpose '{[Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]}'));
/* Error: insert vs update distinguishing case (disagreeing value at common timestamp) */
SELECT asText(insert(
  tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02]',
  tpose '[Pose(Point(1 1), 0.9)@2001-01-02, Pose(Point(1 1), 0.9)@2001-01-03]'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', timestamptz '2001-01-01'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', timestamptz '2001-01-01'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', timestamptz '2001-01-01'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', timestamptz '2001-01-01'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzset '{2001-01-01}'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzset '{2001-01-01}'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzset '{2001-01-01}'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzset '{2001-01-01}'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspan '[2001-01-01, 2001-01-02]'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspan '[2001-01-01, 2001-01-02]'));

SELECT asText(deleteTime(tpose 'Pose(Point(1 1), 0.5)@2001-01-01', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(deleteTime(tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(deleteTime(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', tstzspanset '{[2001-01-01, 2001-01-02]}'));
SELECT asText(deleteTime(tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}', tstzspanset '{[2001-01-01, 2001-01-02]}'));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' = tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' = tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' = tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' = tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' = tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' = tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' = tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' = tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' = tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' = tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' = tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' = tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' = tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' = tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' = tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' = tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' != tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' != tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' != tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' != tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' != tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' != tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' != tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' != tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' != tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' != tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' != tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' != tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' != tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' != tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' != tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' != tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' < tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' < tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' < tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' < tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' < tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' < tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' < tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' < tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' < tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' < tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' < tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' < tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' < tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' < tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' < tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' < tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' <= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' <= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' <= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' <= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' <= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' <= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' <= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' <= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' <= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' <= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' <= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' <= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' <= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' <= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' <= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' <= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' > tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' > tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' > tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' > tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' > tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' > tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' > tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' > tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' > tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' > tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' > tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' > tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' > tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' > tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' > tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' > tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' >= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' >= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' >= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' >= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' >= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' >= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' >= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' >= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' >= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' >= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' >= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' >= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' >= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' >= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' >= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' >= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------/

SELECT numSequences(tposeSeqSetGaps(ARRAY[
  tpose 'Pose(Point(1 1), 0.0)@2001-01-01',
  tpose 'Pose(Point(2 2), 0.5)@2001-01-02',
  tpose 'Pose(Point(3 3), 1.0)@2001-01-03'
]::tpose[], '5 minutes'::interval));

-------------------------------------------------------------------------------
-- Geodetic temporal poses
-------------------------------------------------------------------------------

SELECT asText(tpose '[GeodPose(Point(1 1),0.1)@2001-01-01, GeodPose(Point(2 2),0.2)@2001-01-02]');
SELECT tpose '[GeodPose(Point(1 1),0.1)@2001-01-01, GeodPose(Point(2 2),0.2)@2001-01-02]';

-------------------------------------------------------------------------------/

-- tprecision
SELECT asText(tprecision(tpose '[Pose(Point(0 0),0)@2001-01-01, Pose(Point(4 0),0)@2001-01-01 00:00:04]', interval '2 secs'));

-------------------------------------------------------------------------------
-- Conversion to a temporal point: geometric for planar, geographic for geodetic
-------------------------------------------------------------------------------

SELECT asText(tgeompoint(tpose '[Pose(Point(1 1),0.1)@2001-01-01, Pose(Point(2 2),0.2)@2001-01-02]'));
SELECT asText(tgeogpoint(tpose '[GeodPose(Point(1 1),0.1)@2001-01-01, GeodPose(Point(2 2),0.2)@2001-01-02]'));
SELECT pg_typeof(tgeompoint(tpose '[Pose(Point(1 1),0.1)@2001-01-01]'));
SELECT pg_typeof(tgeogpoint(tpose '[GeodPose(Point(1 1),0.1)@2001-01-01]'));
-- The frame of the pose must agree with the type asked for
SELECT asText(tgeogpoint(tpose '[Pose(Point(1 1),0.1)@2001-01-01]'));
SELECT asText(tgeompoint(tpose '[GeodPose(Point(1 1),0.1)@2001-01-01]'));

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------
-- MF-JSON output of the temporal poses built from real AIS data, that is, the
-- recorded vessel positions with the course computed from them. The AIS table
-- keeps the full precision of the source data, which is what exposes the
-- output corruption reported at
-- https://github.com/MobilityDB/MobilityDB/issues/850
-- Every query below reports the rows whose output is not valid JSON, and must
-- thus report zero
-------------------------------------------------------------------------------

WITH ais(mmsi, temp) AS (
  SELECT mmsi, tposeSeq(array_agg(tpose(pose(geom, heading), t) ORDER BY t))
  FROM tbl_ais_instant GROUP BY mmsi )
-- The MF-JSON angle of a 2D pose is its yaw; a 3D pose carries a quaternion.
SELECT asMFJSON(tpose 'SRID=4326;Pose(Point(1 2), 0.5)@2001-01-01');
SELECT asMFJSON(tpose 'SRID=4326;Pose(Point Z(1 2 3), 1, 0, 0, 0)@2001-01-01');
-- Both names of the 2D angle are read, so a document written by MobilityDB
-- 1.3 under the name 'rotation' still parses.
SELECT asText(tposeFromMFJSON('{"type":"MovingPose","crs":{"type":"Name","properties":{"name":"EPSG:4326"}},"values":[{"position":{"lat":2,"lon":1},"yaw":0.5}],"datetimes":["2001-01-01T00:00:00+00"],"interpolation":"None"}'));
SELECT asText(tposeFromMFJSON('{"type":"MovingPose","crs":{"type":"Name","properties":{"name":"EPSG:4326"}},"values":[{"position":{"lat":2,"lon":1},"rotation":0.5}],"datetimes":["2001-01-01T00:00:00+00"],"interpolation":"None"}'));

SELECT count(*) FROM ais WHERE asMFJSON(temp)::jsonb IS NULL;

WITH ais(mmsi, temp) AS (
  SELECT mmsi, tposeSeq(array_agg(tpose(pose(geom, heading), t) ORDER BY t))
  FROM tbl_ais_instant GROUP BY mmsi )
SELECT count(*) FROM ais, generate_series(0, 15) AS d
  WHERE asMFJSON(temp, 1, 0, d)::jsonb IS NULL;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Multidimensional tiling
-------------------------------------------------------------------------------

SELECT spans(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]');
SELECT splitNSpans(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 2);
SELECT splitEachNSpans(tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]', 2);
