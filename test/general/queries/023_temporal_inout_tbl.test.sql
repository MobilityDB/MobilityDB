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
-- Combination of input/output functions

-- We need to add asewkt to avoid problems due to floating point precision
-- SELECT DISTINCT asText(tintFromText(asText(temp))) = asText(temp) FROM tbl_tint;
-- SELECT DISTINCT asText(tfloatFromText(asText(temp))) = asText(temp) FROM tbl_tfloat;

-- We need to add asewkt to avoid problems due to floating point precision
-- SELECT DISTINCT asEWKT(tintFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tint;
-- SELECT DISTINCT asEWKT(tfloatFromEWKT(asEWKT(temp))) = asEWKT(temp) FROM tbl_tfloat;

-- We need to add asewkt to avoid problems due to floating point precision
-- SELECT DISTINCT asEWKT(tintFromMFJSON(asMFJSON(temp))) = asEWKT(temp) FROM tbl_tint;
-- SELECT DISTINCT asEWKT(tfloatFromMFJSON(asMFJSON(temp))) = asEWKT(temp) FROM tbl_tfloat;

SELECT DISTINCT tboolFromBinary(asBinary(temp)) = temp FROM tbl_tbool;
SELECT DISTINCT tintFromBinary(asBinary(temp)) = temp FROM tbl_tint;
SELECT DISTINCT tfloatFromBinary(asBinary(temp)) = temp FROM tbl_tfloat;
SELECT DISTINCT ttextFromBinary(asBinary(temp)) = temp FROM tbl_ttext;

SELECT DISTINCT tboolFromHexWKB(asHexWKB(temp)) = temp FROM tbl_tbool;
SELECT DISTINCT tintFromHexWKB(asHexWKB(temp)) = temp FROM tbl_tint;
SELECT DISTINCT tfloatFromHexWKB(asHexWKB(temp)) = temp FROM tbl_tfloat;
SELECT DISTINCT ttextFromHexWKB(asHexWKB(temp)) = temp FROM tbl_ttext;


------------------------------------------------------------------------------
