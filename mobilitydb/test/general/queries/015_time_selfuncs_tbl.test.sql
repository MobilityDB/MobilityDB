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
-- Test all operators without having collected statistics
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tstzset WHERE ts = tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts < tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <= tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts > tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts >= tstzset '{2001-06-01, 2001-07-07}';

SELECT COUNT(*) FROM tbl_period WHERE p = period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p < period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <= period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p > period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p >= period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_periodset WHERE ps = periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps < periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <= periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps > periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps >= periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_tstzset WHERE ts @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_tstzset WHERE ts && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p -|- timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p -|- tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT COUNT(*) FROM tbl_period WHERE period '[2001-01-01, 2001-06-01]' <<# p;
SELECT COUNT(*) FROM tbl_period WHERE period '[2001-01-01, 2001-06-01]' &<# p;

-------------------------------------------------------------------------------

analyze tbl_period;
analyze tbl_periodset;
analyze tbl_tstzset;

-------------------------------------------------------------------------------
-- Test all operators after having collected statistics
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tstzset WHERE ts = tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts < tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <= tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts > tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts >= tstzset '{2001-06-01, 2001-07-07}';

SELECT COUNT(*) FROM tbl_period WHERE p = period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p < period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <= period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p > period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p >= period '[2001-06-01, 2001-07-01]';

SELECT COUNT(*) FROM tbl_periodset WHERE ps = periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps < periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <= periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps > periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps >= periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_tstzset WHERE ts @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p @> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps @> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <@ periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <@ period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <@ periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_tstzset WHERE ts && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p && periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps && periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p <<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps <<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p #>> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #>> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p &<# periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps &<# periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p #&> periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps #&> periodset '{[2001-06-01, 2001-07-01]}';

SELECT COUNT(*) FROM tbl_timestamptz WHERE t -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_timestamptz WHERE t -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_tstzset WHERE ts -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_period WHERE p -|- timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_period WHERE p -|- tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_period WHERE p -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_period WHERE p -|- periodset '{[2001-06-01, 2001-07-01]}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- timestamptz '2001-06-01';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- tstzset '{2001-06-01, 2001-07-07}';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- period '[2001-06-01, 2001-07-01]';
SELECT COUNT(*) FROM tbl_periodset WHERE ps -|- periodset '{[2001-06-01, 2001-07-01]}';

-- Test the commutator
SELECT COUNT(*) FROM tbl_period WHERE period '[2001-01-01, 2001-06-01]' <<# p;
SELECT COUNT(*) FROM tbl_period WHERE period '[2001-01-01, 2001-06-01]' &<# p;

-------------------------------------------------------------------------------

--SELECT period_statistics_validate();
--VACUUM ANALYSE tbl_period;
--VACUUM ANALYSE tbl_periodset;
--VACUUM ANALYSE tbl_tstzset;
--SELECT COUNT(*) FROM execution_stats WHERE abs(PlanRows-ActualRows) > 10
-- STATISTICS COLLECTION FUNCTIONS

CREATE OR REPLACE FUNCTION period_statistics_validate()
RETURNS CHAR(10) AS $$
DECLARE
  Query CHAR(5);
  PlanRows BIGINT;
  ActualRows BIGINT;
  QFilter VARCHAR;
  RowsRemovedbyFilter BIGINT;
  J JSON;
  StartTime TIMESTAMP;
  RandTimestamp TIMESTAMPTZ;
  RandPeriod period;
  RandTimestampSet tstzset;
  RandPeriodset periodset;
  k INT;
BEGIN

CREATE TABLE IF NOT EXISTS execution_stats
(Query CHAR(5),
StartTime TIMESTAMP,
QFilter VARCHAR,
PlanRows BIGINT,
ActualRows BIGINT,
RowsRemovedByFilter BIGINT,
J JSON);

TRUNCATE TABLE execution_stats;

SET log_error_verbosity TO terse;
k:= 0;

-----------------------------------------------
---- OPERATOR @>-------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p @> RandTimeStamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p @> RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p @> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE RandPeriod @> p
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p @> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE RandPeriodset @> p
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

----------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps @> RandTimeStamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps @> RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps @> RandTimeStamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps @> RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps @> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE RandPeriod @> ps
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps @> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;


k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE RandPeriodset @> ps
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR <@-------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE RandTimeStamp <@ ps
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE RandTimeStamp <@ p
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <@ RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <@ RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <@ RandPeriodSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <@ RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <@ RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <@ RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <@ RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR &&-------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimeStampset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps && RandTimeStampset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps && RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps && RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p && RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p && RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p && RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps && RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps && RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps && RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR <<#------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE RandTimestamp <<# p
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE RandTimestamp <<# ps
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE RandTimestamp <<# ps
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <<# RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps <<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <<# RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p <<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <<# RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps <<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR #>>------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #>> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #>> RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #>> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #>> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #>> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #>> RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #>> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #>> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #>> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #>> RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #>> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #>> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR &<#------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps &<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps &<# RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps &<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps &<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p &<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p &<# RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p &<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodSet:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p &<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps &<# RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestampSet:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps &<# RandTimestampSet
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps &<# RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps &<# RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR #&>------------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #&> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #&> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #&> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #&> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #&> RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #&> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  Randtstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #&> RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps #&> RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #&> RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #&> RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p #&> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps #&> RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

-----------------------------------------------
---- OPERATOR -|-  ----------------------------
-----------------------------------------------

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps -|- RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_tstzset
  WHERE ps -|- RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p -|- RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  Randtstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p -|- RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p -|- RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_period
  WHERE p -|- RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  Randtimestamp:= random_timestamptz('2000-10-01', '2002-1-31');
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps -|- RandTimestamp
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandTstzset:= random_tstzset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps -|- RandTstzset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriod:= random_period('2000-10-01', '2002-1-31', 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps -|- RandPeriod
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

k:= k+1;
FOR i IN 1..100 LOOP
  RandPeriodset:= random_periodset('2000-10-01', '2002-1-31', 10, 10);
  EXPLAIN (ANALYZE, FORMAT JSON)
  SELECT *
  FROM tbl_periodset
  WHERE ps -|- RandPeriodset
  INTO J;

  StartTime := clock_timestamp();
  PlanRows:= (J->0->'Plan'->>'Plan Rows')::BIGINT;
  ActualRows:=  (J->0->'Plan'->>'Actual Rows')::BIGINT;
  QFilter:=  substring((J->0->'Plan'->>'Filter') for 100);
  RowsRemovedbyFilter:= (J->0->'Plan'->>'Rows Removed by Filter'):: BIGINT;

  Query:= 'Q' || k;
  INSERT INTO execution_stats VALUES (Query, StartTime, QFilter, PlanRows, ActualRows, RowsRemovedByFilter, J);
END LOOP;

RETURN 'THE END';
END;
$$ LANGUAGE 'plpgsql';

-------------------------------------------------------------------------------
