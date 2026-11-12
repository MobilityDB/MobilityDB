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

-- Temporal spatial relationships between two ts2cell values. Each converts
-- both operands to the temporal geography of their boundary and delegates to
-- the temporal relationship of two temporal geographies, answered on the
-- sphere, so the answer must equal the same relationship asked of
-- cellToBoundary directly.

-------------------------------------------------------------------------------
-- The delegation identity
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]',
    ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]'))
  = asText(tIntersects(cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]'),
      cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]')));
SELECT asText(tDisjoint(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]',
    ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]'))
  = asText(tDisjoint(cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c5@2001-01-02]'),
      cellToBoundary(ts2cell '[47c3c3@2001-01-01, 47c3c3@2001-01-02]')));

-------------------------------------------------------------------------------
-- The answers themselves
-------------------------------------------------------------------------------

SELECT asText(tIntersects(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]'));
SELECT asText(tDisjoint(ts2cell '[47c3c3@2001-01-01]', ts2cell '[47c3c3@2001-01-01]'));

-------------------------------------------------------------------------------
