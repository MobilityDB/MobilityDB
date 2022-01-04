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

ANALYZE tbl_tint;
DROP INDEX IF EXISTS tbl_tint_spgist_idx;
CREATE INDEX tbl_tint_spgist_idx ON tbl_tint USING SPGIST(temp);

SELECT k, temp |=| intrange '[100,100]'::tbox FROM tbl_tint ORDER BY 2, 1 LIMIT 3;
SELECT k, temp |=| tint '[1@2001-06-01, 2@2001-07-01]' FROM tbl_tint ORDER BY 2, 1 LIMIT 3;

DROP INDEX tbl_tint_spgist_idx;

-------------------------------------------------------------------------------

ANALYZE tbl_tfloat;
DROP INDEX IF EXISTS tbl_tfloat_spgist_idx;
CREATE INDEX tbl_tfloat_spgist_idx ON tbl_tfloat USING SPGIST(temp);

SELECT k, round((temp |=| floatrange '[100,100]'::tbox)::numeric, 6) FROM tbl_tfloat ORDER BY 2, 1 LIMIT 3;
SELECT k, round((temp |=| tfloat '[1.5@2001-06-01, 2.5@2001-07-01]')::numeric, 6) FROM tbl_tfloat ORDER BY 2, 1 LIMIT 3;

DROP INDEX tbl_tfloat_spgist_idx;

-------------------------------------------------------------------------------
