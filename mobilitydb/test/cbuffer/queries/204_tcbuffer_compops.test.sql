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

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-- A segment passing through the circular buffer inside it
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)' #= cbuffer 'Cbuffer(Point(1 1), 0.3)';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.3)' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)';
SELECT tcbuffer '[Cbuffer(Point(0.1 1), 0.5)@2001-01-01, Cbuffer(Point(0.4 1), 0.5)@2001-01-04)' #= cbuffer 'Cbuffer(Point(0.3 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(0.2 0.2), 0.2)@2001-01-01, Cbuffer(Point(0.4 0.4), 0.4)@2001-01-03)' #= cbuffer 'Cbuffer(Point(0.3 0.3), 0.3)';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-- A segment passing through the circular buffer inside it
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)' #<> cbuffer 'Cbuffer(Point(1 1), 0.3)';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.3)' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)';
SELECT tcbuffer '[Cbuffer(Point(0.1 1), 0.5)@2001-01-01, Cbuffer(Point(0.4 1), 0.5)@2001-01-04)' #<> cbuffer 'Cbuffer(Point(0.3 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(0.2 0.2), 0.2)@2001-01-01, Cbuffer(Point(0.4 0.4), 0.4)@2001-01-03)' #<> cbuffer 'Cbuffer(Point(0.3 0.3), 0.3)';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

/* Errors */
SELECT tcbuffer 'Cbuffer(Point(1 1),1)@2001-01-01' #= tcbuffer 'SRID=3812;Cbuffer(Point(1 1),1)@2001-01-01';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box

SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' && stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 04:00, 2001-01-01 06:00])';
SELECT stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 08:00, 2001-01-01 10:00])' ?&& tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' %&& stbox 'STBOX X((-1,-1),(11,11))';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(10,10))';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(10 10), 1)@2001-01-02]' ?&& stbox 'STBOX T([2001-01-03, 2001-01-04])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(10 10), 1)@2001-01-02]' %&& stbox 'STBOX T([2001-01-01, 2001-01-04])';
/* Errors */
SELECT tcbuffer 'Cbuffer(Point(0 0), 1)@2001-01-01' ?&& stbox 'SRID=4326;STBOX X((0,0),(1,1))';
