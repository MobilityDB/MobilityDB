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
-- Tests for extensions of range data type.
-- File Range.c
-------------------------------------------------------------------------------

SELECT round(floatrange '[1.123456789,2.123456789]',6);
SELECT round(floatrange '[,2.123456789]',6);
SELECT round(floatrange '[-inf,2.123456789]',6);
select round(floatrange '[1.123456789,inf]',6);
SELECT round(floatrange '[1.123456789,]',6);

-------------------------------------------------------------------------------

SELECT intrange 'empty' << 5;
SELECT intrange '[3,5)' << 5;
SELECT 5 << intrange 'empty';
SELECT 5 << intrange '[3,5)';

SELECT intrange 'empty' >> 5;
SELECT intrange '[3,5)' >> 5;
SELECT 5 >> intrange 'empty';
SELECT 5 >> intrange '[3,5)';

SELECT intrange 'empty' &< 5;
SELECT intrange '[3,5)' &< 5;
SELECT 5 &< intrange 'empty';
SELECT 5 &< intrange '[3,5)';

SELECT intrange 'empty' &> 5;
SELECT intrange '[3,5)' &> 5;
SELECT 5 &> intrange 'empty';
SELECT 5 &> intrange '[3,5)';

SELECT intrange 'empty' -|- 5;
SELECT intrange '[3,5)' -|- 5;
SELECT 5 -|- intrange 'empty';
SELECT 5 -|- intrange '[3,5)';

-------------------------------------------------------------------------------

SELECT floatrange 'empty' << 5.5;
SELECT floatrange '[3.5, 5.5]' << 5.5;
SELECT 5.5 << floatrange 'empty';
SELECT 5.5 << floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' >> 5.5;
SELECT floatrange '[3.5, 5.5]' >> 5.5;
SELECT 5.5 >> floatrange 'empty';
SELECT 5.5 >> floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' &< 5.5;
SELECT floatrange '[3.5, 5.5]' &< 5.5;
SELECT 5.5 &< floatrange 'empty';
SELECT 5.5 &< floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' &> 5.5;
SELECT floatrange '[3.5, 5.5]' &> 5.5;
SELECT 5.5 &> floatrange 'empty';
SELECT 5.5 &> floatrange '[3.5, 5.5]';

SELECT floatrange 'empty' -|- 5.5;
SELECT floatrange '[3.5, 5.5]' -|- 5.5;
SELECT 5.5 -|- floatrange 'empty';
SELECT 5.5 -|- floatrange '[3.5, 5.5]';

-------------------------------------------------------------------------------
