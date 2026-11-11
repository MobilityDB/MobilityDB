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
-- Temporal equal
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= pose 'Pose(Point(1 1), 0.5)';

SELECT pose 'Pose(Point(1 1), 0.5)' #= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT pose 'Pose(Point(1 1), 0.5)' #= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT pose 'Pose(Point(1 1), 0.5)' #= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT pose 'Pose(Point(1 1), 0.5)' #= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #= tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #<> pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> pose 'Pose(Point(1 1), 0.5)';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> pose 'Pose(Point(1 1), 0.5)';

SELECT pose 'Pose(Point(1 1), 0.5)' #<> tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT pose 'Pose(Point(1 1), 0.5)' #<> tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT pose 'Pose(Point(1 1), 0.5)' #<> tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT pose 'Pose(Point(1 1), 0.5)' #<> tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #<> tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #<> tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #<> tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #<> tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}' #<> tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]' #<> tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> tpose 'Pose(Point(1 1), 0.5)@2001-01-01';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> tpose '{Pose(Point(1 1), 0.3)@2001-01-01, Pose(Point(1 1), 0.5)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03}';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03]';
SELECT tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}' #<> tpose '{[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-02, Pose(Point(1 1), 0.5)@2001-01-03], [Pose(Point(2 2), 0.6)@2001-01-04, Pose(Point(2 2), 0.6)@2001-01-05]}';

/* Errors */
SELECT tpose 'Pose(Point(1 1), 0.5)@2001-01-01' #= tpose 'SRID=3812;Pose(Point(1 1), 0.5)@2001-01-01';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box, and two values

SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' && stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 04:00, 2001-01-01 06:00])';
SELECT stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 08:00, 2001-01-01 10:00])' ?&& tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(10,10))';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01 00:00, Pose(Point(10 10), 0)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(9,9))';
-- Two temporal poses at the same position with different orientations
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-03]' ?&& tpose '[Pose(Point(1 1), 0.6)@2001-01-01, Pose(Point(1 1), 0.8)@2001-01-03]';
SELECT tpose '[Pose(Point(1 1), 0.2)@2001-01-01, Pose(Point(1 1), 0.4)@2001-01-03]' ?= tpose '[Pose(Point(1 1), 0.6)@2001-01-01, Pose(Point(1 1), 0.8)@2001-01-03]';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01, Pose(Point(2 2), 0)@2001-01-03]' %&& tpose '[Pose(Point(2 0), 0)@2001-01-01, Pose(Point(0 2), 0)@2001-01-03]';
SELECT tpose '[Pose(Point(0 0), 0)@2001-01-01, Pose(Point(2 2), 0)@2001-01-03]' ?&& tpose '[Pose(Point(2 0), 0)@2001-01-01, Pose(Point(0 2), 0)@2001-01-03]';
/* Errors */
SELECT tpose 'Pose(Point(0 0), 0)@2001-01-01' ?&& stbox 'SRID=4326;STBOX X((0,0),(1,1))';
