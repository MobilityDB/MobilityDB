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

-----------------------------------------------------------------------
-- Boolean operators
-----------------------------------------------------------------------

SELECT TRUE & tbool 't@2000-01-01';
SELECT TRUE & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT TRUE & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT TRUE & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT FALSE & tbool 't@2000-01-01';
SELECT FALSE & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT FALSE & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT FALSE & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' & TRUE;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' & TRUE;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' & TRUE;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' & TRUE;

SELECT tbool 't@2000-01-01' & tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' & tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' & tbool 't@2000-01-01';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' & tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' & tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' & tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' & tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT TRUE | tbool 't@2000-01-01';
SELECT TRUE | tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT TRUE | tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT TRUE | tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' | TRUE;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' | TRUE;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' | TRUE;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' | TRUE;

SELECT tbool 't@2000-01-01' | tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' | tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' | tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' | tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' | tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' | tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' | tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' | tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' | tbool 't@2000-01-01';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' | tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' | tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' | tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' | tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' | tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' | tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' | tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT ~ tbool 't@2000-01-01';
SELECT ~ tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT ~ tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT ~ tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

-------------------------------------------------------------------------------

