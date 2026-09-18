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

add_library(pg_quadbin OBJECT
  quadbin.c
  quadbin_ops.c
  tquadbin.c
  tquadbin_ops.c
  tquadbin_compops.c
  tquadbin_spatialrels.c
  # The square subset shared with every DGGS family plus the
  # quadbin-unique tile / quadkey conversions; the hexagon-only H3
  # families (directed edges, vertices, pentagon / base-cell /
  # class-III inspection, local-IJ traversal, great-circle metrics)
  # have no quadbin analogue and are not wrapped.
  #
  # 296_quadbinset.in.sql routes to the generic Set_* symbols and
  # 298_tquadbin.in.sql routes the temporal I/O / comparison to the
  # generic Temporal_* symbols, so no standalone C wrapper file is
  # needed for the quadbinset machinery nor for the tquadbin I/O.
  )
