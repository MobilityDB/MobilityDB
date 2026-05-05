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

-------------------------------------------------------------------------------
-- Box functions
-------------------------------------------------------------------------------

SELECT spaceBoxes(NULL::tcbuffer, 2.0);

SELECT spaceBoxes(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', 2.0);
SELECT spaceBoxes(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', 2.0, 4.0);
SELECT timeBoxes(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', interval '2 days');
SELECT spaceTimeBoxes(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', 2.0, interval '2 days');

-------------------------------------------------------------------------------
-- Split functions
-------------------------------------------------------------------------------

SELECT (sp).point, asText((sp).tcbuffer) FROM
  (SELECT spaceSplit(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', 2.0) AS sp) t
  ORDER BY 1;

SELECT (sp).point, (sp).time, asText((sp).tcbuffer) FROM
  (SELECT spaceTimeSplit(tcbuffer '[Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(5 5),0.5)@2000-01-05]', 2.0, interval '2 days') AS sp) t
  ORDER BY 2, 1;

-------------------------------------------------------------------------------
