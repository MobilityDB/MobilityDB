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

--------------------------------------------------

SELECT wmin(temp, NULL) FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wmax(temp, NULL) FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wmax(temp, NULL) FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wmax(temp, interval '5 minutes') FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wmax(temp, interval '5 minutes') FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wcount(temp, NULL) FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wcount(temp, NULL) FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wsum(temp, NULL) FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wsum(temp, NULL) FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);

SELECT wavg(temp, NULL) FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wavg(temp, NULL) FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES (NULL::tint),(NULL::tint)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES (NULL::tint),('1@2000-01-01'::tint)) t(temp);

--------------------------------------------------

SELECT wmin(temp, NULL) FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, NULL) FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmin(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wmax(temp, NULL) FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmax(temp, NULL) FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wmax(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wmax(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wcount(temp, NULL) FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wcount(temp, NULL) FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wcount(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wsum(temp, NULL) FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wsum(temp, NULL) FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wsum(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

SELECT wavg(temp, NULL) FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wavg(temp, NULL) FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),(NULL::tfloat)) t(temp);
SELECT wavg(temp, interval '5 minutes') FROM (VALUES (NULL::tfloat),('1@2000-01-01'::tfloat)) t(temp);

--------------------------------------------------

SELECT wmax(temp, interval '1 day') FROM (VALUES (tfloat '[1@2000-01-01, 1@2000-01-02]'),('[1@2000-01-03, 1@2000-01-04]')) t(temp);

/* Errors */
SELECT wsum(temp, interval '1 day') FROM (VALUES (tfloat '[1@2000-01-01, 1@2000-01-02]'),('[1@2000-01-03, 1@2000-01-04]')) t(temp);

--------------------------------------------------



