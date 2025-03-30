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

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));  Pose (  Point(1 1)  ,   0.5  )  @  2000-01-01  ');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));  {  Pose( Point(1 1) , 0.3 ) @ 2000-01-01  , Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5) @  2000-01-03   }   ');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));  [  Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));  {  [  Pose(Point(1 1), 0.2)@2000-01-01 ,  Pose  (  Point(1 1) , 0.4 ) @2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

-- Normalization
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05, Pose(Point(2 2), 0.6)@2000-01-06]}');

-------------------------------------------------------------------------------
-- Input/output in WKT, WKB and HexWKB representation
-------------------------------------------------------------------------------

SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.123456789)@2000-01-01', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03}', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asText(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asEWKT(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.123456789)@2000-01-01', 6);
SELECT asEWKT(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03}', 6);
SELECT asEWKT(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asEWKT(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);
SELECT asEWKT(trgeometry 'SRID=5676,Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asEWKT(trgeometry 'SRID=5676,Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------

SELECT asMFJSON(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.123456789)@2000-01-01', 6);
SELECT asMFJSON(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03}', 6);
SELECT asMFJSON(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asMFJSON(trgeometry 'SRID=5676;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);
SELECT asMFJSON(trgeometry 'SRID=5676,Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03]', 6);
SELECT asMFJSON(trgeometry 'SRID=5676,Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2000-01-01, Pose(Point(1 1), 0.523456789)@2000-01-02, Pose(Point(1 1), 0.123456789)@2000-01-03],[Pose(Point(2 2), 0.723456789)@2000-01-04, Pose(Point(2 2), 0.723456789)@2000-01-05]}', 6);

-------------------------------------------------------------------------------
-- Maximum decimal digits

SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromMFJSON(asMFJSON(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));

SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));

SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));

SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(trgeometryFromBinary(asBinary(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));

-------------------------------------------------------------------------------

SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}')));

SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 'NDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'NDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'NDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'NDR')));

SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 'XDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'XDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'XDR')));
SELECT asText(trgeometryFromHexEWKB(asHexEWKB(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'XDR')));

-------------------------------------------------------------------------------
-- Constructors
-------------------------------------------------------------------------------

SELECT asText(trgeometry('Polygon((1 1,2 2,3 1,1 1))', tpose(pose 'Pose(Point(1 1),0)', timestamp '2012-01-01')));

SELECT asText(trgeometrySeq(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0)@2012-01-01', 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(2 2),1)@2012-02-01'], 'discrete'));

SELECT asText(trgeometrySeq(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),0)@2012-01-01', 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1),1)@2012-02-01'], 'linear', true, false));

SELECT asText(trgeometrySeqSet(ARRAY[trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1),0)@2012-01-01, Pose(Point(1 1),1)@2012-02-01]', 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(2 2),0)@2012-03-01, Pose(Point(2 2),1)@2012-04-01]']));

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT asText(trgeometryInst(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(setInterp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 'discrete'));
SELECT asText(trgeometrySeq(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(trgeometrySeqSet(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(trgeometryInst(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(setInterp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 'discrete'));
SELECT asText(trgeometrySeq(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01}'));
SELECT asText(trgeometrySeqSet(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));

SELECT asText(trgeometryInst(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.3)@2000-01-01]'));
SELECT asText(setInterp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.3)@2000-01-01]', 'discrete'));
SELECT asText(trgeometrySeq(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(trgeometrySeqSet(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));

SELECT asText(trgeometryInst(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.3)@2000-01-01]}'));
SELECT asText(setInterp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.3)@2000-01-01], [Pose(Point(2 2), 0.6)@2000-01-04]}', 'discrete'));
SELECT asText(trgeometrySeq(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]}'));
SELECT asText(trgeometrySeqSet(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(setInterp(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 'linear'));
SELECT asText(setInterp(trgeometry 'Interp=Step;Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 'linear'));

SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.123456789)@2012-01-01, Pose(Point(1 1), 0.5)@2012-01-02)}', 6));

-------------------------------------------------------------------------------
-- Append functions
-------------------------------------------------------------------------------

SELECT asText(appendInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.7)@2000-01-02'));
SELECT asText(appendInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.7)@2000-01-04'));
SELECT asText(appendInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(2 2), 0.7)@2000-01-06'));

-------------------------------------------------------------------------------

SELECT asText(appendSequence(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(2 2), 0.4)@2000-01-02}', trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(3 3), 0.6)@2000-01-03}'));
SELECT asText(appendSequence(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02]', trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.6)@2000-01-03]'));

-------------------------------------------------------------------------------
-- Conversion functions
-------------------------------------------------------------------------------

SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'::tpose, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::tpose, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::tpose, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'::tpose, 6));

SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'::tgeompoint, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::tgeompoint, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::tgeompoint, 6));
SELECT asText(round(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'::tgeompoint, 6));

SELECT ST_AsText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'::geometry);
SELECT ST_AsText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'::geometry);
SELECT ST_AsText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'::geometry);
SELECT ST_AsText(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'::geometry);

-------------------------------------------------------------------------------
-- Accessor Functions
-------------------------------------------------------------------------------

SELECT tempSubtype(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT tempSubtype(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT tempSubtype(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT tempSubtype(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT memSize(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT memSize(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT memSize(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT memSize(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(getValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));

SELECT asText(getValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(getValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(getValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(getValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT getTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT getTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT getTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT getTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timeSpan(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timeSpan(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timeSpan(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timeSpan(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', true);
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', true);
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', true);
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', true);

SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT duration(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT getTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');

-------------------------------------------------------------------------------
-- Shift and scale functions
-------------------------------------------------------------------------------

SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', '5 min'));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '5 min'));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '5 min'));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '5 min'));

SELECT asText(scaleTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', '1 day'));
SELECT asText(scaleTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '1 day'));
SELECT asText(scaleTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '1 day'));
SELECT asText(scaleTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '1 day'));

-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' ?= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' ?= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' ?= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' ?= geometry 'Polygon((1 1,2 2,3 1,1 1))';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' %= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' %= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' %= geometry 'Polygon((1 1,2 2,3 1,1 1))';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' %= geometry 'Polygon((1 1,2 2,3 1,1 1))';

SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', '1 year'::interval));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '1 year'::interval));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '1 year'::interval));
SELECT asText(shiftTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '1 year'::interval));

SELECT ST_AsText(startValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT ST_AsText(startValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT ST_AsText(startValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT ST_AsText(startValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT ST_AsText(endValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT ST_AsText(endValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT ST_AsText(endValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT ST_AsText(endValue(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT ST_AsText(valueN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 1));
SELECT ST_AsText(valueN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT ST_AsText(valueN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT ST_AsText(valueN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT numInstants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT numInstants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT numInstants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT numInstants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT asText(startInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(startInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(startInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(startInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(endInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(endInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(endInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(endInstant(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT asText(instantN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 1));
SELECT asText(instantN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1));
SELECT asText(instantN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1));
SELECT asText(instantN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));

SELECT asText(instants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01'));
SELECT asText(instants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}'));
SELECT asText(instants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]'));
SELECT asText(instants(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT numTimestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT numTimestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT numTimestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT numTimestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT numSequences(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');
SELECT asText(startSequence(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(endSequence(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));
SELECT asText(sequenceN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1));
SELECT asText(sequences(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}'));

SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT startTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT endTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', 1);
SELECT timestampN(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', 1);

SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]');
SELECT timestamps(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}');

-------------------------------------------------------------------------------
-- Restriction Functions
-------------------------------------------------------------------------------

-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', geometry 'Polygon((1 1,2 2,3 1,1 1))'));

-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', geometry 'Polygon((1 1,2 2,3 1,1 1))'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', geometry 'Polygon((1 1,2 2,3 1,1 1))'));

-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(atValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));

-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));
-- SELECT asText(minusValues(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', geomset '{"Polygon((1 1,2 2,3 1,1 1))"}'));

SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT ST_AsText(valueAtTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', '2000-01-01'));
SELECT ST_AsText(valueAtTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', '2000-01-01'));
SELECT ST_AsText(valueAtTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', '2000-01-01'));
SELECT ST_AsText(valueAtTimestamp(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', '2000-01-01'));

SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(atTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(minusTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------
-- Modification functions
-------------------------------------------------------------------------------

SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', timestamptz '2000-01-01'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', timestamptz '2000-01-01'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', timestamptz '2000-01-01'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', timestamptz '2000-01-01'));

SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzset '{2000-01-01}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzset '{2000-01-01}'));

SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspan '[2000-01-01, 2000-01-02]'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspan '[2000-01-01, 2000-01-02]'));

SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]', tstzspanset '{[2000-01-01, 2000-01-02]}'));
SELECT asText(deleteTime(trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}', tstzspanset '{[2000-01-01, 2000-01-02]}'));

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' = trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' != trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' < trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' <= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' > trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));Pose(Point(1 1), 0.5)@2000-01-01';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{Pose(Point(1 1), 0.3)@2000-01-01, Pose(Point(1 1), 0.5)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03}';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03]';
SELECT trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}' >= trgeometry 'Polygon((1 1,2 2,3 1,1 1));{[Pose(Point(1 1), 0.2)@2000-01-01, Pose(Point(1 1), 0.4)@2000-01-02, Pose(Point(1 1), 0.5)@2000-01-03], [Pose(Point(2 2), 0.6)@2000-01-04, Pose(Point(2 2), 0.6)@2000-01-05]}';

-------------------------------------------------------------------------------
