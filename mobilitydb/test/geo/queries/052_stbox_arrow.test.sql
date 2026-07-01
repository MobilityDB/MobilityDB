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

-- Round-trip the spatiotemporal box, a finite subset of the space x time
-- product domain, through the Arrow C Data Interface. The result must equal
-- the input for every flag-gated combination of the spatial (2D / 3D),
-- temporal, geodetic and spatial-reference dimensions.

SELECT arrowRoundtrip(stbox 'STBOX X((1,1),(2,2))');
SELECT arrowRoundtrip(stbox 'STBOX Z((1,1,1),(2,2,2))');
SELECT arrowRoundtrip(stbox 'STBOX T([2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(stbox 'STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(stbox 'STBOX ZT(((1,1,1),(5,5,5)),[2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(stbox 'SRID=5676;STBOX X((1,1),(2,2))');
SELECT arrowRoundtrip(stbox 'SRID=4326;STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])');
SELECT arrowRoundtrip(stbox 'GEODSTBOX T([2000-01-01,2000-01-02])');
SELECT arrowRoundtrip(stbox 'GEODSTBOX X((1,1),(1,1))');
SELECT arrowRoundtrip(stbox 'SRID=4326;GEODSTBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])');

-- Value identity over the flag-gated spatiotemporal-box surface

SELECT arrowRoundtrip(stbox 'STBOX X((1,1),(2,2))') = stbox 'STBOX X((1,1),(2,2))';
SELECT arrowRoundtrip(stbox 'STBOX Z((1,1,1),(2,2,2))') = stbox 'STBOX Z((1,1,1),(2,2,2))';
SELECT arrowRoundtrip(stbox 'STBOX T([2000-01-01,2000-01-05])') = stbox 'STBOX T([2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(stbox 'STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])') = stbox 'STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(stbox 'STBOX ZT(((1,1,1),(5,5,5)),[2000-01-01,2000-01-05])') = stbox 'STBOX ZT(((1,1,1),(5,5,5)),[2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(stbox 'SRID=5676;STBOX X((1,1),(2,2))') = stbox 'SRID=5676;STBOX X((1,1),(2,2))';
SELECT arrowRoundtrip(stbox 'SRID=4326;STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])') = stbox 'SRID=4326;STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])';
SELECT arrowRoundtrip(stbox 'GEODSTBOX T([2000-01-01,2000-01-02])') = stbox 'GEODSTBOX T([2000-01-01,2000-01-02])';
SELECT arrowRoundtrip(stbox 'GEODSTBOX X((1,1),(1,1))') = stbox 'GEODSTBOX X((1,1),(1,1))';
SELECT arrowRoundtrip(stbox 'SRID=4326;GEODSTBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])') = stbox 'SRID=4326;GEODSTBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])';

-------------------------------------------------------------------------------
