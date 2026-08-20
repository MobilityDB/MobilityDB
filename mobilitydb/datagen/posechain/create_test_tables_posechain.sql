/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation FOR any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Function generating a set of test tables for pose chain types
 */

DROP FUNCTION IF EXISTS create_test_tables_posechain();
CREATE OR REPLACE FUNCTION create_test_tables_posechain(size int DEFAULT 100,
  perc int DEFAULT 1)
RETURNS text AS $$
BEGIN

------------------------------------------------------------------------------
-- Static pose chain
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_posechain2d;
CREATE TABLE tbl_posechain2d AS
SELECT k, random_posechain2d(-100, 100, -100, 100, radians(-pi()),
  radians(pi()), 1, 5, 3812) AS pc
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_posechain3d;
CREATE TABLE tbl_posechain3d AS
SELECT k, random_posechain3d(-100, 100, -100, 100, -100, 100, -1, 1, -1, 1,
  -1, 1, -1, 1, 1, 5, 3812) AS pc
FROM generate_series(1, size) k;

DROP TABLE IF EXISTS tbl_posechain;
CREATE TABLE tbl_posechain(k, pc) AS
SELECT k, pc FROM tbl_posechain2d UNION ALL
SELECT k + size, pc FROM tbl_posechain3d;

/* Add perc NULL values */
UPDATE tbl_posechain
SET pc = NULL
WHERE k IN (SELECT i FROM generate_series(1, perc) i);

------------------------------------------------------------------------------

RETURN 'The End';
END;
$$ LANGUAGE 'plpgsql';

-- SELECT create_test_tables_posechain(100);

------------------------------------------------------------------------------
