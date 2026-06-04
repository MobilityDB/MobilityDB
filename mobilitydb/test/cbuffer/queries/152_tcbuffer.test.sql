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

SELECT asText(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT asText(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT asText(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }');

SELECT asText(tcbuffer '  Cbuffer (  Point(1 1)  ,   0.5  )  @  2000-01-01  ');
SELECT asText(tcbuffer '  {  Cbuffer( Point(1 1) , 0.3 ) @ 2000-01-01  , Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5) @  2000-01-03   }   ');
SELECT asText(tcbuffer '  [  Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tcbuffer '  {  [  Cbuffer(Point(1 1), 0.2)@2000-01-01 ,  Cbuffer  (  Point(1 1) , 0.4 ) @2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

-- Normalization
SELECT asText(tcbuffer '[Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(tcbuffer'{[Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05, Cbuffer(Point(2 2), 0.6)@2000-01-06]}');

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB formats
-------------------------------------------------------------------------------

SELECT asText(tcbuffer 'Cbuffer(Point(1 1), 0.123456789)@2000-01-01', 6);
SELECT asText(tcbuffer '{Cbuffer(Point(1 1), 0.123456789)@2000-01-01, Cbuffer(Point(1 1), 0.523456789)@2000-01-02, Cbuffer(Point(1 1), 0.123456789)@2000-01-03}', 6);
SELECT asText(tcbuffer '[Cbuffer(Point(1 1), 0.123456789)@2000-01-01, Cbuffer(Point(1 1), 0.523456789)@2000-01-02, Cbuffer(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(tcbuffer '{[Cbuffer(Point(1 1), 0.123456789)@2000-01-01, Cbuffer(Point(1 1), 0.523456789)@2000-01-02, Cbuffer(Point(1 1), 0.123456789)@2000-01-03],[Cbuffer(Point(2 2), 0.723456789)@2000-01-04, Cbuffer(Point(2 2), 0.723456789)@2000-01-05]}', 6);
SELECT asText(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.123456789)@2000-01-01, Cbuffer(Point(1 1), 0.523456789)@2000-01-02, Cbuffer(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.123456789)@2000-01-01, Cbuffer(Point(1 1), 0.523456789)@2000-01-02, Cbuffer(Point(1 1), 0.123456789)@2000-01-03],[Cbuffer(Point(2 2), 0.723456789)@2000-01-04, Cbuffer(Point(2 2), 0.723456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }')));

SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }', 'NDR')));

SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tcbufferFromBinary(asBinary(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }', 'XDR')));

-------------------------------------------------------------------------------

SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }')));

SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }', 'NDR')));

SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(tcbufferFromHexEWKB(asHexEWKB(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }', 'XDR')));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(tcbuffer('Cbuffer(Point(1 1),0)'::cbuffer, '2012-01-01'::timestamp));

SELECT asText(tcbufferSeq(ARRAY['Cbuffer(Point(1 1),0)@2012-01-01'::tcbuffer, 'Cbuffer(Point(2 2),1)@2012-02-01'::tcbuffer], 'discrete'));

SELECT asText(tcbufferSeq(ARRAY['Cbuffer(Point(1 1),0)@2012-01-01'::tcbuffer, 'Cbuffer(Point(1 1),1)@2012-02-01'::tcbuffer], 'linear', true, false));

SELECT asText(tcbufferSeqSet(ARRAY[tcbuffer '[Cbuffer(Point(1 1),0)@2012-01-01, Cbuffer(Point(1 1),1)@2012-02-01]', '[Cbuffer(Point(2 2),0)@2012-03-01, Cbuffer(Point(2 2),1)@2012-04-01]']));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asText(tcbufferInst(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(setInterp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 'discrete'));
SELECT asText(tcbufferSeq(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(tcbufferSeqSet(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(tcbufferInst(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(setInterp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 'discrete'));
SELECT asText(tcbufferSeq(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(tcbufferSeqSet(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));

SELECT asText(tcbufferInst(tcbuffer '[Cbuffer(Point(1 1), 0.3)@2000-01-01]'));
SELECT asText(setInterp(tcbuffer '[Cbuffer(Point(1 1), 0.3)@2000-01-01]', 'discrete'));
SELECT asText(tcbufferSeq(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(tcbufferSeqSet(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));

SELECT asText(tcbufferInst(tcbuffer '{[Cbuffer(Point(1 1), 0.3)@2000-01-01]}'));
SELECT asText(setInterp(tcbuffer '{[Cbuffer(Point(1 1), 0.3)@2000-01-01], [Cbuffer(Point(2 2), 0.6)@2000-01-04]}', 'discrete'));
SELECT asText(tcbufferSeq(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]}'));
SELECT asText(tcbufferSeqSet(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(setInterp(tcbuffer 'Interp=Step;[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 'linear'));
SELECT asText(setInterp(tcbuffer 'Interp=Step;{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 'linear'));

SELECT asText(round(tcbuffer '{[Cbuffer(Point(1 1), 0.123456789)@2012-01-01, Cbuffer(Point(1 1), 0.5)@2012-01-02)}', 6));

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT asText(appendInstant(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tcbuffer 'Cbuffer(Point(1 1), 0.7)@2000-01-02'));
SELECT asText(appendInstant(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tcbuffer 'Cbuffer(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tcbuffer 'Cbuffer(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }', tcbuffer 'Cbuffer(Point(2 2), 0.7)@2000-01-06'));

-------------------------------------------------------------------------------

SELECT asText(appendSequence(tcbuffer '{Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(2 2), 0.4)@2000-01-02}', tcbuffer '{Cbuffer(Point(3 3), 0.6)@2000-01-03}'));
SELECT asText(appendSequence(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02]', tcbuffer '[Cbuffer(Point(1 1), 0.6)@2000-01-03]'));

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT asText(round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'::tgeompoint, 6));
SELECT asText(round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'::tgeompoint, 6));
SELECT asText(round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'::tgeompoint, 6));
SELECT asText(round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }'::tgeompoint, 6));

SELECT round(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'::tfloat, 6);
SELECT round(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'::tfloat, 6);
SELECT round(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'::tfloat, 6);
SELECT round(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }'::tfloat, 6);

-- SELECT asText(round((tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'::tgeompoint)::tcbuffer, 6));
-- SELECT asText(round((tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'::tgeompoint)::tcbuffer, 6));
-- SELECT asText(round((tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'::tgeompoint)::tcbuffer, 6));
-- SELECT asText(round((tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05] }'::tgeompoint)::tcbuffer, 6));
-- NULL
-- SELECT asText(tgeompoint 'SRID=5676;Point(-1 -1)@2000-01-01'::tcbuffer;
-- SELECT asText(tgeompoint 'SRID=5676;{POINT(48.7186629128278 77.7640705101509)@2000-01-01, POINT(48.71 77.76)@2000-01-02}'::tcbuffer;
-- SELECT asText(tgeompoint 'SRID=5676;[POINT(48.7186629128278 77.7640705101509)@2000-01-01, POINT(48.71 77.76)@2000-01-02]'::tcbuffer;
-- SELECT asText(tgeompoint 'SRID=5676;{[POINT(62.7866330839742 80.1435561997142)@2000-01-01, POINT(62.7866330839742 80.1435561997142)@2000-01-02],[POINT(48.7186629128278 77.7640705101509)@2000-01-03, POINT(48.71 77.76)@2000-01-04]}'::tcbuffer;

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT tempSubtype(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT tempSubtype(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT tempSubtype(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT memSize(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT memSize(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT memSize(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT memSize(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(getValue(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(getValues(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(getValues(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(getValues(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(getValues(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT getTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT getTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT getTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT getTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT timeSpan(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT timeSpan(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT timeSpan(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT timeSpan(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT duration(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', true);
SELECT duration(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', true);
SELECT duration(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', true);
SELECT duration(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', true);

SELECT duration(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT duration(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT duration(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT duration(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT getTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');

-------------------------------------------------------------------------------

SELECT asEWKT(points(tcbuffer 'SRID=5676;{Cbuffer(Point(3 3), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02}'));
SELECT radius(tcbuffer 'SRID=5676;{Cbuffer(Point(3 3), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02}');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT asText(shiftTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', '5 min'));
SELECT asText(shiftTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', '5 min'));
SELECT asText(shiftTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', '5 min'));
SELECT asText(shiftTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', '5 min'));

SELECT asText(scaleTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', '1 day'));
SELECT asText(scaleTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', '1 day'));
SELECT asText(scaleTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', '1 day'));
SELECT asText(scaleTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', '1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' ?= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' ?= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' ?= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' ?= cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' %= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' %= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' %= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' %= cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT asText(shiftTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', '1 year'::interval));
SELECT asText(shiftTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', '1 year'::interval));
SELECT asText(shiftTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', '1 year'::interval));
SELECT asText(shiftTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', '1 year'::interval));

SELECT asText(startValue(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(startValue(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(startValue(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(startValue(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(endValue(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(endValue(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(endValue(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(endValue(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(valueN(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 1));
SELECT asText(valueN(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT asText(valueN(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT asText(valueN(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT numInstants(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT numInstants(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT numInstants(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT numInstants(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(startInstant(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(startInstant(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(startInstant(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(startInstant(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(endInstant(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(endInstant(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(endInstant(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(endInstant(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(instantN(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 1));
SELECT asText(instantN(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT asText(instantN(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT asText(instantN(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT asText(instants(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(instants(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(instants(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(instants(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT numTimestamps(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT numTimestamps(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT numTimestamps(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT numTimestamps(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT startTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT numSequences(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(startSequence(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(endSequence(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(sequenceN(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 1));
SELECT asText(sequences(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}'));

SELECT startTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

SELECT asText(atValues(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(atValues(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(atValues(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(atValues(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1), 0.5)'));

SELECT asText(minusValues(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(minusValues(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(minusValues(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', cbuffer 'Cbuffer(Point(1 1), 0.5)'));
SELECT asText(minusValues(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', cbuffer 'Cbuffer(Point(1 1), 0.5)'));

SELECT asText(atValues(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(atValues(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));

SELECT asText(minusValues(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));
SELECT asText(minusValues(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', cbufferset '{"Cbuffer(Point(1 1), 0.5)"}'));

SELECT asText(atTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(valueAtTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', '2000-01-01'));
SELECT asText(valueAtTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', '2000-01-01'));
SELECT asText(valueAtTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', '2000-01-01'));
SELECT asText(valueAtTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', '2000-01-01'));

SELECT asText(minusTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(atTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(atTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(atTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(atTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(minusTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(minusTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(atTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(minusTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(atTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

SELECT asText(minusTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------

SELECT asText(beforeTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(beforeTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(beforeTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(beforeTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01', false));

SELECT asText(afterTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(afterTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(afterTimestamp(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01', false));
SELECT asText(afterTimestamp(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01', false));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(deleteTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(deleteTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(deleteTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(deleteTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(deleteTime(tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' = tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' = tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' = tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' = tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' = tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' = tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' = tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' = tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' = tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' = tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' = tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' = tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' = tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' = tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' = tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' = tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' != tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' != tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' != tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' != tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' != tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' != tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' != tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' != tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' != tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' != tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' != tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' != tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' != tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' != tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' != tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' != tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' < tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' < tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' < tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' < tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' < tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' < tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' < tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' < tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' < tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' < tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' < tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' < tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' < tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' < tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' < tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' < tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' <= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' <= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' <= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' <= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' > tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' > tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' > tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' > tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' > tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' > tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' > tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' > tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' > tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' > tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' > tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' > tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' > tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' > tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' > tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' > tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' >= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' >= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' >= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01' >= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' >= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' >= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' >= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}' >= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' >= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' >= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' >= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]' >= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' >= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' >= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2000-01-01, Cbuffer(Point(1 1), 0.5)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' >= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}' >= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2000-01-01, Cbuffer(Point(1 1), 0.4)@2000-01-02, Cbuffer(Point(1 1), 0.5)@2000-01-03], [Cbuffer(Point(2 2), 0.6)@2000-01-04, Cbuffer(Point(2 2), 0.6)@2000-01-05]}';

-------------------------------------------------------------------------------/
