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

set(TEMPORAL_SOURCES
  doublen.c
  lifting.c
  meos.c
  meos_array.c
  meos_catalog.c
  meos_error.c
  skiplist.c
  set.c
  set_ops.c
  span.c
  span_aggfuncs.c
  span_index.c
  span_ops.c
  span_spgist.c
  spanset.c
  spanset_ops.c
  tbool_ops.c
  tbox.c
  tbox_index.c
  tcellindex.c
  temporal.c
  temporal_aggfuncs.c
  temporal_analytics.c
  temporal_boxops.c
  temporal_compops.c
  temporal_modif.c
  temporal_restrict.c
  temporal_rtree.c
  temporal_sptree.c
  temporal_tile.c
  temporal_waggfuncs.c
  tinstant.c
  tnumber_distance.c
  tnumber_mathfuncs.c
  tnumber_spgist.c
  tsequence.c
  tsequenceset.c
  ttext_funcs.c
  type_in.c
  type_out.c
  type_parser.c
  type_util.c
)

if(MEOS)
  list(APPEND TEMPORAL_SOURCES
    set_aggfuncs_meos.c
    set_meos.c
    set_ops_meos.c
    span_meos.c
    span_tile_meos.c
    span_aggfuncs_meos.c
    span_ops_meos.c
    spanset_meos.c
    spanset_ops_meos.c
    tbool_ops_meos.c
    temporal_aggfuncs_meos.c
    temporal_boxops_meos.c
    temporal_compops_meos.c
    temporal_meos.c
    temporal_posops_meos.c
    temporal_restrict_meos.c
    temporal_tile_meos.c
    tinstant_meos.c
    tnumber_distance_meos.c
    tnumber_mathfuncs_meos.c
    tsequence_meos.c
    tsequenceset_meos.c
    ttext_funcs_meos.c
    type_in_meos.c
)
endif()

add_library(temporal OBJECT ${TEMPORAL_SOURCES})
