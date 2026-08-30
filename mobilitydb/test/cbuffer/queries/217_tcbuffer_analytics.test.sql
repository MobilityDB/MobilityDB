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
-- Simplification functions
-------------------------------------------------------------------------------

SELECT minDistSimplify(NULL::tcbuffer, 1.0);
SELECT minTimeDeltaSimplify(NULL::tcbuffer, interval '1 day');

SELECT minDistSimplify(tcbuffer 'Cbuffer(Point(1 1),0.5)@2001-01-01', 1.0);
SELECT minDistSimplify(tcbuffer '{Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(1 1),0.5)@2001-01-03}', 1.0);
SELECT minDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 2.0);

SELECT minTimeDeltaSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', interval '2 days');

SELECT maxDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0);
SELECT maxDistSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0, false);

SELECT douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0);
SELECT douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(1 1),0.5)@2001-01-01, Cbuffer(Point(2 2),0.5)@2001-01-02, Cbuffer(Point(3 1),0.5)@2001-01-03, Cbuffer(Point(4 4),0.5)@2001-01-04]', 1.0, false);

-------------------------------------------------------------------------------
-- Simplification keeps the interpolation and the sequence segmentation of the
-- value it simplifies. Restricting to the surviving timestamps would return a
-- discrete value instead, which has no value between its instants.
-------------------------------------------------------------------------------

SELECT interp(minDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));
SELECT interp(minTimeDeltaSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', interval '2 days'));
SELECT interp(maxDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));
SELECT interp(douglasPeuckerSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));

-- A step value stays step: the interpolation is preserved, not forced linear.
SELECT interp(minDistSimplify(tcbuffer 'Interp=Step;[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 2.0));

-- When nothing is dropped the value comes back unchanged, not NULL.
SELECT minDistSimplify(tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]', 0.5) =
  tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03]';

-- The gap of a sequence set survives: the result has two sequences, not one
-- run of instants spanning the gap.
SELECT numSequences(minDistSimplify(tcbuffer '{[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03], [Cbuffer(Point(9 0),1)@2001-01-05, Cbuffer(Point(20 0),1)@2001-01-06]}', 2.0));
SELECT minDistSimplify(tcbuffer '{[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-02, Cbuffer(Point(4 0),1)@2001-01-03], [Cbuffer(Point(9 0),1)@2001-01-05, Cbuffer(Point(20 0),1)@2001-01-06]}', 2.0);

-------------------------------------------------------------------------------
