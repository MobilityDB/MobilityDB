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

-- The bounding box of an H3 cell CONTAINS the cell, and the containment is
-- what the index keys on. A box taken as the plain minimum and maximum of the
-- boundary vertices does not have that property: a cell holding a pole reaches
-- a latitude no vertex carries, a cell crossing the antimeridian has vertex
-- longitudes near both -180 and +180 whose extremes name the complement of the
-- cell, and every cell edge is a geodesic that rises beyond both of the
-- vertices it joins.

-------------------------------------------------------------------------------
-- A cell holding a pole reaches it and spans every longitude
-------------------------------------------------------------------------------

SELECT round(stbox(h3index '8001fffffffffff'), 6);
SELECT round(stbox(h3index '81033ffffffffff'), 6);
SELECT round(stbox(h3index '80f3fffffffffff'), 6);
SELECT round(stbox(h3index '81f2bffffffffff'), 6);

-------------------------------------------------------------------------------
-- A cell crossing the antimeridian takes the full longitude range
-------------------------------------------------------------------------------

SELECT round(stbox(h3index '807ffffffffffff'), 6);
SELECT round(stbox(h3index '817ebffffffffff'), 6);

-------------------------------------------------------------------------------
-- A cell away from both takes the extent of its own boundary
-------------------------------------------------------------------------------

SELECT round(stbox(h3index '801ffffffffffff'), 6);
SELECT round(stbox(h3index '821fa7fffffffff'), 6);

-------------------------------------------------------------------------------
-- A temporal cell carries the same spatial extent as the cell it takes
-------------------------------------------------------------------------------

SELECT round(stbox(th3index '8001fffffffffff@2001-01-01'), 6);
SELECT round(stbox(th3index '807ffffffffffff@2001-01-01'), 6);
SELECT round(stbox(th3index '[8001fffffffffff@2001-01-01, 801ffffffffffff@2001-01-02]'), 6);

-------------------------------------------------------------------------------
-- No parent/child containment property is asserted here. H3 cells are NOT
-- geometrically nested: a child's boundary vertices reach beyond the parent's
-- boundary, measured at up to 1.93 degrees over the fixture above, so
-- `stbox(child) <@ stbox(parent)` is false for H3 by construction and says
-- nothing about this box.
-------------------------------------------------------------------------------
