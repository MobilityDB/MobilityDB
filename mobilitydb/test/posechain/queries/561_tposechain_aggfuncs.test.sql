-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
-- Aggregate functions for temporal pose chains
-------------------------------------------------------------------------------

-- extent(tposechain): mixed temporal subtypes folded together, NULLs filtered
SELECT extent(temp) FROM ( VALUES
  (NULL::tposechain),
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain '{PoseChain(Pose(Point(2 2), 0.2))@2001-01-01, PoseChain(Pose(Point(3 3), 0.3))@2001-01-02}')) t(temp);
SELECT extent(temp) FROM ( VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (NULL)) t(temp);

-- extent(tposechain): single row and all-NULL degenerate cases
SELECT extent(temp) FROM ( VALUES (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01')) t(temp);
SELECT extent(temp) FROM ( VALUES (NULL::tposechain)) t(temp);

-------------------------------------------------------------------------------

WITH Temp(temp) AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(1 1), 0.3))@2001-01-03)' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.2))@2001-01-02, PoseChain(Pose(Point(1 1), 0.4))@2001-01-04)' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.3))@2001-01-03, PoseChain(Pose(Point(1 1), 0.5))@2001-01-05)' )
SELECT tCount(Temp) FROM Temp;

WITH Temp(temp) AS (
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01, PoseChain(Pose(Point(1 1), 0.3))@2001-01-03]' UNION
  SELECT tposechain '[PoseChain(Pose(Point(1 1), 0.2))@2001-01-02, PoseChain(Pose(Point(1 1), 0.4))@2001-01-04]' )
SELECT wCount(Temp, interval '2 days') FROM Temp;

-------------------------------------------------------------------------------

SELECT asText(merge(temp)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(temp);
SELECT asText(mergeAgg(temp)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(temp);

-------------------------------------------------------------------------------

SELECT asText(appendInstant(inst)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(inst);
SELECT asText(appendInstantAgg(inst)) FROM (VALUES
  (tposechain 'PoseChain(Pose(Point(1 1), 0.1))@2001-01-01'),
  (tposechain 'PoseChain(Pose(Point(2 2), 0.2))@2001-01-02')) t(inst);

SELECT asText(appendSequence(seq)) FROM (VALUES
  (tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01]'),
  (tposechain '[PoseChain(Pose(Point(2 2), 0.2))@2001-01-02]')) t(seq);
SELECT asText(appendSequenceAgg(seq)) FROM (VALUES
  (tposechain '[PoseChain(Pose(Point(1 1), 0.1))@2001-01-01]'),
  (tposechain '[PoseChain(Pose(Point(2 2), 0.2))@2001-01-02]')) t(seq);

-------------------------------------------------------------------------------
