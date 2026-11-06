-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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
-- An error of MEOS raises the SQLSTATE PostgreSQL raises for an error of its
-- class. Each MobilityDB statement follows a PostgreSQL statement whose error
-- is of the same class, and only the state of each error is shown
-------------------------------------------------------------------------------

\set VERBOSITY sqlstate

-- A number out of range
SELECT 2000000000 + 2000000000;
SELECT tsum(temp) FROM (VALUES
('2000000000@2001-01-01'::tint),('2000000000@2001-01-01'::tint)) t(temp);
SELECT 1e300::float8 * 1e300::float8;
SELECT integral(tfloat '[1e308@2001-01-01, 1e308@2001-01-03]');

-- A date or a time out of range
SELECT timestamptz '294277-01-01 00:00:00+00';
SELECT tstzspan '[294277-01-01 00:00:00+00, 294277-01-02 00:00:00+00]';

-- A division by zero
SELECT 1 / 0;
SELECT tint '[1@2001-01-01, 2@2001-01-02]' / 0;

-- An invalid argument value
SELECT generate_series(1, 2, 0);
SELECT span(timestamptz '2001-01-02', '2001-01-01');

-- An input that is not the text of a value of its type
SELECT int4 'abc';
SELECT tint 'abc';

-- A feature not supported
SELECT tgeompoint '[Point(-1e200 0)@2001-01-01, Point(-1e200 4)@2001-01-02]' <->
  geometry 'Linestring(1e200 -1,1e200 1)';

-- An error with no class of its own raises the internal error
SELECT tintFromMFJSON('{"type":"MovingInteger"}');

\set VERBOSITY default

-------------------------------------------------------------------------------
