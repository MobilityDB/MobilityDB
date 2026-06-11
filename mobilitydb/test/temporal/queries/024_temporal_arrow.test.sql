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

-- Round-trip a temporal float through the Arrow C Data Interface. The result
-- must equal the input for every subtype and interpolation.

SELECT arrowRoundtrip(tfloat '42.5@2000-01-01');
SELECT arrowRoundtrip(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1@2000-01-03)');
SELECT arrowRoundtrip(tfloat '(0@2000-01-01, -3.25@2000-01-02, 7@2000-01-03)');
SELECT arrowRoundtrip(tfloat '[42@2000-01-01]');
SELECT arrowRoundtrip(tfloat '[1@2000-01-01, 1@2000-01-02, 2@2000-01-03, 2@2000-01-04]');
SELECT arrowRoundtrip(tfloat '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}');
SELECT arrowRoundtrip(tfloat 'Interp=Step;[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]');
SELECT arrowRoundtrip(tfloat '{[1@2000-01-01, 2@2000-01-02), [3@2000-01-03, 4@2000-01-04]}');
SELECT arrowRoundtrip(tfloat 'Interp=Step;{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 9@2000-01-04]}');

-- Equality with the input across the same shapes
SELECT arrowRoundtrip(tfloat '42.5@2000-01-01') = tfloat '42.5@2000-01-01';
SELECT arrowRoundtrip(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1@2000-01-03)') = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1@2000-01-03)';
SELECT arrowRoundtrip(tfloat '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}') = tfloat '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}';
SELECT arrowRoundtrip(tfloat 'Interp=Step;{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 9@2000-01-04]}') = tfloat 'Interp=Step;{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 9@2000-01-04]}';

-- Temporal integer: every subtype round-trips through the Int32 value leaf

SELECT arrowRoundtrip(tint '42@2000-01-01');
SELECT arrowRoundtrip(tint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)');
SELECT arrowRoundtrip(tint '[0@2000-01-01, -3@2000-01-02, 7@2000-01-03]');
SELECT arrowRoundtrip(tint '[42@2000-01-01]');
SELECT arrowRoundtrip(tint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}');
SELECT arrowRoundtrip(tint 'Interp=Step;[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]');
SELECT arrowRoundtrip(tint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}');

SELECT arrowRoundtrip(tint '42@2000-01-01') = tint '42@2000-01-01';
SELECT arrowRoundtrip(tint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)') = tint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)';
SELECT arrowRoundtrip(tint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}') = tint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}';
SELECT arrowRoundtrip(tint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}') = tint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}';

-- Temporal big integer: every subtype round-trips through the Int64 value
-- leaf; a 64-bit value exercises the full width

SELECT arrowRoundtrip(tbigint '42@2000-01-01');
SELECT arrowRoundtrip(tbigint '9000000000000000000@2000-01-01');
SELECT arrowRoundtrip(tbigint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)');
SELECT arrowRoundtrip(tbigint '[0@2000-01-01, -3@2000-01-02, 7@2000-01-03]');
SELECT arrowRoundtrip(tbigint '[42@2000-01-01]');
SELECT arrowRoundtrip(tbigint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}');
SELECT arrowRoundtrip(tbigint 'Interp=Step;[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]');
SELECT arrowRoundtrip(tbigint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}');

SELECT arrowRoundtrip(tbigint '42@2000-01-01') = tbigint '42@2000-01-01';
SELECT arrowRoundtrip(tbigint '9000000000000000000@2000-01-01') = tbigint '9000000000000000000@2000-01-01';
SELECT arrowRoundtrip(tbigint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)') = tbigint '[1@2000-01-01, 2@2000-01-02, 2@2000-01-03)';
SELECT arrowRoundtrip(tbigint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}') = tbigint '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}';
SELECT arrowRoundtrip(tbigint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}') = tbigint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}';

-- Temporal boolean: every subtype round-trips through the bit-packed leaf

SELECT arrowRoundtrip(tbool 't@2000-01-01');
SELECT arrowRoundtrip(tbool '[t@2000-01-01, f@2000-01-02, f@2000-01-03)');
SELECT arrowRoundtrip(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT arrowRoundtrip(tbool '[t@2000-01-01]');
SELECT arrowRoundtrip(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT arrowRoundtrip(tbool 'Interp=Step;[f@2000-01-01, t@2000-01-02, f@2000-01-03]');
SELECT arrowRoundtrip(tbool '{[t@2000-01-01, f@2000-01-02], [t@2000-01-03, f@2000-01-04]}');

SELECT arrowRoundtrip(tbool 't@2000-01-01') = tbool 't@2000-01-01';
SELECT arrowRoundtrip(tbool '[t@2000-01-01, f@2000-01-02, f@2000-01-03)') = tbool '[t@2000-01-01, f@2000-01-02, f@2000-01-03)';
SELECT arrowRoundtrip(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}') = tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT arrowRoundtrip(tbool '{[t@2000-01-01, f@2000-01-02], [t@2000-01-03, f@2000-01-04]}') = tbool '{[t@2000-01-01, f@2000-01-02], [t@2000-01-03, f@2000-01-04]}';

-- Temporal text: every subtype round-trips through the variable-length
-- utf8 value leaf; values of differing length exercise the offsets

SELECT arrowRoundtrip(ttext 'a@2000-01-01');
SELECT arrowRoundtrip(ttext '[xx@2000-01-01, yyy@2000-01-02, yyy@2000-01-03)');
SELECT arrowRoundtrip(ttext '[p@2000-01-01, qqqq@2000-01-02, r@2000-01-03]');
SELECT arrowRoundtrip(ttext '[solo@2000-01-01]');
SELECT arrowRoundtrip(ttext '{a@2000-01-01, bb@2000-01-02, ccc@2000-01-03}');
SELECT arrowRoundtrip(ttext 'Interp=Step;[one@2000-01-01, two@2000-01-02, three@2000-01-03]');
SELECT arrowRoundtrip(ttext '{[aa@2000-01-01, b@2000-01-02], [cccc@2000-01-03, dd@2000-01-04]}');

SELECT arrowRoundtrip(ttext 'a@2000-01-01') = ttext 'a@2000-01-01';
SELECT arrowRoundtrip(ttext '[p@2000-01-01, qqqq@2000-01-02, r@2000-01-03]') = ttext '[p@2000-01-01, qqqq@2000-01-02, r@2000-01-03]';
SELECT arrowRoundtrip(ttext '{a@2000-01-01, bb@2000-01-02, ccc@2000-01-03}') = ttext '{a@2000-01-01, bb@2000-01-02, ccc@2000-01-03}';
SELECT arrowRoundtrip(ttext '{[aa@2000-01-01, b@2000-01-02], [cccc@2000-01-03, dd@2000-01-04]}') = ttext '{[aa@2000-01-01, b@2000-01-02], [cccc@2000-01-03, dd@2000-01-04]}';

-- Temporal point: x,y,z? Struct value leaf + the SRID slot. Covers 2D/3D,
-- a non-zero SRID, geometry and geography, and every subtype

SELECT arrowRoundtrip(tgeompoint 'Point(1 2)@2000-01-01');
SELECT arrowRoundtrip(tgeompoint 'SRID=3812;Point(1 2)@2000-01-01');
SELECT arrowRoundtrip(tgeompoint 'Point(1 2 3)@2000-01-01');
SELECT arrowRoundtrip(tgeompoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03)');
SELECT arrowRoundtrip(tgeompoint 'SRID=3812;[Point(1 1 1)@2000-01-01, Point(2 2 2)@2000-01-02]');
SELECT arrowRoundtrip(tgeompoint '{Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03}');
SELECT arrowRoundtrip(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}');
SELECT arrowRoundtrip(tgeogpoint 'Point(1 2)@2000-01-01');
SELECT arrowRoundtrip(tgeogpoint 'Point(1 2 3)@2000-01-01');
SELECT arrowRoundtrip(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]');

SELECT arrowRoundtrip(tgeompoint 'Point(1 2)@2000-01-01') = tgeompoint 'Point(1 2)@2000-01-01';
SELECT arrowRoundtrip(tgeompoint 'SRID=3812;Point(1 2)@2000-01-01') = tgeompoint 'SRID=3812;Point(1 2)@2000-01-01';
SELECT arrowRoundtrip(tgeompoint 'Point(1 2 3)@2000-01-01') = tgeompoint 'Point(1 2 3)@2000-01-01';
SELECT arrowRoundtrip(tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}') = tgeompoint '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}';
SELECT arrowRoundtrip(tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]') = tgeogpoint '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]';

-- General temporal geometry and geography: the value leaf is an opaque
-- LargeBinary of each instant's EWKB, so arbitrary geometries (not only
-- points) round-trip, the SRID and the Z dimension travelling in the EWKB

SELECT arrowRoundtrip(tgeometry 'Linestring(1 1,2 2)@2000-01-01');
SELECT arrowRoundtrip(tgeometry 'Polygon((1 1,4 4,7 1,1 1))@2000-01-01');
SELECT arrowRoundtrip(tgeometry 'SRID=3812;Polygon((1 1,4 4,7 1,1 1))@2000-01-01');
SELECT arrowRoundtrip(tgeometry 'Linestring(1 1 1,2 2 2)@2000-01-01');
SELECT arrowRoundtrip(tgeometry '{Point(1 1)@2000-01-01, Linestring(1 1,3 3)@2000-01-02, Polygon((1 1,4 4,7 1,1 1))@2000-01-03}');
SELECT arrowRoundtrip(tgeometry 'Interp=Step;[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]');
SELECT arrowRoundtrip(tgeometry '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 1)@2000-01-03]');
SELECT arrowRoundtrip(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}');
SELECT arrowRoundtrip(tgeography 'Point(1 2)@2000-01-01');
SELECT arrowRoundtrip(tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]');

SELECT arrowRoundtrip(tgeometry 'Polygon((1 1,4 4,7 1,1 1))@2000-01-01') = tgeometry 'Polygon((1 1,4 4,7 1,1 1))@2000-01-01';
SELECT arrowRoundtrip(tgeometry 'SRID=3812;Polygon((1 1,4 4,7 1,1 1))@2000-01-01') = tgeometry 'SRID=3812;Polygon((1 1,4 4,7 1,1 1))@2000-01-01';
SELECT arrowRoundtrip(tgeometry 'Linestring(1 1 1,2 2 2)@2000-01-01') = tgeometry 'Linestring(1 1 1,2 2 2)@2000-01-01';
SELECT arrowRoundtrip(tgeometry '{Point(1 1)@2000-01-01, Linestring(1 1,3 3)@2000-01-02, Polygon((1 1,4 4,7 1,1 1))@2000-01-03}') = tgeometry '{Point(1 1)@2000-01-01, Linestring(1 1,3 3)@2000-01-02, Polygon((1 1,4 4,7 1,1 1))@2000-01-03}';
SELECT arrowRoundtrip(tgeometry 'Interp=Step;[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]') = tgeometry 'Interp=Step;[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, Point(1 1)@2000-01-03]';
SELECT arrowRoundtrip(tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}') = tgeometry '{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], [Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}';
SELECT arrowRoundtrip(tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]') = tgeography '[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, Point(3 3)@2000-01-03]';

-------------------------------------------------------------------------------
