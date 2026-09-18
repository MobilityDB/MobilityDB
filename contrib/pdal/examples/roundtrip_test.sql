/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
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

-- Test fixture for the readers.tpcpatch / writers.tpcpatch plugins.
-- Run against a database with mobilitydb + pointcloud + postgis loaded.
-- Assumes the pointcloud-review build of MobilityDB is installed.

-- 1. Register a PC_NONE schema (uncompressed double X/Y/Z) — the plugin
--    only supports PC_NONE for now.
INSERT INTO pointcloud_formats (pcid, srid, schema) VALUES (
  100, 0,
  '<?xml version="1.0" encoding="UTF-8"?>
<pc:PointCloudSchema xmlns:pc="http://pointcloud.org/schemas/PC/1.1">
  <pc:dimension><pc:position>1</pc:position><pc:size>8</pc:size><pc:name>X</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>2</pc:position><pc:size>8</pc:size><pc:name>Y</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:dimension><pc:position>3</pc:position><pc:size>8</pc:size><pc:name>Z</pc:name><pc:interpretation>double</pc:interpretation><pc:scale>1</pc:scale></pc:dimension>
  <pc:metadata><Metadata name="compression">none</Metadata></pc:metadata>
</pc:PointCloudSchema>'
) ON CONFLICT (pcid) DO UPDATE SET schema = EXCLUDED.schema;

-- 2. Build a tpcpatch trajectory with two timestamped patches.
DROP TABLE IF EXISTS trajectories;
CREATE TABLE trajectories (id int PRIMARY KEY, traj tpcpatch);

INSERT INTO trajectories VALUES (1,
  tpcpatchSeq(ARRAY[
    tpcpatch(
      PC_Patch(ARRAY[
        PC_MakePoint(100, ARRAY[1.0,  2.0,  3.0]),
        PC_MakePoint(100, ARRAY[4.0,  5.0,  6.0])
      ]),
      '2024-01-01 12:00:00'::timestamptz
    ),
    tpcpatch(
      PC_Patch(ARRAY[
        PC_MakePoint(100, ARRAY[7.0,  8.0,  9.0]),
        PC_MakePoint(100, ARRAY[10.0, 11.0, 12.0]),
        PC_MakePoint(100, ARRAY[13.0, 14.0, 15.0])
      ]),
      '2024-01-01 12:00:01'::timestamptz
    )
  ])
);

-- 3. Sanity check: 5 points across 2 timestamps.
SELECT id,
       numInstants(traj)        AS n_instants,
       startTimestamp(traj)     AS t0,
       endTimestamp(traj)       AS t1
FROM trajectories;

-- 4. Verify the read query the plugin issues:
SELECT (EXTRACT(EPOCH FROM (asPatches(traj)).t) * 1000000)::bigint AS t,
       (asPatches(traj)).pcp                                       AS pcp,
       getPCID((asPatches(traj)).pcp)                              AS pcid
FROM trajectories
WHERE id = 1;
