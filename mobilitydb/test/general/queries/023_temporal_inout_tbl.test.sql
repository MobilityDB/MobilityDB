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

-- Maximum decimal digits
SELECT asText(tfloat '0.123456789@2000-01-01', 6);
SELECT asText(tfloat '{0.123456789@2000-01-01, 1.523456789@2000-01-02, 0.123456789@2000-01-03}', 6);
SELECT asText(tfloat '[0.123456789@2000-01-01, 1.523456789@2000-01-02, 0.123456789@2000-01-03]', 6);
SELECT asText(tfloat '{[0.123456789@2000-01-01, 1.523456789@2000-01-02, 0.123456789@2000-01-03],[3.723456789@2000-01-04, 3.723456789@2000-01-05]}', 6);
SELECT asText(tfloat 'Interp=Step;[0.123456789@2000-01-01, 1.523456789@2000-01-02, 0.123456789@2000-01-03]', 6);
SELECT asText(tfloat 'Interp=Step;{[0.123456789@2000-01-01, 1.523456789@2000-01-02, 0.123456789@2000-01-03],[3.723456789@2000-01-04, 3.723456789@2000-01-05]}', 6);

-- Array of temporal values
SELECT asText('{}'::tfloat[]);
SELECT asText(ARRAY[tbool 'true@2000-01-01']);
SELECT asText(ARRAY[tint '1@2000-01-01']);
SELECT asText(ARRAY[tfloat '1@2000-01-01']);
SELECT asText(ARRAY[ttext 'ABC@2000-01-01']);

-------------------------------------------------------------------------------
-- Additional pretty-print attribute
-- Notice that the Linux and Mac versions of json_c produce slightly different versions of the pretty-print JSON
SELECT tfloatFromMFJSON(asMFJSON(tfloat '1@2000-01-01', 1, 3, 15));
SELECT tfloatFromMFJSON(asMFJSON(tfloat '{1@2000-01-01, 2@2000-01-02}', 1, 3, 15));
SELECT tfloatFromMFJSON(asMFJSON(tfloat '[1@2000-01-01, 2@2000-01-02]', 1, 3, 15));
SELECT tfloatFromMFJSON(asMFJSON(tfloat '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 3@2000-01-04]}', 1, 3, 15));

SELECT ttextFromMFJSON(asMFJSON(ttext 'AAA@2000-01-01', 1, 5));
SELECT ttextFromMFJSON(asMFJSON(ttext '{AAA@2000-01-01, BBB@2000-01-02}', 1, 5));
SELECT ttextFromMFJSON(asMFJSON(ttext '[AAA@2000-01-01, BBB@2000-01-02]', 1, 5));
SELECT ttextFromMFJSON(asMFJSON(ttext '{[AAA@2000-01-01, BBB@2000-01-02], [CCC@2000-01-03, CCC@2000-01-04]}', 1, 5));

-------------------------------------------------------------------------------
-- Combination of input/output functions
-------------------------------------------------------------------------------

SELECT COUNT(*) FROM tbl_tbool WHERE temp IS NOT NULL AND tboolFromMFJSON(asMFJSON(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tint WHERE temp IS NOT NULL AND tintFromMFJSON(asMFJSON(temp)) <> temp;
-- We need to add asText to avoid problems due to floating point precision
SELECT COUNT(*) from tbl_tfloat WHERE temp IS NOT NULL AND asText(tfloatFromMFJSON(asMFJSON(temp))) <> asText(temp);
SELECT COUNT(*) FROM tbl_ttext WHERE temp IS NOT NULL AND ttextFromMFJSON(asMFJSON(temp)) <> temp;

SELECT COUNT(*) FROM tbl_tbool WHERE temp IS NOT NULL AND tboolFromBinary(asBinary(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tint WHERE temp IS NOT NULL AND tintFromBinary(asBinary(temp)) <> temp;
SELECT COUNT(*) from tbl_tfloat WHERE temp IS NOT NULL AND tfloatFromBinary(asBinary(temp)) <> temp;
SELECT COUNT(*) FROM tbl_ttext WHERE temp IS NOT NULL AND ttextFromBinary(asBinary(temp)) <> temp;

SELECT COUNT(*) FROM tbl_tbool WHERE temp IS NOT NULL AND tboolFromHexWKB(asHexWKB(temp)) <> temp;
SELECT COUNT(*) FROM tbl_tint WHERE temp IS NOT NULL AND tintFromHexWKB(asHexWKB(temp)) <> temp;
SELECT COUNT(*) from tbl_tfloat WHERE temp IS NOT NULL AND tfloatFromHexWKB(asHexWKB(temp)) <> temp;
SELECT COUNT(*) FROM tbl_ttext WHERE temp IS NOT NULL AND ttextFromHexWKB(asHexWKB(temp)) <> temp;

------------------------------------------------------------------------------
