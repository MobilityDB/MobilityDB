-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- Temporal text concatenation
-------------------------------------------------------------------------------

SELECT text 'A' || ttext 'AA@2000-01-01';
SELECT text 'A' || ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}';
SELECT text 'A' || ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]';
SELECT text 'A' || ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}';

SELECT ttext 'AA@2000-01-01' || text 'A';
SELECT ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}' || text 'A';
SELECT ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]' || text 'A';
SELECT ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}' || text 'A';

SELECT ttext 'AA@2000-01-01' || ttext 'AA@2000-01-01';
SELECT ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}' || ttext 'AA@2000-01-01';
SELECT ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]' || ttext 'AA@2000-01-01';
SELECT ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}' || ttext 'AA@2000-01-01';
SELECT ttext 'AA@2000-01-01' || ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}';
SELECT ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}' || ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}';
SELECT ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]' || ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}';
SELECT ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}' || ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}';
SELECT ttext 'AA@2000-01-01' || ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]';
SELECT ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}' || ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]';
SELECT ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]' || ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]';
SELECT ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}' || ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]';
SELECT ttext 'AA@2000-01-01' || ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}';
SELECT ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}' || ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}';
SELECT ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]' || ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}';
SELECT ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}' || ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT upper(ttext 'AA@2000-01-01');
SELECT upper(ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}');
SELECT upper(ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]');
SELECT upper(ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}');

SELECT lower(ttext 'AA@2000-01-01');
SELECT lower(ttext '{AA@2000-01-01, BB@2000-01-02, AA@2000-01-03}');
SELECT lower(ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]');
SELECT lower(ttext '{[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03],[CC@2000-01-04, CC@2000-01-05]}');

-------------------------------------------------------------------------------
