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

-- Round-trip a temporal circular buffer through the Arrow C Data Interface.
-- The value leaf is a Struct{x,y,r} (2D centre plus radius); the SRID is
-- preserved. The result must equal the input for every subtype.

SELECT arrowRoundtrip(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01');
SELECT arrowRoundtrip(tcbuffer 'SRID=3812;Cbuffer(Point(1 2),1.5)@2000-01-01');
SELECT arrowRoundtrip(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.7)@2000-01-02, Cbuffer(Point(1 1),0.9)@2000-01-03}');
SELECT arrowRoundtrip(tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(3 3),0.7)@2000-01-03]');
SELECT arrowRoundtrip(tcbuffer 'SRID=3812;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.6)@2000-01-02]');
SELECT arrowRoundtrip(tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(4 4),0.8)@2000-01-05]}');

SELECT arrowRoundtrip(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01') = tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT arrowRoundtrip(tcbuffer 'SRID=3812;Cbuffer(Point(1 2),1.5)@2000-01-01') = tcbuffer 'SRID=3812;Cbuffer(Point(1 2),1.5)@2000-01-01';
SELECT arrowRoundtrip(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.7)@2000-01-02, Cbuffer(Point(1 1),0.9)@2000-01-03}') = tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.7)@2000-01-02, Cbuffer(Point(1 1),0.9)@2000-01-03}';
SELECT arrowRoundtrip(tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(3 3),0.7)@2000-01-03]') = tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(3 3),0.7)@2000-01-03]';
SELECT arrowRoundtrip(tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(4 4),0.8)@2000-01-05]}') = tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02], [Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(4 4),0.8)@2000-01-05]}';

-------------------------------------------------------------------------------
