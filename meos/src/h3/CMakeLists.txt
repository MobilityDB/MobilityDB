#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

set(H3_SRCS
  # Internal — auto-generated static h3 wrappers.
  h3_generated.c

  # Static h3index SQL type — parser, output, comparison/hash
  # helpers, and every `datum_h3_*` Datum-calling-convention wrapper
  # used by the lifting layer (cbuffer convention: all Datum
  # wrappers live alongside the base type they belong to).
  h3index.c
  # Static set-returning h3-pg ports.
  h3index_sets.c

  # The h3_cellops descriptor that plugs the kernel into the shared temporal
  # cell-index machinery (meos/src/temporal/tcellindex.c), with the square-metre
  # adapter its area slot takes. The quadbin analogue is tquadbin_ops.c.
  th3index_ops.c

  # Public inheritance boilerplate: inout, constructors,
  # conversions, accessors, validators.
  th3index.c

  # Bounding box functions: geodetic STBox from H3 cell boundary.
  th3index_boxops.c

  # Per-section lifted public API, listed in the same order as
  # the @defgroup sequence in meos/include/h3/doxygen_meos_h3.h
  # and the chapter sections in doc/temporal_cell_index.xml. Each of
  # these also carries the hand-written static h3 adapters that
  # could not be auto-generated (GSERIALIZED conversions, unit-
  # string dispatch, next-resolution conveniences).
  th3index_inspection.c
  th3index_hierarchy.c
  th3index_latlng.c
  th3index_edges.c
  th3index_vertices.c
  th3index_traversal.c
  th3index_metrics.c

  # Comparison operators — last group in the doxygen hierarchy,
  # so last in the source list too.
  th3index_compops.c

  # Static-geometry → H3 cell / cell set (POINT, LINESTRING, POLYGON,
  # MULTI*, GEOMETRYCOLLECTION) + the ever-intersects predicate that
  # consumes the resulting cell set against a th3index trip path.
  h3_geo.c
)

add_library(h3 OBJECT ${H3_SRCS})
