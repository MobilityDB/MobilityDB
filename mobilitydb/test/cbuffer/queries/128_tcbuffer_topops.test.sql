﻿-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2024, PostGIS contributors
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

SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)'), 6);
SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)', timestamptz '2000-01-01'), 6);
SELECT round(stbox(cbuffer 'Cbuffer(Point(1 1),0.5)', tstzspan '[2000-01-01, 2000-01-02]'), 6);

SELECT round(stbox(tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01'), 6);
SELECT round(stbox(tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}'), 6);
SELECT round(stbox(tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]'), 6);
SELECT round(stbox(tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}'), 6);

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' && tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && tstzspan '[2000-01-01,2000-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' && tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' @> tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> tstzspan '[2000-01-01,2000-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' @> tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' <@ tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ tstzspan '[2000-01-01,2000-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' <@ tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' ~= tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= tstzspan '[2000-01-01,2000-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' ~= tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))' -|- tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- tstzspan '[2000-01-01,2000-01-02]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- tstzspan '[2000-01-01,2000-01-02]';

SELECT tcbuffer 'SRID=5676;Cbuffer(Point(1 1),0.5)@2000-01-01' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';
SELECT tcbuffer 'SRID=5676;{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- stbox 'SRID=5676;STBOX X((1.0,2.0),(1.0,2.0))';

SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1),0.5)@2000-01-01' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{Cbuffer(Point(1 1),0.5)@2000-01-01, Cbuffer(Point(2 2),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03}' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03]' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';
SELECT tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}' -|- tcbuffer '{[Cbuffer(Point(1 1),0.4)@2000-01-01, Cbuffer(Point(1 1),0.5)@2000-01-02, Cbuffer(Point(1 1),0.7)@2000-01-03],[Cbuffer(Point(3 3),0.5)@2000-01-04, Cbuffer(Point(3 3),0.5)@2000-01-05]}';

-------------------------------------------------------------------------------
-- Selectivity tests
-------------------------------------------------------------------------------

  SELECT COUNT(*) FROM tbl_tcbuffer WHERE temp && stbox 'SRID=5676;STBOX X((0,0),(50,50))';
SELECT COUNT(*) FROM tbl_tcbuffer WHERE temp && tcbuffer 'SRID=5676;[Cbuffer(Point(1 1),0.1)@2001-06-01, Cbuffer(Point(1 1),0.9)@2001-07-01]';

-------------------------------------------------------------------------------
