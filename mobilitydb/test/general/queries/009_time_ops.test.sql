-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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
-- File time_ops.c
-------------------------------------------------------------------------------

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' @> timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' @> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' @> tstzset '{2000-01-02, 2000-01-04}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' @> timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-02]' @> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' @> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' @> timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' @> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-02, 2000-01-04],(2000-01-05, 2000-01-06),[2000-01-07, 2000-01-08]}' @> tstzspan '(2000-01-05, 2000-01-06)';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}' @> tstzspanset '{[2000-01-02, 2000-01-06]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}' @> tstzspanset '{[2000-01-03, 2000-01-04],[2000-01-05, 2000-01-06]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}' @> tstzspanset '{[2000-01-05, 2000-01-06]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-07]}' @> tstzspanset '{[2000-01-05, 2000-01-06]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06],[2000-01-07, 2000-01-08]}' @> tstzspanset '{[2000-01-05, 2000-01-07]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-07]}' @> tstzspanset '{[2000-01-04, 2000-01-08]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-04], [2000-01-05, 2000-01-06]}' @> tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' @> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' <@ tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestamptz '2000-01-01' <@ tstzspan '[2000-01-01, 2000-01-02]';
SELECT timestamptz '2000-01-01' <@ tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' <@ tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' <@ tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' <@ tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' <@ tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' <@ tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' && tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01, 2000-01-03}' && tstzset '{2000-01-02, 2000-01-04}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' && tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' && tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';
SELECT tstzspan '[2000-01-03, 2000-01-04]' && tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-05, 2000-01-06]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' && tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' && tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-05, 2000-01-06]}' && tstzspanset '{[2000-01-03, 2000-01-04], [2000-01-07, 2000-01-08]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02), [2000-01-05, 2000-01-06]}' && tstzspanset '{[2000-01-02,2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' -|- tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' -|- tstzspan '(2000-01-01, 2000-01-03]';

SELECT timestamptz '2000-01-01' -|- tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-01' -|- tstzspanset '{(2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzspan '[2000-01-01, 2000-01-03]' -|- timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' -|- tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' -|- tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' -|- timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' -|- tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' -|- tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' = tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT tstzset '{2000-01-01, 2000-01-03}' = tstzset '{2000-01-02, 2000-01-04}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' = tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' = tstzspan '(2000-01-01, 2000-01-02]';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' = tstzspanset '{[2000-01-01, 2000-01-02]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' = tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-05, 2000-01-06]}' = tstzspanset '{[2000-01-03, 2000-01-04], [2000-01-07, 2000-01-08]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02), [2000-01-05, 2000-01-06]}' = tstzspanset '{[2000-01-02,2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' <<# tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestamptz '2000-01-01' <<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT timestamptz '2000-01-01' <<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' <<# timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' <<# tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' <<# timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-02]' <<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' <<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' <<# timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' <<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' <<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' &<# tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestamptz '2000-01-01' &<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT timestamptz '2000-01-01' &<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' &<# timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' &<# tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' &<# timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-02]' &<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' &<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' &<# timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' &<# tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' &<# tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' #>> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestamptz '2000-01-01' #>> tstzspan '[2000-01-01, 2000-01-02]';
SELECT timestamptz '2000-01-01' #>> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' #>> timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' #>> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' #>> timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-02]' #>> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' #>> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #>> timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #>> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #>> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' #&> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';
SELECT timestamptz '2000-01-01' #&> tstzspan '[2000-01-01, 2000-01-02]';
SELECT timestamptz '2000-01-01' #&> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' #&> timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-02, 2000-01-03}' #&> tstzset '{2000-01-01, 2000-01-02, 2000-01-03}';

SELECT tstzspan '[2000-01-01, 2000-01-02]' #&> timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-02]' #&> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspan '[2000-01-01, 2000-01-02]' #&> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #&> timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #&> tstzspan '[2000-01-01, 2000-01-02]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}' #&> tstzspanset '{[2000-01-01, 2000-01-02], [2000-01-03, 2000-01-04]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' + tstzset '{2000-01-02, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' + tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-05' + tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-06' + tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' + tstzspan '[2000-01-02, 2000-01-03]';
SELECT timestamptz '2000-01-01' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' + tstzspan '(2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-02' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' + tstzspan '[2000-01-01, 2000-01-03)';
SELECT timestamptz '2000-01-05' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' + tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-01' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-03' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-05, 2000-01-05]}';
SELECT timestamptz '2000-01-05' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-06' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' + timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' + tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' + tstzset '{2000-01-03, 2000-01-05, 2000-01-07}';

SELECT tstzspan '[2000-01-01, 2000-01-03]' + timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' + tstzspan '(2000-01-03, 2000-01-05]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' + tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' + timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' + tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' + tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';


SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-01,2000-01-02]';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-05,2000-01-06]';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-09,2000-01-10]';

SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-01,2000-01-03)';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-05,2000-01-07)';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '(2000-01-08,2000-01-10]';

SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-01,2000-01-03]';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-05,2000-01-07]';
SELECT tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-08,2000-01-10]';

SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-03,2000-01-06]';
SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-04,2000-01-06]';
SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-03,2000-01-05]';

SELECT tstzspanset '{[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-01,2000-01-09]';
SELECT tstzspanset '{[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-01,2000-01-06]';
SELECT tstzspanset '{[2000-01-04,2000-01-05],[2000-01-07,2000-01-08]}' + tstzspan '[2000-01-06,2000-01-09]';

SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-05,2000-01-06]}' + tstzspanset '{[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}';
SELECT tstzspanset '{[2000-01-01,2000-01-02],[2000-01-05,2000-01-06]}' + tstzspanset '{[2000-01-01,2000-01-02],[2000-01-03,2000-01-04],[2000-01-07,2000-01-08]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04], [2000-01-06, 2000-01-07]}' + tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}' + tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04], [2000-01-06, 2000-01-07]}';

SELECT tstzspanset '{[2000-01-05, 2000-01-07], [2000-01-08, 2000-01-09], [2000-01-10, 2000-01-12]}' + tstzspanset '{[2000-01-06, 2000-01-11]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' - tstzset '{2000-01-02, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' - tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-05' - tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-06' - tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' - tstzspan '[2000-01-02, 2000-01-03]';
SELECT timestamptz '2000-01-01' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' - tstzspan '(2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-02' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' - tstzspan '[2000-01-01, 2000-01-03)';
SELECT timestamptz '2000-01-05' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' - tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-01' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-03' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-05, 2000-01-05]}';
SELECT timestamptz '2000-01-05' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-06' - tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzset '{2000-01-01}' - timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' - timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' - timestamptz '2000-01-02';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' - tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' - tstzset '{2000-01-03, 2000-01-05, 2000-01-07}';

SELECT tstzspan '[2000-01-01, 2000-01-01]' - timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' - timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' - tstzspan '(2000-01-03, 2000-01-05]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' - tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-04, 2000-01-05]}';
SELECT tstzspan '[2000-01-01, 2000-01-03]' - tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';


SELECT tstzspan '[2000-01-02, 2000-01-04]' - timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-02, 2000-01-04]' - timestamptz '2000-01-02';
SELECT tstzspan '[2000-01-02, 2000-01-04]' - timestamptz '2000-01-03';
SELECT tstzspan '[2000-01-02, 2000-01-04]' - timestamptz '2000-01-04';
SELECT tstzspan '[2000-01-02, 2000-01-04]' - timestamptz '2000-01-05';
SELECT tstzspan '(2000-01-02, 2000-01-04)' - timestamptz '2000-01-01';
SELECT tstzspan '(2000-01-02, 2000-01-04)' - timestamptz '2000-01-02';
SELECT tstzspan '(2000-01-02, 2000-01-04)' - timestamptz '2000-01-03';
SELECT tstzspan '(2000-01-02, 2000-01-04)' - timestamptz '2000-01-04';
SELECT tstzspan '(2000-01-02, 2000-01-04)' - timestamptz '2000-01-05';

SELECT tstzspanset '{[2000-01-01, 2000-01-01]}' - timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' - timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03]}' - tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' - tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' - tstzspanset '{[2000-01-04, 2000-01-05]}';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' - tstzspanset '{[2000-01-01, 2000-01-03]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' * tstzset '{2000-01-02, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' * tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-05' * tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-06' * tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' * tstzspan '[2000-01-02, 2000-01-03]';
SELECT timestamptz '2000-01-01' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' * tstzspan '(2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-02' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' * tstzspan '[2000-01-01, 2000-01-03)';
SELECT timestamptz '2000-01-05' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' * tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-01' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-03' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-05, 2000-01-05]}';
SELECT timestamptz '2000-01-05' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-06' * tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' * timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' * tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' * tstzset '{2000-01-03, 2000-01-05, 2000-01-07}';
SELECT tstzset '{2000-01-01, 2000-01-03}' * tstzset '{2000-01-02, 2000-01-04}';

SELECT tstzspan '[2000-01-01, 2000-01-03]' * timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' * tstzspan '(2000-01-03, 2000-01-05]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' * tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspan '[2000-01-03, 2000-01-04]' * tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' * timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' * tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}' * tstzspan '[2000-01-01, 2000-01-04]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' * tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspanset '{[2000-01-03, 2000-01-04],[2000-01-07, 2000-01-08]}' * tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}';

-------------------------------------------------------------------------------

SELECT timestamptz '2000-01-01' <-> timestamptz '2000-01-01';
SELECT timestamptz '2000-01-01' <-> timestamptz '2000-01-02';
SELECT timestamptz '2000-01-01' <-> tstzset '{2000-01-02, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' <-> tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-05' <-> tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-06' <-> tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT timestamptz '2000-01-01' <-> tstzspan '[2000-01-02, 2000-01-03]';
SELECT timestamptz '2000-01-01' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' <-> tstzspan '(2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-02' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-03' <-> tstzspan '[2000-01-01, 2000-01-03)';
SELECT timestamptz '2000-01-05' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT timestamptz '2000-01-01' <-> tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-01' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-03' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-04' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-05, 2000-01-05]}';
SELECT timestamptz '2000-01-05' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT timestamptz '2000-01-06' <-> tstzspanset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}';

SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' <-> timestamptz '2000-01-01';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' <-> tstzset '{2000-01-01, 2000-01-03, 2000-01-05}';
SELECT tstzset '{2000-01-01, 2000-01-03, 2000-01-05}' <-> tstzset '{2000-01-03, 2000-01-05, 2000-01-07}';
SELECT tstzset '{2000-01-01, 2000-01-03}' <-> tstzset '{2000-01-02, 2000-01-04}';

SELECT tstzspan '[2000-01-01, 2000-01-03]' <-> timestamptz '2000-01-01';
SELECT tstzspan '[2000-01-01, 2000-01-03]' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' <-> tstzspan '(2000-01-03, 2000-01-05]';
SELECT tstzspan '[2000-01-01, 2000-01-03]' <-> tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspan '[2000-01-03, 2000-01-04]' <-> tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}';

SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' <-> timestamptz '2000-01-01';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' <-> tstzspan '[2000-01-01, 2000-01-03]';
SELECT tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}' <-> tstzspan '[2000-01-01, 2000-01-04]';
SELECT tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}' <-> tstzspanset '{[2000-01-01, 2000-01-03],[2000-01-04, 2000-01-05]}';
SELECT tstzspanset '{[2000-01-03, 2000-01-04],[2000-01-07, 2000-01-08]}' <-> tstzspanset '{[2000-01-01, 2000-01-02],[2000-01-05, 2000-01-06]}';

-------------------------------------------------------------------------------
-- Nearest neighbor search
ANALYZE tbl_tstzspan_big;

CREATE INDEX tbl_tstzspan_big_quadtree_idx ON tbl_tstzspan_big USING SPGIST(p);

-- EXPLAIN ANALYZE
SELECT p <-> timestamptz '2001-06-01' FROM tbl_tstzspan_big ORDER BY 1 LIMIT 3;
SELECT p <-> tstzspan '[2001-06-01, 2001-07-01]' FROM tbl_tstzspan_big ORDER BY 1 LIMIT 3;
SELECT p <-> tstzspanset '{[2001-01-01, 2001-01-15], [2001-02-01, 2001-02-15]}' FROM tbl_tstzspan_big ORDER BY 1 LIMIT 3;

DROP INDEX tbl_tstzspan_big_quadtree_idx;

-------------------------------------------------------------------------------
-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tstzset_big_rtree_idx ON tbl_tstzset_big USING gist(t);
CREATE INDEX tbl_tstzspan_big_rtree_idx ON tbl_tstzspan_big USING gist(p);
CREATE INDEX tbl_tstzspanset_big_rtree_idx ON tbl_tstzspanset_big USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE t && tstzset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspanset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps && tstzspanset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE t @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> timestamptz '2001-06-01';

SELECT COUNT(*) FROM tbl_tstzset_big WHERE t @> tstzset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspanset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> tstzspanset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE timestamptz '2001-06-01' <@ t;
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE timestamptz '2001-06-01' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE timestamptz '2001-06-01' <@ ps;

SELECT COUNT(*) FROM tbl_tstzset_big WHERE tstzset '{2001-06-01, 2001-07-01}' <@ t;

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE tstzspan '[2001-06-01, 2001-07-01]' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE tstzspan '[2001-06-01, 2001-07-01]' <@ ps;

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE tstzspanset '{[2001-06-01, 2001-07-01]}' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE tstzspanset '{[2001-06-01, 2001-07-01]}' <@ ps;

DROP INDEX tbl_tstzset_big_rtree_idx;
DROP INDEX tbl_tstzspan_big_rtree_idx;
DROP INDEX tbl_tstzspanset_big_rtree_idx;

-------------------------------------------------------------------------------
-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tstzset_rtree_idx ON tbl_tstzset USING gist(t);
CREATE INDEX tbl_tstzspan_rtree_idx ON tbl_tstzspan USING gist(p);
CREATE INDEX tbl_tstzspanset_rtree_idx ON tbl_tstzspanset USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps && t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p && t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps @> t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p @> t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps <@ t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p <@ t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_tstzset_rtree_idx;
DROP INDEX tbl_tstzspan_rtree_idx;
DROP INDEX tbl_tstzspanset_rtree_idx;

-------------------------------------------------------------------------------
-- RESTRICTION SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tstzset_big_quadtree_idx ON tbl_tstzset_big USING spgist(t);
CREATE INDEX tbl_tstzspan_big_quadtree_idx ON tbl_tstzspan_big USING spgist(p);
CREATE INDEX tbl_tstzspanset_big_quadtree_idx ON tbl_tstzspanset_big USING spgist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE t && tstzset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps && tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p && tstzspanset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps && tstzspanset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE t @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> timestamptz '2001-06-01';

SELECT COUNT(*) FROM tbl_tstzset_big WHERE t @> tstzset '{2001-06-01, 2001-07-01}';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspan '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> tstzspan '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE p @> tstzspanset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE ps @> tstzspanset '{[2001-06-01, 2001-07-01]}';

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset_big WHERE timestamptz '2001-06-01' <@ t;
SELECT COUNT(*) FROM tbl_tstzspan_big WHERE timestamptz '2001-06-01' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE timestamptz '2001-06-01' <@ ps;

SELECT COUNT(*) FROM tbl_tstzset_big WHERE tstzset '{2001-06-01, 2001-07-01}' <@ t;

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE tstzspan '[2001-06-01, 2001-07-01]' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE tstzspan '[2001-06-01, 2001-07-01]' <@ ps;

SELECT COUNT(*) FROM tbl_tstzspan_big WHERE tstzspanset '{[2001-06-01, 2001-07-01]}' <@ p;
SELECT COUNT(*) FROM tbl_tstzspanset_big WHERE tstzspanset '{[2001-06-01, 2001-07-01]}' <@ ps;

DROP INDEX tbl_tstzset_big_quadtree_idx;
DROP INDEX tbl_tstzspan_big_quadtree_idx;
DROP INDEX tbl_tstzspanset_big_quadtree_idx;

-------------------------------------------------------------------------------
-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tstzset_quadtree_idx ON tbl_tstzset USING gist(t);
CREATE INDEX tbl_tstzspan_quadtree_idx ON tbl_tstzspan USING gist(p);
CREATE INDEX tbl_tstzspanset_quadtree_idx ON tbl_tstzspanset USING gist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;
SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_tstzset_quadtree_idx;
DROP INDEX tbl_tstzspan_quadtree_idx;
DROP INDEX tbl_tstzspanset_quadtree_idx;

-------------------------------------------------------------------------------
-- JOIN SELECTIVITY
-- Test index support function

CREATE INDEX tbl_tstzset_quadtree_idx ON tbl_tstzset USING spgist(t);
CREATE INDEX tbl_tstzspan_quadtree_idx ON tbl_tstzspan USING spgist(p);
CREATE INDEX tbl_tstzspanset_quadtree_idx ON tbl_tstzspanset USING spgist(ps);

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t && t2.t;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p && t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps && t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p && t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps && t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t @> t2.t;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p @> t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps @> t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p @> t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps @> t2.ps;

-- EXPLAIN ANALYZE
SELECT COUNT(*) FROM tbl_tstzset t1, tbl_tstzset t2 WHERE t1.t <@ t2.t;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspan t2 WHERE t1.p <@ t2.p;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspan t2 WHERE t1.ps <@ t2.p;

SELECT COUNT(*) FROM tbl_tstzspan t1, tbl_tstzspanset t2 WHERE t1.p <@ t2.ps;
SELECT COUNT(*) FROM tbl_tstzspanset t1, tbl_tstzspanset t2 WHERE t1.ps <@ t2.ps;

DROP INDEX tbl_tstzset_quadtree_idx;
DROP INDEX tbl_tstzspan_quadtree_idx;
DROP INDEX tbl_tstzspanset_quadtree_idx;

-------------------------------------------------------------------------------
