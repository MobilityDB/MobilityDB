-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2026, PostGIS contributors
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

-- The extension reads a point cloud schema through its own copy of libpc. This
-- file loads pgPointCloud BEFORE the extension: its first statement calls
-- pgPointCloud alone, so the library of pgPointCloud, which exports the names
-- of its own libpc, is in the session when the extension's library loads. The
-- schema of pcid 1 of the fixture, stated as an XML document without
-- pc:active, has three dimensions that hold values, so the count below is
-- three only while the extension reads it with its own libpc, that is, while
-- the extension does not export the names of libpc. No statement calling the
-- extension may precede the first one.
SELECT PC_AsText(PC_MakePoint(1, ARRAY[1, 2, 3]::float8[]));

SELECT pointCloudSchemaNDims(1);

SELECT getX(pcpoint(1, 1, 2, 3)), getY(pcpoint(1, 1, 2, 3)),
  getZ(pcpoint(1, 1, 2, 3));
