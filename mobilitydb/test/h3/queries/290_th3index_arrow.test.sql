-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
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

-- Round-trip a temporal H3 index through the Arrow C Data Interface. An H3
-- cell index is a 64-bit value, so it uses the UInt64 "L" value leaf,
-- distinct from the Int64 "l" leaf of temporal big integer. The result
-- must equal the input for every subtype.

SELECT arrowRoundtrip(th3index '831c02fffffffff@2001-01-01');
SELECT arrowRoundtrip(th3index '590464338553208831@2001-01-01');
SELECT arrowRoundtrip(th3index '[831c02fffffffff@2001-01-01]');
SELECT arrowRoundtrip(th3index '{831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03}');
SELECT arrowRoundtrip(th3index '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]');
SELECT arrowRoundtrip(th3index 'Interp=Step;[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]');
SELECT arrowRoundtrip(th3index '{[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02], [880326b885fffff@2001-01-03, 880326b88dfffff@2001-01-04]}');

SELECT arrowRoundtrip(th3index '831c02fffffffff@2001-01-01') = th3index '831c02fffffffff@2001-01-01';
SELECT arrowRoundtrip(th3index '590464338553208831@2001-01-01') = th3index '590464338553208831@2001-01-01';
SELECT arrowRoundtrip(th3index '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]') = th3index '[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]';
SELECT arrowRoundtrip(th3index 'Interp=Step;[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]') = th3index 'Interp=Step;[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02, 880326b885fffff@2001-01-03]';
SELECT arrowRoundtrip(th3index '{[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02], [880326b885fffff@2001-01-03, 880326b88dfffff@2001-01-04]}') = th3index '{[831c02fffffffff@2001-01-01, 831c00fffffffff@2001-01-02], [880326b885fffff@2001-01-03, 880326b88dfffff@2001-01-04]}';

-------------------------------------------------------------------------------
