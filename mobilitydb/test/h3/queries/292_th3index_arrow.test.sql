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

-- Round-trip a temporal H3 index through the Arrow C Data Interface.
-- The value leaf is the 64-bit H3 cell index. The result must equal the input
-- for every subtype.

SELECT arrowRoundtrip(th3index '8a2a100d6447fff@2012-01-01 08:00:00');
SELECT arrowRoundtrip(th3index '{8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00}');
SELECT arrowRoundtrip(th3index '[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00]');
SELECT arrowRoundtrip(th3index '{[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00], [8a2a100d6457fff@2001-01-01 08:10:00, 8a2a100d645ffff@2001-01-01 08:15:00]}');

SELECT arrowRoundtrip(th3index '8a2a100d6447fff@2012-01-01 08:00:00') = th3index '8a2a100d6447fff@2012-01-01 08:00:00';
SELECT arrowRoundtrip(th3index '{8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00}') = th3index '{8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00}';
SELECT arrowRoundtrip(th3index '[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00]') = th3index '[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00, 8a2a100d6457fff@2001-01-01 08:06:00]';
SELECT arrowRoundtrip(th3index '{[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00], [8a2a100d6457fff@2001-01-01 08:10:00, 8a2a100d645ffff@2001-01-01 08:15:00]}') = th3index '{[8a2a100d6447fff@2001-01-01 08:00:00, 8a2a100d644ffff@2001-01-01 08:05:00], [8a2a100d6457fff@2001-01-01 08:10:00, 8a2a100d645ffff@2001-01-01 08:15:00]}';

-------------------------------------------------------------------------------
